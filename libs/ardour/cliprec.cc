/*
 * Copyright (C) 2023 Paul Davis <paul@linuxaudiosystems.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "pbd/compose.h"
#include "pbd/debug.h"
#include "pbd/pthread_utils.h"

#include "ardour/audio_buffer.h"
#include "ardour/cliprec.h"
#include "ardour/debug.h"
#include "ardour/midi_track.h"

using namespace ARDOUR;
using namespace PBD;

PBD::Thread* ClipRecProcessor::_thread (0);
bool ClipRecProcessor::thread_should_run (false);

ClipRecProcessor::ClipRecProcessor (Session& s, Track& t, std::string const & name)
	: Processor (s, name, Temporal::BeatTime)
	, _track (t)
	, channels (new ChannelList)
{
	if (!_thread) {
		thread_should_run = true;
		_thread = PBD::Thread::create (&ClipRecProcessor::thread_work);
	}
}

void
ClipRecProcessor::thread_work ()
{
	while (thread_should_run) {
	}
}

bool
ClipRecProcessor::can_support_io_configuration (const ChanCount& in, ChanCount& out)
{
	if (in.n_midi() != 0 && in.n_midi() != 1) {
		/* we only support zero or 1 MIDI stream */
		return false;
	}

	/* currently no way to deliver different channels that we receive */
	out = in;

	return true;
}

void
ClipRecProcessor::run (BufferSet& bufs, samplepos_t start_sample, samplepos_t end_sample, double speed, pframes_t nframes, bool result_required)
{
	if (!check_active()) {
		return;
	}

	const size_t n_buffers = bufs.count().n_audio();
	boost::shared_ptr<ChannelList> c = channels.reader();
	ChannelList::iterator chan;
	size_t n;

	if (n_buffers) {

		/* AUDIO */

		for (chan = c->begin(), n = 0; chan != c->end(); ++chan, ++n) {

			ChannelInfo* chaninfo (*chan);
			AudioBuffer& buf (bufs.get_audio (n%n_buffers));

			chaninfo->buf->get_write_vector (&chaninfo->rw_vector);

			if (nframes <= (samplecnt_t) chaninfo->rw_vector.len[0]) {

				Sample *incoming = buf.data ();
				memcpy (chaninfo->rw_vector.buf[0], incoming, sizeof (Sample) * nframes);

			} else {

				samplecnt_t total = chaninfo->rw_vector.len[0] + chaninfo->rw_vector.len[1];

				if (nframes > total) {
					DEBUG_TRACE (DEBUG::ClipRecording, string_compose ("%1 overrun in %2, rec_nframes = %3 total space = %4\n",
					                                            DEBUG_THREAD_SELF, name(), nframes, total));
					return;
				}

				Sample *incoming = buf.data ();
				samplecnt_t first = chaninfo->rw_vector.len[0];

				memcpy (chaninfo->rw_vector.buf[0], incoming, sizeof (Sample) * first);
				memcpy (chaninfo->rw_vector.buf[1], incoming + first, sizeof (Sample) * (nframes - first));
			}

			chaninfo->buf->increment_write_ptr (nframes);
		}
	}

	/* MIDI */

	if (_midi_buf) {

		// Pump entire port buffer into the ring buffer (TODO: split cycles?)
		MidiBuffer& buf    = bufs.get_midi (0);
		MidiTrack* mt = dynamic_cast<MidiTrack*>(&_track);
		MidiChannelFilter* filter = mt ? &mt->capture_filter() : 0;

		assert (buf.size() == 0 || _midi_buf);

		for (MidiBuffer::iterator i = buf.begin(); i != buf.end(); ++i) {
			Evoral::Event<MidiBuffer::TimeType> ev (*i, false);
 			if (ev.time() > nframes) {
				break;
			}

			bool skip_event = false;

			if (mt) {
				/* skip injected immediate/out-of-band events */
				MidiBuffer const& ieb (mt->immediate_event_buffer());
				for (MidiBuffer::const_iterator j = ieb.begin(); j != ieb.end(); ++j) {
					if (*j == ev) {
						skip_event = true;
					}
				}
			}

			if (!skip_event && (!filter || !filter->filter(ev.buffer(), ev.size()))) {
				const samplepos_t event_time = start_sample + ev.time();
				_midi_buf->write (event_time, ev.event_type(), ev.size(), ev.buffer());
			}
		}
	}
}
