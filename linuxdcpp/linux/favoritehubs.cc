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

FavoriteHubs::FavoriteHubs():
	BookEntry("Favorite Hubs")
{
	FavoriteManager::getInstance()->addListener(this);

	GladeXML *xml = getGladeXML("favoritehubs.glade");

	// Initialize FavoriteHubs widget
	GtkWidget *window = glade_xml_get_widget(xml, "favoriteHubsWindow");
	mainBox = glade_xml_get_widget(xml, "mainBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	deleteDialog = GTK_DIALOG (glade_xml_get_widget(xml, "deleteFavoriteDialog"));
	errorDialog = GTK_DIALOG(glade_xml_get_widget(xml, "errorDialog"));
	errorLabel = GTK_LABEL(glade_xml_get_widget(xml, "errorLabel"));

	// Initialize bottom toolbar
	button["New"] = glade_xml_get_widget(xml, "buttonNew");
	g_signal_connect(G_OBJECT(button["New"]), "clicked", G_CALLBACK(onAddEntry_gui), (gpointer)this);
	button["Properties"] = glade_xml_get_widget(xml, "buttonProperties");
	g_signal_connect(G_OBJECT(button["Properties"]), "clicked", G_CALLBACK(onEditEntry_gui), (gpointer)this);
	button["Remove"] = glade_xml_get_widget(xml, "buttonRemove");
	g_signal_connect(G_OBJECT(button["Remove"]), "clicked", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);
	button["Connect"] = glade_xml_get_widget (xml, "buttonConnect");
	g_signal_connect(G_OBJECT(button["Connect"]), "clicked", G_CALLBACK(onConnect_gui), (gpointer)this);

	// Initialize favorite hubs dialog
	dialog["Window"] = glade_xml_get_widget(xml, "favoriteHubsDialog");
	dialog["Name"] = glade_xml_get_widget(xml, "entryName");
	dialog["Address"] = glade_xml_get_widget(xml, "entryAddress");
	dialog["Description"] = glade_xml_get_widget(xml, "entryDescription");
	dialog["Nick"] = glade_xml_get_widget(xml, "entryNick");
	dialog["Password"] = glade_xml_get_widget(xml, "entryPassword");
	dialog["User description"] = glade_xml_get_widget(xml, "entryUDescription");

	gtk_dialog_set_alternative_button_order(deleteDialog, GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog["Window"]), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	// Create popup menu
	menu = GTK_MENU(gtk_menu_new());
	menuItems["Connect"] = gtk_menu_item_new_with_label("Connect");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItems["Connect"]);
	g_signal_connect(G_OBJECT(menuItems["Connect"]), "activate", G_CALLBACK(onConnect_gui), (gpointer)this);
	menuItems["New"] = gtk_menu_item_new_with_label("New");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItems["New"]);
	g_signal_connect(G_OBJECT(menuItems["New"]), "activate", G_CALLBACK(onAddEntry_gui), (gpointer)this);
	menuItems["Properties"] = gtk_menu_item_new_with_label("Properties");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItems["Properties"]);
	g_signal_connect(G_OBJECT(menuItems["Properties"]), "activate", G_CALLBACK(onEditEntry_gui), (gpointer)this);
	menuItems["Remove"] = gtk_menu_item_new_with_label("Remove");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItems["Remove"]);
	g_signal_connect(G_OBJECT(menuItems["Remove"]), "activate", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);
	gtk_widget_show_all(GTK_WIDGET(menu));

	// Initialize favorite hub list treeview
	GtkTreeView *view = GTK_TREE_VIEW(glade_xml_get_widget(xml, "favoriteView"));
	favoriteView.setView(view, TRUE, "favoritehubs");
	favoriteView.insertColumn("Auto Connect", G_TYPE_BOOLEAN, TreeView::BOOL, 95);
	favoriteView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, 150);
	favoriteView.insertColumn("Description", G_TYPE_STRING, TreeView::STRING, 250);
	favoriteView.insertColumn("Nick", G_TYPE_STRING, TreeView::STRING, 100);
	favoriteView.insertColumn("Password", G_TYPE_STRING, TreeView::STRING, 100);
	favoriteView.insertColumn("Server", G_TYPE_STRING, TreeView::STRING, 175);
	favoriteView.insertColumn("User Description", G_TYPE_STRING, TreeView::STRING, 125);
	favoriteView.insertHiddenColumn("Entry", G_TYPE_POINTER);
	favoriteView.finalize();
	favoriteStore = gtk_list_store_newv(favoriteView.getColCount(), favoriteView.getGTypes());
	gtk_tree_view_set_model(favoriteView.get(), GTK_TREE_MODEL(favoriteStore));
	gtk_tree_view_set_fixed_height_mode(favoriteView.get(), TRUE);
	favoriteSelection = gtk_tree_view_get_selection(favoriteView.get());
	g_object_unref(favoriteStore);

	GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(favoriteView.get(), favoriteView.col("Auto Connect")));
	GtkCellRenderer *renderer = (GtkCellRenderer*)g_list_nth_data(list, 0);
	g_list_free(list);

	g_signal_connect(renderer, "toggled", G_CALLBACK(onToggledClicked_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(favoriteView.get()), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(favoriteView.get()), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(favoriteView.get()), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);

	updateList_gui();
}

