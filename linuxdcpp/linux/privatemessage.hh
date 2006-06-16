/* 
 * Copyright © 2004-2006 Jens Oknelid, paskharen@gmail.com
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

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/User.h>

#include <string>

#include "bookentry.hh"
#include "callback.hh"

class PrivateMessage:
	public BookEntry
{
	public:
		//constructor is only to be called from gui thread
		PrivateMessage(User::Ptr user);
		
		GtkWidget *getWidget();
		
		//client thread functions
		void sendMessage_client(std::string message);
		
		//gui thread functions
		void addMessage_gui(User::Ptr from, std::string message);
		void addStatusMessage_gui(std::string message);
		void sendMessage_gui(GtkEntry *entry, gpointer data);

	private:
		Callback2<PrivateMessage, void, GtkEntry *> enterCallback;
	
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
