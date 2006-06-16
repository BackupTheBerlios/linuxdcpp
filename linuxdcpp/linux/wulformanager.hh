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
#include "finishedtransfers.hh"

class WulforManager
{
	public:
		static void start();
		static void stop();
		static WulforManager *get();

		WulforManager();
		~WulforManager();

		std::string getPath();
		MainWindow *createMainWindow();
		MainWindow *getMainWindow();
		void dispatchGuiFunc(FuncBase *func);
		void dispatchClientFunc(FuncBase *func);

		// BookEntry functions
		BookEntry *getBookEntry_gui(std::string id, bool raise = TRUE);
		PublicHubs *addPublicHubs_gui();
		Hub *addHub_gui(std::string address, std::string nick="", std::string desc="", std::string password="");
		PrivateMessage *addPrivMsg_gui(User::Ptr user);
		DownloadQueue *addDownloadQueue_gui();
		FavoriteHubs *addFavoriteHubs_gui();
		Settings *openSettingsDialog_gui();
		Search *addSearch_gui();
		ShareBrowser *addShareBrowser_gui(User::Ptr user, std::string file);
		FinishedTransfers *addFinishedTransfers_gui(std::string title);
		void deleteAllBookEntries();

		// DialogEntry functions
		Hash *openHashDialog_gui();
		void deleteAllDialogEntries();

	private:
		// BookEntry functions
		void insertBookEntry_gui(BookEntry *entry);
		void deleteBookEntry_gui(BookEntry *entry);

		// DialogEntry functions
		DialogEntry *getDialogEntry_gui(std::string id);
		void insertDialogEntry_gui(DialogEntry *entry);
		void hideDialogEntry_gui(DialogEntry *entry);
		void deleteDialogEntry_gui(DialogEntry *entry);

		// Thread-related functions
		static void *threadFunc_gui(void *data);
		static void *threadFunc_client(void *data);
		void processGuiQueue();
		void processClientQueue();

		// Callbacks
		static void closeBookEntry_callback(GtkWidget *widget, gpointer data);
		static void closeDialogEntry_callback(GtkDialog *dialog, gint response, gpointer data);

		static WulforManager *manager;
		MainWindow *mainWin;

		std::vector<FuncBase *> guiFuncs;
		std::vector<FuncBase *> clientFuncs;
		hash_map<std::string, BookEntry *> bookEntries;
		hash_map<std::string, DialogEntry *> dialogEntries;

		pthread_mutex_t clientCallLock;
		pthread_mutex_t guiQueueLock, clientQueueLock;
		pthread_mutex_t bookEntryLock;
		pthread_mutex_t dialogEntryLock;
		sem_t guiSem, clientSem;
		pthread_t guiThread, clientThread;
};

#else
class WulforManager;
#endif
