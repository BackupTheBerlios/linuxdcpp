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

#ifndef WULFOR_MANAGER_HH
#define WULFOR_MANAGER_HH

#include <gtk/gtk.h>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <vector>

#include "func.hh"
#include "mainwindow.hh"
#include "bookentry.hh"
#include "dialogentry.hh"
#include "hub.hh"
#include "publichubs.hh"
#include "privatemessage.hh"
#include "downloadqueue.hh"
#include "favoritehubs.hh"
#include "settingsdialog.hh"
#include "sharebrowser.hh"
#include "search.hh"
#include "hashdialog.hh"

class WulforManager {
	public:
		static WulforManager *get();
		static void start();
		static void stop();

		void dispatchGuiFunc(FuncBase *func);
		void dispatchClientFunc(FuncBase *func);

		WulforManager();
		~WulforManager();
		std::string getPath();

		MainWindow *createMainWindow();
		MainWindow *getMainWindow();
		PrivateMessage *getPrivMsg_gui(User::Ptr user);
		PrivateMessage *getPrivMsg_client(User::Ptr user);

		PublicHubs *addPublicHubs_gui();
		Hub *addHub_gui(std::string address, std::string nick="", std::string desc="", std::string password="");
		PrivateMessage *addPrivMsg_gui(User::Ptr user);
		DownloadQueue *addDownloadQueue_gui ();
		FavoriteHubs *addFavoriteHubs_gui ();
		Settings *openSettingsDialog_gui ();
		Hash *openHashDialog_gui ();
		Search *addSearch_gui();
		ShareBrowser *addShareBrowser_gui(User::Ptr user, std::string file);

		BookEntry *getBookEntry_gui(int type, string id, bool raise);
		BookEntry *getBookEntry_gui(int nr);
		BookEntry *getBookEntry_client(int type, string id, bool raise);
		void deleteBookEntry_gui(BookEntry *entry);
		
		void deleteDialogEntry_gui();
		

		enum {
			PUBLIC_HUBS,
			HUB,
			SEARCH,
			PRIVATE_MSG,
			SHARE_BROWSER,
			DOWNLOAD_QUEUE,
			FAVORITE_HUBS,
			SETTINGS,
			HASH
		};

	private:
		static void *threadFunc_gui(void *data);
		static void *threadFunc_client(void *data);
		static gboolean guiCallback(gpointer data);
		static void closeEntry_callback(GtkWidget *widget, gpointer data);
		static void dialogCloseEntry_callback(GtkWidget *widget, gpointer data);
		void callGuiFunc();
		void processGuiQueue();
		void processClientQueue();

		static WulforManager *manager;

		std::vector<FuncBase *> guiFuncs;
		std::vector<FuncBase *> clientFuncs;

		pthread_mutex_t guiCallLock, clientCallLock;
		pthread_mutex_t guiQueueLock, clientQueueLock;
		pthread_mutex_t bookEntryLock;
		sem_t guiSem, clientSem;
		pthread_t guiThread, clientThread;

		MainWindow *mainWin;
		std::vector<BookEntry *> bookEntrys;
		DialogEntry *dialogEntry;
};

#else
class WulforManager;
#endif
