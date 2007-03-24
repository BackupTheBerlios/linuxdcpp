/*
 * Copyright © 2004-2007 Jens Oknelid, paskharen@gmail.com
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

#ifndef WULFOR_MANAGER_HH
#define WULFOR_MANAGER_HH

#include <gtk/gtk.h>
#include <pthread.h>
#include <semaphore.h>
#include <string>

#include "bookentry.hh"
#include "dialogentry.hh"
#include "func.hh"
#include "mainwindow.hh"

class WulforManager
{
	public:
		static void start();
		static void stop();
		static WulforManager *get();

		WulforManager();
		~WulforManager();

		std::string getPath();
		MainWindow *getMainWindow();
		void dispatchGuiFunc(FuncBase *func);
		void dispatchClientFunc(FuncBase *func);

		// BookEntry functions
		BookEntry *addPublicHubs_gui();
		BookEntry *addHub_gui(const std::string &address, const std::string &encoding = "");
		BookEntry *addPrivMsg_gui(User::Ptr user, bool raise = TRUE);
		BookEntry *addDownloadQueue_gui();
		BookEntry *addFavoriteHubs_gui();
		BookEntry *addSearch_gui();
		BookEntry *addShareBrowser_gui(User::Ptr user, const std::string &file, const std::string &dir, bool raise = TRUE);
		BookEntry *addFinishedTransfers_gui(const std::string &title);
		void deleteEntry_gui(Entry *entry);

		// DialogEntry functions
		int openHashDialog_gui();
		int openSettingsDialog_gui();

	private:
		// MainWindow-related functions
		void createMainWindow();

		// Entry functions
		void insertBookEntry_gui(BookEntry *entry, bool raise = TRUE);
		void insertDialogEntry_gui(DialogEntry *entry);
		BookEntry *getBookEntry_gui(const std::string &id, bool raise = TRUE);
		DialogEntry *getDialogEntry_gui(const std::string &id);
		void deleteAllEntries();

		// Thread-related functions
		static void *threadFunc_gui(void *data);
		static void *threadFunc_client(void *data);
		void processGuiQueue();
		void processClientQueue();

		// Callbacks
		static gboolean onCloseWindow_gui(GtkWidget *widget, GdkEvent *event, gpointer data);
		static void onCloseBookEntry_gui(GtkWidget *widget, gpointer data);
		static void onCloseDialogEntry_gui(GtkDialog *dialog, gint response, gpointer data);

		static WulforManager *manager;
		MainWindow *mainWin;
		std::string path;
		std::vector<FuncBase *> guiFuncs;
		std::vector<FuncBase *> clientFuncs;
		hash_map<std::string, Entry *> entries;
		pthread_mutex_t clientCallLock;
		pthread_rwlock_t guiQueueLock;
		pthread_rwlock_t clientQueueLock;
		pthread_rwlock_t entryLock;
		sem_t guiSem, clientSem;
		pthread_t guiThread, clientThread;
};

#else
class WulforManager;
#endif
