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

#ifndef WULFOR_PUBLIC_HUBS
#define WULFOR_PUBLIC_HUBS

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"
#include "../client/HubManager.h"
#include "../client/StringSearch.h"

#include <gtkmm.h>

#include "mainwindow.hh"
#include "bookentry.hh"

class HubListColumns: public Gtk::TreeModel::ColumnRecord {
	public:
		HubListColumns() {
			add(name);
			add(description);
			add(users);
			add(address);
		}

		Gtk::TreeModelColumn<Glib::ustring> name, description, users, address;
};

class PublicHubs:
	public BookEntry,
	public HubManagerListener
{
	public:
		PublicHubs(MainWindow *mw);
		~PublicHubs();
		void updateList();
		
		//from HubManagerListener
		void on(HubManagerListener::DownloadStarting, 
			const string &file) throw();
		void on(HubManagerListener::DownloadFailed, 
			const string &file) throw();
		void on(HubManagerListener::DownloadFinished, 
			const string &file) throw();
		void on(HubManagerListener::FavoriteAdded, 
			const FavoriteHubEntry *fav) throw();
		void on(HubManagerListener::FavoriteRemoved, 
			const FavoriteHubEntry *fav) throw();
		void on(HubManagerListener::UserAdded, 
			const User::Ptr &user) throw();
		void on(HubManagerListener::UserRemoved, 
			const User::Ptr &user) throw();

		void refresh();
		void connect();
		bool operator== (BookEntry &b);
		void close();

	private:
		MainWindow *mw;

		HubEntry::List hubs;
		StringSearch filter;

		Gtk::TreeView hubList;
		Glib::RefPtr<Gtk::ListStore> hubStore;
		HubListColumns columns;
		
		Gtk::Entry filterEntry, connectEntry;
		Gtk::Button refreshButton, connectButton;
		Gtk::Frame filterFrame, connectFrame;
		Gtk::ScrolledWindow listScroll;
		Gtk::HBox fieldBox;
		Gtk::VButtonBox buttonBox;
		
};

#else
class PublicHubs;
#endif
