/* 
* Copyright (C) 2004 Jens Oknelid, paskharen@gmail.com
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

#include "bookentry.hh"

using namespace std;

BookEntry::BookEntry(int type, string id, string title, GCallback closeCallback) {
	this->type = type;
	this->id = id;
	
	box = gtk_hbox_new(FALSE, 5);

	label = GTK_LABEL(gtk_label_new(title.c_str()));
	gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(label), FALSE, TRUE, 0);

	button = GTK_BUTTON(gtk_button_new());
	gtk_container_add(GTK_CONTAINER(button), 
		gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU));
	gtk_widget_set_size_request(GTK_WIDGET(button), 16, 16);
	gtk_button_set_relief(button, GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(button), FALSE, TRUE, 0);

	gtk_widget_show_all(box);
	
    g_signal_connect(G_OBJECT(button), "clicked", 
		closeCallback, (gpointer)this);
}

BookEntry::~BookEntry() {
}

GtkWidget *BookEntry::getTitle() {
	return box;
}

void BookEntry::setLabel_gui(std::string text) {
	if (text.size() > 15) 
		text = text.substr(0, 15) + "...";
	gtk_label_set_text(label, text.c_str());
}

bool BookEntry::isEqual(int type, string id) {
	if (this->type == type) 
		return this->id == id;
	else 
		return false;
}

bool BookEntry::isEqual(BookEntry *entry) {
	if (this->type == entry->type) 
		return this->id == entry->id;
	else 
		return false;
}
