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

#ifndef WULFOR_PRIVATE_MESSAGE_HH
#define WULFOR_PRIVATE_MESSAGE_HH

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/User.h>

#include "bookentry.hh"

class PrivateMessage:
	public BookEntry
{
	public:
		PrivateMessage(User::Ptr user);
		~PrivateMessage();

		// GUI functions
		void addMessage_gui(std::string message);
		void addStatusMessage_gui(std::string message);

	private:
		// GUI functions
		void addLine_gui(const std::string &line);

		// GUI callbacks
		static void onSendMessage_gui(GtkEntry *entry, gpointer data);
		static void onSearchMagnetClicked_gui(GtkMenuItem *item, gpointer data);
		static void onCopyMagnetClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMagnetPropertiesClicked_gui(GtkMenuItem *item, gpointer data);
		static gboolean onKeyPress_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onMagnetTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
		static gboolean onChatPointerMoved_gui(GtkWidget *widget, GdkEventMotion *event, gpointer data);
		static gboolean onChatVisibilityChanged_gui(GtkWidget* widget, GdkEventVisibility* event, gpointer data);

		// Client functions
		void sendMessage_client(std::string message);

		GtkTextBuffer *buffer;
		GtkTextMark *mark;
		User::Ptr user;
		std::vector<std::string> history;
		int historyIndex;
		bool sentAwayMessage;
		static const int maxLines = 500; ///@todo: make these preferences
		static const int maxHistory = 20;
		GdkCursor* handCursor;
		bool aboveMagnet;
		std::string selectedMagnet;
};

#else
class PrivateMessage;
#endif
