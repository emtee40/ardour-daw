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

#ifndef __ardour_cliprec_h__
#define __ardour_cliprec_h__


#include "pbd/ringbufferNPT.h"

#include "ardour/processor.h"

namespace PBD {
class Thread;
}

namespace ARDOUR {

class AudioFileSource;
class Session;
class Track;

template<typename T> class MidiRingBuffer;

class LIBARDOUR_API ClipRecProcessor : public Processor
{
  public:
	ClipRecProcessor (Session&, Track&, std::string const & name);
	void run (BufferSet& bufs, samplepos_t start_sample, samplepos_t end_sample, double speed, pframes_t nframes, bool result_required);
	bool can_support_io_configuration (const ChanCount& in, ChanCount& out);

  private:
	Track& _track;

	/** Information about one audio channel, playback or capture
	 * (depending on the derived class)
	 */
	struct ChannelInfo : public boost::noncopyable {

		ChannelInfo (samplecnt_t buffer_size);
		virtual ~ChannelInfo ();

		/** A ringbuffer for data to be recorded back, written to in the
		 * process thread, read from in the butler thread.
		 */
		PBD::RingBufferNPT<Sample>* buf;
		PBD::RingBufferNPT<Sample>::rw_vector rw_vector;

		/* used only by capture */
		boost::shared_ptr<AudioFileSource> write_source;

		/* used in the butler thread only */
		samplecnt_t curr_capture_cnt;

		virtual void resize (samplecnt_t) = 0;
	};

	typedef std::vector<ChannelInfo*> ChannelList;
	SerializedRCUManager<ChannelList> channels;

	/* The MIDI stuff */

	MidiRingBuffer<samplepos_t>*  _midi_buf;

	/* private (to class) butler thread */

	static PBD::Thread* _thread;
	static bool thread_should_run;
	static void thread_work ();
};

} /* namespace */

#endif /* __ardour_cliprec_h__ */
