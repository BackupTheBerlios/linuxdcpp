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
#include "finishedtransfers.hh"

class WulforManager {
	public:
		static WulforManager *get();
		static void start();
		static void stop();

		WulforManager();
		~WulforManager();
		std::string getPath();

		MainWindow *createMainWindow();
		MainWindow *getMainWindow();
		PrivateMessage *getPrivMsg(User::Ptr user);

		PublicHubs *addPublicHubs();
		Hub *addHub(std::string address, std::string nick="", std::string desc="", std::string password="");
		PrivateMessage *addPrivMsg(User::Ptr user);
		DownloadQueue *addDownloadQueue ();
		FavoriteHubs *addFavoriteHubs ();
		Search *addSearch();
		ShareBrowser *addShareBrowser(User::Ptr user, std::string file);
		FinishedTransfers *addFinishedUploads();
		FinishedTransfers *addFinishedDownloads();

		BookEntry *getBookEntry(int type, string id, bool raise);
		void deleteBookEntry(BookEntry *entry);
		void openFileList(std::string user, std::string path) const;

		enum {
			PUBLIC_HUBS,
			HUB,
			SEARCH,
			PRIVATE_MSG,
			SHARE_BROWSER,
			DOWNLOAD_QUEUE,
			FAVORITE_HUBS,
			SETTINGS,
			HASH,
			FINISHED_UPLOADS,
			FINISHED_DOWNLOADS
		};

	private:
		static gboolean guiCallback(gpointer data);
		static void closeEntry_callback(GtkWidget *widget, gpointer data);

		static WulforManager *manager;

		MainWindow *mainWin;
		std::vector<BookEntry *> bookEntrys;
};

#else
class WulforManager;
#endif
