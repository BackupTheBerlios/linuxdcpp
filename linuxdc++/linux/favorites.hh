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

#ifndef WULFOR_FAVORITES
#define WULFOR_FAVORITES

#include "../client/DCPlusPlus.h"
#include "../client/HubManager.h"

#include <gtkmm.h>
#include <iostream>

#include "bookentry.hh"
#include "mainwindow.hh"
#include "util.hh"

class FavoriteHubs : 	public HubManagerListener,
									public BookEntry
{
public:
	FavoriteHubs (MainWindow *mw);
	~FavoriteHubs ();

	bool operator== (BookEntry &b);
	void close();	
private:

	class Columns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Columns ()
		{
			add (autoconnect);
			add (name);
			add (description);
			add (nick);
			add (password);
			add (server);
			add (userdescription);
			add (entry);
		}

		Gtk::TreeModelColumn<Glib::ustring> 	name,
																		description,
																		nick,
																		password,
																		server,
																		userdescription;
		Gtk::TreeModelColumn<bool> autoconnect;
		Gtk::TreeModelColumn<FavoriteHubEntry*> entry;
	};

	Columns columns;

	void updateList(const FavoriteHubEntry::List& fl)
	{
		for(FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i)
			addEntry(*i, hubStore->children ().size ());
	}
	Gtk::TreeIter addEntry(const FavoriteHubEntry* entry, int pos);
	void preNew ();
	void preEdit ();
	void addDialog (bool edit, Glib::ustring uname="", Glib::ustring uaddress="", Glib::ustring udesc="", Glib::ustring unick="", Glib::ustring upassword="", Glib::ustring uuserdesc="");
	void remove ();
	void moveUp ();
	void moveDown ();
	void properties ();
	void connect ();
	void on(HubManagerListener::FavoriteAdded, const FavoriteHubEntry *entry) throw();
	void on(HubManagerListener::FavoriteRemoved, const FavoriteHubEntry *entry) throw();

	//Gtk::VBox mainBox;
	Gtk::TreeView hubView;
	Gtk::ScrolledWindow hubScroll;
	Glib::RefPtr<Gtk::ListStore> hubStore;
	Gtk::Menu menu;
	Gtk::VBox box;
	Gtk::Button bNew, bProperties, bRemove, bMoveUp, bMoveDown, bConnect;
	Gtk::HButtonBox buttonBox;

	void buttonPressed (GdkEventButton *event);
	void buttonReleased (GdkEventButton *event);
	void toggleClicked (const Glib::ustring &path_str);
	
	static int columnSize[];
	GdkEventType previous;
	MainWindow *mainwindow;
};

#endif
