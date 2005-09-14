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

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <set>
#include <map>

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/Client.h>
#include <client/ClientManager.h>
#include <client/User.h>

#include "bookentry.hh"
#include "callback.hh"

class Hub:
	public BookEntry,
	public ClientListener
{
	public:
		//constructor should be called by gui thread
		Hub(std::string address, GCallback closeCallback);
		~Hub();

		//from bookentry
		GtkWidget *getWidget();

		void connectClient(std::string address, std::string nick, std::string desc, std::string password);
		void setStatus(GtkStatusbar *status, std::string text);
		void updateUser(User::Ptr user);
		void findUser(std::string nick, GtkTreeIter *iter);
		void removeUser(std::string nick);
		void updateCounts();

		static void sendMessage(GtkEntry *entry, gpointer data);
		
		static gboolean popupNickMenu(GtkWidget *, GdkEventButton *button, gpointer data);
		static void browseItemClicked(GtkMenuItem *, gpointer data);
		static void msgItemClicked(GtkMenuItem *, gpointer data);
		static void grantItemClicked(GtkMenuItem *, gpointer data);
		static void doTabCompletion(GtkWidget *, GdkEventKey *key, gpointer data);
		static void setChatEntryFocus(GtkWidget *, GdkEventKey *key, gpointer data);

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
		bool listFrozen;
		Client *client;

		GtkMenu *nickMenu;
		GtkMenuItem *browseItem, *msgItem, *grantItem;

		std::map<std::string, GdkPixbuf *> userIcons;

		GtkWidget *mainBox;
		GtkDialog *passwordDialog;
		GtkEntry *passwordEntry;
		GtkEntry *chatEntry;
		GtkTextView *chatText;
		GtkTextBuffer *chatBuffer;
		GtkStatusbar *mainStatus, *usersStatus, *sharedStatus;
		GtkTreeView *nickView;
		GtkListStore *nickStore;
		GtkTreeSelection *nickSelection;
		GtkScrolledWindow *chatScroll;
		GtkTextMark *chatMark;
		GtkTreeIter completion_iter;

		string prev_nick;
		string nick_completion;
		
		const int WIDTH_ICON, WIDTH_NICK, WIDTH_SHARED, WIDTH_DESCRIPTION, WIDTH_TAG, WIDTH_CONNECTION, WIDTH_EMAIL;
		
		enum {
			COLUMN_ICON,
			COLUMN_NICK,
			COLUMN_SHARED,
			COLUMN_DESCRIPTION,
			COLUMN_TAG,
			COLUMN_CONNECTION,
			COLUMN_EMAIL,
			COLUMN_SHARED_BYTES
		};
};

#else
class Hub;
#endif
