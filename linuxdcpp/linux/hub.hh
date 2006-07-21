/*
 * Copyright © 2004-2006 Jens Oknelid, paskharen@gmail.com
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef WULFOR_HUB_HH
#define WULFOR_HUB_HH

#include "bookentry.hh"
#include "settingsmanager.hh"
#include "treeview.hh"
#include "wulformanager.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/Client.h>
#include <client/ClientManager.h>
#include <client/TimerManager.h>
#include <client/User.h>

class Hub:
	public BookEntry,
	public ClientListener,
	public TimerManagerListener
{
	public:
		Hub(std::string address);
		~Hub();

		// From BookEntry
		GtkWidget *getWidget();

		// Client functions
		void connectClient_client(string address, string nick = "", string desc = "", string password = "");

	private:
		// GUI functions
		void setStatus_gui(GtkStatusbar *status, std::string text);
		void findUser_gui(std::string nick, GtkTreeIter *iter);
		void updateUser_gui(Identity id);
		void removeUser_gui(std::string nick);
		void clearNickList_gui();
		void getPassword_gui();
		void addMessage_gui(std::string message);
		void addStatusMessage_gui(std::string message);
		void addPrivateMessage_gui(Identity id, std::string message);
		void sortList_gui();

		// GUI callbacks
		static void onSendMessage_gui(GtkEntry *entry, gpointer data);
		static gboolean onNickListButtonPress_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onNickListButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onNickListKeyRelease_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onEntryKeyPress_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMsgItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onGrantItemClicked_gui(GtkMenuItem *item, gpointer data);

		// Client functions
		void setPassword_client(std::string password);
		void sendMessage_client(std::string message);
		void getFileList_client(std::string nick);
		void grantSlot_client(std::string nick);
		void redirect_client(std::string address);
		void rebuildHashData_client();
		void refreshFileList_client();
		void addAsFavorite_client();

		// Client callbacks
		// From ClientListener
		void on(ClientListener::Connecting, Client *) throw();
		void on(ClientListener::Connected, Client *) throw();
		void on(ClientListener::BadPassword, Client *) throw();
		void on(ClientListener::UserUpdated, Client *, const OnlineUser &user) throw();
		void on(ClientListener::UsersUpdated, Client *, const OnlineUser::List &list) throw();
		void on(ClientListener::UserRemoved, Client *, const OnlineUser &user) throw();
		void on(ClientListener::Redirect, Client *, const string &address) throw();
		void on(ClientListener::Failed, Client *, const string &reason) throw();
		void on(ClientListener::GetPassword, Client *) throw();
		void on(ClientListener::HubUpdated, Client *) throw();
		void on(ClientListener::Message, Client *, const OnlineUser &user, const string &message) throw();
		void on(ClientListener::StatusMessage, Client *, const string &message) throw();
		void on(ClientListener::PrivateMessage, Client *, const OnlineUser &from,
			const OnlineUser &to, const OnlineUser &replyTo, const string &message) throw();
		void on(ClientListener::NickTaken, Client *) throw();
		void on(ClientListener::SearchFlood, Client *, const string &message) throw();

		// From TimerManagerListener
		void on(TimerManagerListener::Second, u_int32_t tics) throw();
		void on(TimerManagerListener::Minute, u_int32_t tics) throw();

		hash_map<std::string, Identity> idMap;
		hash_map<std::string, GdkPixbuf *> userIcons;
		Client *client;
		bool usersUpdated, sorted;
		GtkMenu *nickMenu;
		GtkWidget *mainBox;
		GtkPaned *nickPane;
		GtkDialog *passwordDialog;
		GtkEntry *passwordEntry;
		GtkStatusbar *mainStatus, *usersStatus, *sharedStatus;
		TreeView nickView;
		GtkListStore *nickStore;
		GtkTreeSelection *nickSelection;
		GtkWidget *scrolledwindow2;
		GtkTextView *chatText;
		GtkTextBuffer *chatBuffer;
		GtkScrolledWindow *chatScroll;
		GtkTextMark *chatMark;
		GtkEntry *chatEntry;
		GtkEntryCompletion *completion;
		gint oldType;
		std::vector<std::string> history;
		int historyIndex;
		static const int maxLines = 1000;
		static const int maxHistory = 20;
};

#else
class Hub;
#endif
