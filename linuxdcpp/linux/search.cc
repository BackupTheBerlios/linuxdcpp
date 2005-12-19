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

#include "search.hh"
#include "wulformanager.hh"
#include <iostream>
#include <client/StringTokenizer.h>

/* Temporay, move when Util is added. */
string linuxSeparator (const string &ps)
{
	string str = ps;
	for (string::iterator it=str.begin (); it != str.end (); it++)
		if ((*it) == '\\')
			(*it) = '/';
	return str;
}
/* Temporay, move when Util is added. */
string windowsSeparator (const string &ps)
{
	string str = ps;
	for (string::iterator it=str.begin (); it != str.end (); it++)
		if ((*it) == '/')
			(*it) = '\\';
	return str;
}

int Search::columnSize[] = { 200, 100, 50, 80, 100, 40, 70, 150, 80, 100, 125 };
TStringList Search::lastSearches;
string Search::lastDir;

Search::~Search ()
{
	ClientManager::getInstance()->removeListener(this);
	SearchManager::getInstance()->removeListener(this);
	pthread_mutex_destroy (&searchLock);
}

GtkWidget *Search::getWidget() 
{
	return mainBox;
}
string getTextFromMenu (GtkMenuItem *item)
{
	if (GTK_BIN (item)->child)
    	{
      		GtkWidget *child = GTK_BIN (item)->child;
		gchar *text;
		if (GTK_IS_LABEL (child))
    		{
			gtk_label_get (GTK_LABEL (child), &text);
			return string (text);
		}
	}
	return "";
}

void Search::putValue (const string &str, int64_t size, SearchManager::SizeModes mode, SearchManager::TypeModes type)
{
	gtk_entry_set_text (GTK_ENTRY (GTK_BIN (searchItems["Search"])->child), str.c_str ());
	gtk_entry_set_text (GTK_ENTRY (searchItems["Size"]), Util::toString (size).c_str ());
	gtk_combo_box_set_active(GTK_COMBO_BOX (searchItems["Diffrence"]), (int)mode);
	gtk_combo_box_set_active(GTK_COMBO_BOX (searchItems["File type"]), (int)type);
	
	search_gui ();
}

