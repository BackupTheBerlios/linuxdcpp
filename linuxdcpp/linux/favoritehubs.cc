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

#include "favoritehubs.hh"
#include "wulformanager.hh"

FavoriteHubs::FavoriteHubs():
	BookEntry("Favorite Hubs", "favoritehubs.glade")
{
	FavoriteManager::getInstance()->addListener(this);

	// Configure the dialogs
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("deleteFavoriteDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("favoriteHubsDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	// Initialize favorite hub list treeview
	GtkTreeView *view = GTK_TREE_VIEW(getWidget("favoriteView"));
	favoriteView.setView(view, TRUE, "favoritehubs");
	favoriteView.insertColumn("Auto Connect", G_TYPE_BOOLEAN, TreeView::BOOL, 100);
	favoriteView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, 150);
	favoriteView.insertColumn("Description", G_TYPE_STRING, TreeView::STRING, 250);
	favoriteView.insertColumn("Nick", G_TYPE_STRING, TreeView::STRING, 100);
	favoriteView.insertColumn("Password", G_TYPE_STRING, TreeView::STRING, 100);
	favoriteView.insertColumn("Address", G_TYPE_STRING, TreeView::STRING, 175);
	favoriteView.insertColumn("User Description", G_TYPE_STRING, TreeView::STRING, 125);
	favoriteView.insertHiddenColumn("Hidden Password", G_TYPE_STRING);
	favoriteView.finalize();
	favoriteStore = gtk_list_store_newv(favoriteView.getColCount(), favoriteView.getGTypes());
	gtk_tree_view_set_model(favoriteView.get(), GTK_TREE_MODEL(favoriteStore));
	gtk_tree_view_set_fixed_height_mode(favoriteView.get(), TRUE);
	favoriteSelection = gtk_tree_view_get_selection(favoriteView.get());
	g_object_unref(favoriteStore);
	GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(favoriteView.get(), favoriteView.col("Auto Connect")));
	GtkCellRenderer *renderer = (GtkCellRenderer *)g_list_nth_data(list, 0);
	g_list_free(list);

	// Connect the signals to their callback functions.
	g_signal_connect(getWidget("buttonNew"), "clicked", G_CALLBACK(onAddEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonConnect"), "clicked", G_CALLBACK(onConnect_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonProperties"), "clicked", G_CALLBACK(onEditEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonRemove"), "clicked", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("addMenuItem"), "activate", G_CALLBACK(onAddEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("connectMenuItem"), "activate", G_CALLBACK(onConnect_gui), (gpointer)this);
	g_signal_connect(getWidget("propertiesMenuItem"), "activate", G_CALLBACK(onEditEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("removeMenuItem"), "activate", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);
	g_signal_connect(renderer, "toggled", G_CALLBACK(onToggledClicked_gui), (gpointer)this);
	g_signal_connect(favoriteView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(favoriteView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(favoriteView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);

	WulforManager::get()->dispatchClientFunc(new Func0<FavoriteHubs>(this, &FavoriteHubs::initializeList_client));
}

FavoriteHubs::~FavoriteHubs()
{
	FavoriteManager::getInstance()->removeListener(this);
	gtk_widget_destroy(getWidget("deleteFavoriteDialog"));
	gtk_widget_destroy(getWidget("errorDialog"));
}

void FavoriteHubs::addEntry_gui(StringMap params)
{
	GtkTreeIter iter;
	gtk_list_store_append(favoriteStore, &iter);
	editEntry_gui(params, &iter);
}

void FavoriteHubs::editEntry_gui(StringMap &params, GtkTreeIter *iter)
{
	if (params.find("Hidden Password") != params.end())
	{
		if (!params["Hidden Password"].empty())
			params["Password"] = string(8, '*');
		else
			params["Password"] = "";
	}

	for (StringMap::const_iterator it = params.begin(); it != params.end(); ++it)
	{
		if (it->first == "Auto Connect")
			gtk_list_store_set(favoriteStore, iter, favoriteView.col(it->first), Util::toInt(it->second), -1);
		else
			gtk_list_store_set(favoriteStore, iter, favoriteView.col(it->first), it->second.c_str(), -1);
	}
}

void FavoriteHubs::removeEntry_gui(string address)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(favoriteStore);
	bool valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (favoriteView.getString(&iter, "Address") == address)
		{
			gtk_list_store_remove(favoriteStore, &iter);
			break;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void FavoriteHubs::showErrorDialog_gui(const string &description)
{
	gtk_label_set_text(GTK_LABEL(getWidget("errorLabel")), description.c_str());
	gtk_dialog_run(GTK_DIALOG(getWidget("errorDialog")));
	gtk_widget_hide(getWidget("errorDialog"));
}

void FavoriteHubs::connect_gui(GtkTreeIter *iter)
{
	WulforManager::get()->addHub_gui(
		favoriteView.getString(iter, "Address"),
		favoriteView.getString(iter, "Nick"),
		favoriteView.getString(iter, "User Description"),
		favoriteView.getString(iter, "Hidden Password"));
}

void FavoriteHubs::popupMenu_gui()
{
	if (!gtk_tree_selection_get_selected(favoriteSelection, NULL, NULL))
	{
		gtk_widget_set_sensitive(getWidget("propertiesMenuItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("removeMenuItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("connectMenuItem"), FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(getWidget("propertiesMenuItem"), TRUE);
		gtk_widget_set_sensitive(getWidget("removeMenuItem"), TRUE);
		gtk_widget_set_sensitive(getWidget("connectMenuItem"), TRUE);
	}
	gtk_menu_popup(GTK_MENU(getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

gboolean FavoriteHubs::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	fh->previous = event->type;
	return FALSE;
}

gboolean FavoriteHubs::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (!gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
	{
		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), FALSE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), FALSE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), TRUE);

		if (fh->previous == GDK_BUTTON_PRESS && event->button == 3)
			fh->popupMenu_gui();
		else if (fh->previous == GDK_2BUTTON_PRESS && event->button == 1)
			fh->connect_gui(&iter);
	}

	return FALSE;
}

gboolean FavoriteHubs::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
	{
		if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
			fh->connect_gui(&iter);
		else if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
			fh->onRemoveEntry_gui(widget, data);
		else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
			fh->popupMenu_gui();
	}

	return FALSE;
}

void FavoriteHubs::onAddEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;

	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryName")), "");
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryAddress")), "");
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryDescription")), "");
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryNick")), "");
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryPassword")), "");
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryUDescription")), "");	

	int response = gtk_dialog_run(GTK_DIALOG(fh->getWidget("favoriteHubsDialog")));
	gtk_widget_hide(fh->getWidget("favoriteHubsDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		StringMap params;
		params["Name"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryName")));
		params["Address"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryAddress")));
		params["Description"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryDescription")));
		params["Nick"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryNick")));
		params["Hidden Password"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryPassword")));
		params["User Description"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryUDescription")));

		if (params["Name"].empty() || params["Address"].empty())
		{
			fh->showErrorDialog_gui("The name and address fields are required");
		}
		else
		{
			typedef Func1<FavoriteHubs, StringMap> F1;
			F1 *func = new F1(fh, &FavoriteHubs::addEntry_client, params);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void FavoriteHubs::onEditEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;
	StringMap params;

	if (!gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
		return;

	params["Name"] = fh->favoriteView.getString(&iter, "Name");
	params["Address"] = fh->favoriteView.getString(&iter, "Address");
	params["Description"] = fh->favoriteView.getString(&iter, "Description");
	params["Nick"] = fh->favoriteView.getString(&iter, "Nick");
	params["Hidden Password"] = fh->favoriteView.getString(&iter, "Hidden Password");
	params["User Description"] = fh->favoriteView.getString(&iter, "User Description");

	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryName")), params["Name"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryAddress")), params["Address"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryDescription")), params["Description"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryNick")), params["Nick"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryPassword")), params["Hidden Password"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryUDescription")), params["User Description"].c_str());

	gint response = gtk_dialog_run(GTK_DIALOG(fh->getWidget("favoriteHubsDialog")));
	gtk_widget_hide(fh->getWidget("favoriteHubsDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		params.clear();
		params["Name"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryName")));
		params["Address"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryAddress")));
		params["Description"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryDescription")));
		params["Nick"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryNick")));
		params["Hidden Password"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryPassword")));
		params["User Description"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryUDescription")));

		if (params["Name"].empty() || params["Address"].empty())
		{
			fh->showErrorDialog_gui("The name and address fields are required");
		}
		else
		{
			string address = fh->favoriteView.getString(&iter, "Address");
			fh->editEntry_gui(params, &iter);

			typedef Func2<FavoriteHubs, string, StringMap> F2;
			F2 *func = new F2(fh, &FavoriteHubs::editEntry_client, address, params);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void FavoriteHubs::onRemoveEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
	{
		if (BOOLSETTING(CONFIRM_HUB_REMOVAL))
		{
			gint response = gtk_dialog_run(GTK_DIALOG(fh->getWidget("deleteFavoriteDialog")));
			gtk_widget_hide(fh->getWidget("deleteFavoriteDialog"));
			if (response != GTK_RESPONSE_OK)
				return;
		}

		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), FALSE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), FALSE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), FALSE);

		string address = fh->favoriteView.getString(&iter, "Address");

		typedef Func1<FavoriteHubs, string> F1;
		F1 *func = new F1(fh, &FavoriteHubs::removeEntry_client, address);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void FavoriteHubs::onConnect_gui(GtkButton *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
		fh->connect_gui(&iter);
}

void FavoriteHubs::onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(fh->favoriteStore), &iter, path))
	{
		bool fixed = fh->favoriteView.getValue<gboolean>(&iter, "Auto Connect");
		fixed = !fixed;
		gtk_list_store_set(fh->favoriteStore, &iter, fh->favoriteView.col("Auto Connect"), fixed, -1);

		string address = fh->favoriteView.getString(&iter, "Address");
		typedef Func2<FavoriteHubs, string, bool> F2;
		F2 *func = new F2(fh, &FavoriteHubs::setConnect_client, address, fixed);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void FavoriteHubs::initializeList_client()
{
	StringMap params;
	typedef Func1<FavoriteHubs, StringMap> F1;
	F1 *func;
	const FavoriteHubEntry::List& fl = FavoriteManager::getInstance()->getFavoriteHubs();

	for (FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i)
	{
		params.clear();
		params = getFavHubParams_client(*i);
		func = new F1(this, &FavoriteHubs::addEntry_gui, params);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

StringMap FavoriteHubs::getFavHubParams_client(const FavoriteHubEntry *entry)
{
	StringMap params;

	params["Auto Connect"] = entry->getConnect() ? "1" : "0";
	params["Name"] = entry->getName();
	params["Description"] = entry->getDescription();
	params["Nick"] = entry->getNick();
	if (!entry->getPassword().empty())
		params["Password"] = string(8, '*');
	params["Hidden Password"] = entry->getPassword();
	params["Address"] = entry->getServer();
	params["User Description"] = entry->getUserDescription();

	return params;
}

void FavoriteHubs::addEntry_client(StringMap params)
{
	FavoriteHubEntry entry;
	entry.setConnect(FALSE);
	entry.setName(params["Name"]);
	entry.setServer(params["Address"]);
	entry.setDescription(params["Description"]);
	entry.setNick(params["Nick"]);
	entry.setPassword(params["Hidden Password"]);
	entry.setUserDescription(params["User Description"]);
	FavoriteManager::getInstance()->addFavorite(entry);
}

void FavoriteHubs::editEntry_client(string address, StringMap params)
{
	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address);

	if (entry)
	{
		entry->setName(params["Name"]);
		entry->setServer(params["Address"]);
		entry->setDescription(params["Description"]);
		entry->setNick(params["Nick"]);
		entry->setPassword(params["Hidden Password"]);
		entry->setUserDescription(params["User Description"]);
		FavoriteManager::getInstance()->save();
	}
}

void FavoriteHubs::removeEntry_client(string address)
{
	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address);

	if (entry)
		FavoriteManager::getInstance()->removeFavorite(entry);
}

void FavoriteHubs::setConnect_client(string address, bool active)
{
	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address);

	if (entry)
	{
		entry->setConnect(active);
		FavoriteManager::getInstance()->save();
	}
}

void FavoriteHubs::on(FavoriteManagerListener::FavoriteAdded, const FavoriteHubEntry *entry) throw()
{
	StringMap params = getFavHubParams_client(entry);

	typedef Func1<FavoriteHubs, StringMap> F1;
	F1 *func = new F1(this, &FavoriteHubs::addEntry_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void FavoriteHubs::on(FavoriteManagerListener::FavoriteRemoved, const FavoriteHubEntry *entry) throw()
{
	typedef Func1<FavoriteHubs, string> F1;
	F1 *func = new F1(this, &FavoriteHubs::removeEntry_gui, entry->getServer());
	WulforManager::get()->dispatchGuiFunc(func);
}
