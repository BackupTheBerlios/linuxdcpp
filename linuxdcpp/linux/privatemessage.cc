/* 
* Copyright (C) 2001-2003 Jens Oknelid, paskharen@spray.se
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

PrivateMessage::PrivateMessage(User::Ptr user, GCallback closeCallback):
	BookEntry(WulforManager::PRIVATE_MSG, user->getFullNick(), 
		user->getNick(), closeCallback),
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
	string text = "[" + Util::getShortTimeString() + "] " + message + "\n";
	
	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_buffer_insert(buffer, &iter, text.c_str(), text.size());
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

