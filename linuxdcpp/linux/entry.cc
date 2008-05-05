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

#include "entry.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/Util.h>
#include "wulformanager.hh"

using namespace std;

const string Entry::DOWNLOAD_QUEUE = "DOWNLOAD_QUEUE";
const string Entry::FAVORITE_HUBS = "FAVORITE_HUBS";
const string Entry::FINISHED_DOWNLOADS = "FINISHED_DOWNLOADS";
const string Entry::FINISHED_UPLOADS = "FINISHED_UPLOADS";
const string Entry::HASH_DIALOG = "HASH_DIALOG";
const string Entry::HUB = "HUB";
const string Entry::MAIN_WINDOW = "MAIN_WINDOW";
const string Entry::PRIVATE_MESSAGE = "PRIVATE_MESSAGE";
const string Entry::PUBLIC_HUBS = "PUBLIC_HUBS";
const string Entry::SEARCH = "SEARCH";
const string Entry::SETTINGS_DIALOG = "SETTINGS_DIALOG";
const string Entry::SHARE_BROWSER = "SHARE_BROWSER";

Entry::Entry(const string &id, const string &glade, bool duplicates):
	xml(NULL),
	id(id)
{
	// To allow duplicate entries we need to generate an unique ID
	if (duplicates)
		this->id += Util::toString((long)this);

	// Load the Glade XML file, if applicable
	if (!glade.empty())
	{
		string file = WulforManager::get()->getPath() + "/glade/" + glade;
		xml = glade_xml_new(file.c_str(), NULL, NULL);
		if (xml == NULL)
			gtk_main_quit();
	}
}

Entry::~Entry()
{
	if (xml)
	{
		g_object_unref(xml);
		xml = NULL;
	}
}

const string& Entry::getID()
{
	return id;
}

void Entry::remove()
{
	removeChildren();
	WulforManager::get()->deleteEntry_gui(this);
}

GtkWidget *Entry::getWidget(const string &name)
{
	dcassert(xml && !name.empty());
	GtkWidget *widget = glade_xml_get_widget(xml, name.c_str());
	dcassert(widget);
	return widget;
}

void Entry::addChild(Entry *entry)
{
	children.insert(make_pair(entry->getID(), entry));
	WulforManager::get()->insertEntry_gui(entry);
}

Entry *Entry::getChild(const string &id)
{
	map<string, Entry *>::const_iterator it = children.find(id);

	if (it == children.end())
		return NULL;
	else
		return it->second;
}

void Entry::removeChild(const string &id)
{
	Entry *entry = getChild(id);
	removeChild(entry);
}

void Entry::removeChild(Entry *entry)
{
	if (entry != NULL)
	{
		entry->removeChildren();
		children.erase(entry->getID());
		WulforManager::get()->deleteEntry_gui(entry);
	}
}

void Entry::removeChildren()
{
	while (!children.empty())
		removeChild(children.begin()->second);
}