Search::Search (GCallback closeCallback) : BookEntry (WulforManager::SEARCH, "", "Search", closeCallback)
{
	ClientManager::getInstance()->addListener(this);
	SearchManager::getInstance()->addListener(this);
	
	string file = WulforManager::get()->getPath() + "/glade/search.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "searchWindow");
	mainBox = glade_xml_get_widget(xml, "mainBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);
	
	searchItems["Search"] = glade_xml_get_widget(xml, "comboboxentrySearch");
	g_signal_connect (G_OBJECT (GTK_BIN (searchItems["Search"])->child), "activate", G_CALLBACK (onSearchEntryDown_gui), (gpointer)this);
	searchItems["Diffrence"] = glade_xml_get_widget(xml, "comboboxSize");
	searchItems["Size"] = glade_xml_get_widget(xml, "entrySize");
	searchItems["Unit"] = glade_xml_get_widget(xml, "comboboxUnit");
	searchItems["File type"] = glade_xml_get_widget(xml, "comboboxFile");
	searchItems["Slots"] = glade_xml_get_widget(xml, "checkbuttonSlots");
	GtkTreeView *hubView = GTK_TREE_VIEW (glade_xml_get_widget(xml, "treeviewHubs"));
	searchItems["Button"] = glade_xml_get_widget(xml, "buttonSearch");
	g_signal_connect (G_OBJECT (searchItems["Button"]), "clicked", G_CALLBACK (onSearchButtonClicked_gui), (gpointer)this);
	GtkTreeView *resultView = GTK_TREE_VIEW (glade_xml_get_widget(xml, "treeviewResult"));
	searchItems["Status1"] = glade_xml_get_widget(xml, "statusbar1");
	searchItems["Status2"] = glade_xml_get_widget(xml, "statusbar2");
	searchItems["Status3"] = glade_xml_get_widget(xml, "statusbar3");
	
	gtk_combo_box_set_active(GTK_COMBO_BOX (searchItems["Diffrence"]), 1);
	gtk_combo_box_set_active(GTK_COMBO_BOX (searchItems["Unit"]), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX (searchItems["File type"]), 0);
	
	hubStore = gtk_list_store_new (3, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_POINTER);
	gtk_tree_view_set_model(hubView, GTK_TREE_MODEL (hubStore));
	hubs = new TreeViewFactory (hubView);
	
	hubs->addColumn_gui (HUB_SEARCH, "", TreeViewFactory::BOOL, -1);
	hubs->addColumn_gui (HUB_NAME, "", TreeViewFactory::STRING, -1);
	
	resultStore = gtk_list_store_new (13, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
									G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT64);
	gtk_tree_view_set_model (resultView, GTK_TREE_MODEL (resultStore));
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(resultView), GTK_SELECTION_MULTIPLE);
	g_signal_connect(G_OBJECT (resultView), "button_press_event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(G_OBJECT (resultView), "popup_menu", G_CALLBACK(onPopupMenu_gui), (gpointer)this);
	result = new TreeViewFactory (resultView);
	result->addColumn_gui (RESULT_FILE, "File", TreeViewFactory::STRING, columnSize[RESULT_FILE]);
	result->addColumn_gui (RESULT_NICK, "Nick", TreeViewFactory::STRING, columnSize[RESULT_NICK]);
	result->addColumn_gui (RESULT_TYPE, "Type", TreeViewFactory::STRING, columnSize[RESULT_TYPE]);
	result->addColumn_gui (RESULT_FILESIZE, "Size", TreeViewFactory::STRING, columnSize[RESULT_FILESIZE]);
	result->addColumn_gui (RESULT_PATH, "Path", TreeViewFactory::STRING, columnSize[RESULT_PATH]);
	result->addColumn_gui (RESULT_SLOTS, "Slots", TreeViewFactory::STRING, columnSize[RESULT_SLOTS]);
	result->addColumn_gui (RESULT_CONNECTION, "Connection", TreeViewFactory::STRING, columnSize[RESULT_CONNECTION]);
	result->addColumn_gui (RESULT_HUB, "Hub", TreeViewFactory::STRING, columnSize[RESULT_HUB]);
	result->addColumn_gui (RESULT_EXACT_SIZE, "Exact size", TreeViewFactory::STRING, columnSize[RESULT_EXACT_SIZE]);
	result->addColumn_gui (RESULT_IP, "IP", TreeViewFactory::STRING, columnSize[RESULT_IP]);
	result->addColumn_gui (RESULT_TTH, "TTH", TreeViewFactory::STRING, columnSize[RESULT_TTH]);
	result->setSortColumn_gui(RESULT_FILESIZE, RESULT_SIZE);
	result->setSortColumn_gui(RESULT_EXACT_SIZE, RESULT_SIZE);

	GList *list = gtk_tree_view_column_get_cell_renderers (gtk_tree_view_get_column(hubView, HUB_SEARCH));
	GtkCellRenderer *renderer = (GtkCellRenderer*)list->data;
	g_signal_connect (renderer, "toggled", G_CALLBACK (onToggledClicked_gui), (gpointer)this);	
	
	mainMenu = GTK_MENU (gtk_menu_new ());
	downloadMenu = GTK_MENU (gtk_menu_new ());
	downloadDirMenu = GTK_MENU (gtk_menu_new ());
	menuItems["Download"] = gtk_menu_item_new_with_label ("Download");
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), menuItems["Download"]);
	g_signal_connect (G_OBJECT (menuItems["Download"]), "activate", G_CALLBACK (onDownloadClicked_gui), (gpointer)this);
	
	menuItems["DownloadTo"] = gtk_menu_item_new_with_label ("Download to...");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuItems["DownloadTo"]), GTK_WIDGET (downloadMenu));
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), menuItems["DownloadTo"]);
	
	menuItems["DownloadWholeDir"] = gtk_menu_item_new_with_label ("Download whole directory");
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), menuItems["DownloadWholeDir"]);
	g_signal_connect (G_OBJECT (menuItems["DownloadWholeDir"]), "activate", G_CALLBACK (onDownloadDirClicked_gui), (gpointer)this);
	
	menuItems["DownloadWholeDirTo"] = gtk_menu_item_new_with_label ("Download whole directory to...");
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), menuItems["DownloadWholeDirTo"]);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuItems["DownloadWholeDirTo"]), GTK_WIDGET (downloadDirMenu));
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), gtk_separator_menu_item_new ());
	menuItems["SearchByTTH"] = gtk_menu_item_new_with_label ("Search by TTH");
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), menuItems["SearchByTTH"]);
	g_signal_connect (G_OBJECT (menuItems["SearchByTTH"]), "activate", G_CALLBACK (onSearchForTTHClicked_gui), (gpointer)this);
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), gtk_separator_menu_item_new ());
	menuItems["GetFileList"] = gtk_menu_item_new_with_label ("Get file list");
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), menuItems["GetFileList"]);
	g_signal_connect (G_OBJECT (menuItems["GetFileList"]), "activate", G_CALLBACK (onGetFileListClicked_gui), (gpointer)this);
	menuItems["MatchQueue"] = gtk_menu_item_new_with_label ("Match queue");
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), menuItems["MatchQueue"]);
	g_signal_connect (G_OBJECT (menuItems["MatchQueue"]), "activate", G_CALLBACK (onMatchQueueClicked_gui), (gpointer)this);
	menuItems["SendPrivateMessage"] = gtk_menu_item_new_with_label ("Send private message");
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), menuItems["SendPrivateMessage"]);
	g_signal_connect (G_OBJECT (menuItems["SendPrivateMessage"]), "activate", G_CALLBACK (onPrivateMessageClicked_gui), (gpointer)this);
	menuItems["AddToFavorites"] = gtk_menu_item_new_with_label ("Add to favorites");
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), menuItems["AddToFavorites"]);
	g_signal_connect (G_OBJECT (menuItems["AddToFavorites"]), "activate", G_CALLBACK (onAddFavoriteUserClicked_gui), (gpointer)this);
	menuItems["GrantExtraSlot"] = gtk_menu_item_new_with_label ("Grant extra slot");
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), menuItems["GrantExtraSlot"]);
	g_signal_connect (G_OBJECT (menuItems["GrantExtraSlot"]), "activate", G_CALLBACK (onGrantExtraSlotClicked_gui), (gpointer)this);
	menuItems["RemoveUserFromQueue"] = gtk_menu_item_new_with_label ("Remove user from queue");
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), menuItems["RemoveUserFromQueue"]);
	g_signal_connect (G_OBJECT (menuItems["RemoveUserFromQueue"]), "activate", G_CALLBACK (onRemoveUserFromQueueClicked_gui), (gpointer)this);
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), gtk_separator_menu_item_new ());
	menuItems["Remove"] = gtk_menu_item_new_with_label ("Remove");
	gtk_menu_shell_append (GTK_MENU_SHELL (mainMenu), menuItems["Remove"]);
	g_signal_connect (G_OBJECT (menuItems["Remove"]), "activate", G_CALLBACK (onRemoveClicked_gui), (gpointer)this);
	gtk_widget_show_all (GTK_WIDGET (mainMenu));
			
	lastSearch = 0;
	listItems = 0;
	
	pthread_mutex_init (&searchLock, NULL);
	
	for (TStringList::iterator it = lastSearches.begin (); it != lastSearches.end (); it++)
	{
		listItems++;
		GtkTreeIter iter;
		GtkTreeModel *m = gtk_combo_box_get_model (GTK_COMBO_BOX (searchItems["Search"]));
		gtk_list_store_append (GTK_LIST_STORE (m), &iter);
		gtk_list_store_set (GTK_LIST_STORE (m), &iter, 0, (*it).c_str (), -1);
	}
	
	initHubs_gui ();
}

