/*
 * Copyright © 2004-2007 Jens Oknelid, paskharen@gmail.com
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

#ifndef WULFOR_DIALOG_ENTRY_HH
#define WULFOR_DIALOG_ENTRY_HH

#include "entry.hh"

class DialogEntry : public Entry
{
	public:
		DialogEntry() {}
		DialogEntry(const std::string &id, const std::string &glade);
		virtual ~DialogEntry();

		GtkWidget *getContainer();
		void setResponseID(int responseID);
		static int getResponseID();
		virtual void applyCallback(GCallback closeCallback);

	private:
		static int responseID;
};

#else
class DialogEntry;
#endif
