/* 
* Copyright (C) 2004 Jens Oknelid, paskharen@spray.se
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

#include <gtkmm.h>

#ifndef WULFOR_BOOK_ENTRY
#define WULFOR_BOOK_ENTRY

enum
{
	BOOK_PUBLIC_HUBS,
	BOOK_HUB,
	BOOK_SEARCH,
	BOOK_PRIVATE_MESSAGE,
	BOOK_FILE_LIST,
	BOOk_DOWNLOAD_QUEUE,
	BOOK_NUMBER
};

class BookEntry: public Gtk::VBox {
	public:
		BookEntry();
		virtual ~BookEntry();

		virtual bool operator== (BookEntry &b) = 0;

		virtual void close();
		Gtk::Label &getLabel();
		Gtk::HBox &getBox();

		int getID ();

		void setParent (Gtk::Notebook *p);
		Gtk::Notebook *getParent () { return parent; }
		
		const static bool useMultipleTabs[];
		
	protected:
		Gtk::HBox box;
		Gtk::Label label;
		Gtk::Button button;

		Gtk::Notebook *parent;

		int ID;
};

#else
class BookEntry;
#endif

