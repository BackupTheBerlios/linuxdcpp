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

#include "privatemessage.hh"
#include "wulformanager.hh"

#include <iostream>

using namespace std;

PrivateMessage::PrivateMessage(User::Ptr user):
	BookEntry("PM: " + user->getNick()),
	enterCallback(this, &PrivateMessage::sendMessage_gui)
{
	string file = WulforManager::get()->getPath() + "/glade/privatemessage.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "window");
	box = glade_xml_get_widget(xml, "box");
	gtk_widget_ref(box);
	gtk_container_remove(GTK_CONTAINER(window), box);
	gtk_widget_destroy(window);

	entry = GTK_ENTRY(glade_xml_get_widget(xml, "entry"));
	text = GTK_TEXT_VIEW(glade_xml_get_widget(xml, "text"));
	scroll = GTK_SCROLLED_WINDOW(glade_xml_get_widget(xml, "scroll"));

	buffer = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(text, buffer);
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(buffer, &iter);
	mark = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);
	
	enterCallback.connect(G_OBJECT(entry), "activate", NULL);

	this->user = user;
}

GtkWidget *PrivateMessage::getWidget() {
	return box;
}

void PrivateMessage::sendMessage_client(std::string message) {
	user->privateMessage(message);
}

void PrivateMessage::addMessage_gui(std::string message) {
	GtkTextIter iter;
	string msg = "[" + Util::getShortTimeString() + "] " + message + "\n";
	GtkAdjustment *adj;
	bool setBottom;
	
	adj = gtk_scrolled_window_get_vadjustment(scroll);
	setBottom = gtk_adjustment_get_value(adj) >= (adj->upper - adj->page_size);
	
	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_buffer_insert(buffer, &iter, msg.c_str(), msg.size());
	
	if (setBottom) {
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_move_mark(buffer, mark, &iter);
		gtk_text_view_scroll_to_mark(text, mark, 0, FALSE, 0, 0);
	}
}

void PrivateMessage::sendMessage_gui(GtkEntry *e, gpointer d) {
	string message, text = gtk_entry_get_text(entry);
	typedef Func1<PrivateMessage, string> F1;
	F1 *func;

	if (!text.empty()) {
		gtk_entry_set_text(entry, "");
		message = "<" + user->getClientNick() + "> " + text;
		addMessage_gui(message);
		func = new F1(this, &PrivateMessage::sendMessage_client, text);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

