/*
 * Copyright © 2004-2008 Jens Oknelid, paskharen@gmail.com
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

BookEntry::BookEntry(const string &title, const string &id, const string &glade, bool duplicates):
	Entry(id, glade, duplicates),
	windowItem(NULL),
	bold(FALSE)
{
	labelBox = gtk_hbox_new(FALSE, 5);

	eventBox = gtk_event_box_new();
	gtk_event_box_set_above_child(GTK_EVENT_BOX(eventBox), TRUE);
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(eventBox), FALSE);
	gtk_box_pack_start(GTK_BOX(labelBox), GTK_WIDGET(eventBox), FALSE, TRUE, 0);

	label = GTK_LABEL(gtk_label_new(title.c_str()));
	gtk_container_add(GTK_CONTAINER(eventBox), GTK_WIDGET(label));

	closeButton = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(closeButton), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click(GTK_BUTTON(closeButton), FALSE);

	// Shrink the padding around the close button
	GtkRcStyle *rcstyle = gtk_rc_style_new();
	rcstyle->xthickness = rcstyle->ythickness = 0;
	gtk_widget_modify_style(closeButton, rcstyle);
	gtk_rc_style_unref(rcstyle);

	// Add the stock icon to the close button
	GtkWidget *image = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
	gtk_container_add(GTK_CONTAINER(closeButton), image);
	gtk_box_pack_start(GTK_BOX(labelBox), closeButton, FALSE, FALSE, 0);

	tips = gtk_tooltips_new();
	gtk_tooltips_enable(tips);
	gtk_tooltips_set_tip(tips, closeButton, _("Close tab"), NULL);

	gtk_widget_show_all(labelBox);

	setLabel_gui(title);

	// Associates entry to the widget for later retrieval in MainWindow::switchPage_gui()
	g_object_set_data(G_OBJECT(getContainer()), "entry", (gpointer)this);
}

GtkWidget* BookEntry::getContainer()
{
	return getWidget("mainBox");
}

void BookEntry::setWindowItem(GtkWidget *item)
{
	dcassert(windowItem == NULL);
	windowItem = item;
}

GtkWidget *BookEntry::getWindowItem()
{
	dcassert(windowItem);
	return windowItem;
}

void BookEntry::setLabel_gui(string text)
{
	gtk_tooltips_set_tip(tips, eventBox, text.c_str(), text.c_str());
	if (text.size() > labelSize)
		text = text.substr(0, labelSize - 3) + "...";
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