FavoriteHubs::~FavoriteHubs()
{
	FavoriteManager::getInstance()->removeListener(this);
	gtk_widget_destroy(GTK_WIDGET(deleteDialog));
	gtk_widget_destroy(GTK_WIDGET(errorDialog));
}

GtkWidget *FavoriteHubs::getWidget() 
{
	return mainBox;
}

void FavoriteHubs::updateList_gui()
{
	const FavoriteHubEntry::List& fl = FavoriteManager::getInstance()->getFavoriteHubs();
	for (FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); i++)
		addEntry_gui(*i);
}

void FavoriteHubs::addEntry_gui(const FavoriteHubEntry *entry)
{
	GtkTreeIter iter;

	gtk_list_store_append(favoriteStore, &iter);
	gtk_list_store_set(favoriteStore, &iter,
		favoriteView.col("Auto Connect"), entry->getConnect(),
		favoriteView.col("Name"), entry->getName().c_str(),
		favoriteView.col("Description"), entry->getDescription().c_str(),
		favoriteView.col("Nick"), entry->getNick().c_str(),
		favoriteView.col("Password"), string(entry->getPassword().size (), '*').c_str(),
		favoriteView.col("Server"), entry->getServer().c_str(),
		favoriteView.col("User Description"), entry->getUserDescription().c_str(),
		favoriteView.col("Entry"), (gpointer)entry,
		-1);
}

void FavoriteHubs::removeEntry_gui(const FavoriteHubEntry *entry)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(favoriteStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (favoriteView.getValue<gpointer, FavoriteHubEntry *>(&iter, "Entry") == entry)
		{
			gtk_list_store_remove(favoriteStore, &iter);
			break;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void FavoriteHubs::showErrorDialog_gui(string description)
{
	gtk_label_set_text(errorLabel, description.c_str());
	gtk_dialog_run(errorDialog);
	gtk_widget_hide(GTK_WIDGET(errorDialog));
}

void FavoriteHubs::connect_gui(GtkTreeIter iter)
{
	WulforManager::get()->addHub_gui(
		favoriteView.getString(&iter, "Server"),
		favoriteView.getString(&iter, "Nick"),
		favoriteView.getString(&iter, "User Description"),
		favoriteView.getString(&iter, "Password"));
}

void FavoriteHubs::setConnect_client(FavoriteHubEntry *entry, bool active)
{
	entry->setConnect(active);
	FavoriteManager::getInstance()->save();
}

void FavoriteHubs::addEntry_client(const FavoriteHubEntry entry)
{
	FavoriteManager::getInstance()->addFavorite(entry);
	FavoriteManager::getInstance()->save();
}

void FavoriteHubs::editEntry_client(FavoriteHubEntry *oldEntry, const FavoriteHubEntry newEntry)
{
	oldEntry->setName(newEntry.getName());
	oldEntry->setServer(newEntry.getServer());
	oldEntry->setDescription(newEntry.getDescription());
	oldEntry->setNick(newEntry.getNick());
	string pass = newEntry.getPassword();
	if (pass != string(pass.size(), '*') || pass.empty())
		oldEntry->setPassword(pass);
	oldEntry->setUserDescription(newEntry.getUserDescription());
	FavoriteManager::getInstance()->save();
}

void FavoriteHubs::removeEntry_client(FavoriteHubEntry *entry)
{
	FavoriteManager::getInstance()->removeFavorite(entry);
}

void FavoriteHubs::onConnect_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs*)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
		fh->connect_gui(iter);
}

