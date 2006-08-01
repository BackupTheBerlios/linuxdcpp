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

#ifndef WULFOR_PUBLIC_HUBS_HH
#define WULFOR_PUBLIC_HUBS_HH

#include <pthread.h>

#include "bookentry.hh"
#include "treeview.hh"
#include "wulformanager.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/FavoriteManager.h>
#include <client/StringSearch.h>

class PublicHubs:
	public BookEntry,
	public FavoriteManagerListener
{
	public:
		PublicHubs();
		~PublicHubs();

		// From BookEntry
		GtkWidget *getWidget();

		// Client functions
		void downloadList_client();

	private:
		// GUI functions
		void updateList_gui();
		void setStatus_gui(GtkStatusbar *status, std::string text);

		// GUI callbacks
		static gboolean onButtonPress_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onKeyRelease_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onFilterHubs_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onConnect_gui(GtkWidget *widget, gpointer data);
		static void onRefresh_gui(GtkWidget *widget, gpointer data);
		static void onAddFav_gui(GtkMenuItem *item, gpointer data);
		static void onConfigure_gui(GtkWidget *widget, gpointer data);
		static void onAdd_gui(GtkWidget *widget, gpointer data);
		static void onMoveUp_gui(GtkWidget *widget, gpointer data);
		static void onMoveDown_gui(GtkWidget *widget, gpointer data);
		static void onRemove_gui(GtkWidget *widget, gpointer data);
		static void onCellEdited_gui(GtkCellRendererText *cell, char *path, char *text, gpointer data);

		// Client functions
		void refresh_client();
		void addFav_client(FavoriteHubEntry entry);

		// Client callbacks
		virtual void on(FavoriteManagerListener::DownloadStarting, const std::string &file) throw();
		virtual void on(FavoriteManagerListener::DownloadFailed, const std::string &file) throw();
		virtual void on(FavoriteManagerListener::DownloadFinished, const std::string &file) throw();

		pthread_mutex_t hubLock;
		HubEntry::List hubs;
		StringSearch filter;
		GtkDialog *configureDialog;
		GtkComboBox *comboBox;
		GtkWidget *mainBox;
		GtkEntry *filterEntry;
		TreeView listsView, hubView;
		GtkTreeSelection *hubSelection, *listsSelection;
		GtkListStore *hubStore, *listsStore;
		GtkStatusbar *statusMain, *statusHubs, *statusUsers;
		GtkMenu *menu;
		guint oldButton, oldType;
};

#else
class PublicHubs;
#endif
