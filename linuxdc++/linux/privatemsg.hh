/* 
* Copyright (C) 2004 Jens Oknelid, paskharen@spray.se
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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <gtkmm.h>
#include <string>

#ifndef WULFOR_PRIVATE_MESSAGE
#define WULFOR_PRIVATE_MESSAGE

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"
#include "../client/User.h"

#include "bookentry.hh"
#include "mainwindow.hh"

class PrivateMsg: public BookEntry {
	public:	
		PrivateMsg(User::Ptr userName, MainWindow *mw);
		
		bool operator== (BookEntry &b);
		bool operator== (User::Ptr &otherUser);

		void update();
		void enterPressed ();
		void addMsg(std::string msg);
		void close();

	private:
		User::Ptr user;
		MainWindow *mw;

		Gtk::ScrolledWindow scroll;
		Gtk::Entry entry;
		Gtk::VBox box;
		Gtk::TextView text;
};

#else
class PrivateMsg;
#endif
