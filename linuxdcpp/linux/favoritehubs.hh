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

#ifndef WULFOR_FAVORITE_HUBS_HH
#define WULFOR_FAVORITE_HUBS_HH

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <iostream>
#include <ext/hash_map>

#include "bookentry.hh"
#include "treeview.hh"
#include "wulformanager.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/HubManager.h>

class FavoriteHubs:
	public BookEntry,
	public HubManagerListener
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
	void removeEntry_gui(GtkTreeIter iter);
	void showErrorDialog_gui(std::string description);
	void popupMenu_gui(GdkEventButton *event, gpointer data);

	// Client functions
	void setConnect_client(FavoriteHubEntry *entry, bool active);
	void addEntry_client(const FavoriteHubEntry entry);
	void editEntry_client(FavoriteHubEntry *oldEntry, const FavoriteHubEntry newEntry);
	void removeEntry_client(FavoriteHubEntry *entry);

	// GUI callbacks
	static void connect(GtkWidget *widget, gpointer data);
	static void onToggledClicked(GtkCellRendererToggle *cell, gchar *path, gpointer data);
	static gboolean onButtonPressed(GtkWidget *widget, GdkEventButton *event, gpointer data);
	static gboolean onButtonReleased(GtkWidget *widget, GdkEventButton *event, gpointer data);
  	static gboolean onPopupMenu(GtkWidget *widget, gpointer data);
	static void addEntry(GtkWidget *widget, gpointer data);
	static void editEntry(GtkWidget *widget, gpointer data);
	static void removeEntry(GtkWidget *widget, gpointer data);

	// From HubManagerListener
	virtual	void on(HubManagerListener::FavoriteAdded, const FavoriteHubEntry *entry) throw();
	virtual void on(HubManagerListener::FavoriteRemoved, const FavoriteHubEntry *entry) throw();

	GtkWidget *mainBox;
	GtkDialog *deleteDialog;
	GtkDialog *errorDialog;
	GtkLabel *errorLabel;
	GtkMenu *menu;
	hash_map<std::string, GtkWidget *, WulforUtil::HashString> button;
	hash_map<std::string, GtkWidget *, WulforUtil::HashString> dialog;
	hash_map<std::string, GtkWidget *, WulforUtil::HashString> menuItems;
	TreeView favoriteView;
	GtkListStore *favoriteStore;
	GtkTreeSelection *favoriteSelection;
	GdkEventType previous;
};

#else
class FavoriteHubs;
#endif
