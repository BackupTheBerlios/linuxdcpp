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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "bookentry.hh"
#include "wulformanager.hh"

using namespace std;

BookEntry::BookEntry(const string &title, const string &glade):
	Entry(title, glade)
{
	bold = FALSE;
	box = gtk_hbox_new(FALSE, 5);

	eventBox = gtk_event_box_new();
	gtk_event_box_set_above_child(GTK_EVENT_BOX(eventBox), TRUE);
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(eventBox), FALSE);
	gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(eventBox), FALSE, TRUE, 0);

	label = GTK_LABEL(gtk_label_new(title.c_str()));
	gtk_container_add(GTK_CONTAINER(eventBox), GTK_WIDGET(label));

	button = GTK_BUTTON(gtk_button_new());
	gtk_container_add(GTK_CONTAINER(button), gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU));
	gtk_widget_set_size_request(GTK_WIDGET(button), 16, 16);
	gtk_button_set_relief(button, GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(button), FALSE, TRUE, 0);

	tips = gtk_tooltips_new();
	gtk_tooltips_enable(tips);

	gtk_widget_show_all(box);

	setLabel_gui(title);
}

GtkWidget* BookEntry::getContainer()
{
	return getWidget("mainBox");
}

GtkWidget *BookEntry::getTitle()
{
	return box;
}

void BookEntry::applyCallback(GCallback closeCallback)
{
	g_signal_connect(button, "clicked", closeCallback, (gpointer)this);
}

void BookEntry::setLabel_gui(string text)
{
	gtk_tooltips_set_tip(tips, eventBox, text.c_str(), text.c_str());
	if (text.size() > 20)
		text = text.substr(0, 17) + "...";
	gtk_label_set_text(label, text.c_str());
	title = text;
}

void BookEntry::setBold_gui()
{
	if (!bold && WulforManager::get()->getMainWindow()->currentPage_gui() != getContainer())
	{
		char *markup = g_markup_printf_escaped("<b>%s</b>", title.c_str());
		gtk_label_set_markup(label, markup);
		g_free(markup);
		bold = TRUE;
	}
}

void BookEntry::unsetBold_gui()
{
	if (bold)
	{
		gtk_label_set_text(label, title.c_str());
		bold = FALSE;
	}
}