void Search::buildDownloadMenu_gui (int menu)
{	
	pthread_mutex_lock (&searchLock);
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (resultStore);
	selection = gtk_tree_view_get_selection(result->get ());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<GtkTreeIter> iter;
	GtkTreeIter tmpiter;
	int count=0;
	
	if (!tmp)
		return;
	
	while (1)
	{
		if (gtk_tree_model_get_iter (m, &tmpiter, (GtkTreePath*)tmp->data))
		{
			iter.push_back (tmpiter);
			count++;	
		}
		
		tmp = g_list_next (tmp);
		if (!tmp)
			break;
	}
	
	if (count == 1)
	{
		SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter[0], RESULT_INFO);
		if (si->result->getTTH ())
			gtk_widget_set_sensitive (menuItems["SearchByTTH"], true);
		else
			gtk_widget_set_sensitive (menuItems["SearchByTTH"], false);
			
		gtk_widget_set_sensitive (menuItems["GetFileList"], true);
		gtk_widget_set_sensitive (menuItems["SendPrivateMessage"], true);
		gtk_widget_set_sensitive (menuItems["AddToFavorites"], true);
		gtk_widget_set_sensitive (menuItems["MatchQueue"], true);
		gtk_widget_set_sensitive (menuItems["GrantExtraSlot"], true);
		gtk_widget_set_sensitive (menuItems["RemoveUserFromQueue"], true);
	}
	else
	{
		gtk_widget_set_sensitive (menuItems["SearchByTTH"], false);
		gtk_widget_set_sensitive (menuItems["GetFileList"], false);
		gtk_widget_set_sensitive (menuItems["SendPrivateMessage"], false);
		gtk_widget_set_sensitive (menuItems["AddToFavorites"], false);
		gtk_widget_set_sensitive (menuItems["MatchQueue"], false);
		gtk_widget_set_sensitive (menuItems["GrantExtraSlot"], false);
		gtk_widget_set_sensitive (menuItems["RemoveUserFromQueue"], false);
	}
	if (menu == 1)
	{
		for (vector<GtkWidget*>::iterator it=downloadItems.begin (); it != downloadItems.end (); it++)
			gtk_container_remove (GTK_CONTAINER (downloadMenu), *it);

		downloadItems.clear ();
						
		StringPairList spl = HubManager::getInstance()->getFavoriteDirs();
		if (spl.size() > 0) 
		{
			for(StringPairIter i = spl.begin(); i != spl.end(); i++) 
			{
				downloadItems.push_back (gtk_menu_item_new_with_label (i->second.c_str()));
				gtk_menu_shell_append (GTK_MENU_SHELL (downloadMenu), downloadItems.back ());
				g_signal_connect (G_OBJECT (downloadItems.back ()), "activate", G_CALLBACK (onDownloadFavoriteClicked_gui), (gpointer)this);
			}
			downloadItems.push_back (gtk_separator_menu_item_new ());
			gtk_menu_shell_append (GTK_MENU_SHELL (downloadMenu), downloadItems.back ());
		}
		downloadItems.push_back (gtk_menu_item_new_with_label ("Browse..."));
		gtk_menu_shell_append (GTK_MENU_SHELL (downloadMenu), downloadItems.back ());
		g_signal_connect (G_OBJECT (downloadItems.back ()), "activate", G_CALLBACK (onDownloadToClicked_gui), (gpointer)this);
		gtk_widget_show_all (GTK_WIDGET (downloadMenu));
	}
	else if (menu == 2)
	{
		for (vector<GtkWidget*>::iterator it=downloadDirItems.begin (); it != downloadDirItems.end (); it++)
			gtk_container_remove (GTK_CONTAINER (downloadDirMenu), *it);
			
		downloadDirItems.clear ();			
			
		StringPairList spl = HubManager::getInstance()->getFavoriteDirs();
		if (spl.size() > 0) 
		{
			for(StringPairIter i = spl.begin(); i != spl.end(); i++) 
			{
				downloadDirItems.push_back (gtk_menu_item_new_with_label (i->second.c_str()));
				gtk_menu_shell_append (GTK_MENU_SHELL (downloadDirMenu), downloadDirItems.back ());
				g_signal_connect (G_OBJECT (downloadDirItems.back ()), "activate", G_CALLBACK (onDownloadFavoriteDirClicked_gui), (gpointer)this);
			}
			downloadDirItems.push_back (gtk_separator_menu_item_new ());
			gtk_menu_shell_append (GTK_MENU_SHELL (downloadDirMenu), downloadDirItems.back ());
		}
		downloadDirItems.push_back (gtk_menu_item_new_with_label ("Browse..."));
		gtk_menu_shell_append (GTK_MENU_SHELL (downloadDirMenu), downloadDirItems.back ());
		g_signal_connect (G_OBJECT (downloadDirItems.back ()), "activate", G_CALLBACK (onDownloadDirToClicked_gui), (gpointer)this);
		gtk_widget_show_all (GTK_WIDGET (downloadDirMenu));	
	}
	pthread_mutex_unlock (&searchLock);
}

void Search::onSearchForTTHClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	
	if (!tmp)
		return;
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return;
	SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter, RESULT_INFO);
	if (!si->result->getTTH ())
		return;
		
	s->putValue (si->result->getTTH ()->toBase32 (), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
}

void Search::onGetFileListClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	
	if (!tmp)
		return;
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return;
	SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter, RESULT_INFO);
	try 
	{
		QueueManager::getInstance()->addList(si->result->getUser (), QueueItem::FLAG_CLIENT_VIEW);
	} 
	catch(const Exception&) 
	{
	}
}

