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

#include "dialogentry.hh"
#include "wulformanager.hh"

using namespace std;

DialogEntry::DialogEntry(const EntryType type, const std::string &glade):
	Entry(type, glade),
	responseID(GTK_RESPONSE_NONE)
{
	gtk_window_set_role(GTK_WINDOW(getContainer()), getID().c_str());

	WulforManager::get()->insertEntry_gui(this);
}

DialogEntry::~DialogEntry()
{
	gtk_widget_destroy(getContainer());
}

GtkWidget* DialogEntry::getContainer()
{
	return getWidget("dialog");
}

gint DialogEntry::run()
{
	responseID = gtk_dialog_run(GTK_DIALOG(getContainer()));
	WulforManager::get()->deleteEntry_gui(this);

	return responseID;
}

gint DialogEntry::getResponseID()
{
	return responseID;
}

