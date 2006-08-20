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

#include "dialogentry.hh"
#include "wulformanager.hh"
#include <iostream>

using namespace std;

int DialogEntry::responseID = GTK_RESPONSE_NONE;

DialogEntry::DialogEntry(string id, string glade)
{
	this->id = id;

	string file = WulforManager::get()->getPath() + "/glade/" + glade;
	xml = glade_xml_new(file.c_str(), NULL, NULL);

	if (xml == NULL)
		gtk_main_quit();
}

DialogEntry::~DialogEntry()
{
	g_object_unref(xml);
}

GtkWidget* DialogEntry::getWidget(string name)
{
	dcassert(!name.empty());
	GtkWidget *widget = glade_xml_get_widget(xml, name.c_str());
	dcassert(widget);
	return widget;
}

string DialogEntry::getID()
{
	return id;
}

void DialogEntry::setResponseID(int responseID)
{
	this->responseID = responseID;
}

int DialogEntry::getResponseID()
{
	return responseID;
}

void DialogEntry::applyCallback(GCallback closeCallback)
{
	g_signal_connect(getWidget("dialog"), "response", closeCallback, (gpointer)this);
}
