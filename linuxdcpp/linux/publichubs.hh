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
#include <pthread.h>

#include "bookentry.hh"

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

		void downloadList();
		void updateList();
		
		static gboolean buttonEvent(
			GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void addFav(GtkMenuItem *i, gpointer d);
		static void filterHubs(GtkWidget *w, gpointer d);
		static void connect(GtkWidget *w, gpointer d);
		static void refresh (GtkWidget *widget, gpointer data);
		static void configure (GtkWidget *widget, gpointer data);
		static void add (GtkWidget *widget, gpointer data);
		static void moveUp (GtkWidget *widget, gpointer data);
		static void moveDown (GtkWidget *widget, gpointer data);
		static void remove (GtkWidget *widget, gpointer data);
		static void cellEdited(GtkCellRendererText *cell, 
			char *path, char *text, gpointer data);
		static void setStatus (GtkStatusbar *status, string text);

		//from HubManagerListener
		void on(HubManagerListener::DownloadStarting, 
			const string &file) throw();
		void on(HubManagerListener::DownloadFailed, 
			const string &file) throw();
		void on(HubManagerListener::DownloadFinished, 
			const string &file) throw();

	private:
		pthread_mutex_t hubLock;

		HubEntry::List hubs;
		StringSearch filter;

		GtkDialog *configureDialog;
		GtkComboBox *combo;
		GtkListStore *comboStore, *listsStore;
		GtkWidget *mainBox;
		GtkEntry *filterEntry;
		GtkTreeView *hubView, *listsView;
		GtkListStore *hubStore;
		GtkStatusbar *statusMain, *statusHubs, *statusUsers;
		
		GtkMenu *menu;
		GtkMenuItem *conItem, *favItem;

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
