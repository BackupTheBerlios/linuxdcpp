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

#ifndef WULFOR_FAVORITE_HUBS_HH
#define WULFOR_FAVORITE_HUBS_HH

#include <iostream>

#include "bookentry.hh"
#include "treeview.hh"
#include "wulformanager.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/FavoriteManager.h>

class FavoriteHubs:
	public BookEntry,
	public FavoriteManagerListener
{
	public:
		FavoriteHubs();
		~FavoriteHubs();

		// From BookEntry
		GtkWidget *getWidget();

	private:
		// GUI functions
		void updateList_gui();
		void addEntry_gui(const FavoriteHubEntry *entry);
		void removeEntry_gui(const FavoriteHubEntry *entry);
		void showErrorDialog_gui(std::string description);
		void connect_gui(GtkTreeIter iter);
		void popupMenu_gui();

		// GUI callbacks
		static void connect(GtkWidget *widget, gpointer data);
		static void onToggledClicked(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static gboolean onButtonPressed(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onKeyReleased(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void addEntry(GtkWidget *widget, gpointer data);
		static void editEntry(GtkWidget *widget, gpointer data);
		static void removeEntry(GtkWidget *widget, gpointer data);

		// Client functions
		void setConnect_client(FavoriteHubEntry *entry, bool active);
		void addEntry_client(const FavoriteHubEntry entry);
		void editEntry_client(FavoriteHubEntry *oldEntry, const FavoriteHubEntry newEntry);
		void removeEntry_client(FavoriteHubEntry *entry);

		// Client callbacks
		virtual void on(FavoriteManagerListener::FavoriteAdded, const FavoriteHubEntry *entry) throw();
		virtual void on(FavoriteManagerListener::FavoriteRemoved, const FavoriteHubEntry *entry) throw();

		GtkWidget *mainBox;
		GtkDialog *deleteDialog;
		GtkDialog *errorDialog;
		GtkLabel *errorLabel;
		GtkMenu *menu;
		hash_map<std::string, GtkWidget *> button;
		hash_map<std::string, GtkWidget *> dialog;
		hash_map<std::string, GtkWidget *> menuItems;
		TreeView favoriteView;
		GtkListStore *favoriteStore;
		GtkTreeSelection *favoriteSelection;
		GdkEventType previous;
};

#else
class FavoriteHubs;
#endif
