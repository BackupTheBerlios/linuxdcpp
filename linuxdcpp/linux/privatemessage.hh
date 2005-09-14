/* 
* Copyright (C) 2001-2003 Jens Oknelid, paskharen@gmail.com
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

#ifndef WULFOR_PRIVATE_MESSAGE_HH
#define WULFOR_PRIVATE_MESSAGE_HH

#include <gtk/gtk.h>
#include <glade/glade.h>

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/User.h>

#include <string>

#include "bookentry.hh"
#include "callback.hh"

class PrivateMessage: public BookEntry {
	public:
		PrivateMessage(User::Ptr userName, GCallback closeCallback);
		
		GtkWidget *getWidget();
		
		void addMessage(std::string message);
		static void sendMessage(GtkEntry *entry, gpointer data);

	private:
		User::Ptr user;

		GtkWidget *box;
		GtkTextView *text;
		GtkTextBuffer *buffer;
		GtkEntry *entry;
		GtkScrolledWindow *scroll;
		GtkTextMark *mark;
};

#else
class PrivateMessage;
#endif
