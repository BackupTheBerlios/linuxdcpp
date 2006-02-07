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

#include "favoritehubs.hh"

FavoriteHubs::FavoriteHubs (GCallback closeCallback):
	BookEntry(WulforManager::FAVORITE_HUBS, "", "Favorite Hubs", closeCallback)
{
	HubManager::getInstance()->addListener(this);

	string file = WulforManager::get()->getPath() + "/glade/favoritehubs.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "favoriteHubsWindow");
	mainBox = glade_xml_get_widget(xml, "mainBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	deleteDialog = GTK_DIALOG (glade_xml_get_widget (xml, "deleteFavoriteDialog"));
	errorDialog = GTK_DIALOG(glade_xml_get_widget(xml, "errorDialog"));
	errorLabel = GTK_LABEL(glade_xml_get_widget(xml, "errorLabel"));
	
	button["New"] = glade_xml_get_widget (xml, "buttonNew");
	g_signal_connect(G_OBJECT (button["New"]), "clicked", G_CALLBACK(preNew_gui), (gpointer)this);
	button["Properties"] = glade_xml_get_widget (xml, "buttonProperties");
	g_signal_connect(G_OBJECT (button["Properties"]), "clicked", G_CALLBACK(preEdit_gui), (gpointer)this);
	button["Remove"] = glade_xml_get_widget (xml, "buttonRemove");
	g_signal_connect(G_OBJECT (button["Remove"]), "clicked", G_CALLBACK(remove_gui), (gpointer)this);
	button["Connect"] = glade_xml_get_widget (xml, "buttonConnect");
	g_signal_connect(G_OBJECT (button["Connect"]), "clicked", G_CALLBACK(connect_gui), (gpointer)this);

	dialog["Window"] = glade_xml_get_widget (xml, "favoriteHubsDialog");
	dialog["Name"] = glade_xml_get_widget (xml, "entryName");
	dialog["Address"] = glade_xml_get_widget (xml, "entryAddress");
	dialog["Description"] = glade_xml_get_widget (xml, "entryDescription");
	dialog["Nick"] = glade_xml_get_widget (xml, "entryNick");
	dialog["Password"] = glade_xml_get_widget (xml, "entryPassword");
	dialog["User description"] = glade_xml_get_widget (xml, "entryUDescription");

	menu = GTK_MENU (gtk_menu_new ());
	menuItems["Connect"] = gtk_menu_item_new_with_label ("Connect");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuItems["Connect"]);
	g_signal_connect(G_OBJECT (menuItems["Connect"]), "activate", G_CALLBACK(connect_gui), (gpointer)this);
	menuItems["New"] = gtk_menu_item_new_with_label ("New...");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuItems["New"]);
	g_signal_connect(G_OBJECT (menuItems["New"]), "activate", G_CALLBACK(preNew_gui), (gpointer)this);
	menuItems["Properties"] = gtk_menu_item_new_with_label ("Properites");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuItems["Properties"]);
	g_signal_connect(G_OBJECT (menuItems["Properties"]), "activate", G_CALLBACK(preEdit_gui), (gpointer)this);
	menuItems["Remove"] = gtk_menu_item_new_with_label ("Remove");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuItems["Remove"]);
	g_signal_connect(G_OBJECT (menuItems["Remove"]), "activate", G_CALLBACK(remove_gui), (gpointer)this);
	gtk_widget_show_all (GTK_WIDGET (menu));

	// Initialize favorite hub list treeview
	favoriteView.setView(
		GTK_TREE_VIEW(glade_xml_get_widget(xml, "favoriteView")), 
		true, SettingsManager::FAVORITESFRAME_ORDER,
		SettingsManager::FAVORITESFRAME_WIDTHS);
	favoriteView.insertColumn("Auto Connect", 0, G_TYPE_BOOLEAN, TreeView::BOOL, 95);
	favoriteView.insertColumn("Name", 1, G_TYPE_STRING, TreeView::STRING, 150);
	favoriteView.insertColumn("Description", 2, G_TYPE_STRING, TreeView::STRING, 250);
	favoriteView.insertColumn("Nick", 3, G_TYPE_STRING, TreeView::STRING, 100);
	favoriteView.insertColumn("Password", 4, G_TYPE_STRING, TreeView::STRING, 100);
	favoriteView.insertColumn("Server", 5, G_TYPE_STRING, TreeView::STRING, 175);
	favoriteView.insertColumn("User Description", 6, G_TYPE_STRING, TreeView::STRING, 125);
	favoriteView.insertHiddenColumn("Entry", 7, G_TYPE_POINTER);
	favoriteView.finalize();
	favoriteStore = gtk_list_store_newv(favoriteView.getSize(), favoriteView.getGTypes());
	gtk_tree_view_set_model(favoriteView.get(), GTK_TREE_MODEL(favoriteStore));

	GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(favoriteView.get(), favoriteView.col("Auto Connect")));
	GtkCellRenderer *renderer = (GtkCellRenderer*)list->data;
	g_signal_connect(renderer, "toggled", G_CALLBACK(onToggledClicked_gui), (gpointer)this);
	//gtk_widget_set_events(GTK_WIDGET (favoriteView.get()), GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	g_signal_connect(G_OBJECT(favoriteView.get()), "button_press_event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(favoriteView.get()), "button_release_event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(favoriteView.get()), "popup_menu", G_CALLBACK(onPopupMenu_gui), (gpointer)this);
	gtk_tree_view_set_column_drag_function(favoriteView.get(), NULL, NULL, NULL);

	entrys = 0;
	updateList_gui ();
}

