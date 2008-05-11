/*
 * Copyright © 2004-2008 Jens Oknelid, paskharen@gmail.com
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

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/Client.h>

#include "bookentry.hh"
#include "treeview.hh"

class UserCommandMenu;

class Hub:
	public BookEntry,
	public ClientListener
{
	public:
		Hub(const std::string &address, const std::string &encoding);
		virtual ~Hub();
		virtual void show();

		// Client functions
		void reconnect_client();

	private:
		typedef std::map<string, string> ParamMap;

		// GUI functions
		void setStatus_gui(std::string statusBar, std::string text);
		bool findUser_gui(const std::string &nick, GtkTreeIter *iter);
		void updateUser_gui(ParamMap id);
		void removeUser_gui(std::string nick);
		void clearNickList_gui();
		void popupNickMenu_gui();
		void getPassword_gui();
		void addMessage_gui(std::string message);
		void applyTags_gui(const string &line);
		void addStatusMessage_gui(std::string message);
		void updateCursor_gui(GtkWidget *widget);

		// GUI callbacks
		static gboolean onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data);
		static gboolean onNickListButtonPress_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onNickListButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onNickListKeyRelease_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onEntryKeyPress_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onNickTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
		static gboolean onLinkTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
		static gboolean onHubTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
		static gboolean onMagnetTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
		static gboolean onChatPointerMoved_gui(GtkWidget *widget, GdkEventMotion *event, gpointer data);
		static gboolean onChatVisibilityChanged_gui(GtkWidget *widget, GdkEventVisibility *event, gpointer data);
		static void onChatScroll_gui(GtkAdjustment *adjustment, gpointer data);
		static void onChatResize_gui(GtkAdjustment *adjustment, gpointer data);
		static void onSendMessage_gui(GtkEntry *entry, gpointer data);
		static void onCopyNickItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMatchItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMsgItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onGrantItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveUserItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onCopyURIClicked_gui(GtkMenuItem *item, gpointer data);
		static void onOpenLinkClicked_gui(GtkMenuItem *item, gpointer data);
		static void onOpenHubClicked_gui(GtkMenuItem *item, gpointer data);
		static void onSearchMagnetClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMagnetPropertiesClicked_gui(GtkMenuItem *item, gpointer data);

		// Client functions
		void connectClient_client(string address, string encoding);
		void disconnect_client();
		void setPassword_client(std::string password);
		void sendMessage_client(std::string message);
		void getFileList_client(std::string cid, bool match);
		void grantSlot_client(std::string cid);
		void removeUserFromQueue_client(std::string cid);
		void redirect_client(std::string address);
		void rebuildHashData_client();
		void refreshFileList_client();
		void addAsFavorite_client();
		void checkFavoriteUserJoin_client(std::string cid);
		void getParams_client(ParamMap &user, Identity &id);

		// Client callbacks
		virtual void on(ClientListener::Connecting, Client *) throw();
		virtual void on(ClientListener::Connected, Client *) throw();
		virtual void on(ClientListener::UserUpdated, Client *, const OnlineUser &user) throw();
		virtual void on(ClientListener::UsersUpdated, Client *, const OnlineUser::List &list) throw();
		virtual void on(ClientListener::UserRemoved, Client *, const OnlineUser &user) throw();
		virtual void on(ClientListener::Redirect, Client *, const string &address) throw();
		virtual void on(ClientListener::Failed, Client *, const string &reason) throw();
		virtual void on(ClientListener::GetPassword, Client *) throw();
		virtual void on(ClientListener::HubUpdated, Client *) throw();
		virtual void on(ClientListener::Message, Client *, const OnlineUser &user, const string &message) throw();
		virtual void on(ClientListener::StatusMessage, Client *, const string &message) throw();
		virtual void on(ClientListener::PrivateMessage, Client *, const OnlineUser &from,
			const OnlineUser &to, const OnlineUser &replyTo, const string &message) throw();
		virtual void on(ClientListener::NickTaken, Client *) throw();
		virtual void on(ClientListener::SearchFlood, Client *, const string &message) throw();

		hash_map<std::string, std::string> userMap;
		hash_map<std::string, GtkTreeIter> userIters;
		hash_map<std::string, GdkPixbuf *> userIcons;
		std::string completionKey;
		Client *client;
		TreeView nickView;
		GtkListStore *nickStore;
		GtkTreeSelection *nickSelection;
		GtkTextBuffer *chatBuffer;
		GtkTextMark *chatMark;
		gint oldType;
		std::vector<std::string> history;
		int historyIndex;
		static const int maxLines = 1000;
		static const int maxHistory = 20;
		int64_t totalShared;
		GdkCursor *handCursor;
		bool aboveTag;
		std::string selectedTag;
		UserCommandMenu *userCommandMenu;
		std::string address;
		std::string encoding;
		bool scrollToBottom;
};

#else
class Hub;
#endif