void Search::onMatchQueueClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	
	if (!tmp)
		return;
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return;
	SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter, RESULT_INFO);
	try 
	{
		QueueManager::getInstance()->addList(si->result->getUser (), QueueItem::FLAG_MATCH_QUEUE);
	} 
	catch(const Exception&) 
	{
	}
}

void Search::onPrivateMessageClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	
	if (!tmp)
		return;
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return;
	SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter, RESULT_INFO);
	WulforManager::get()->addPrivMsg_gui(si->result->getUser ());
}

void Search::onAddFavoriteUserClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	
	if (!tmp)
		return;
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return;
	SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter, RESULT_INFO);
	HubManager::getInstance()->addFavoriteUser(si->result->getUser ());
}

void Search::onGrantExtraSlotClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	
	if (!tmp)
		return;
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return;
	SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter, RESULT_INFO);
	UploadManager::getInstance()->reserveSlot(si->result->getUser ());
}

void Search::onRemoveUserFromQueueClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	
	if (!tmp)
		return;
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return;	
	SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter, RESULT_INFO);
	QueueManager::getInstance()->removeSources(si->result->getUser (), QueueItem::Source::FLAG_REMOVED);
}

void Search::onDownloadFavoriteClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<GtkTreeIter> iter;
	GtkTreeIter tmpiter;
	
	if (!tmp)
		return;
	
	while (1)
	{
		if (gtk_tree_model_get_iter (m, &tmpiter, (GtkTreePath*)tmp->data))
			iter.push_back (tmpiter);
		
		tmp = g_list_next (tmp);
		if (!tmp)
			break;
	}	
	
	string label = getTextFromMenu (item), path;
	StringPairList spl = HubManager::getInstance()->getFavoriteDirs();
	bool found=false;
	if (spl.size() > 0) 
	{
		for(StringPairIter i = spl.begin(); i != spl.end(); i++) 
			if (label == i->second)
			{
				found=true;
				path = i->first;
				break;
			}
	}
	if (!found)
		return;	
	for (int i=0;i<iter.size ();i++)
	{
		SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter[i], RESULT_INFO);
		si->downloadTo (path + Util::getFileName (linuxSeparator (si->result->getFile ())));
	}
}

void Search::onDownloadFavoriteDirClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<GtkTreeIter> iter;
	GtkTreeIter tmpiter;
	
	if (!tmp)
		return;
	
	while (1)
	{
		if (gtk_tree_model_get_iter (m, &tmpiter, (GtkTreePath*)tmp->data))
			iter.push_back (tmpiter);
		
		tmp = g_list_next (tmp);
		if (!tmp)
			break;
	}	
	
	string label = getTextFromMenu (item), path;
	StringPairList spl = HubManager::getInstance()->getFavoriteDirs();
	bool found=false;
	if (spl.size() > 0) 
	{
		for(StringPairIter i = spl.begin(); i != spl.end(); i++) 
			if (label == i->second)
			{
				found=true;
				path = i->first;
				break;
			}
	}
	if (!found)
		return;	
	
	for (int i=0;i<iter.size ();i++)
	{	
		SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter[i], RESULT_INFO);
		si->downloadDirTo (path);
	}
}

void Search::onSearchEntryDown_gui (GtkEntry *entry, gpointer user_data)
{
	((Search*)user_data)->search_gui ();
}

gboolean Search::onButtonPressed_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if (event->type == GDK_BUTTON_PRESS)
    	{
		if (event->button == 3)
		{
        		GtkTreeSelection *selection;

        		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
			GtkTreeModel *m = gtk_tree_view_get_model (GTK_TREE_VIEW (widget));
			GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
			GList *tmp = g_list_first (list);
			GtkTreeIter iter;
	
			if (!tmp)
				return FALSE;
			if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
				return FALSE;
				
			pthread_mutex_lock (&((Search*)user_data)->searchLock);
			SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter, RESULT_INFO);
			pthread_mutex_unlock (&((Search*)user_data)->searchLock);
			((Search*)user_data)->buildDownloadMenu_gui (1);
			((Search*)user_data)->buildDownloadMenu_gui (2);
			((Search*)user_data)->popup_menu_gui(event, user_data);			
			return TRUE;
		}
	}
	else if (event->type == GDK_2BUTTON_PRESS)
	{
		if (event->button == 1)
		{
			Search *s = (Search*)user_data;
			GtkTreeSelection *selection;
			GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
			selection = gtk_tree_view_get_selection(s->result->get ());
			int count = gtk_tree_selection_count_selected_rows (selection);
			GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
			GList *tmp = g_list_first (list);
			GtkTreeIter iter;
						
			if (count < 1 && count > 1)
				return FALSE;
		
			if (!tmp)
				return FALSE;
			if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
				return FALSE;	
				
			SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter, RESULT_INFO);
			si->download ();
			return TRUE;
		}
	}

	return FALSE;
}
gboolean Search::onPopupMenu_gui (GtkWidget *widget, gpointer user_data)
{
	((Search*)user_data)->popup_menu_gui(NULL, user_data);
	return TRUE;
}
void Search::popup_menu_gui (GdkEventButton *event, gpointer user_data)
{
    gtk_menu_popup(mainMenu, NULL, NULL, NULL, NULL,
                   (event != NULL) ? 1 : 0,
                   gdk_event_get_time((GdkEvent*)event));
}

void Search::onToggledClicked_gui (GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
  	GtkTreeIter iter;
  	gboolean fixed;
	GtkTreeModel *m = gtk_tree_view_get_model (((Search*)data)->hubs->get ());
  	gtk_tree_model_get_iter (m, &iter, gtk_tree_path_new_from_string (path_str));
  	gtk_tree_model_get (m, &iter, Search::HUB_SEARCH, &fixed, -1);
  	fixed = !fixed;
  	gtk_list_store_set (GTK_LIST_STORE (m), &iter, Search::HUB_SEARCH, fixed, -1);
}

void Search::onSearchButtonClicked_gui (GtkWidget *widget, gpointer user_data)
{
	((Search*)user_data)->search_gui ();
}

