/*
 * Copyright © 2004-2008 Jens Oknelid, paskharen@gmail.com
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef WULFOR_HASH_HH
#define WULFOR_HASH_HH

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/TimerManager.h>

#include "dialogentry.hh"

class Hash:
	public DialogEntry,
	public TimerManagerListener
{
	public:
		Hash(GtkWindow* parent = NULL);
		~Hash();

	private:
		// GUI functions
		void updateStats_gui(std::string file, int64_t bytes, size_t files, uint32_t tick);

		// Client callbacks
		virtual void on(TimerManagerListener::Second, uint32_t tics) throw();

		int64_t startBytes;
		size_t startFiles;
		uint32_t startTime;
};

#else
class Hash;
#endif
