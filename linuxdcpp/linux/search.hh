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

#ifndef WULFOR_SEARCH_HH
#define WULFOR_SEARCH_HH

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/ClientManager.h>
#include <client/QueueManager.h>
#include <client/SearchManager.h>
#include <client/User.h>

#include "bookentry.hh"
#include "treeview.hh"

class Search:
	public BookEntry,
	public SearchManagerListener,
	public ClientManagerListener
{
	public:
		Search();
		~Search();

		void putValue_gui(const std::string &str, int64_t size, SearchManager::SizeModes mode, SearchManager::TypeModes type);

	private:
		// GUI functions
		void initHubs_gui();
		void addHub_gui(std::string name, std::string url, bool op);
		void modifyHub_gui(std::string name, std::string url, bool op);
		void removeHub_gui(std::string url);
		void buildDownloadMenu_gui();
		void popupMenu_gui();
		void setStatus_gui(std::string statusBar, std::string text);
		void search_gui();
		void addResult_gui(SearchResult *result);
		void clearList_gui();

		// GUI callbacks
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onSearchEntryKeyPressed_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean searchFilterFunc_gui(GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
		static void onComboBoxChanged_gui(GtkWidget *widget, gpointer data);
		static void onSearchButtonClicked_gui(GtkWidget *widget, gpointer data);
		static void onButtonToggled_gui(GtkToggleButton *button, gpointer data);
		static void onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onDownloadClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadFavoriteClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadToClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadToMatchClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadDirClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadFavoriteDirClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadDirToClicked_gui(GtkMenuItem *item, gpointer data);
		static void onSearchByTTHClicked_gui(GtkMenuItem *item, gpointer data);
		static void onGetFileListClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMatchQueueClicked_gui(GtkMenuItem *item, gpointer data);
		static void onPrivateMessageClicked_gui(GtkMenuItem *item, gpointer data);
		static void onAddFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data);
		static void onGrantExtraSlotClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveUserFromQueueClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveClicked_gui(GtkMenuItem *item, gpointer data);

		// Client functions
		void download_client(std::string target, SearchResult *result);
		void downloadDir_client(std::string target, SearchResult *result);
		void addSource_client(std::string source, SearchResult *result);
		void getFileList_client(User::Ptr &user, QueueItem::FileFlags flags);
		void addFavUser_client(User::Ptr &user);
		void grantSlot_client(User::Ptr &user);
		void removeSource_client(User::Ptr &user);

		// Client callbacks
		virtual void on(ClientManagerListener::ClientConnected, Client *client) throw();
	 	virtual void on(ClientManagerListener::ClientUpdated, Client *client) throw();
		virtual void on(ClientManagerListener::ClientDisconnected, Client *client) throw();
		virtual void on(SearchManagerListener::SR, SearchResult *result) throw();

		TreeView hubView, resultView;
		GtkListStore *hubStore;
		GtkListStore *resultStore;
		GtkTreeModel *searchFilterModel;
		GtkTreeModel *sortedFilterModel;
		GtkTreeSelection *selection;
		GdkEventType oldEventType;
		GtkWidget *searchEntry;
		TStringList searchlist;
		static GtkTreeModel *searchEntriesModel;
		GdkPixbuf *iconFile;
		GdkPixbuf *iconDirectory;
		int droppedResult;
		int searchHits;
		bool isHash;
		bool onlyFree;
		bool onlyTTH;
		static bool onlyOp;
};

#else
class Search;
#endif