void Search::onDownloadClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<GtkTreeIter> iter;
	GtkTreeIter tmpiter;
	
	if (!tmp)
		return;
	
	while (1)
	{
		if (gtk_tree_model_get_iter (m, &tmpiter, (GtkTreePath*)tmp->data))
			iter.push_back (tmpiter);
		
		tmp = g_list_next (tmp);
		if (!tmp)
			break;
	}	
	for (int i=0;i<iter.size ();i++)
	{
		SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter[i], RESULT_INFO);
		si->download ();
	}
}
void Search::onDownloadToClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());
	
	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<GtkTreeIter> iter;
	GtkTreeIter tmpiter;	
	int count=0;
	
	if (!tmp)
		return;
		
	while (1)
	{
		if (gtk_tree_model_get_iter (m, &tmpiter, (GtkTreePath*)tmp->data))
		{
			iter.push_back (tmpiter);
			count++;
		}
		
		tmp = g_list_next (tmp);
		if (!tmp)
			break;
	}	
	if (count == 1)
	{
		SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter[0], RESULT_INFO);
		si->browse(true);
	}
	
	GtkWidget *dirChooser = gtk_file_chooser_dialog_new ("Choose location",
												WulforManager::get ()->getMainWindow ()->getWindow (),
												GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
												GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
												GTK_STOCK_OPEN, GTK_RESPONSE_OK,
												NULL);								
	if (Search::lastDir != "")
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dirChooser), Search::lastDir.c_str ());
	else
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dirChooser), SETTING (DOWNLOAD_DIRECTORY).c_str ());			
	gint response = gtk_dialog_run (GTK_DIALOG (dirChooser));
	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dirChooser));
		if (path[path.length ()-1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		Search::lastDir = path;
		
		for (int i=0;i<iter.size ();i++)
		{
			SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter[i], RESULT_INFO);
			si->downloadTo (path+Util::getFileName (si->result->getFile ()));	
		}			
	}
	gtk_widget_hide (dirChooser);
	dirChooser = NULL;	
}
void Search::onDownloadDirClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<GtkTreeIter> iter;
	GtkTreeIter tmpiter;
	
	if (!tmp)
		return;
	
	while (1)
	{
		if (gtk_tree_model_get_iter (m, &tmpiter, (GtkTreePath*)tmp->data))
			iter.push_back (tmpiter);
		
		tmp = g_list_next (tmp);
		if (!tmp)
			break;
	}	
	for (int i=0;i<iter.size ();i++)
	{
		SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter[i], RESULT_INFO);
		si->downloadDir ();
	}
}
void Search::onDownloadDirToClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());
	
	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<GtkTreeIter> iter;
	GtkTreeIter tmpiter;	
	int count=0;
	
	if (!tmp)
		return;
		
	while (1)
	{
		if (gtk_tree_model_get_iter (m, &tmpiter, (GtkTreePath*)tmp->data))
		{
			iter.push_back (tmpiter);
			count++;
		}
		
		tmp = g_list_next (tmp);
		if (!tmp)
			break;
	}	
	if (count == 1)
	{
		SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter[0], RESULT_INFO);
		si->browse(false);
	}	
	
	GtkWidget *dirChooser = gtk_file_chooser_dialog_new ("Choose location",
												WulforManager::get ()->getMainWindow ()->getWindow (),
												GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
												GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
												GTK_STOCK_OPEN, GTK_RESPONSE_OK,
												NULL);								
	if (Search::lastDir != "")
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dirChooser), Search::lastDir.c_str ());
	else
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dirChooser), SETTING (DOWNLOAD_DIRECTORY).c_str ());			
	gint response = gtk_dialog_run (GTK_DIALOG (dirChooser));
	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dirChooser));
		if (path[path.length ()-1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		Search::lastDir = path;
		
		for (int i=0;i<iter.size ();i++)
		{
			SearchInfo *si = TreeViewFactory::getValue<gpointer,SearchInfo*>(m, &iter[i], RESULT_INFO);
			si->downloadDirTo (path);	
		}			
	}
	gtk_widget_hide (dirChooser);
	dirChooser = NULL;	
}

void Search::onRemoveClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	Search *s = (Search*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (s->resultStore);
	selection = gtk_tree_view_get_selection(s->result->get ());
	
	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<GtkTreeIter> iter;
	GtkTreeIter tmpiter;
	
	if (!tmp)
		return;
	
	while (1)
	{
		if (gtk_tree_model_get_iter (m, &tmpiter, (GtkTreePath*)tmp->data))
			iter.push_back (tmpiter);
		
		tmp = g_list_next (tmp);
		if (!tmp)
			break;
	}
	
	for (int i=0;i<iter.size ();i++)
	{
		gtk_list_store_remove (s->resultStore, &iter[i]);
	}	
}

