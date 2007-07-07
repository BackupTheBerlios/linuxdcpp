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

#include "entry.hh"
#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/Util.h>
#include "wulformanager.hh"

using namespace std;

Entry::Entry(const string &id, const string &glade):
	xml(NULL)
{
	// Special case: Allow search tab to have many tabs with the same title.
	if (id == "Search")
		this->id = id + Util::toString((long)this);
	else
		this->id = id;

	// Load the Glade XML file
	string file = WulforManager::get()->getPath() + "/glade/" + glade;
	xml = glade_xml_new(file.c_str(), NULL, NULL);
	if (xml == NULL)
		gtk_main_quit();
}

Entry::~Entry()
{
	g_object_unref(xml);
	xml = NULL;
}

GtkWidget *Entry::getWidget(const string &name)
{
	dcassert(xml && !name.empty());
	GtkWidget *widget = glade_xml_get_widget(xml, name.c_str());
	dcassert(widget);
	return widget;
}

const string& Entry::getID()
{
	return id;
}
