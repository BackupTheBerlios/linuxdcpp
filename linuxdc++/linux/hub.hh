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

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"
#include "../client/Client.h"
#include "../client/ClientManager.h"
#include "../client/User.h"

#include <gtkmm.h>
#include <string>
#include <iostream>
#include <map>

#include "bookentry.hh"
#include "mainwindow.hh"

#ifndef WULFOR_HUB
#define WULFOR_HUB

enum 
{
	HUB_STATUS_MAIN,
	HUB_STATUS_USERS,
	HUB_STATUS_SHARED,
	HUB_NUM_MESSAGES
};

class Hub:
	public ClientListener,
	public BookEntry
{
	public:
		Hub(std::string address, MainWindow *mw);
		~Hub();
		
		bool operator== (BookEntry &b);

		//all this from ClientListener...
		void on(ClientListener::Connecting, Client *client) throw();
		void on(ClientListener::Connected, Client *client) throw();
		void on(ClientListener::BadPassword, Client *client) throw();
		void on(ClientListener::UserUpdated, Client *client, 
			const User::Ptr&) throw();
		void on(ClientListener::UsersUpdated, 
			Client *client, const User::List &list) throw();
		void on(ClientListener::UserRemoved, 
			Client *client, const User::Ptr &user) throw();
		void on(ClientListener::Redirect, 
			Client *client, const string &msg) throw();
		void on(ClientListener::Failed, 
			Client *client, const string &reason) throw();
		void on(ClientListener::GetPassword, Client *client) throw();
		void on(ClientListener::HubUpdated, Client *client) throw();
		void on(ClientListener::Message, 
			Client *client, const string &msg) throw();
		void on(ClientListener::PrivateMessage, 
			Client *client, const User::Ptr &user, const string &msg) throw();
		void on(ClientListener::UserCommand, Client *client, 
			int, int, const string&, const string&) throw();
		void on(ClientListener::HubFull, Client *client) throw();
		void on(ClientListener::NickTaken, Client *client) throw();
		void on(ClientListener::SearchFlood, Client *client, 
			const string &msg) throw();
		void on(ClientListener::NmdcSearch, Client *client, const string&, 
			int, int64_t, int, const string&) throw();

		void browseClicked();
		void pmClicked();
		void favClicked();
		void showPopupMenu(GdkEventButton* event);
		void enterPressed();
		void somethingChanged();
		void valueChanged();
		void setStatus (std::string text, int num);
		void close ();
		void showPasswordDialog ();
	
	private:
		class NickListColumns: public Gtk::TreeModel::ColumnRecord {
			public:
				NickListColumns() {
					add(nick);
					add(shared);
				}

				Gtk::TreeModelColumn<Glib::ustring> nick, shared;
		};

		Gtk::TreeModel::iterator findUser(Glib::ustring nick);
		void updateUser (const User::Ptr &user);
		std::string address;

		Gtk::VBox mainBox;
		Gtk::HBox statusBox;		
		
		Gtk::HPaned pane;
		Gtk::ScrolledWindow nickScroll, chatScroll;
		Gtk::Entry chatEntry;
		Gtk::VBox chatBox;
		Gtk::TextView chat;
		Gtk::Statusbar statusBar[HUB_NUM_MESSAGES];
		
		Glib::RefPtr<Gtk::ListStore> nickStore;
		Gtk::TreeView nickView;
		NickListColumns columns;
	
		Gtk::Menu popupMenu;
		Gtk::MenuItem browseItem, pmItem, favItem;
			
		Client *client;
		MainWindow *mw;
		double scrollVal;
		std::map<Glib::ustring, Gtk::TreeModel::iterator> userMap;
};

#else
class Hub;
#endif