void Search::search_gui ()
{
	StringList clients;
	GtkTreeIter iter;
	string text, size, s;
	
	text = gtk_entry_get_text (GTK_ENTRY (GTK_BIN (searchItems["Search"])->child));
	
	if(!(text.length () > 0 && lastSearch + 3*1000 < TimerManager::getInstance()->getTick()))
		return;

	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (hubStore), &iter))
		return;
	
	bool op = TreeViewFactory::getValue<gboolean>(GTK_TREE_MODEL (hubStore), &iter, HUB_SEARCH);
	while (1)
	{
		HubInfo *hi = TreeViewFactory::getValue<gpointer,HubInfo*> (GTK_TREE_MODEL (hubStore), &iter, HUB_INFO);
		if (op)
		{
			if (hi->op)
				clients.push_back (hi->ipPort);
		}
		else if (TreeViewFactory::getValue<gboolean>(GTK_TREE_MODEL (hubStore), &iter, HUB_SEARCH))
		{
			clients.push_back (hi->ipPort);
		}
		
		if (!gtk_tree_model_iter_next (GTK_TREE_MODEL (hubStore), &iter))
			break;
	}
	if(!clients.size())
		return;

	size = gtk_entry_get_text (GTK_ENTRY (searchItems["Size"]));
	
	double lsize = Util::toDouble(size);
	switch(gtk_combo_box_get_active (GTK_COMBO_BOX (searchItems["Unit"]))) {
	case 1:
		lsize*=1024.0; break;
	case 2:
		lsize*=1024.0*1024.0; break;
	case 3:
		lsize*=1024.0*1024.0*1024.0; break;
	}

	int64_t llsize = (int64_t)lsize;

	gtk_list_store_clear (resultStore);
	s = text;
	{
		Lock l(cs);
		searchlist = StringTokenizer<tstring>(s, ' ').getTokens();
	}

	//strip out terms beginning with -
	s.clear();
	for (TStringList::const_iterator si = searchlist.begin(); si != searchlist.end(); ++si)
		if ((*si)[0] != '-' || si->size() != 1) 
			s += *si + ' ';	//Shouldn't get 0-length tokens, so safely assume at least a first char.
	s = s.substr(0, max(s.size(), static_cast<tstring::size_type>(1)) - 1);

	SearchManager::SizeModes mode((SearchManager::SizeModes)gtk_combo_box_get_active (GTK_COMBO_BOX (searchItems["Diffrence"])));
	if(llsize == 0)
		mode = SearchManager::SIZE_DONTCARE;

	int ftype = gtk_combo_box_get_active (GTK_COMBO_BOX (searchItems["File type"]));

	if(BOOLSETTING(CLEAR_SEARCH))
		gtk_entry_set_text (GTK_ENTRY (GTK_BIN (searchItems["Search"])->child), "");
	else
		lastSearch = TimerManager::getInstance()->getTick();

	// Add new searches to the last-search dropdown list
	if(find(lastSearches.begin(), lastSearches.end(), s) == lastSearches.end()) 
	{
		GtkTreeModel *m = gtk_combo_box_get_model (GTK_COMBO_BOX (searchItems["Search"]));
		if(listItems > 9)
		{
			if (gtk_tree_model_get_iter_first (m, &iter))
			{
				int count=0;
				while (gtk_tree_model_iter_next (m, &iter))
				{
					if (count == 8)
					{
						gtk_list_store_remove (GTK_LIST_STORE (m), &iter);
						break;
					}
					count++;	
				}
			}	
		}
		else
			listItems++;
		gtk_list_store_insert (GTK_LIST_STORE (m), &iter, 0);
		gtk_list_store_set (GTK_LIST_STORE (m), &iter, 0, s.c_str(), -1);

		if(lastSearches.size() > 9)
			lastSearches.erase(lastSearches.begin());
		lastSearches.push_back(s);
	}
	
	droppedResult = 0;
	isHash = (ftype == SearchManager::TYPE_TTH);

	gtk_list_store_clear (resultStore);
	searchHits = 0;
	gtk_statusbar_pop(GTK_STATUSBAR (searchItems["Status1"]), 0);
	gtk_statusbar_push(GTK_STATUSBAR (searchItems["Status1"]), 0, string ("Searching for " + s + "...").c_str ());
	gtk_statusbar_pop(GTK_STATUSBAR (searchItems["Status2"]), 0);
	gtk_statusbar_push(GTK_STATUSBAR (searchItems["Status2"]), 0, "");
	gtk_statusbar_pop(GTK_STATUSBAR (searchItems["Status3"]), 0);
	gtk_statusbar_push(GTK_STATUSBAR (searchItems["Status3"]), 0, "");	
	SearchManager::getInstance()->search(clients, s, llsize, (SearchManager::TypeModes)ftype, (SearchManager::SizeModes)mode, "");
}

void Search::initHubs_gui ()
{
	GtkTreeIter iter;
	gtk_list_store_append (hubStore, &iter);
	gtk_list_store_set (hubStore, &iter, HUB_SEARCH, false, HUB_NAME, "Only where I'm op", HUB_INFO, new HubInfo ("", "Only where I'm op", false), -1);

	ClientManager* clientMgr = ClientManager::getInstance();
	clientMgr->lock();

	Client::List& clients = clientMgr->getClients();

	Client::List::iterator it;
	Client::List::iterator endIt = clients.end();
	for(it = clients.begin(); it != endIt; ++it) {
		Client* client = *it;
		if (!client->isConnected())
			continue;

		changeHubs_gui (0, new HubInfo (client->getIpPort (), client->getName (), client->getOp ()));
	}

	clientMgr->unlock();
}																		

void Search::changeHubs_gui (int mode, HubInfo *i)
{
	if (mode == 0)
	{
		GtkTreeIter iter;
		pthread_mutex_lock (&searchLock);
		gtk_list_store_append (hubStore, &iter);	
		gtk_list_store_set (hubStore, &iter, HUB_SEARCH, true, HUB_NAME, (i->name=="") ? "Connecting..." : i->name.c_str (), HUB_INFO, (gpointer)i, -1);
		pthread_mutex_unlock (&searchLock);
	}
	else if (mode == 1)
	{
		GtkTreeIter iter;
		if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (hubStore), &iter))
			return;
			
		while (1)
		{
			HubInfo *hi = TreeViewFactory::getValue<gpointer,HubInfo*>(GTK_TREE_MODEL (hubStore), &iter, HUB_INFO);
			if (hi->ipPort == i->ipPort)
			{
				if (hi->name == i->name)
					return;
				pthread_mutex_lock (&searchLock);
				gtk_list_store_set (hubStore, &iter, HUB_NAME, i->name.c_str (), HUB_INFO, (gpointer)i, -1);
				pthread_mutex_unlock (&searchLock);
				return;
			}
			if (!gtk_tree_model_iter_next (GTK_TREE_MODEL (hubStore), &iter))
				return;
		}
	}
	else if (mode == 2)
	{
		GtkTreeIter iter;
		if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (hubStore), &iter))
			return;
			
		while (1)
		{
			if (TreeViewFactory::getValue<gpointer,HubInfo*>(GTK_TREE_MODEL (hubStore), &iter, HUB_INFO)->ipPort == i->ipPort)
			{
				pthread_mutex_lock (&searchLock);
				gtk_list_store_remove (hubStore, &iter);
				pthread_mutex_unlock (&searchLock);
				return;
			}
			if (!gtk_tree_model_iter_next (GTK_TREE_MODEL (hubStore), &iter))
				return;
		}	
	}
}

