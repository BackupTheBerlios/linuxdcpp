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
#include "../client/QueueManager.h"

#include "bookentry.hh"
#include "mainwindow.hh"

class Search: public BookEntry, public SearchManagerListener {
	public:	
		Search(MainWindow *mw);
		
		bool operator== (BookEntry &b);
		
		void searchPressed();
		void searchFor (Glib::ustring searchString);

		//from SearchManagerListener
		void on(SearchManagerListener::SR, SearchResult *result) throw();
		void close();

	private:

		class SearchInfo
		{
			public:
				SearchInfo (SearchResult *s) { s->incRef (); sr = s; }
				~SearchInfo () { sr->decRef (); }
				void download ();
				void downloadDir ();
				void getFilelist ();
			private:
				SearchResult *sr;
		};

		class SearchColumns: public Gtk::TreeModel::ColumnRecord {
			public:
				SearchColumns()
				{
					add(user);
					add(file);
					add(slots);
					add(filesize);
					add(path);
					add(type);
					add(connection);
					add(hub);
					add(exactsize);
					add(ip);
					add(tth);
					add(info);
				}
	
				Gtk::TreeModelColumn<Glib::ustring> user, file, filesize, path, slots, type, connection, hub, exactsize, ip, tth;
				Gtk::TreeModelColumn<SearchInfo*> info;
		};
	
		enum {
			COLUMN_FIRST,
			COLUMN_FILENAME = COLUMN_FIRST,
			COLUMN_NICK,
			COLUMN_TYPE,
			COLUMN_SIZE,
			COLUMN_PATH,
			COLUMN_SLOTS,
			COLUMN_CONNECTION,
			COLUMN_HUB,
			COLUMN_EXACT_SIZE,
			COLUMN_IP,
			COLUMN_TTH,
			COLUMN_LAST
		};	
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
		Gtk::Menu sizeMenu, unitMenu, typeMenu, popupMenu;
		Gtk::CheckButton slotCB;
		Gtk::Button searchButton;
		Gtk::VBox barBox, superMainBox;
		Gtk::HBox mainBox, statusbar;
		Gtk::Statusbar status; 
		Gtk::Entry sizeEntry;

		static int columnSize[];
		GdkEventType resultPrevious;

		void buttonPressedResult (GdkEventButton *event);
		void buttonReleasedResult (GdkEventButton *event);
		void getFileList ();
		void download ();
		void downloadDir ();
};

#else
class Search;
#endif
