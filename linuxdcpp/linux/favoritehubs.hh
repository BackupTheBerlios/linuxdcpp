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

	private:
		// GUI functions
		void updateList_gui();
		void addEntry_gui(const FavoriteHubEntry *entry);
		void removeEntry_gui(const FavoriteHubEntry *entry);
		void showErrorDialog_gui(std::string description);
		void connect_gui(GtkTreeIter iter);
		void popupMenu_gui();

		// GUI callbacks
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onAddEntry_gui(GtkWidget *widget, gpointer data);
		static void onEditEntry_gui(GtkWidget *widget, gpointer data);
		static void onRemoveEntry_gui(GtkWidget *widget, gpointer data);
		static void onConnect_gui(GtkButton *widget, gpointer data);
		static void onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);

		// Client functions
		void addEntry_client(const FavoriteHubEntry entry);
		void editEntry_client(FavoriteHubEntry *oldEntry, const FavoriteHubEntry newEntry);
		void removeEntry_client(FavoriteHubEntry *entry);
		void setConnect_client(FavoriteHubEntry *entry, bool active);

		// Client callbacks
		virtual void on(FavoriteManagerListener::FavoriteAdded, const FavoriteHubEntry *entry) throw();
		virtual void on(FavoriteManagerListener::FavoriteRemoved, const FavoriteHubEntry *entry) throw();

		TreeView favoriteView;
		GtkListStore *favoriteStore;
		GtkTreeSelection *favoriteSelection;
		GdkEventType previous;
};

#else
class FavoriteHubs;
#endif
