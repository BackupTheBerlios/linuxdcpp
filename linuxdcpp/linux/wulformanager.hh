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

#ifndef WULFOR_MANAGER_HH
#define WULFOR_MANAGER_HH

#include <gtk/gtk.h>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <queue>
#include <vector>

#include "func.hh"
#include "mainwindow.hh"
#include "bookentry.hh"

class WulforManager {
	public:
		static WulforManager *get();
		static void start();
		static void stop();

		void createMainWindow();
		void dispatchGuiFunc(FuncBase *func);
		void dispatchClientFunc(FuncBase *func);

		WulforManager();
		~WulforManager();
		std::string getPath();

		MainWindow *getMainWindow();

		void addPublicHubs_gui();
		void addHub_gui(std::string address);

		BookEntry *getBookEntry(int type, string id, bool raise);
		void deleteBookEntry_gui(BookEntry *entry);

		enum {
			PUBLIC_HUBS,
			HUB,
			SEARCH,
			PRIVATE_MSG,
			SHAREBROWSER,
			DOWNLOAD_QUEUE,
			FAVOURITE_HUBS
		};

	private:
		static void *threadFunc_gui(void *data);
		static void *threadFunc_client(void *data);
		static gboolean guiCallback(gpointer data);
		static void closeEntry_callback(GtkWidget *widget, gpointer data);
		void callGuiFunc();
		void processGuiQueue();
		void processClientQueue();

		static WulforManager *manager;

		std::queue<FuncBase *> guiFuncs;
		std::queue<FuncBase *> clientFuncs;

		pthread_mutex_t guiLock, clientLock;
		sem_t guiSem, clientSem;
		pthread_t guiThread, clientThread;

		MainWindow *mainWin;
		std::vector<BookEntry *> bookEntrys;
};

#else
class WulforManager;
#endif
