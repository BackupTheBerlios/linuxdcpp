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
	BookEntry("PM: " + WulforUtil::getNicks(user)),
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

	if (SETTING(USE_OEM_MONOFONT))
	{
		PangoFontDescription *font_desc;
		font_desc = pango_font_description_from_string("Mono 10");
		gtk_widget_modify_font(GTK_WIDGET(text), font_desc);
		pango_font_description_free(font_desc);
	}

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
	ClientManager::getInstance()->privateMessage(user, message);
}

void PrivateMessage::addMessage_gui(User::Ptr from, std::string message) {
	if(BOOLSETTING(LOG_PRIVATE_CHAT)) {
		StringMap params;
		params["message"] = Text::acpToUtf8(message);
		params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(user->getCID()));
		params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(user->getCID()));
		params["userCID"] = user->getCID().toBase32();
		params["userNI"] = ClientManager::getInstance()->getNicks(user->getCID())[0];
		params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
		LOG(LogManager::PM, params);
	}

	GtkTextIter iter;
	string line;
	if(BOOLSETTING(TIME_STAMPS)) {
		line = "[" + Util::getShortTimeString() + "] ";
	} else {
		line = "";
	}
	line += "<" + ClientManager::getInstance()->getIdentity(from).getNick() + "> ";

	string msg = line + message + "\n";
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

	if (BOOLSETTING(BOLD_PM))
		setBold_gui();
}

void PrivateMessage::addStatusMessage_gui(std::string message) {
	GtkTextIter iter;
	string line;
	if(BOOLSETTING(TIME_STAMPS)) {
		line = "[" + Util::getShortTimeString() + "] ";
	} else {
		line = "";
	}
	string msg = line + message + "\n";
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

	if (BOOLSETTING(BOLD_PM))
		setBold_gui();
}

void PrivateMessage::sendMessage_gui(GtkEntry *e, gpointer d) {
	string message, text = gtk_entry_get_text(entry);
	typedef Func1<PrivateMessage, string> F1;
	F1 *func;

	if (!text.empty()) {
		gtk_entry_set_text(entry, "");
		message = "<" + ClientManager::getInstance()->getIdentity(user).getNick() + "> " + text;
		func = new F1(this, &PrivateMessage::sendMessage_client, text);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

