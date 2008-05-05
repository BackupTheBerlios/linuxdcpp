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

#ifndef WULFOR_BOOK_ENTRY_HH
#define WULFOR_BOOK_ENTRY_HH

#include "entry.hh"

class BookEntry : public Entry
{
	public:
		BookEntry() {}
		BookEntry(const std::string &title, const std::string &id, const std::string &glade, bool duplicates = FALSE);
		virtual ~BookEntry() {}

		GtkWidget *getContainer();
		GtkWidget *getLabelBox() { return labelBox; }
		GtkWidget *getCloseButton() { return closeButton; }
		void setLabel_gui(std::string text);
		void setBold_gui();
		void unsetBold_gui();
		void setWindowItem(GtkWidget *windowItem);
		virtual void show() = 0;

	protected:
		GtkWidget *getWindowItem();

	private:
		std::string title;
		GtkWidget *eventBox;
		GtkWidget *labelBox;
		GtkWidget *windowItem;
		GtkWidget *closeButton;
		GtkLabel *label;
		GtkTooltips *tips;
		bool bold;
		static const guint labelSize = 20; ///@todo: make a preference?
};

#else
class BookEntry;
#endif