void FavoriteHubs::onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs*)data;
  	GtkTreeIter iter;
  	gboolean fixed;
	GtkTreeModel *m = GTK_TREE_MODEL(fh->favoriteStore);

  	if (gtk_tree_model_get_iter_from_string(m, &iter, path))
  	{
  		fixed = fh->favoriteView.getValue<gboolean>(&iter, "Auto Connect");
		fixed = !fixed;
  		gtk_list_store_set(fh->favoriteStore, &iter, fh->favoriteView.col("Auto Connect"), fixed, -1);

		FavoriteHubEntry *entry = fh->favoriteView.getValue<gpointer, FavoriteHubEntry*>(&iter, "Entry");
		typedef Func2<FavoriteHubs, FavoriteHubEntry*, bool> F2;
		F2 *func = new F2(fh, &FavoriteHubs::setConnect_client, entry, fixed);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

gboolean FavoriteHubs::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs*)data;
	fh->previous = event->type;
	return FALSE;
}

gboolean FavoriteHubs::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs*)data;

	if (!gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, NULL))
	{
		gtk_widget_set_sensitive(fh->button["Properties"], FALSE);
		gtk_widget_set_sensitive(fh->button["Remove"], FALSE);
		gtk_widget_set_sensitive(fh->button["Connect"], FALSE);
		return FALSE;
	}
	else
	{
		gtk_widget_set_sensitive(fh->button["Properties"], TRUE);
		gtk_widget_set_sensitive(fh->button["Remove"], TRUE);
		gtk_widget_set_sensitive(fh->button["Connect"], TRUE);
	}

	if (fh->previous == GDK_BUTTON_PRESS && event->button == 3)
		fh->popupMenu_gui();
	else if (fh->previous == GDK_2BUTTON_PRESS && event->button == 1)
		fh->onConnect_gui(widget, data);

	return FALSE;
}

void FavoriteHubs::popupMenu_gui()
{
	if (!gtk_tree_selection_get_selected(favoriteSelection, NULL, NULL))
	{
		gtk_widget_set_sensitive(menuItems["Properties"], FALSE);
		gtk_widget_set_sensitive(menuItems["Remove"], FALSE);
		gtk_widget_set_sensitive(menuItems["Connect"], FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(menuItems["Properties"], TRUE);
		gtk_widget_set_sensitive(menuItems["Remove"], TRUE);
		gtk_widget_set_sensitive(menuItems["Connect"], TRUE);
	}
	gtk_menu_popup(menu, NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

gboolean FavoriteHubs::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;

	if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		fh->onConnect_gui(widget, data);
	else if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		fh->onRemoveEntry_gui(widget, data);
	else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		fh->popupMenu_gui();

	return FALSE;
}

void FavoriteHubs::onAddEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	FavoriteHubEntry entry;
	GtkTreeIter iter;
	string name, server, description, nick, password, userDescription;

	gtk_entry_set_text(GTK_ENTRY(fh->dialog["Name"]), "");
	gtk_entry_set_text(GTK_ENTRY(fh->dialog["Address"]), "");
	gtk_entry_set_text(GTK_ENTRY(fh->dialog["Description"]), "");
	gtk_entry_set_text(GTK_ENTRY(fh->dialog["Nick"]), "");
	gtk_entry_set_text(GTK_ENTRY(fh->dialog["Password"]), "");
	gtk_entry_set_text(GTK_ENTRY(fh->dialog["User description"]), "");	

	gint response = gtk_dialog_run(GTK_DIALOG(fh->dialog["Window"]));
	gtk_widget_hide(fh->dialog["Window"]);

	if (response == GTK_RESPONSE_OK)
	{
		name = gtk_entry_get_text(GTK_ENTRY(fh->dialog["Name"]));
		server = gtk_entry_get_text(GTK_ENTRY(fh->dialog["Address"]));
		description = gtk_entry_get_text(GTK_ENTRY(fh->dialog["Description"]));
		nick = gtk_entry_get_text(GTK_ENTRY(fh->dialog["Nick"]));
		password = gtk_entry_get_text(GTK_ENTRY(fh->dialog["Password"]));
		userDescription = gtk_entry_get_text(GTK_ENTRY(fh->dialog["User description"]));

		if (name.empty() || server.empty())
		{
			fh->showErrorDialog_gui("The name and address fields are required");
			return;
		}

		entry.setConnect(FALSE);
		entry.setName(name);
		entry.setServer(server);
		entry.setDescription(description);
		entry.setNick(nick);
		entry.setPassword(password);
		entry.setUserDescription(userDescription);

		typedef Func1<FavoriteHubs, const FavoriteHubEntry> F1;
		F1 *f1 = new F1(fh, &FavoriteHubs::addEntry_client, entry);
		WulforManager::get()->dispatchClientFunc(f1);
	}
}

void FavoriteHubs::onEditEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;
	string name, server, description, nick, password, userDescription;
	FavoriteHubEntry *oldEntry, newEntry;

	if (!gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
		return;

	name = fh->favoriteView.getString(&iter, "Name");
	server = fh->favoriteView.getString(&iter, "Server");
	description = fh->favoriteView.getString(&iter, "Description");
	nick = fh->favoriteView.getString(&iter, "Nick");
	password = fh->favoriteView.getString(&iter, "Password");
	userDescription = fh->favoriteView.getString(&iter, "User Description");

	gtk_entry_set_text(GTK_ENTRY(fh->dialog["Name"]), name.c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->dialog["Address"]), server.c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->dialog["Description"]), description.c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->dialog["Nick"]), nick.c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->dialog["Password"]), password.c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->dialog["User description"]), userDescription.c_str());

	gint response = gtk_dialog_run(GTK_DIALOG(fh->dialog["Window"]));
	gtk_widget_hide(fh->dialog["Window"]);

	if (response == GTK_RESPONSE_OK)
	{
		name = gtk_entry_get_text(GTK_ENTRY(fh->dialog["Name"]));
		server = gtk_entry_get_text(GTK_ENTRY(fh->dialog["Address"]));
		description = gtk_entry_get_text(GTK_ENTRY(fh->dialog["Description"]));
		nick = gtk_entry_get_text(GTK_ENTRY(fh->dialog["Nick"]));
		password = gtk_entry_get_text(GTK_ENTRY(fh->dialog["Password"]));
		userDescription = gtk_entry_get_text(GTK_ENTRY(fh->dialog["User description"]));

		if (name.empty() || server.empty())
		{
			fh->showErrorDialog_gui("The name and address fields are required");
			return;
		}

		oldEntry = fh->favoriteView.getValue<gpointer, FavoriteHubEntry *>(&iter, "Entry");
		newEntry.setName(name);
		newEntry.setServer(server);
		newEntry.setDescription(description);
		newEntry.setNick(nick);
		newEntry.setPassword(password);
		newEntry.setUserDescription(userDescription);

		gtk_list_store_set(fh->favoriteStore, &iter,
			fh->favoriteView.col("Name"), name.c_str(),
			fh->favoriteView.col("Server"), server.c_str(),
			fh->favoriteView.col("Description"), description.c_str(),
			fh->favoriteView.col("Nick"), nick.c_str(),
			fh->favoriteView.col("Password"), string(password.size(), '*').c_str(),
			fh->favoriteView.col("User Description"), userDescription.c_str(),
			-1);

		typedef Func2<FavoriteHubs, FavoriteHubEntry *, const FavoriteHubEntry> F2;
		F2 *f2 = new F2(fh, &FavoriteHubs::editEntry_client, oldEntry, newEntry);
		WulforManager::get()->dispatchClientFunc(f2);
	}
}