void Search::addResult_gui (SearchInfo *info)
{
	SearchResult *aResult = info->result;
	if (!aResult)
		return;
	pthread_mutex_lock (&searchLock);
	// Check that this is really a relevant search result...
	{

		if(searchlist.empty())
		{
			pthread_mutex_unlock (&searchLock);
			return;
		}
			
		if(isHash) {
			if(aResult->getTTH() == NULL)
			{
				pthread_mutex_unlock (&searchLock);
				return;
			}
			if(Util::stricmp(aResult->getTTH()->toBase32(), searchlist[0]) != 0)
			{
				pthread_mutex_unlock (&searchLock);
				return;
			}
		} 
		else {
			// match all here
			for(TStringIter j = searchlist.begin(); j != searchlist.end(); ++j) 
			{
				if((*j->begin() != '-' && Util::findSubString(aResult->getUtf8() ? aResult->getFile() : Text::acpToUtf8(aResult->getFile()), *j) == -1) ||
					(*j->begin() == '-' && j->size() != 1 && Util::findSubString(aResult->getUtf8() ? aResult->getFile() : Text::acpToUtf8(aResult->getFile()), j->substr(1)) != -1)
					) 
				{
					droppedResult++;
					gtk_statusbar_pop(GTK_STATUSBAR (searchItems["Status3"]), 0);
					gtk_statusbar_push(GTK_STATUSBAR (searchItems["Status3"]), 0, string (Util::toString(droppedResult) + " filtered").c_str ());
					pthread_mutex_unlock (&searchLock);
					return;
				}
			}
		}
	}

	// Reject results without free slots if selected
	if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (searchItems["Slots"])) && aResult->getFreeSlots() < 1)
	{
		pthread_mutex_unlock (&searchLock);
		return;
	}

	SearchResult *r = aResult;
	GtkTreeIter iter;
	
	string fileName, path, type, size, exactSize, nick, connection, hubName, slots, ip, TTH;
	if(r->getType() == SearchResult::TYPE_FILE) 
	{
		string file = linuxSeparator (r->getFile ());
		if(file.rfind('/') == tstring::npos)
			fileName = r->getUtf8() ? file : Text::acpToUtf8(file);
		else 
		{
			fileName = Util::getFileName(r->getUtf8() ? file : Text::acpToUtf8(file));
			path = Util::getFilePath(r->getUtf8() ? file : Text::acpToUtf8(file));
		}

		type = Util::getFileExt(fileName);
		if(!type.empty() && type[0] == '.')
			type.erase(0, 1);
		size = Util::formatBytes(r->getSize());
		exactSize = Util::formatExactSize(r->getSize());
	} 
	else 
	{
		fileName = linuxSeparator (r->getUtf8() ? r->getFileName() : Text::acpToUtf8(r->getFileName()));
		path = linuxSeparator (r->getUtf8() ? r->getFile() : Text::acpToUtf8(r->getFile()));
		type = "Directory";
	}
	slots = r->getSlotString ();
	connection = r->getUser ()->getConnection ();
	hubName = r->getHubName ();
	ip = r->getIP ();
	if(r->getTTH() != NULL)
		TTH = r->getTTH ()->toBase32 ();

	//Check that it's not a duplicate
	GtkTreeIter i;
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(resultStore), &i);
	while (gtk_list_store_iter_is_valid(resultStore, &i)) {
		char *fileChar, *ipChar;
		string fileString, ipString;
		gtk_tree_model_get(GTK_TREE_MODEL(resultStore), &i,
			RESULT_FILE, &fileChar,
			RESULT_IP, &ipChar,
			-1);
		fileString = fileChar;
		ipString = ipChar;
		g_free(fileChar);
		g_free(ipChar);
		if (ipString == ip && fileString == fileName) {
			pthread_mutex_unlock (&searchLock);
			return;
		}

		gtk_tree_model_iter_next(GTK_TREE_MODEL(resultStore), &i);
	}
	
	gtk_list_store_append (resultStore, &iter);
	gtk_list_store_set (resultStore, &iter,
		RESULT_NICK, r->getUser()->getNick().c_str(),
		RESULT_FILE, fileName.c_str (),
		RESULT_SLOTS, slots.c_str (),
		RESULT_FILESIZE, size.c_str (),
		RESULT_PATH, path.c_str (),
		RESULT_TYPE, type.c_str (),
		RESULT_CONNECTION, connection.c_str (),
		RESULT_HUB, hubName.c_str (),
		RESULT_EXACT_SIZE, exactSize.c_str (),
		RESULT_IP, ip.c_str (),
		RESULT_TTH, TTH.c_str (),
		RESULT_INFO, (gpointer)(new SearchInfo(aResult)), 
		RESULT_SIZE, r->getSize (),
		-1);
	searchHits++;
	gtk_statusbar_pop(GTK_STATUSBAR (searchItems["Status2"]), 0);
	gtk_statusbar_push(GTK_STATUSBAR (searchItems["Status2"]), 0, string (Util::toString(searchHits) + " items").c_str ());
	pthread_mutex_unlock (&searchLock);
}
void Search::on(ClientManagerListener::ClientConnected, Client *c) throw ()
{
	Lock l(cs);
	changeHubs_gui (0, new HubInfo (c->getIpPort (), c->getName (), c->getOp ()));
}
void Search::on(ClientManagerListener::ClientUpdated, Client *c) throw ()
{
	Lock l(cs);
	changeHubs_gui (1, new HubInfo (c->getIpPort (), c->getName (), c->getOp ()));
}
void Search::on(ClientManagerListener::ClientDisconnected, Client *c) throw ()
{
	Lock l(cs);
	changeHubs_gui (2, new HubInfo (c->getIpPort (), c->getName (), c->getOp ()));
}

