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
#include <string>

#ifndef WULFOR_SEARCH
#define WULFOR_SEARCH

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"
#include "../client/User.h"
#include "../client/Util.h"
#include "../client/SearchManager.h"
#include "../client/SearchManagerListener.h"

#include "bookentry.hh"
#include "mainwindow.hh"

class SearchColumns: public Gtk::TreeModel::ColumnRecord {
	public:
		SearchColumns() {
			add(user);
			add(file);
			add(slots);
			add(size);
			add(path);
		}

		Gtk::TreeModelColumn<Glib::ustring> user, file, size, path, slots;
};

class Search: public BookEntry, public SearchManagerListener {
	public:	
		Search(MainWindow *mw);
		
		bool operator== (BookEntry &b);
		
		void searchPressed();
		void searchFor (Glib::ustring searchString);

		//from SearchManagerListener
		void on(SearchManagerListener::SR, SearchResult *result) throw();

	private:
		MainWindow *mw;
		SearchColumns columns;

		Gtk::HPaned pane;
		Gtk::TreeView resultsView;
		Glib::RefPtr<Gtk::ListStore> resultsStore;
		Gtk::ScrolledWindow scroll;
		Gtk::Table barTable;
		Gtk::Label searchLabel, sizeLabel, typeLabel, optionLabel;
		Gtk::Combo search;
		Gtk::OptionMenu sizeOM, unitOM, typeOM;
		Gtk::Menu sizeMenu, unitMenu, typeMenu;
		Gtk::CheckButton slotCB;
		Gtk::Button searchButton;
		Gtk::VBox barBox;
		Gtk::HBox mainBox;
		Gtk::Entry sizeEntry;
};

#else
class Search;
#endif
