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
#include <sstream>

#include "bookentry.hh"
#include "treeview.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/HubManager.h>

using namespace std;

class FavoriteHubs : 	public BookEntry,
						public HubManagerListener
{
public:
	FavoriteHubs (GCallback closeCallback);
	~FavoriteHubs ();

	// From BookEntry
	GtkWidget *getWidget();

	virtual	void on(HubManagerListener::FavoriteAdded, const FavoriteHubEntry *entry) throw();
	virtual void on(HubManagerListener::FavoriteRemoved, const FavoriteHubEntry *entry) throw();

private:
	GtkWidget *mainBox;
	GtkDialog *deleteDialog;
	GtkDialog *errorDialog;
	GtkLabel *errorLabel;
	GtkMenu *menu;
	std::map<string,GtkWidget*> button;
	std::map<string,GtkWidget*> dialog;
	std::map<string,GtkWidget*> menuItems;
	TreeView favoriteView;
	GtkListStore *favoriteStore;

	int entrys;
	GdkEventType previous;

	void updateList_gui ()
	{
		const FavoriteHubEntry::List& fl = HubManager::getInstance ()->getFavoriteHubs();
		for(FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i)
			addEntry_gui (*i, entrys);
	}	
	GtkTreeIter addEntry_gui(const FavoriteHubEntry* entry, int pos);
	void addDialog_gui (bool edit, const string uname="", const string uaddress="", const string udesc="", const string unick="", const string upassword="", const string uuserdesc="");
	static void preNew_gui (GtkWidget *widget, gpointer data);
	static void preEdit_gui (GtkWidget *widget, gpointer data);
	void edit_client (FavoriteHubEntry *e);
	void add_client (FavoriteHubEntry e);
	void remove_client (FavoriteHubEntry *e);
	static void remove_gui (GtkWidget *widget, gpointer data);
	static void connect_gui (GtkWidget *widget, gpointer data);
	void showErrorDialog(string description);

	static void onToggledClicked_gui (GtkCellRendererToggle *cell, gchar *path_str, gpointer data);
	void setConnect_client (FavoriteHubEntry *e, bool a);
	static gboolean onButtonPressed_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
	static gboolean onButtonReleased_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
  	static gboolean onPopupMenu_gui (GtkWidget *widget, gpointer user_data);
	void popup_menu_gui (GdkEventButton *event, gpointer user_data);
};
#else
class FavoriteHubs;
#endif