void Search::on(SearchManagerListener::SR, SearchResult* aResult) throw()
{
	typedef Func1<Search, SearchInfo*> F1;
	F1 *func;
	func = new F1 (this, &Search::addResult_gui, new SearchInfo(aResult));
	WulforManager::get()->dispatchGuiFunc (func);
}

void Search::SearchInfo::browse (bool file)
{
	if (file)
	{
		GtkWidget *fileChooser = gtk_file_chooser_dialog_new (	"Choose location",
											WulforManager::get ()->getMainWindow ()->getWindow (),
											 GTK_FILE_CHOOSER_ACTION_SAVE,
											GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											GTK_STOCK_OPEN, GTK_RESPONSE_OK,
											NULL);
		if (result->getUtf8()) 
			gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fileChooser), Util::getFileName (linuxSeparator (result->getFile ())).c_str ());
		else 
			gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fileChooser), Text::acpToUtf8 (Util::getFileName (linuxSeparator (result->getFile ()))).c_str ());
		if (Search::lastDir != "")
			gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fileChooser), Search::lastDir.c_str ());
		else
			gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fileChooser), SETTING (DOWNLOAD_DIRECTORY).c_str ());
		gint response = gtk_dialog_run (GTK_DIALOG (fileChooser));
		if (response == GTK_RESPONSE_OK)
		{
			string path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooser));
			Search::lastDir = Util::getFilePath (path);
			downloadTo (path);
		}
		gtk_widget_hide (fileChooser);
		fileChooser = NULL;
	}
	else
	{
		GtkWidget *dirChooser = gtk_file_chooser_dialog_new ("Choose location",
													WulforManager::get ()->getMainWindow ()->getWindow (),
													GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
													GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
													GTK_STOCK_OPEN, GTK_RESPONSE_OK,
													NULL);								
		if (Search::lastDir != "")
			gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dirChooser), Search::lastDir.c_str ());
		else
			gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dirChooser), SETTING (DOWNLOAD_DIRECTORY).c_str ());			
		gint response = gtk_dialog_run (GTK_DIALOG (dirChooser));
		if (response == GTK_RESPONSE_OK)
		{
			string path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dirChooser));
			if (path[path.length ()-1] != PATH_SEPARATOR)
				path += PATH_SEPARATOR;
			Search::lastDir = path;
			downloadDirTo (path);
		}
		gtk_widget_hide (dirChooser);
		dirChooser = NULL;
	}
}

void Search::SearchInfo::download ()
{
	string dir = Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY));
	string target = dir;
	target += Util::getFileName (linuxSeparator (result->getFile ()));
	
	if (result->getUtf8())
		target = Text::utf8ToAcp(target);
	
	try 
	{
		if(result->getType() == SearchResult::TYPE_FILE) 
		{
			QueueManager::getInstance()->add(result->getFile(), result->getSize(), result->getUser(), 
				target, result->getTTH(), QueueItem::FLAG_RESUME | (result->getUtf8() ? QueueItem::FLAG_SOURCE_UTF8 : 0),
				QueueItem::DEFAULT);
		} 
		else 
		{
			QueueManager::getInstance()->addDirectory(result->getFile(), result->getUser(), dir,
				QueueItem::DEFAULT);
		}
	} 
	catch(const Exception&) 
	{
		cout << "Couldn't add file to queue." << endl;
	}
}

void Search::SearchInfo::downloadTo (string target)
{
	try 
	{
		if(result->getType() == SearchResult::TYPE_FILE) 
		{
			QueueManager::getInstance()->add(result->getFile(), result->getSize(), result->getUser(), 
				target, result->getTTH(), QueueItem::FLAG_RESUME | (result->getUtf8() ? QueueItem::FLAG_SOURCE_UTF8 : 0),
				QueueItem::DEFAULT);
		} 
		else 
		{
			QueueManager::getInstance()->addDirectory(result->getFile(), result->getUser(), Util::getFilePath (target),
				QueueItem::DEFAULT);
		}
	} 
	catch(const Exception&) 
	{
		cout << "Couldn't add file to queue." << endl;
	}	
}

void Search::SearchInfo::downloadDir ()
{
	try
	{
		if(result->getType() == SearchResult::TYPE_FILE) 
		{
			QueueManager::getInstance()->addDirectory(windowsSeparator (Util::getFilePath (linuxSeparator (result->getFile ()))),
				result->getUser(),
				SETTING (DOWNLOAD_DIRECTORY), 
				QueueItem::DEFAULT);
		}
		else 
		{
			QueueManager::getInstance()->addDirectory(
				result->getFile (),
				result->getUser(),
				SETTING (DOWNLOAD_DIRECTORY), 
				QueueItem::DEFAULT);
		}
	}
	catch(...)
	{
		cout << "Couldn't add dir for download." << endl;
	}	
}
void Search::SearchInfo::downloadDirTo (string dir)
{
	try
	{
		if(result->getType() == SearchResult::TYPE_FILE) 
		{
			QueueManager::getInstance()->addDirectory(windowsSeparator (Util::getFilePath (linuxSeparator (result->getFile ()))),
				result->getUser(),
				dir+'/', 
				QueueItem::DEFAULT);
		}
		else 
		{
			QueueManager::getInstance()->addDirectory(
				result->getFile (),
				result->getUser(),
				dir+'/', 
				QueueItem::DEFAULT);
		}
	}
	catch(...)
	{
		cout << "Couldn't add dir for download." << endl;
	}	
}