void FavoriteHubs::onRemoveEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs*)data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(fh->favoriteStore);

	if (!gtk_tree_selection_get_selected(fh->favoriteSelection, &m, &iter))
		return;

	if (BOOLSETTING(CONFIRM_HUB_REMOVAL))
	{
		gint response = gtk_dialog_run(fh->deleteDialog);
		gtk_widget_hide(GTK_WIDGET(fh->deleteDialog));
		if (response != GTK_RESPONSE_OK)
			return;
	}

	gtk_widget_set_sensitive(fh->button["Properties"], FALSE);
	gtk_widget_set_sensitive(fh->button["Remove"], FALSE);
	gtk_widget_set_sensitive(fh->button["Connect"], FALSE);

	typedef Func1<FavoriteHubs, FavoriteHubEntry *> F1;
	F1 *f1 = new F1(fh, &FavoriteHubs::removeEntry_client,
		fh->favoriteView.getValue<gpointer, FavoriteHubEntry *>(&iter, "Entry"));
	WulforManager::get()->dispatchClientFunc(f1);
}

void FavoriteHubs::on(FavoriteManagerListener::FavoriteAdded, const FavoriteHubEntry *entry) throw()
{
	typedef Func1<FavoriteHubs, const FavoriteHubEntry *> F1;
	F1 *f1 = new F1(this, &FavoriteHubs::addEntry_gui, entry);
	WulforManager::get()->dispatchGuiFunc(f1);
}

void FavoriteHubs::on(FavoriteManagerListener::FavoriteRemoved, const FavoriteHubEntry *entry) throw()
{
	typedef Func1<FavoriteHubs, const FavoriteHubEntry *> F1;
	F1 *f1 = new F1(this, &FavoriteHubs::removeEntry_gui, entry);
	WulforManager::get()->dispatchGuiFunc(f1);
}
