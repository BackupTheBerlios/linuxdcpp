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
#include "wulformanager.hh"
#include <iostream>
#include <sstream>

int FavoriteHubs::columnSize[] = { 95, 150, 250, 100, 100, 175, 125 };
using namespace std;
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
	
	GtkTreeView *view = GTK_TREE_VIEW (glade_xml_get_widget (xml, "favoriteView"));
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

	favoriteStore = gtk_list_store_new (8, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	gtk_tree_view_set_model(view, GTK_TREE_MODEL (favoriteStore));
	//gtk_widget_set_events (GTK_WIDGET (view), GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	g_signal_connect(G_OBJECT (view), "button_press_event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
    	g_signal_connect(G_OBJECT (view), "button_release_event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
    	g_signal_connect(G_OBJECT (view), "popup_menu", G_CALLBACK(onPopupMenu_gui), (gpointer)this);
	favoriteView = new TreeViewFactory (view);

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

	favoriteView->addColumn_gui (COLUMN_AUTOCONNECT, "Auto connect", TreeViewFactory::BOOL, columnSize[COLUMN_AUTOCONNECT]);
	favoriteView->addColumn_gui (COLUMN_NAME, "Name", TreeViewFactory::STRING, columnSize[COLUMN_NAME]);
	favoriteView->addColumn_gui (COLUMN_DESCRIPTION, "Description", TreeViewFactory::STRING, columnSize[COLUMN_DESCRIPTION]);
	favoriteView->addColumn_gui (COLUMN_NICK, "Nick", TreeViewFactory::STRING, columnSize[COLUMN_NICK]);
	favoriteView->addColumn_gui (COLUMN_SERVER, "Server", TreeViewFactory::STRING, columnSize[COLUMN_SERVER]);
	favoriteView->addColumn_gui (COLUMN_PASSWORD, "Password", TreeViewFactory::STRING, columnSize[COLUMN_PASSWORD]);
	favoriteView->addColumn_gui (COLUMN_USERDESCRIPTION, "User description", TreeViewFactory::STRING, columnSize[COLUMN_USERDESCRIPTION]);

	GList *list = gtk_tree_view_column_get_cell_renderers (gtk_tree_view_get_column(view, COLUMN_AUTOCONNECT));
	GtkCellRenderer *renderer = (GtkCellRenderer*)list->data;
	g_signal_connect (renderer, "toggled", G_CALLBACK (onToggledClicked_gui), (gpointer)this);
	
	entrys = 0;
	updateList_gui ();
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
	GtkTreeModel *m = gtk_tree_view_get_model (((FavoriteHubs*)data)->favoriteView->get ());
  	gtk_tree_model_get_iter (m, &iter, gtk_tree_path_new_from_string (path_str));
  	gtk_tree_model_get (m, &iter, FavoriteHubs::COLUMN_AUTOCONNECT, &fixed, -1);
  	fixed = !fixed;
  	gtk_list_store_set (GTK_LIST_STORE (m), &iter, FavoriteHubs::COLUMN_AUTOCONNECT, fixed, -1);

	FavoriteHubEntry *e = TreeViewFactory::getValue<gpointer,FavoriteHubEntry*>(m, &iter, COLUMN_ENTRY);
	typedef Func2<FavoriteHubs, FavoriteHubEntry*, bool> F2;
	F2 *func;
	func = new F2 ((FavoriteHubs*)data, &FavoriteHubs::setConnect_client, e, fixed);
	WulforManager::get()->dispatchClientFunc(func);
}
GtkTreeIter FavoriteHubs::addEntry_gui(const FavoriteHubEntry* entry, int pos)
{
	GtkTreeIter row;
	gtk_list_store_insert (favoriteStore, &row, pos);
	gtk_list_store_set (favoriteStore, &row, 	COLUMN_AUTOCONNECT, entry->getConnect (),
																	 	COLUMN_NAME, entry->getName ().c_str (),
																		COLUMN_DESCRIPTION, entry->getDescription ().c_str (),
																		COLUMN_NICK, entry->getNick ().c_str (),
																		COLUMN_PASSWORD, string (entry->getPassword ().size (), '*').c_str (),
																		COLUMN_SERVER, entry->getServer ().c_str (),
																		COLUMN_USERDESCRIPTION, entry->getUserDescription ().c_str (),
																		COLUMN_ENTRY, (gpointer)entry,
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
	GtkTreeSelection *selection = gtk_tree_view_get_selection(f->favoriteView->get ());

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
			WulforManager::get()->addHub_gui(	TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_SERVER),
											TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_NICK),
											TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_USERDESCRIPTION),
											TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_PASSWORD));					
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
	selection = gtk_tree_view_get_selection(f->favoriteView->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	if (BOOLSETTING (CONFIRM_HUB_REMOVAL))
	{
		gint response = gtk_dialog_run (f->deleteDialog);
		gtk_widget_hide (GTK_WIDGET (f->deleteDialog));
		if (response != GTK_RESPONSE_OK)
			return;
	}
		
	WulforManager::get ()->dispatchClientFunc (new Func1<FavoriteHubs, FavoriteHubEntry*> (f, &FavoriteHubs::remove_client, TreeViewFactory::getValue<gpointer,FavoriteHubEntry*>(m, &iter, COLUMN_ENTRY)));
	
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
	selection = gtk_tree_view_get_selection(f->favoriteView->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
	f->addDialog_gui (true, 	TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_NAME),
								TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_SERVER),
								TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_DESCRIPTION),
								TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_NICK),
								TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_PASSWORD),
								TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_USERDESCRIPTION));
}
void FavoriteHubs::connect_gui (GtkWidget *widget, gpointer data)
{
	FavoriteHubs *f = ((FavoriteHubs*)data);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (f->favoriteStore);
	selection = gtk_tree_view_get_selection(f->favoriteView->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	WulforManager::get()->addHub_gui(	TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_SERVER),
																TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_NICK),
																TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_USERDESCRIPTION),
																TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_PASSWORD));
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
			selection = gtk_tree_view_get_selection(favoriteView->get());

			if (!gtk_tree_selection_get_selected(selection, &m, &iter))
				return;
			FavoriteHubEntry::List &fh = HubManager::getInstance()->getFavoriteHubs();
			for (int i = 0; i < TreeViewFactory::getCount(m); i++)
			{
				if (fh[i] == TreeViewFactory::getValue<gpointer,FavoriteHubEntry*>(m, &iter, COLUMN_ENTRY))
				{
					WulforManager::get()->dispatchClientFunc (new Func1<FavoriteHubs, FavoriteHubEntry*> (this, &FavoriteHubs::edit_client, fh[i]));
					gtk_list_store_set(favoriteStore, &iter, 	COLUMN_NAME, name.c_str(),
																COLUMN_SERVER, server.c_str(),
																COLUMN_DESCRIPTION, description.c_str(),
																COLUMN_NICK, nick.c_str(),
																COLUMN_PASSWORD, string(password.size(), '*').c_str(),
																COLUMN_USERDESCRIPTION, userDescription.c_str(),
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
		if (TreeViewFactory::getValue<gpointer,FavoriteHubEntry*>(GTK_TREE_MODEL (favoriteStore), &it, COLUMN_ENTRY) == entry)
		{
			gtk_list_store_remove (favoriteStore, &it);
			entrys--;
			break;
		}
		if (!gtk_tree_model_iter_next(GTK_TREE_MODEL (favoriteStore), &it))
			return;
	}
}
