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

#ifndef WULFOR_PUBLIC_HUBS_HH
#define WULFOR_PUBLIC_HUBS_HH

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <iostream>
#include <pthread.h>
#include <sstream>

#include "bookentry.hh"
#include "callback.hh"
#include "treeview.hh"
#include "wulformanager.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/HubManager.h>
#include <client/StringSearch.h>

using namespace std;

class PublicHubs: 
	public BookEntry,
	public HubManagerListener
{
	public:
		//constructor must be called from gui thread
		PublicHubs();
		~PublicHubs();

		// From BookEntry
		GtkWidget *getWidget();

		//only to be called from client thread
		void downloadList_client();
		void refresh_client();
		void addFav_client(FavoriteHubEntry entry);

		//only to be called from the gui thread
		void filterHubs_gui(GtkWidget *widget, gpointer data);
		void connect_gui(GtkWidget *widget, gpointer data);
		void refresh_gui(GtkWidget *widget, gpointer data);
		void configure_gui(GtkWidget *widget, gpointer data);

		void moveUp_gui(GtkWidget *widget, gpointer data);
		void moveDown_gui(GtkWidget *widget, gpointer data);
		void add_gui(GtkWidget *widget, gpointer data);
		void remove_gui(GtkWidget *widget, gpointer data);

		void cellEdited_gui(GtkCellRendererText *cell, 
			char *path, char *text, gpointer data);
		gboolean buttonEvent_gui(
			GtkWidget *widget, GdkEventButton *event, gpointer);
		void addFav_gui(GtkMenuItem *i, gpointer d);

		void updateList_gui();
		void setStatus_gui(GtkStatusbar *status, std::string text);

		//from HubManagerListener
		void on(HubManagerListener::DownloadStarting, 
			const string &file) throw();
		void on(HubManagerListener::DownloadFailed, 
			const string &file) throw();
		void on(HubManagerListener::DownloadFinished, 
			const string &file) throw();

	private:
		Callback2<PublicHubs, void, GtkWidget *> 
			filterCallback, connectCallback, refreshCallback, configureCallback;
		Callback2<PublicHubs, void, GtkWidget *> 
			upCallback, downCallback, addCallback, removeCallback;
		Callback4<PublicHubs, void, GtkCellRendererText *, char *, char *>
			editCallback;
		Callback3<PublicHubs, gboolean, GtkWidget *, GdkEventButton *> 
			mouseButtonCallback;
		Callback2<PublicHubs, void, GtkMenuItem *> addFavCallback;
		
		pthread_mutex_t hubLock;

		HubEntry::List hubs;
		StringSearch filter;

		GtkDialog *configureDialog;
		GtkComboBox *combo;
		GtkWidget *mainBox;
		GtkEntry *filterEntry;
		TreeView listsView, hubView;
		GtkListStore *hubStore, *comboStore, *listsStore;
		GtkStatusbar *statusMain, *statusHubs, *statusUsers;
		
		GtkMenu *menu;
		GtkMenuItem *conItem, *favItem;
};

#else
class PublicHubs;
#endif
