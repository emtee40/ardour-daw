/*
 * Copyright (C) 2012 Carl Hetherington <carl@carlh.net>
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

#include "audio_region_test.h"

class PlaylistEquivalentRegionsTest : public AudioRegionTest
{
	CPPUNIT_TEST_SUITE (PlaylistEquivalentRegionsTest);
	CPPUNIT_TEST (basicsTest);
	CPPUNIT_TEST (multiLayerTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp ();
	void tearDown ();
	void basicsTest ();
	void multiLayerTest ();

private:
	std::shared_ptr<ARDOUR::Playlist> _playlist_b;
};
