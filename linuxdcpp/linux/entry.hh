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

#ifndef WULFOR_ENTRY_HH
#define WULFOR_ENTRY_HH

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <string>
#include <map>

class Entry
{
	public:
		const static std::string DOWNLOAD_QUEUE;
		const static std::string FAVORITE_HUBS;
		const static std::string FINISHED_DOWNLOADS;
		const static std::string FINISHED_UPLOADS;
		const static std::string HASH_DIALOG;
		const static std::string HUB;
		const static std::string MAIN_WINDOW;
		const static std::string PRIVATE_MESSAGE;
		const static std::string PUBLIC_HUBS;
		const static std::string SEARCH;
		const static std::string SETTINGS_DIALOG;
		const static std::string SHARE_BROWSER;

		Entry() {}
		Entry(const std::string &id, const std::string &glade = "", bool duplicates = FALSE);
		virtual ~Entry();
		const std::string& getID();
		virtual GtkWidget *getContainer() = 0;
		void remove();

	protected:
		GtkWidget *getWidget(const std::string &name);
		void addChild(Entry *entry);
		Entry *getChild(const std::string &id);
		void removeChild(const std::string &id);
		void removeChild(Entry *entry);
		void removeChildren();

	private:
		GladeXML *xml;
		std::string id;
		std::map<std::string, Entry *> children;
};

#else
class Entry;
#endif
