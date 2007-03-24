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

#ifndef WULFOR_ENTRY_HH
#define WULFOR_ENTRY_HH

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <string>


class Entry
{
	public:

		Entry() {}
		Entry(const std::string &id, const std::string &glade);
		virtual ~Entry();
		const std::string& getID();
		virtual void applyCallback(GCallback closeCallback) = 0;
		virtual GtkWidget *getContainer() = 0;

	protected:
		GtkWidget *getWidget(const std::string &name);

	private:
		GladeXML *xml;
		std::string id;
};

#else
class Entry;
#endif
