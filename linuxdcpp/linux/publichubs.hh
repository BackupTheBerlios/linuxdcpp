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

#ifndef WULFOR_PUBLIC_HUBS_HH
#define WULFOR_PUBLIC_HUBS_HH

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <pthread.h>
#include "bookentry.hh"
#include "treeview.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/HubManager.h>
#include <client/StringSearch.h>

class PublicHubs: 
	public BookEntry,
	public HubManagerListener
{
	public:
		//constructor must be called from gui thread
		PublicHubs(GCallback closeCallback);
		~PublicHubs();

		GtkWidget *getWidget();

		//only to be called from client thread
		void PublicHubs::downloadList_client();

		//only to be called from the gui thread
		void filterHubs_gui();
		void connect_gui();
		void updateList_gui();
		void setStatus_gui(GtkStatusbar *status, std::string text);

		//from HubManagerListener
		void on(HubManagerListener::DownloadStarting, 
			const string &file) throw();
		void on(HubManagerListener::DownloadFailed, 
			const string &file) throw();
		void on(HubManagerListener::DownloadFinished, 
			const string &file) throw();
		void on(HubManagerListener::FavoriteAdded, 
			const FavoriteHubEntry *fav) throw();
		void on(HubManagerListener::FavoriteRemoved, 
			const FavoriteHubEntry *fav) throw();
		void on(HubManagerListener::UserAdded, 
			const User::Ptr &user) throw();
		void on(HubManagerListener::UserRemoved, 
			const User::Ptr &user) throw();

	private:
		static void filter_callback(GtkWidget *widget, gpointer data);
		static void connect_callback(GtkWidget *widget, gpointer data);

		pthread_mutex_t hubLock;

		HubEntry::List hubs;
		StringSearch filter;

		GtkWidget *mainBox;
		GtkEntry *filterEntry, *connectEntry;
		GtkTreeView *hubView;
		GtkListStore *hubStore;
		GtkTreeViewColumn *nameColumn, *descColumn, *usersColumn, *addressColumn;
		GtkStatusbar *statusMain, *statusHubs, *statusUsers;
		
		enum {
			COLUMN_NAME,
			COLUMN_DESC,
			COLUMN_USERS,
			COLUMN_ADDRESS
		};

		const int WIDTH_NAME, WIDTH_DESC, WIDTH_USERS, WIDTH_ADDRESS;
};

#else
class PublicHubs;
#endif
