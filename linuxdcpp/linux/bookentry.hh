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

#ifndef WULFOR_BOOK_ENTRY_HH
#define WULFOR_BOOK_ENTRY_HH

#include <gtk/gtk.h>
#include <string>

class BookEntry {
	public:
		BookEntry(std::string title, GCallback closeCallback);
		virtual ~BookEntry();

		//only to be called by gui thread
		void setLabel_gui(std::string text);
		void setLabelBold_gui (std::string text);
		GtkWidget *getTitle();
		virtual GtkWidget *getWidget() = 0;
		std::string getID() { return id; };

	private:
		std::string id;
		GtkWidget *box, *eventBox;
		GtkButton *button;
		GtkLabel *label;
		GtkTooltips *tips;
};

#else
class BookEntry;
#endif
