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

#include "bookentry.hh"
#include "treeview.hh"
#include "wulformanager.hh"

#include <client/Client.h>
#include <client/ClientManagerListener.h>
#include <client/CriticalSection.h>
#include <client/DCPlusPlus.h>
#include <client/FavoriteManager.h>
#include <client/QueueManager.h>
#include <client/SearchManager.h>
#include <client/stdinc.h>
#include <client/StringTokenizer.h>
#include <client/TimerManager.h>
#include <client/User.h>
#include <client/Util.h>

class Search : public BookEntry,
			public SearchManagerListener,
			public ClientManagerListener
{
public:
	Search();
	~Search ();

	void putValue (const string &str, int64_t size, SearchManager::SizeModes mode, SearchManager::TypeModes type);
		
	// From BookEntry
	GtkWidget *getWidget();

	virtual void on(SearchManagerListener::SR, SearchResult* aResult) throw();

	// ClientManagerListener
	virtual void on(ClientManagerListener::ClientConnected, Client* c) throw();
 	virtual void on(ClientManagerListener::ClientUpdated, Client* c) throw();
	virtual void on(ClientManagerListener::ClientDisconnected, Client* c) throw();

private:
	GtkWidget *mainBox;	
	
	hash_map<string, GtkWidget *> searchItems;

	TreeView hubView, resultView;
	GtkListStore *hubStore;
	GtkListStore *resultStore;
	GtkWidget *dirChooser;
	GtkWidget *fileChooser;
	GtkMenu *mainMenu, *downloadMenu, *downloadDirMenu;
	hash_map<string, GtkWidget *> menuItems;
	vector<GtkWidget*> downloadItems, downloadDirItems;
	GdkEventType previous;
	
	int64_t lastSearch;
	int droppedResult;
	int searchHits;
	bool isHash;
	TStringList searchlist;
	static string lastDir;
	int listItems;
	CriticalSection cs;
	static TStringList lastSearches;
	
	pthread_mutex_t searchLock;
	
	class HubInfo;
	class SearchInfo;

	string getTextFromMenu(GtkMenuItem *item);
	void changeHubs_gui (int mode, HubInfo *i); // Add, remove and changes name on hubs.
	void initHubs_gui ();  // Adds the current connected hubs to the list.
	void buildDownloadMenu_gui (int menu);
	void addResult_gui (SearchInfo *info);
	void search_gui ();
	static void onToggledClicked_gui (GtkCellRendererToggle *cell, gchar *path_str, gpointer data); // Is called when the checkbox in the "Hubs" is clicked.
	static void onSearchButtonClicked_gui (GtkWidget *widget, gpointer user_data);
	static void onSearchEntryDown_gui (GtkEntry *entry, gpointer user_data);
	static void onDownloadClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onDownloadToClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onDownloadFavoriteClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onDownloadFavoriteDirClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onDownloadDirClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onDownloadDirToClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onSearchForTTHClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onGetFileListClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onMatchQueueClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onPrivateMessageClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onAddFavoriteUserClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onGrantExtraSlotClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onRemoveUserFromQueueClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onRemoveClicked_gui (GtkMenuItem *item, gpointer user_data);
	static gboolean onButtonPressed_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
  	static gboolean onPopupMenu_gui (GtkWidget *widget, gpointer user_data);
	void popup_menu_gui (GdkEventButton *event, gpointer user_data);

	// To keep record of the connected hubs
	class HubInfo
	{
	public:
		HubInfo (Client* aClient, const string aIpPort, const string aName, bool aOp) :
		client(aClient), ipPort (aIpPort), name (aName), op (aOp) { };
		~HubInfo () { };
		
		Client* client;
		string ipPort;
		string name;
		bool op;
	};
	// Makes it easier to keep record of the searchresults, and simplifies download.
	class SearchInfo
	{
	public:
		SearchInfo (SearchResult *s) { s->incRef (); result = s; }
		~SearchInfo () { result->decRef (); }
		void download ();
		void downloadTo (string target);
		void downloadDir ();
		void downloadDirTo (string dir);
		void browse (bool file);
		SearchResult *result;
		string data;
	};
};

#else
class Search;
#endif
