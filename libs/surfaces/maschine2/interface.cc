/*
 * Copyright (C) 2016 Robin Gareus <robin@gareus.org>
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

#include <stdexcept>

#include "pbd/error.h"
#include "ardour/rc_configuration.h"
#include "control_protocol/control_protocol.h"
#include "maschine2.h"

using namespace ARDOUR;
using namespace PBD;
using namespace ArdourSurface;

static ControlProtocol*
new_maschine2 (Session* s)
{
	Maschine2* m2 = 0;

	try {
		m2 = new Maschine2 (*s);
	}
	catch (std::exception & e) {
		PBD::error << "Failed to instantiate Maschine2: " << e.what() << endmsg;
		delete m2;
		m2 = 0;
	}

	m2->set_active (true);
	return m2;
}

static void
delete_maschine2 (ControlProtocol* cp)
{
	delete cp;
}

static ControlProtocolDescriptor maschine2_descriptor = {
	/* name       */ "NI Maschine2",
	/* id         */ "uri://ardour.org/surfaces/maschine2:0",
	/* module     */ 0,
	/* available  */ Maschine2::available,
	/* probe_port */ 0,
	/* match usb  */ 0,
	/* initialize */ new_maschine2,
	/* destroy    */ delete_maschine2,
};

extern "C" ARDOURSURFACE_API ControlProtocolDescriptor* protocol_descriptor () { return &maschine2_descriptor; }