FavoriteHubs::~FavoriteHubs ()
{
	HubManager::getInstance()->removeListener (this);
	gtk_widget_destroy(GTK_WIDGET(deleteDialog));
	gtk_widget_destroy(GTK_WIDGET(errorDialog));
}

GtkWidget *FavoriteHubs::getWidget() 
{
	return mainBox;
}
void FavoriteHubs::setConnect_client (FavoriteHubEntry *e, bool a)
{
	e->setConnect (a);
	HubManager::getInstance ()->save ();
}
void FavoriteHubs::onToggledClicked_gui (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  	GtkTreeIter iter;
  	gboolean fixed;
	FavoriteHubs *f = (FavoriteHubs*)data;
	GtkTreeModel *m = gtk_tree_view_get_model(f->favoriteView.get());
  	gtk_tree_model_get_iter (m, &iter, gtk_tree_path_new_from_string (path_str));
  	gtk_tree_model_get (m, &iter, f->favoriteView.col("Auto Connect"), &fixed, -1);
  	fixed = !fixed;
  	gtk_list_store_set(GTK_LIST_STORE (m), &iter, f->favoriteView.col("Auto Connect"), fixed, -1);

	FavoriteHubEntry *e = f->favoriteView.getValue<gpointer,FavoriteHubEntry*>(&iter, "Entry");
	typedef Func2<FavoriteHubs, FavoriteHubEntry*, bool> F2;
	F2 *func;
	func = new F2 ((FavoriteHubs*)data, &FavoriteHubs::setConnect_client, e, fixed);
	WulforManager::get()->dispatchClientFunc(func);
}
GtkTreeIter FavoriteHubs::addEntry_gui(const FavoriteHubEntry* entry, int pos)
{
	GtkTreeIter row;
	gtk_list_store_insert (favoriteStore, &row, pos);
	gtk_list_store_set(favoriteStore, &row, favoriteView.col("Auto Connect"), entry->getConnect(),
											favoriteView.col("Name"), entry->getName().c_str(),
											favoriteView.col("Description"), entry->getDescription().c_str(),
											favoriteView.col("Nick"), entry->getNick().c_str(),
											favoriteView.col("Password"), string(entry->getPassword().size (), '*').c_str(),
											favoriteView.col("Server"), entry->getServer().c_str(),
											favoriteView.col("User Description"), entry->getUserDescription().c_str(),
											favoriteView.col("Entry"), (gpointer)entry,
											-1);
	entrys++;
	return row;
}
gboolean FavoriteHubs::onButtonPressed_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	((FavoriteHubs*)user_data)->previous = event->type;
	return FALSE;
}
gboolean FavoriteHubs::onButtonReleased_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	FavoriteHubs *f = (FavoriteHubs*)user_data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (f->favoriteStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(f->favoriteView.get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
	{
		gtk_widget_set_sensitive (f->button["Properties"], FALSE);
		gtk_widget_set_sensitive (f->button["Remove"], FALSE);
		gtk_widget_set_sensitive (f->button["Connect"], FALSE);
		return FALSE;
	}
	gtk_widget_set_sensitive (f->button["Properties"], TRUE);
	gtk_widget_set_sensitive (f->button["Remove"], TRUE);
	gtk_widget_set_sensitive (f->button["Connect"], TRUE);	
	
	if (((FavoriteHubs*)user_data)->previous == GDK_BUTTON_PRESS)
    	{
		if (event->button == 3)
			((FavoriteHubs*)user_data)->popup_menu_gui(event, user_data);
	}
	else if (((FavoriteHubs*)user_data)->previous == GDK_2BUTTON_PRESS)
	{
		if (event->button == 1)
			WulforManager::get()->addHub_gui(
				f->favoriteView.getValue<gchar*,string>(&iter, "Server"),
				f->favoriteView.getValue<gchar*,string>(&iter, "Nick"),
				f->favoriteView.getValue<gchar*,string>(&iter, "User Description"),
				f->favoriteView.getValue<gchar*,string>(&iter, "Password"));					
	}

	return FALSE;
}
gboolean FavoriteHubs::onPopupMenu_gui (GtkWidget *widget, gpointer user_data)
{
	((FavoriteHubs*)user_data)->popup_menu_gui(NULL, user_data);
	return TRUE;
}
void FavoriteHubs::popup_menu_gui (GdkEventButton *event, gpointer user_data)
{
    gtk_menu_popup(menu, NULL, NULL, NULL, NULL,
                   (event != NULL) ? 1 : 0,
                   gdk_event_get_time((GdkEvent*)event));
}
void FavoriteHubs::remove_client (FavoriteHubEntry *e)
{
	HubManager::getInstance ()->removeFavorite (e);
}
void FavoriteHubs::remove_gui (GtkWidget *widget, gpointer data)
{
	FavoriteHubs *f = ((FavoriteHubs*)data);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (f->favoriteStore);
	selection = gtk_tree_view_get_selection(f->favoriteView.get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	if (BOOLSETTING (CONFIRM_HUB_REMOVAL))
	{
		gint response = gtk_dialog_run (f->deleteDialog);
		gtk_widget_hide (GTK_WIDGET (f->deleteDialog));
		if (response != GTK_RESPONSE_OK)
			return;
	}
		
	WulforManager::get()->dispatchClientFunc (new Func1<FavoriteHubs, FavoriteHubEntry*> (
		f, 
		&FavoriteHubs::remove_client, 
		f->favoriteView.getValue<gpointer,FavoriteHubEntry*>(&iter, "Entry")));
	
	gtk_widget_set_sensitive (f->button["Properties"], FALSE);
	gtk_widget_set_sensitive (f->button["Remove"], FALSE);
	gtk_widget_set_sensitive (f->button["Connect"], FALSE);
}

void FavoriteHubs::preNew_gui (GtkWidget *widget, gpointer data)
{
	((FavoriteHubs*)data)->addDialog_gui (false);
}
void FavoriteHubs::preEdit_gui (GtkWidget *widget, gpointer data)
{
	FavoriteHubs *f = ((FavoriteHubs*)data);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (f->favoriteStore);
	selection = gtk_tree_view_get_selection(f->favoriteView.get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
	f->addDialog_gui(true,
		f->favoriteView.getValue<gchar*,string>(&iter, "Name"),
		f->favoriteView.getValue<gchar*,string>(&iter, "Server"),
		f->favoriteView.getValue<gchar*,string>(&iter, "Description"),
		f->favoriteView.getValue<gchar*,string>(&iter, "Nick"),
		f->favoriteView.getValue<gchar*,string>(&iter, "Password"),
		f->favoriteView.getValue<gchar*,string>(&iter, "User Description"));
}
void FavoriteHubs::connect_gui (GtkWidget *widget, gpointer data)
{
	FavoriteHubs *f = ((FavoriteHubs*)data);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (f->favoriteStore);
	selection = gtk_tree_view_get_selection(f->favoriteView.get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	WulforManager::get()->addHub_gui(
		f->favoriteView.getValue<gchar*,string>(&iter, "Server"),
		f->favoriteView.getValue<gchar*,string>(&iter, "Nick"),
		f->favoriteView.getValue<gchar*,string>(&iter, "User Description"),
		f->favoriteView.getValue<gchar*,string>(&iter, "Password"));
}
void FavoriteHubs::edit_client (FavoriteHubEntry *e)
{
	e->setName (gtk_entry_get_text (GTK_ENTRY (dialog["Name"])));
	e->setServer (gtk_entry_get_text (GTK_ENTRY (dialog["Address"])));
	e->setDescription (gtk_entry_get_text (GTK_ENTRY (dialog["Description"])));
	e->setNick (gtk_entry_get_text (GTK_ENTRY (dialog["Nick"])));
	e->setPassword (gtk_entry_get_text (GTK_ENTRY (dialog["Password"])));
	e->setUserDescription (gtk_entry_get_text (GTK_ENTRY (dialog["User description"])));
	HubManager::getInstance ()->save ();
}
void FavoriteHubs::add_client (FavoriteHubEntry e)
{
	HubManager::getInstance()->addFavorite (e);
	HubManager::getInstance ()->save ();
}

void FavoriteHubs::showErrorDialog(string description)
{
	gtk_label_set_text(errorLabel, description.c_str());
	gtk_dialog_run(errorDialog);
	gtk_widget_hide(GTK_WIDGET(errorDialog));
}

void FavoriteHubs::addDialog_gui (bool edit, string uname, string uaddress, string udesc, string unick, string upassword, string uuserdesc)
{
	GtkDialog *window = GTK_DIALOG (dialog["Window"]);
	if (edit)
	{
		gtk_entry_set_text (GTK_ENTRY(dialog["Name"]), uname.c_str ());
		gtk_entry_set_text (GTK_ENTRY(dialog["Address"]), uaddress.c_str ());
		gtk_entry_set_text (GTK_ENTRY(dialog["Description"]), udesc.c_str ());
		gtk_entry_set_text (GTK_ENTRY(dialog["Nick"]), unick.c_str ());
		gtk_entry_set_text (GTK_ENTRY(dialog["Password"]), upassword.c_str ());
		gtk_entry_set_text (GTK_ENTRY(dialog["User description"]), uuserdesc.c_str ());
	}
	else
	{
		gtk_entry_set_text (GTK_ENTRY(dialog["Name"]), "");
		gtk_entry_set_text (GTK_ENTRY(dialog["Address"]), "");
		gtk_entry_set_text (GTK_ENTRY(dialog["Description"]), "");
		gtk_entry_set_text (GTK_ENTRY(dialog["Nick"]), "");
		gtk_entry_set_text (GTK_ENTRY(dialog["Password"]), "");
		gtk_entry_set_text (GTK_ENTRY(dialog["User description"]), "");	
	}
	gint response = gtk_dialog_run (window);
	gtk_widget_hide (dialog["Window"]);
	if (response == GTK_RESPONSE_OK)
	{
		string name = gtk_entry_get_text(GTK_ENTRY(dialog["Name"]));
		string server = gtk_entry_get_text(GTK_ENTRY(dialog["Address"]));
		string description = gtk_entry_get_text(GTK_ENTRY(dialog["Description"]));
		string nick = gtk_entry_get_text(GTK_ENTRY(dialog["Nick"]));
		string password = gtk_entry_get_text(GTK_ENTRY(dialog["Password"]));
		string userDescription = gtk_entry_get_text(GTK_ENTRY(dialog["User description"]));

		if (name.empty() || server.empty())
		{
			showErrorDialog("The name and address fields are required");
			return;
		}

		if (!edit)
		{
			FavoriteHubEntry e;
			e.setName(name);
			e.setServer(server);
			e.setDescription(description);
			e.setNick(nick);
			e.setPassword(password);
			e.setUserDescription(userDescription);
			WulforManager::get()->dispatchClientFunc(new Func1<FavoriteHubs, FavoriteHubEntry> (this, &FavoriteHubs::add_client, e));
		}
		else
		{
			GtkTreeSelection *selection;
			GtkTreeIter iter;
			GtkTreeModel *m = GTK_TREE_MODEL(favoriteStore);
			selection = gtk_tree_view_get_selection(favoriteView.get());

			if (!gtk_tree_selection_get_selected(selection, &m, &iter))
				return;
			FavoriteHubEntry::List &fh = HubManager::getInstance()->getFavoriteHubs();
			for (int i = 0; i < favoriteView.getCount(); i++)
			{
				if (fh[i] == favoriteView.getValue<gpointer,FavoriteHubEntry*>(&iter, "Entry"))
				{
					WulforManager::get()->dispatchClientFunc (new Func1<FavoriteHubs, FavoriteHubEntry*> (this, &FavoriteHubs::edit_client, fh[i]));
					gtk_list_store_set(favoriteStore, &iter, 	favoriteView.col("Name"), name.c_str(),
																favoriteView.col("Server"), server.c_str(),
																favoriteView.col("Description"), description.c_str(),
																favoriteView.col("Nick"), nick.c_str(),
																favoriteView.col("Password"), string(password.size(), '*').c_str(),
																favoriteView.col("User Description"), userDescription.c_str(),
																-1);
     				break;
				}
			}
		}
	}
}
void FavoriteHubs::on(HubManagerListener::FavoriteAdded, const FavoriteHubEntry *entry) throw()
{
	addEntry_gui(entry, entrys);
}
void FavoriteHubs::on(HubManagerListener::FavoriteRemoved, const FavoriteHubEntry *entry) throw()
{
	GtkTreeIter it;
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL (favoriteStore), &it))
		return;

	while (1)
	{
		if (favoriteView.getValue<gpointer,FavoriteHubEntry*>(&it, "Entry") == entry)
		{
			gtk_list_store_remove (favoriteStore, &it);
			entrys--;
			break;
		}
		if (!gtk_tree_model_iter_next(GTK_TREE_MODEL (favoriteStore), &it))
			return;
	}
}
