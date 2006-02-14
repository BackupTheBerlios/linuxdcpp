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

#ifndef WULFOR_HUB_HH
#define WULFOR_HUB_HH

#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <iostream>
#include <map>
#include <pthread.h>
#include <semaphore.h>
#include <set>
#include <sstream>
#include <string>

#include "bookentry.hh"
#include "callback.hh"
#include "settingsmanager.hh"
#include "treeview.hh"
#include "wulformanager.hh"

#include <client/Client.h>
#include <client/ClientManager.h>
#include <client/DCPlusPlus.h>
#include <client/stdinc.h>
#include <client/TimerManager.h>
#include <client/User.h>

using namespace std;

class Hub:
	public BookEntry,
	public ClientListener
{
	public:
		//constructor should be called by gui thread
		Hub(std::string address, GCallback closeCallback);
		~Hub();

		// From BookEntry
		GtkWidget *getWidget();

		//to be called from gui thread
		void setStatus_gui(GtkStatusbar *status, std::string text);
		void updateUser_gui(std::string nick, int64_t shared, std::string iconFile,
				std::string description, std::string tag, std::string connection, 
				std::string email, bool opstatus=false);
		void findUser_gui(std::string nick, GtkTreeIter *iter);
		void removeUser_gui(std::string nick);
		void clearNickList_gui();
		void getPassword_gui();
		void addMessage_gui(std::string msg);
		void sendMessage_gui(GtkEntry *entry, gpointer data);
		void addPrivateMessage_gui(User::Ptr user, std::string msg);

		void popupNickMenu_gui(GtkWidget *, GdkEventButton *, gpointer);
		void browseItemClicked_gui(GtkMenuItem *, gpointer);
		void msgItemClicked_gui(GtkMenuItem *, gpointer);
		void grantItemClicked_gui(GtkMenuItem *, gpointer);
		void completion_gui(GtkWidget *, GdkEventKey *, gpointer);
                void setChatEntryFocus(GtkWidget *, GdkEventKey *, gpointer);

		//to be called from client thread
		void connectClient_client(string address, 
			string nick="", string desc="", string password="");
		void setPassword_client(std::string password);
		void updateUser_client(User::Ptr user);
		void sendMessage_client(std::string message);
		void getFileList_client(std::string nick);
		void grantSlot_client(string userName);

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
			Client *client, const string &address) throw();
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

	private:
		Client *client;
		
		Callback2<Hub, void, GtkEntry *> enterCallback;
		Callback3<Hub, void, GtkWidget *, GdkEventButton *> nickListCallback;
		Callback2<Hub, void, GtkMenuItem *> browseCallback, msgCallback, grantCallback;
		Callback3<Hub, void, GtkWidget *, GdkEventKey *> completionCallback, setFocusCallback;

		typedef StringMap NicksMap;
		typedef NicksMap::iterator NicksMapIter;

		NicksMap nicks;
		pthread_mutex_t clientLock;
		u_int32_t lastUpdate;

		GtkMenu *nickMenu;
		GtkMenuItem *browseItem, *msgItem, *grantItem;

		std::map<std::string, GdkPixbuf *> userIcons;

		GtkWidget *mainBox;
		GtkPaned *nickPane;
		GtkDialog *passwordDialog;
		GtkEntry *passwordEntry;
		GtkEntry *chatEntry;
		GtkTextView *chatText;
		GtkTextBuffer *chatBuffer;
		GtkStatusbar *mainStatus, *usersStatus, *sharedStatus;
		TreeView nickView;
		GtkListStore *nickStore;
		GtkTreeSelection *nickSelection;
		GtkScrolledWindow *chatScroll;
		GtkTextMark *chatMark;
		GtkTreeIter completion_iter;

		string prev_nick;
		string nick_completion;
};

#else
class Hub;
#endif
