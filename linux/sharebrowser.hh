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

#ifndef WULFOR_SHARE_BROWSER
#define WULFOR_SHARE_BROWSER

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"
#include "../client/User.h"
#include "../client/DirectoryListing.h"
#include "../client/QueueManager.h"

#include "bookentry.hh"
#include "mainwindow.hh"

class ShareBrowser: public BookEntry, public QueueManagerListener {
	public:	
		ShareBrowser(User::Ptr user, MainWindow *mw);
		~ShareBrowser();
		
		void setPosition(std::string pos);
		void dirPressed (const Gtk::TreeModel::Path& path,
			Gtk::TreeViewColumn* column);
		
		bool operator== (BookEntry &b);
		
		//stuff from QueueManagerListener
		void on(QueueManagerListener::Finished, QueueItem *item) throw();

	private:
		class FileColumns: public Gtk::TreeModel::ColumnRecord {
			public:
				FileColumns() {
					add(name);
					add(type);
					add(size);
				}

				Gtk::TreeModelColumn<Glib::ustring> name, type, size;
		};

		class DirColumns: public Gtk::TreeModel::ColumnRecord {
			public:
				DirColumns() {
					add(dir);
					add(files);	//this one is hidden from the user
				}

				Gtk::TreeModelColumn<Glib::ustring> dir;
				Gtk::TreeModelColumn<DirectoryListing::File::List *> files;
		};

		void buildList();
		void processDirectory(DirectoryListing::Directory::List dir, 
			const Gtk::TreeModel::Row &row);
	
		User::Ptr user;
		MainWindow *mw;
		DirectoryListing *listing;

		DirColumns dCol;
		FileColumns fCol;

		Gtk::HPaned pane;
		Gtk::ScrolledWindow fileScroll, dirScroll;
		Gtk::TreeView dirView, fileView;
		Glib::RefPtr<Gtk::TreeStore> dirStore;
		Glib::RefPtr<Gtk::ListStore> fileStore;
		Gtk::Menu popupMenu;
		Gtk::MenuItem dowloadItem, dirItem;
};

#else
class ShareBrowser;
#endif
