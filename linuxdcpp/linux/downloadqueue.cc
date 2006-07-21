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

#include "downloadqueue.hh"

DownloadQueue::DownloadQueue():
	BookEntry("Download Queue")
{
	QueueManager::getInstance()->addListener(this);

	GladeXML *xml = getGladeXML("downloadqueue.glade");

	GtkWidget *window = glade_xml_get_widget(xml, "downloadQueueWindow");
	mainBox = glade_xml_get_widget(xml, "downloadQueueBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	statusbar.push_back(glade_xml_get_widget(xml, "statusMain"));
	statusbar.push_back(glade_xml_get_widget(xml, "statusItems"));
	statusbar.push_back(glade_xml_get_widget(xml, "statusFileSize"));
	statusbar.push_back(glade_xml_get_widget(xml, "statusFiles"));
	statusbar.push_back(glade_xml_get_widget(xml, "statusTotalSize"));

	// Initialize directory treeview
	dirView.setView(GTK_TREE_VIEW(glade_xml_get_widget(xml, "dirView")));
	dirView.insertColumn("Dir", G_TYPE_STRING, TreeView::STRING, -1);
	dirView.insertHiddenColumn("Path", G_TYPE_STRING);
	dirView.finalize();
	dirStore = gtk_tree_store_newv(dirView.getColCount(), dirView.getGTypes());
	gtk_tree_view_set_model(dirView.get(), GTK_TREE_MODEL (dirStore));
	g_object_unref(dirStore);
	gtk_widget_set_events(GTK_WIDGET(dirView.get()), GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
 	g_signal_connect(G_OBJECT(dirView.get()), "button_press_event", G_CALLBACK(dir_onButtonPressed_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(dirView.get()), "button_release_event", G_CALLBACK(dir_onButtonReleased_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(dirView.get()), "popup_menu", G_CALLBACK(dir_onPopupMenu_gui), (gpointer)this);

	// Initialize file treeview
	fileView.setView(GTK_TREE_VIEW(glade_xml_get_widget(xml, "fileView")), true, "downloadqueue");
	fileView.insertColumn("Filename", G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn("Status", G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn("Size", G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn("Downloaded", G_TYPE_STRING, TreeView::STRING, 150);
	fileView.insertColumn("Priority", G_TYPE_STRING, TreeView::STRING, 75);
	fileView.insertColumn("Users", G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn("Path", G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn("Exact Size", G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn("Error", G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn("Added", G_TYPE_STRING, TreeView::STRING, 120);
	fileView.insertColumn("TTH", G_TYPE_STRING, TreeView::STRING, 125);
	fileView.insertHiddenColumn("Info", G_TYPE_POINTER);
	fileView.insertHiddenColumn("Real Size", G_TYPE_INT64);
	fileView.insertHiddenColumn("Download Size", G_TYPE_INT64);
	fileView.finalize();
	fileStore = gtk_list_store_newv(fileView.getColCount(), fileView.getGTypes());
	gtk_tree_view_set_model(fileView.get(), GTK_TREE_MODEL(fileStore));
	g_object_unref(fileStore);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(fileView.get()), GTK_SELECTION_MULTIPLE);
	fileView.setSortColumn_gui("Size", "Real Size");
	fileView.setSortColumn_gui("Exact Size", "Real Size");
	fileView.setSortColumn_gui("Downloaded", "Download Size");

	// Connect callbacks to dir menu
	dirMenu = GTK_MENU(glade_xml_get_widget(xml, "dirMenu"));
	dirItems["Set priority"] = glade_xml_get_widget(xml, "dirSetPriorityItem");
	dirPriority = GTK_MENU(glade_xml_get_widget(xml, "dirPriorityMenu"));
	dirItems["Paused"] = glade_xml_get_widget(xml, "pausedPriorityItem");
	g_signal_connect(G_OBJECT(dirItems["Paused"]), "activate", G_CALLBACK(setDirPriorityClicked_gui), (gpointer)this);
	dirItems["Lowest"] = glade_xml_get_widget(xml, "lowestPriorityItem");
	g_signal_connect(G_OBJECT(dirItems["Lowest"]), "activate", G_CALLBACK(setDirPriorityClicked_gui), (gpointer)this);
	dirItems["Low"] = glade_xml_get_widget(xml, "lowPrioritytem");
	g_signal_connect(G_OBJECT(dirItems["Low"]), "activate", G_CALLBACK(setDirPriorityClicked_gui), (gpointer)this);
	dirItems["Normal"] = glade_xml_get_widget(xml, "normalPriorityItem");
	g_signal_connect(G_OBJECT(dirItems["Normal"]), "activate", G_CALLBACK(setDirPriorityClicked_gui), (gpointer)this);
	dirItems["High"] = glade_xml_get_widget(xml, "highPriorityItem");
	g_signal_connect(G_OBJECT(dirItems["High"]), "activate", G_CALLBACK(setDirPriorityClicked_gui), (gpointer)this);
	dirItems["Highest"] = glade_xml_get_widget(xml, "highestPriorityItem");
	g_signal_connect(G_OBJECT(dirItems["Highest"]), "activate", G_CALLBACK(setDirPriorityClicked_gui), (gpointer)this);
	dirItems["Move/Rename"] = glade_xml_get_widget(xml, "moveRenameItem");
	dirItems["Remove"] = glade_xml_get_widget(xml, "removeDirItem");
	g_signal_connect(G_OBJECT(dirItems["Remove"]), "activate", G_CALLBACK(removeDirClicked_gui), (gpointer)this);

	// Connect callbacks to file menu
	fileMenu = GTK_MENU(glade_xml_get_widget(xml, "fileMenu"));
	fileItems["Search for alternates"] = glade_xml_get_widget(xml, "searchForAlternatesItem");
	g_signal_connect(G_OBJECT(fileItems["Search for alternates"]), "activate", G_CALLBACK(onSearchAlternatesClicked_gui), (gpointer)this);
	fileItems["Search by TTH"] = glade_xml_get_widget(xml, "searchByTTHItem");
	g_signal_connect(G_OBJECT (fileItems["Search by TTH"]), "activate", G_CALLBACK (onSearchByTTHClicked_gui), (gpointer)this);
	fileItems["Set priority"] = glade_xml_get_widget(xml, "fileSetPriorityItem");
	filePriority = GTK_MENU(glade_xml_get_widget(xml, "filePriorityMenu"));
	fileItems["Paused"] = glade_xml_get_widget(xml, "filePausedItem");
	g_signal_connect(G_OBJECT(fileItems["Paused"]), "activate", G_CALLBACK(setFilePriorityClicked_gui), (gpointer)this);
	fileItems["Lowest"] = glade_xml_get_widget(xml, "fileLowestPriorityItem");
	g_signal_connect(G_OBJECT(fileItems["Lowest"]), "activate", G_CALLBACK(setFilePriorityClicked_gui), (gpointer)this);
	fileItems["Low"] = glade_xml_get_widget(xml, "fileLowPriorityItem");
	g_signal_connect(G_OBJECT(fileItems["Low"]), "activate", G_CALLBACK(setFilePriorityClicked_gui), (gpointer)this);
	fileItems["Normal"] = glade_xml_get_widget(xml, "fileNormalPriorityItem");
	g_signal_connect(G_OBJECT(fileItems["Normal"]), "activate", G_CALLBACK(setFilePriorityClicked_gui), (gpointer)this);
	fileItems["High"] = glade_xml_get_widget(xml, "fileHighPriorityItem");
	g_signal_connect(G_OBJECT(fileItems["High"]), "activate", G_CALLBACK(setFilePriorityClicked_gui), (gpointer)this);
	fileItems["Highest"] = glade_xml_get_widget(xml, "fileHighestPriorityItem");
	g_signal_connect(G_OBJECT(fileItems["Highest"]), "activate", G_CALLBACK(setFilePriorityClicked_gui), (gpointer)this);
	fileItems["Get file list"] = glade_xml_get_widget(xml, "getFileListItem");
	browseMenu = GTK_MENU(glade_xml_get_widget(xml, "browseMenu"));
	fileItems["Send private message"] = glade_xml_get_widget(xml, "sendPrivateMessageItem");
	pmMenu = GTK_MENU(glade_xml_get_widget(xml, "pmMenu"));
	fileItems["Re-add source"] = glade_xml_get_widget(xml, "reAddSourceItem");
	readdMenu = GTK_MENU(glade_xml_get_widget(xml, "reAddMenu"));
	fileItems["Remove source"] = glade_xml_get_widget(xml, "removeSourceItem");
	removeMenu = GTK_MENU(glade_xml_get_widget(xml, "removeMenu"));
	fileItems["Remove user from queue"] = glade_xml_get_widget(xml, "removeUserFromQueueItem");
	removeallMenu = GTK_MENU(glade_xml_get_widget(xml, "removeAllMenu"));
	fileItems["Remove"] = glade_xml_get_widget(xml, "fileRemoveItem");
	g_signal_connect(G_OBJECT(fileItems["Remove"]), "activate", G_CALLBACK(removeFileClicked_gui), (gpointer)this);

	g_signal_connect(G_OBJECT(fileView.get()), "button_press_event", G_CALLBACK(file_onButtonPressed_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(fileView.get()), "popup_menu", G_CALLBACK(file_onPopupMenu_gui), (gpointer)this);

	pthread_mutex_init(&queueLock, NULL);

	buildList_gui();
}

DownloadQueue::~DownloadQueue() 
{
	QueueManager::getInstance()->removeListener(this);
	pthread_mutex_destroy(&queueLock);
	hash_map<string, std::vector<QueueItemInfo*> >::iterator iter;
	std::vector<QueueItemInfo*>::iterator iter2;
	for (iter = dirFileMap.begin(); iter != dirFileMap.end(); iter++)
		for (iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
			delete *iter2;
}

GtkWidget *DownloadQueue::getWidget() 
{
	return mainBox;
}

string DownloadQueue::getTextFromMenu(GtkMenuItem *item)
{
	string text;
	GtkWidget *child = gtk_bin_get_child(GTK_BIN(item));

	if (child && GTK_IS_LABEL(child))
		text = gtk_label_get_text(GTK_LABEL(child));

	return text;
}

void DownloadQueue::contentUpdated_gui()
{
	if (BOOLSETTING(BOLD_QUEUE))
		setBold_gui();
}

void DownloadQueue::buildDynamicMenu_gui ()
{
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (fileStore);
	selection = gtk_tree_view_get_selection(fileView.get());

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<GtkTreeIter> iter;
	vector<string> names;
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
		QueueItemInfo *i = fileView.getValue<gpointer,QueueItemInfo*>(&iter[0], "Info");
		if (i->getTTH ())
			gtk_widget_set_sensitive (fileItems["Search by TTH"], true);
		else
			gtk_widget_set_sensitive (fileItems["Search by TTH"], false);
			
		gtk_widget_set_sensitive (fileItems["Search for alternates"], true);
		gtk_widget_set_sensitive (fileItems["Get file list"], true);
		gtk_widget_set_sensitive (fileItems["Send private message"], true);
		gtk_widget_set_sensitive (fileItems["Re-add source"], true);
		gtk_widget_set_sensitive (fileItems["Remove source"], true);
		gtk_widget_set_sensitive (fileItems["Remove user from queue"], true);		
	}
	else
	{
		gtk_widget_set_sensitive (fileItems["Search by TTH"], false);
		gtk_widget_set_sensitive (fileItems["Search for alternates"], false);

		gtk_widget_set_sensitive (fileItems["Get file list"], false);
		gtk_widget_set_sensitive (fileItems["Send private message"], false);
		gtk_widget_set_sensitive (fileItems["Re-add source"], false);
		gtk_widget_set_sensitive (fileItems["Remove source"], false);
		gtk_widget_set_sensitive (fileItems["Remove user from queue"], false);
	}
	
	for (vector<GtkWidget*>::iterator it=browseItems.begin (); it != browseItems.end (); it++)
		gtk_container_remove (GTK_CONTAINER (browseMenu), *it);
	for (vector<GtkWidget*>::iterator it=pmItems.begin (); it != pmItems.end (); it++)
		gtk_container_remove (GTK_CONTAINER (pmMenu), *it);
	for (vector<GtkWidget*>::iterator it=readdItems.begin (); it != readdItems.end (); it++)
		gtk_container_remove (GTK_CONTAINER (readdMenu), *it);
	for (vector<GtkWidget*>::iterator it=removeItems.begin (); it != removeItems.end (); it++)
		gtk_container_remove (GTK_CONTAINER (removeMenu), *it);
	for (vector<GtkWidget*>::iterator it=removeallItems.begin (); it != removeallItems.end (); it++)
		gtk_container_remove (GTK_CONTAINER (removeallMenu), *it);	

	browseItems.clear ();
	pmItems.clear ();
	readdItems.clear ();
	removeItems.clear ();
	removeallItems.clear ();			
	
	for (int i=0;i<iter.size (); i++)
	{	
		QueueItemInfo *ii = fileView.getValue<gpointer,QueueItemInfo*>(&iter[i], "Info");
		for (QueueItemInfo::SourceIter it=ii->getSources ().begin (); it != ii->getSources ().end (); it++)
		{
			bool found=false;
			for (int j=0;j<names.size ();j++)
			{
				if (names[j] == it->getUser()->getFirstNick ())
				{
					found = true;
					break;
				}
			}
			if (!found)
				names.push_back (it->getUser()->getFirstNick ());
		}
	}
	for (int i=0;i<names.size ();i++)
	{
		browseItems.push_back (gtk_menu_item_new_with_label (names[i].c_str ()));
		gtk_menu_shell_append (GTK_MENU_SHELL (browseMenu), browseItems.back ());
		g_signal_connect (G_OBJECT (browseItems.back ()), "activate", G_CALLBACK (onGetFileListClicked_gui), (gpointer)this);

		pmItems.push_back (gtk_menu_item_new_with_label (names[i].c_str ()));
		gtk_menu_shell_append (GTK_MENU_SHELL (pmMenu), pmItems.back ());
		g_signal_connect (G_OBJECT (pmItems.back ()), "activate", G_CALLBACK (onSendPrivateMessageClicked_gui), (gpointer)this);

		readdItems.push_back (gtk_menu_item_new_with_label (names[i].c_str ()));
		gtk_menu_shell_append (GTK_MENU_SHELL (readdMenu), readdItems.back ());
		g_signal_connect (G_OBJECT (readdItems.back ()), "activate", G_CALLBACK (onReAddSourceClicked_gui), (gpointer)this);

		removeItems.push_back (gtk_menu_item_new_with_label (names[i].c_str ()));
		gtk_menu_shell_append (GTK_MENU_SHELL (removeMenu), removeItems.back ());
		g_signal_connect (G_OBJECT (removeItems.back ()), "activate", G_CALLBACK (onRemoveSourceClicked_gui), (gpointer)this);

		removeallItems.push_back (gtk_menu_item_new_with_label (names[i].c_str ()));
		gtk_menu_shell_append (GTK_MENU_SHELL (removeallMenu), removeallItems.back ());
		g_signal_connect (G_OBJECT (removeallItems.back ()), "activate", G_CALLBACK (onRemoveUserFromQueueClicked_gui), (gpointer)this);
	}
	gtk_widget_show_all (GTK_WIDGET (browseMenu));
	gtk_widget_show_all (GTK_WIDGET (pmMenu));
	gtk_widget_show_all (GTK_WIDGET (readdMenu));
	gtk_widget_show_all (GTK_WIDGET (removeMenu));
	gtk_widget_show_all (GTK_WIDGET (removeallMenu));
}

void DownloadQueue::setPriority_client (string target, QueueItem::Priority p)
{
	pthread_mutex_lock (&queueLock);
	QueueManager::getInstance ()->setPriority(target, p);
	pthread_mutex_unlock (&queueLock);
}
void DownloadQueue::setDirPriority_gui (string path, QueueItem::Priority p)
{
	vector<string> iter;
	getChildren (path, &iter);
	for (int i=0;i<iter.size ();i++)
		setPriority_client (iter[i], p);
}

void DownloadQueue::setDirPriorityClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->dirView.get();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection (v);
	GtkTreeIter iter;
	
	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	if (item == GTK_MENU_ITEM (q->dirItems["Paused"]))
		q->setDirPriority_gui(q->dirView.getString(&iter, "Path"), QueueItem::PAUSED);
	else if (item == GTK_MENU_ITEM (q->dirItems["Lowest"]))
		q->setDirPriority_gui(q->dirView.getString(&iter, "Path"), QueueItem::LOWEST);
	else if (item == GTK_MENU_ITEM (q->dirItems["Low"]))
		q->setDirPriority_gui(q->dirView.getString(&iter, "Path"), QueueItem::LOW);
	else if (item == GTK_MENU_ITEM (q->dirItems["Normal"]))
		q->setDirPriority_gui(q->dirView.getString(&iter, "Path"), QueueItem::NORMAL);
	else if (item == GTK_MENU_ITEM (q->dirItems["High"]))
		q->setDirPriority_gui(q->dirView.getString(&iter, "Path"), QueueItem::HIGH);
	else if (item == GTK_MENU_ITEM (q->dirItems["Highest"]))
		q->setDirPriority_gui(q->dirView.getString(&iter, "Path"), QueueItem::HIGHEST);
	
}
void DownloadQueue::setFilePriorityClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView.get();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection (v);
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

	for (int i=0;i<iter.size (); i++)
	{
		if (item == GTK_MENU_ITEM (q->fileItems["Paused"]))
			q->setPriority_client(q->fileView.getValue<gpointer,QueueItemInfo*>(&iter[i], "Info")->getTarget(), QueueItem::PAUSED);
		else if (item == GTK_MENU_ITEM (q->fileItems["Lowest"]))
			q->setPriority_client(q->fileView.getValue<gpointer,QueueItemInfo*>(&iter[i], "Info")->getTarget(), QueueItem::LOWEST);
		else if (item == GTK_MENU_ITEM (q->fileItems["Low"]))
			q->setPriority_client(q->fileView.getValue<gpointer,QueueItemInfo*>(&iter[i], "Info")->getTarget(), QueueItem::LOW);
		else if (item == GTK_MENU_ITEM (q->fileItems["Normal"]))
			q->setPriority_client(q->fileView.getValue<gpointer,QueueItemInfo*>(&iter[i], "Info")->getTarget(), QueueItem::NORMAL);
		else if (item == GTK_MENU_ITEM (q->fileItems["High"]))
			q->setPriority_client(q->fileView.getValue<gpointer,QueueItemInfo*>(&iter[i], "Info")->getTarget(), QueueItem::HIGH);
		else if (item == GTK_MENU_ITEM (q->fileItems["Highest"]))
			q->setPriority_client(q->fileView.getValue<gpointer,QueueItemInfo*>(&iter[i], "Info")->getTarget(), QueueItem::HIGHEST);
	}
}

void DownloadQueue::remove_client (string path)
{
	pthread_mutex_lock (&queueLock);
	QueueManager::getInstance ()->remove (path);
	pthread_mutex_unlock (&queueLock);
}
void DownloadQueue::removeDirClicked_gui (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (((DownloadQueue*)user_data)->dirStore);
	TreeView &view = ((DownloadQueue*)user_data)->dirView;
	selection = gtk_tree_view_get_selection(((DownloadQueue*)user_data)->dirView.get());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;	

	vector<string> iters;
	((DownloadQueue*)user_data)->getChildren(view.getString(&iter, "Path"), &iters);
	for (int i=iters.size ()-1; i>=0; i--)
		((DownloadQueue*)user_data)->remove_client (iters[i]);
}
void DownloadQueue::removeFileClicked_gui (GtkMenuItem *menuitem, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (((DownloadQueue*)user_data)->fileStore);
	selection = gtk_tree_view_get_selection(((DownloadQueue*)user_data)->fileView.get());
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
		q->remove_client((q->fileView.getValue<gpointer,QueueItemInfo*>(&iter[i], "Info"))->getTarget());
}

void DownloadQueue::onSearchAlternatesClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView.get();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection (v);
	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<GtkTreeIter> iter;
	GtkTreeIter tmpiter;
	
	if (!tmp)
		return;
	if (gtk_tree_model_get_iter (m, &tmpiter, (GtkTreePath*)tmp->data))
		iter.push_back (tmpiter);
	
	QueueItemInfo *ii = q->fileView.getValue<gpointer,QueueItemInfo*>(&iter[0], "Info");
	string target = q->fileView.getString(&iter[0], "Filename");
	string searchString = SearchManager::clean(target);
		
	if(!searchString.empty()) 
	{
		bool bigFile = (ii->getSize() > 10*1024*1024);
		Search *s = WulforManager::get ()->addSearch_gui ();
		if(bigFile)
			s->putValue (searchString, ii->getSize ()-1, SearchManager::SIZE_ATLEAST, ShareManager::getInstance()->getType(target));
		else
			s->putValue (searchString, ii->getSize ()+1, SearchManager::SIZE_ATMOST, ShareManager::getInstance()->getType(target));
	}
}

void DownloadQueue::onSearchByTTHClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView.get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection (v);
	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<GtkTreeIter> iter;
	GtkTreeIter tmpiter;
	
	if (!tmp)
		return;
	if (gtk_tree_model_get_iter (m, &tmpiter, (GtkTreePath*)tmp->data))
		iter.push_back (tmpiter);
		
	QueueItemInfo *ii = q->fileView.getValue<gpointer,QueueItemInfo*>(&iter[0], "Info");
		
	Search *s = WulforManager::get ()->addSearch_gui ();
	s->putValue (ii->getTTH ()->toBase32(), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
}
void DownloadQueue::onGetFileListClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView.get();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection (v);
	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	string name = getTextFromMenu (item);
	
	if (!tmp)
		return;
	
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return;
		
	QueueItemInfo *ii = q->fileView.getValue<gpointer,QueueItemInfo*>(&iter, "Info");	
	for (QueueItemInfo::SourceIter it=ii->getSources ().begin (); it != ii->getSources ().end (); it++)
		if (it->getUser ()->getFirstNick () == name)
		{
			ii->getUserList (name);
			break;
		}
}
void DownloadQueue::onSendPrivateMessageClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView.get();
	GtkTreeModel *m = gtk_tree_view_get_model(v);
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection(v);
	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	string name = getTextFromMenu (item);
	
	if (!tmp)
		return;
	
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return;
		
	QueueItemInfo *ii = q->fileView.getValue<gpointer,QueueItemInfo*>(&iter, "Info");	
	for (QueueItemInfo::SourceIter it=ii->getSources ().begin (); it != ii->getSources ().end (); it++)
		if (it->getUser()->getFirstNick() == name)
		{
			ii->sendPrivateMessage (name);
			break;
		}
}
void DownloadQueue::onReAddSourceClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView.get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection (v);
	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	string name = getTextFromMenu (item);
	
	if (!tmp)
		return;
	
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return;
		
	QueueItemInfo *ii = q->fileView.getValue<gpointer,QueueItemInfo*>(&iter, "Info");
	for (QueueItemInfo::SourceIter it=ii->getSources ().begin (); it != ii->getSources ().end (); it++)
		if (it->getUser()->getFirstNick() == name)
		{
			ii->reAddSource (name, q);
			break;
		}
}
void DownloadQueue::onRemoveSourceClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView.get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection (v);
	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	string name = getTextFromMenu (item);
	
	if (!tmp)
		return;
	
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return;

	QueueItemInfo *ii = q->fileView.getValue<gpointer,QueueItemInfo*>(&iter, "Info");
	for (QueueItemInfo::SourceIter it=ii->getSources ().begin (); it != ii->getSources ().end (); it++)
		if (it->getUser()->getFirstNick() == name)
		{
			ii->removeSource (name);
			break;
		}
}
void DownloadQueue::onRemoveUserFromQueueClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView.get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection (v);
	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	string name = getTextFromMenu (item);
	
	if (!tmp)
		return;
	
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return;
			
	QueueItemInfo *ii = q->fileView.getValue<gpointer,QueueItemInfo*>(&iter, "Info");
		for (QueueItemInfo::SourceIter it=ii->getSources ().begin (); it != ii->getSources ().end (); it++)
		if (it->getUser()->getFirstNick() == name)
		{
			ii->removeSource(name);
			break;
		}
}

gboolean DownloadQueue::dir_onButtonPressed_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	((DownloadQueue*)user_data)->dirPrevious = event->type;
	return FALSE;
}
gboolean DownloadQueue::dir_onButtonReleased_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if (((DownloadQueue*)user_data)->dirPrevious == GDK_BUTTON_PRESS)
    	{
		if (event->button == 3)
		{
        		GtkTreeSelection *selection;

        		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
		        if (gtk_tree_selection_count_selected_rows(selection)  == 1)
	  		{
				((DownloadQueue*)user_data)->dir_popup_menu_gui(event, user_data);			
			}
		}
		if (event->button == 1)
		{
			WulforManager::get ()->dispatchGuiFunc (new Func0<DownloadQueue>((DownloadQueue*)user_data, &DownloadQueue::update_gui));
			WulforManager::get ()->dispatchGuiFunc (new Func0<DownloadQueue>((DownloadQueue*)user_data, &DownloadQueue::updateStatus_gui));
		}
	}

	return FALSE;
}
gboolean DownloadQueue::dir_onPopupMenu_gui (GtkWidget *widget, gpointer user_data)
{
	((DownloadQueue*)user_data)->dir_popup_menu_gui(NULL, user_data);
	return TRUE;
}
void DownloadQueue::dir_popup_menu_gui (GdkEventButton *event, gpointer user_data)
{
    gtk_menu_popup(GTK_MENU(dirMenu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? 1 : 0,
                   gdk_event_get_time((GdkEvent*)event));
}

gboolean DownloadQueue::file_onButtonPressed_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if (event->type == GDK_BUTTON_PRESS)
    	{
		if (event->button == 3)
		{
        		GtkTreeSelection *selection;

        		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
		        if (gtk_tree_selection_count_selected_rows(selection)  > 0)
	  		{
				((DownloadQueue*)user_data)->buildDynamicMenu_gui ();
				((DownloadQueue*)user_data)->file_popup_menu_gui(event, user_data);			
				return TRUE;
			}
		}
	}
	return FALSE;	
}
gboolean DownloadQueue::file_onPopupMenu_gui (GtkWidget *widget, gpointer user_data)
{
	((DownloadQueue*)user_data)->file_popup_menu_gui(NULL, user_data);
	return TRUE;
}
void DownloadQueue::file_popup_menu_gui (GdkEventButton *event, gpointer user_data)
{
    gtk_menu_popup(GTK_MENU(fileMenu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? 1 : 0,
                   gdk_event_get_time((GdkEvent*)event));
}

void DownloadQueue::buildList_gui ()
{
	pthread_mutex_lock (&queueLock);
	const QueueItem::StringMap &ll = QueueManager::getInstance ()->lockQueue ();
	pthread_mutex_unlock (&queueLock);
	GtkTreeIter row;
	queueItems = 0;
	queueSize = 0;
	GtkTreeModel *m = GTK_TREE_MODEL (dirStore);
	for (QueueItem::StringMap::const_iterator it = ll.begin (); it != ll.end (); it++)
	{
		string realpath = "/" + getNextSubDir (Util::getFilePath(it->second->getTarget())) + "/";
		if (dirMap.find (realpath) == dirMap.end ())
		{
			gtk_tree_store_append (dirStore, &row, NULL);
			gtk_tree_store_set (	dirStore, &row, 
						dirView.col("Dir"), Text::acpToUtf8(getNextSubDir(Util::getFilePath(it->second->getTarget()))).c_str(),
						dirView.col("Path"), realpath.c_str(),
						-1);
			dirMap[realpath] = row;
			string tmp;
			addDir_gui (getRemainingDir (Util::getFilePath(it->second->getTarget())), &row, tmp);
			fileMap[it->second->getTarget ()] = it->second;
			addFile_gui (new QueueItemInfo (it->second), tmp);
		}
		else
		{
			string tmp;
			addDir_gui (getRemainingDir (Util::getFilePath(it->second->getTarget())), &dirMap[realpath], tmp);
			fileMap[it->second->getTarget ()] = it->second;
			addFile_gui (new QueueItemInfo (it->second), tmp);
		}
	}
	gtk_tree_view_expand_all (dirView.get ());
	pthread_mutex_lock (&queueLock);
	QueueManager::getInstance()->unlockQueue();
	pthread_mutex_unlock (&queueLock);
}
void DownloadQueue::addDir_gui (string path, GtkTreeIter *row, string &current)
{
	GtkTreeIter newRow;
	string tmp = getNextSubDir (path);
	GtkTreeModel *m = GTK_TREE_MODEL (dirStore);
	string rowdata = dirView.getString(row, "Path");
	string realpath = rowdata + tmp + "/";
	if (tmp == "")
	{
		current = rowdata;
		return;
	}
		
	if (dirMap.find (realpath) == dirMap.end ())
	{
		gtk_tree_store_append (dirStore, &newRow, row);
		gtk_tree_store_set (	dirStore, &newRow,
					dirView.col("Dir"), Text::acpToUtf8(tmp).c_str(),
					dirView.col("Path"), realpath.c_str(),
					-1);
		dirMap[realpath] = newRow;
		addDir_gui (getRemainingDir (path), &newRow, current);
	}
	else
		addDir_gui (getRemainingDir (path), &dirMap[realpath], current);
}
void DownloadQueue::addFile_gui (QueueItemInfo *i, string path)
{
	queueSize+=i->getSize();
	queueItems++;
	dirFileMap[path].push_back (i);
}

string DownloadQueue::getTrailingSubDir (string path)
{
	if (path == "")
		return "";
	string tmp = path.substr (0, path.find_last_of ('/', path.size ()-2));
	return tmp + "/";
}

string DownloadQueue::getNextSubDir (string path)
{
	string tmp = path.substr (1, path.find_first_of ('/', 1));
	if (tmp != "")
	{
		tmp.erase (tmp.size ()-1, 1);
		return tmp;
	}
	return tmp;
}
string DownloadQueue::getRemainingDir (string path)
{
	string tmp = path.substr (path.find_first_of ('/', 1), path.size ()-path.find_first_of ('/', 1));
	return tmp;
}

void DownloadQueue::getChildren (string path, vector<GtkTreeIter> *iter)
{
	if (dirMap.find (path) == dirMap.end ())
		return;
	GtkTreeModel *m = gtk_tree_view_get_model (dirView.get ());
	GtkTreeIter it;
	if (!gtk_tree_model_iter_children (m, &it, &dirMap[path]))
		return;
	while (1)
	{
		iter->push_back (it);
		getChildren(dirView.getString(&it, "Path"), iter);
		if (!gtk_tree_model_iter_next (m, &it))
			break;
	}
}
void DownloadQueue::getChildren (string path, vector<string> *target)
{
	if (dirMap.find (path) == dirMap.end ())
		return;


	if (target->empty ())
	{
		for (vector<QueueItemInfo*>::iterator it=dirFileMap[path].begin (); it != dirFileMap[path].end (); it++)
 			target->push_back ((*it)->getTarget ());
	}
	GtkTreeModel *m = gtk_tree_view_get_model (dirView.get ());
	GtkTreeIter it;
	if (!gtk_tree_model_iter_children (m, &it, &dirMap[path]))
		return;
	while (1)
	{
		string lp = dirView.getString(&it, "Path");
		for (vector<QueueItemInfo*>::iterator it2=dirFileMap[lp].begin (); it2 != dirFileMap[lp].end (); it2++)
			target->push_back ((*it2)->getTarget ());
		getChildren (lp, target);
		if (!gtk_tree_model_iter_next (m, &it))
			break;
	}	
}
void DownloadQueue::QueueItemInfo::update (DownloadQueue *dq, bool add)
{
	if (!add)
	{
		bool found=false;
		GtkTreeIter it;
		if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (dq->fileStore), &it))
			return;
		while (1)
		{	
			if (dq->fileView.getValue<gpointer,QueueItemInfo*>(&it, "Info") == this)
			{
				found = true;
				break;	
			}
			if (!gtk_tree_model_iter_next(GTK_TREE_MODEL(dq->fileStore), &it))
				break;	
		}
		if (!found)
			return;	
			
		// Users
		int online=0;
		{
			string tmp;
			QueueItemInfo::SourceIter j;
			for(j = getSources().begin(); j != getSources().end(); ++j)
			{
				if(tmp.size() > 0)
					tmp += ", ";
	
					if(j->getUser()->isOnline())
						online++;
	
					tmp += WulforUtil::getNicks(j->getUser());
			}
			gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Users"), string (tmp.empty() ? "No users" : tmp).c_str (), -1);
		}		
		// Status
		{
			if(getStatus() == QueueItem::STATUS_WAITING)
			{
				char buf[64];
				if(online > 0)
				{
					if(getSources().size() == 1)
						sprintf (buf, "Waiting (User online)");
					else
						sprintf(buf, "Waiting (%d of %d users online)", online, getSources().size());
					gtk_list_store_set (dq->fileStore, &it, dq->fileView.col("Status"), buf, -1);
				}
				else
				{
					if(getSources().size() == 0)
						sprintf (buf, "No users to download from");
					else if(getSources().size() == 1)
						sprintf (buf, "User offline");
					else if(getSources().size() == 2)
						sprintf (buf, "Both users offline");
					else
						sprintf(buf, "All %d users offline", getSources().size());
					gtk_list_store_set (dq->fileStore, &it, dq->fileView.col("Status"), buf, -1);
				}
			}
			else if(getStatus() == QueueItem::STATUS_RUNNING)
				gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Status"), "Running...", -1);
		}
		// Size
		{
			gtk_list_store_set (dq->fileStore, &it,  dq->fileView.col("Size"), (getSize() == -1) ? "Unkown" : Util::formatBytes(getSize()).c_str (), 
								dq->fileView.col("Exact Size"), (getSize() == -1) ? "Unkown" : Util::formatExactSize(getSize()).c_str (),
								dq->fileView.col("Real Size"), getSize (), -1);
		}
		// Downloaded
		{
			if(getSize() > 0)
				gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Downloaded"), string(Util::formatBytes(getDownloadedBytes()) + " (" + Util::toString((double)getDownloadedBytes()*100.0/(double)getSize()) + "%)").c_str(), -1);
			else
				gtk_list_store_set (dq->fileStore, &it, dq->fileView.col("Downloaded"), "", -1);
			
			gtk_list_store_set (dq->fileStore, &it, dq->fileView.col("Download Size"), getSize (), -1);
		}
		// Priority
		{
			switch(getPriority())
			{
				case QueueItem::PAUSED:
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Priority"), "Paused", -1);
					break;
				case QueueItem::LOWEST:
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Priority"), "Lowest", -1);
					break;
				case QueueItem::LOW:
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Priority"), "Low", -1);
					break;
				case QueueItem::NORMAL:
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Priority"), "Normal", -1);
					break;
				case QueueItem::HIGH:
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Priority"), "High", -1);
					break;
				case QueueItem::HIGHEST:
					gtk_list_store_set (dq->fileStore, &it, dq->fileView.col("Priority"), "Highest", -1);
					break;
			}
		}
		// Path
		{
			gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Path"), Util::getFilePath(getTarget()).c_str(), -1);
		}
		// Errors
		{
			string tmp;
			QueueItemInfo::SourceIter j;
			for(j = getBadSources().begin(); j != getBadSources().end(); ++j)
			{
				if(!j->isSet(QueueItem::Source::FLAG_REMOVED))
				{
					if(tmp.size() > 0)
						tmp += ", ";
					tmp += WulforUtil::getNicks(j->getUser());
					tmp += " (";
					if(j->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE))
						tmp += "File not available";
					else if(j->isSet(QueueItem::Source::FLAG_PASSIVE))
						tmp += "Passive user";
					else if(j->isSet(QueueItem::Source::FLAG_ROLLBACK_INCONSISTENCY))
						tmp += "Rollback inconsistency, existing file does not match the one being downloaded";
					else if(j->isSet(QueueItem::Source::FLAG_CRC_FAILED))
						tmp += "CRC32 inconsistency (SFV-Check)";
					else if(j->isSet(QueueItem::Source::FLAG_BAD_TREE))
						tmp += "Downloaded tree does not match TTH root";
					tmp += ")";
				}
			}
			gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Error"), string(tmp.empty() ? "No errors" : tmp).c_str(), -1);
		}	
		// Added
		{
			gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Added"), Util::formatTime("%Y-%m-%d %H:%M", getAdded()).c_str(), -1);
		}		
		// TTH
		{
			if (getTTH () != NULL)
				gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("TTH"), string(getTTH()->toBase32()).c_str(), -1);
		}		
	}
	else
	{
		GtkTreeIter it;
		gtk_list_store_append (dq->fileStore, &it);
		// Item
		gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Info"), (gpointer)this, -1);
		gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Filename"), Text::acpToUtf8(Util::getFileName(getTarget())).c_str(), -1);
		// Users
		int online=0;
		{
			string tmp;
			QueueItemInfo::SourceIter j;
			for(j = getSources().begin(); j != getSources().end(); ++j)
			{
				if(tmp.size() > 0)
					tmp += ", ";
	
					if(j->getUser()->isOnline())
						online++;
	
					tmp += WulforUtil::getNicks(j->getUser());
			}
			gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Users"), string(tmp.empty() ? "No users" : tmp).c_str(), -1);
		}
		// Status
		{
			if(getStatus() == QueueItem::STATUS_WAITING)
			{
				char buf[64];
				if(online > 0)
				{
					if(getSources().size() == 1)
						sprintf (buf, "Waiting (User online)");
					else if (getSources().size() < 0)
						sprintf(buf, "Waiting (%d users online)", online);
					else
						sprintf(buf, "Waiting (%d of %d users online)", online, getSources().size());
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Status"), buf, -1);
				}
				else
				{
					if(getSources().size() == 0)
						sprintf (buf, "No users to download from");
					else if(getSources().size() == 1)
						sprintf (buf, "User offline");
					else if(getSources().size() == 2)
						sprintf (buf, "Both users offline");
					else
						sprintf(buf, "All %d users offline", getSources().size());
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Status"), buf, -1);
				}
			}
			else if(getStatus() == QueueItem::STATUS_RUNNING)
				gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Status"), "Running...", -1);
		}
		// Size
		{
			gtk_list_store_set(dq->fileStore, &it,  dq->fileView.col("Size"), (getSize() == -1) ? "Unkown" : Util::formatBytes(getSize()).c_str(), 
								dq->fileView.col("Exact Size"), (getSize() == -1) ? "Unkown" : Util::formatExactSize(getSize()).c_str(),
								dq->fileView.col("Real Size"), getSize(), -1);
		}
		// Downloaded
		{
			if(getSize() > 0)
				gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Downloaded"), string(Util::formatBytes(getDownloadedBytes()) + " (" + Util::toString((double)getDownloadedBytes()*100.0/(double)getSize()) + "%)").c_str(), -1);
			else
				gtk_list_store_set (dq->fileStore, &it, dq->fileView.col("Downloaded"), "", -1);
				
			gtk_list_store_set (dq->fileStore, &it, dq->fileView.col("Download Size"), getSize (), -1);
		}
		// Priority
		{
			switch(getPriority())
			{
				case QueueItem::PAUSED:
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Priority"), "Paused", -1);
					break;
				case QueueItem::LOWEST:
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Priority"), "Lowest", -1);
					break;
				case QueueItem::LOW:
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Priority"), "Low", -1);
					break;
				case QueueItem::NORMAL:
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Priority"), "Normal", -1);
					break;
				case QueueItem::HIGH:
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Priority"), "High", -1);
					break;
				case QueueItem::HIGHEST:
					gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Priority"), "Highest", -1);
					break;
			}
		}
		// Path
		{
			gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Path"), Util::getFilePath(getTarget()).c_str(), -1);
		}
		// Errors
		{
			string tmp;
			QueueItemInfo::SourceIter j;
			for(j = getBadSources().begin(); j != getBadSources().end(); ++j)
			{
				if(!j->isSet(QueueItem::Source::FLAG_REMOVED))
				{
					if(tmp.size() > 0)
						tmp += ", ";
					tmp += WulforUtil::getNicks(j->getUser());
					tmp += " (";
					if(j->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE))
						tmp += "File not available";
					else if(j->isSet(QueueItem::Source::FLAG_PASSIVE))
						tmp += "Passive user";
					else if(j->isSet(QueueItem::Source::FLAG_ROLLBACK_INCONSISTENCY))
						tmp += "Rollback inconsistency, existing file does not match the one being downloaded";
					else if(j->isSet(QueueItem::Source::FLAG_CRC_FAILED))
						tmp += "CRC32 inconsistency (SFV-Check)";
					else if(j->isSet(QueueItem::Source::FLAG_BAD_TREE))
						tmp += "Downloaded tree does not match TTH root";
					tmp += ")";
				}
			}
			gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Error"), string(tmp.empty() ? "No errors" : tmp).c_str(), -1);
		}	
		// Added
		{
			gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("Added"), Util::formatTime("%Y-%m-%d %H:%M", getAdded()).c_str(), -1);
		}	
		// TTH
		{
			if (getTTH () != NULL)
				gtk_list_store_set(dq->fileStore, &it, dq->fileView.col("TTH"), string (getTTH()->toBase32()).c_str(), -1);
		}
	}
}
void DownloadQueue::updateStatus_gui ()
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (dirStore);
	selection = gtk_tree_view_get_selection(dirView.get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
	{
		setStatus_gui ("Items: 0", STATUS_ITEMS);
		setStatus_gui ("Size: 0 B", STATUS_FILE_SIZE);
		setStatus_gui ("Files: " + Util::toString (queueItems), STATUS_FILES);
		setStatus_gui ("Size: " + Util::formatBytes (queueSize), STATUS_TOTAL_SIZE);
		return;
	}
	string path = dirView.getString(&iter, "Path");
	if (dirFileMap[path].empty ())
	{
		setStatus_gui ("Items: 0", STATUS_ITEMS);
		setStatus_gui ("Size: 0 B", STATUS_FILE_SIZE);
		setStatus_gui ("Files: " + Util::toString (queueItems), STATUS_FILES);
		setStatus_gui ("Size: " + Util::formatBytes (queueSize), STATUS_TOTAL_SIZE);
		return;
	}

	int64_t total=0;
	int count=0;

	{
		if (dirFileMap.find (path) == dirFileMap.end ())
			return;
		for (vector<QueueItemInfo*>::iterator it=dirFileMap[path].begin (); it != dirFileMap[path].end (); it++)
		{
			count++;
			total += ((*it)->getSize() > 0) ? (*it)->getSize() : 0;
		}
	}

	setStatus_gui ("Items: " + Util::toString (count), STATUS_ITEMS);
	setStatus_gui ("Size: " + Util::formatBytes (total), STATUS_FILE_SIZE);
	setStatus_gui ("Files: " + Util::toString (queueItems), STATUS_FILES);
	setStatus_gui ("Size: " + Util::formatBytes (queueSize), STATUS_TOTAL_SIZE);
}
void DownloadQueue::update_gui ()
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (dirStore);
	selection = gtk_tree_view_get_selection(dirView.get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;


	showingDir = dirView.getString(&iter, "Path");
	gtk_list_store_clear (fileStore);
	if (dirFileMap.find (showingDir) == dirFileMap.end ())
		return;

	for (vector<QueueItemInfo*>::iterator it=dirFileMap[showingDir].begin (); it != dirFileMap[showingDir].end (); it++)
		(*it)->update (this, true);
}
void DownloadQueue::updateItem_gui (QueueItemInfo *i, bool add)
{
	i->update (this, add);
}

void DownloadQueue::setStatus_gui (string text, int num)
{
	if (num<0 || num>STATUS_LAST-1) return;

	gtk_statusbar_pop(GTK_STATUSBAR (statusbar[num]), 0);
	if (num == 0)
		gtk_statusbar_push(GTK_STATUSBAR (statusbar[num]), 0, string ("[" + Util::getShortTimeString() + "] " + text).c_str ());
	else
		gtk_statusbar_push(GTK_STATUSBAR (statusbar[num]), 0, text.c_str ());	
}
int DownloadQueue::countFiles_gui (string path)
{
	if (!dirFileMap[path].empty ())
		return 1;
		
	vector<GtkTreeIter> iter;
	getChildren (path, &iter);
	for (int i=0; i<iter.size ();i++)
	{
		string rp = dirView.getString(&iter[i], "Path");
		if (dirFileMap.find (rp) == dirFileMap.end ())
		{
			if (!dirFileMap[rp].empty ())
				return 1;
		}
		else
			return 1;
	}

	return 0;
}
void DownloadQueue::removeDir_gui (string path)
{
	if (path == "" || path == "/")
		return;

	if (countFiles_gui (path) == 0)
	{
		if (showingDir == path)
			gtk_list_store_clear (fileStore);

		dirFileMap.erase (dirFileMap.find (path));
		if (dirMap.find (path) != dirMap.end ())
		{
			gtk_tree_store_remove (dirStore, &dirMap[path]);
			dirMap.erase (dirMap.find (path));
		}
		removeDir_gui (getTrailingSubDir (path));
	}
	else
		return;
}
void DownloadQueue::removeFile_gui (string target)
{
	string path = Util::getFilePath(target);
	if (dirFileMap.find (path) == dirFileMap.end ())
		return;
	for (vector<QueueItemInfo*>::iterator it=dirFileMap[path].begin (); it != dirFileMap[path].end ();)
	{
		if ((*it)->getTarget() == target)
		{
			queueSize-=(*it)->getSize();
			queueItems--;
			if (fileMap.find (target) != fileMap.end ())
				fileMap.erase (fileMap.find (target));
			dirFileMap[path].erase (it);
			break;
		}
		else
			it++;
	}
	if (dirFileMap[path].empty ())
		removeDir_gui (path);
		
	update_gui();
	updateStatus_gui();
}

void DownloadQueue::on(QueueManagerListener::Added, QueueItem* aQI) throw()
{
	WulforManager::get()->dispatchGuiFunc(new Func1<DownloadQueue, QueueItem*>(this, &DownloadQueue::queueItemAdded_gui, aQI));
}

void DownloadQueue::queueItemAdded_gui(QueueItem *aQI)
{
	GtkTreeIter row;
	GtkTreeModel *m = GTK_TREE_MODEL (dirStore);
	QueueItemInfo *i = new QueueItemInfo (aQI);
	string realpath = "/" + getNextSubDir (Util::getFilePath(aQI->getTarget())) + "/";
	if (dirMap.find (realpath) == dirMap.end ())
	{
		gtk_tree_store_append (dirStore, &row, NULL);
		gtk_tree_store_set (	dirStore, &row, 
					dirView.col("Dir"), getNextSubDir (Util::getFilePath(aQI->getTarget())).c_str(),
					dirView.col("Path"), realpath.c_str(),
					-1);
		dirMap[realpath] = row;
		string tmp;
		addDir_gui(getRemainingDir (Util::getFilePath(aQI->getTarget())), &row, tmp);
		fileMap[aQI->getTarget ()] = aQI;
		addFile_gui(i, tmp);
	}
	else
	{
		string tmp;
		addDir_gui(getRemainingDir (Util::getFilePath(aQI->getTarget())), &dirMap[realpath], tmp);
		fileMap[aQI->getTarget ()] = aQI;
		addFile_gui(i, tmp);
	}
	gtk_tree_view_expand_all (dirView.get ());
	contentUpdated_gui();
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	selection = gtk_tree_view_get_selection(dirView.get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
	
	if (showingDir == dirView.getString(&iter, "Path"))
		updateItem_gui(i, true);
}

void DownloadQueue::on(QueueManagerListener::Removed, QueueItem* aQI) throw()
{
	WulforManager::get()->dispatchGuiFunc(new Func1<DownloadQueue, string>(this, &DownloadQueue::removeFile_gui, aQI->getTarget()));
	WulforManager::get()->dispatchGuiFunc(new Func0<DownloadQueue>(this, &DownloadQueue::contentUpdated_gui));
}

void DownloadQueue::on(QueueManagerListener::Finished, QueueItem* aQI, int64_t avSpeed) throw()
{
	
}

void DownloadQueue::on(QueueManagerListener::Moved, QueueItem* aQI) throw()
{
	///@todo: implement
	WulforManager::get()->dispatchGuiFunc(new Func0<DownloadQueue>(this, &DownloadQueue::contentUpdated_gui));
}

void DownloadQueue::updateFiles_gui (QueueItem *aQI)
{
	QueueItemInfo* ii = NULL;
	string path = Util::getFilePath (aQI->getTarget ());
	if (dirFileMap.find (path) == dirFileMap.end ())
		return;
	for (vector<QueueItemInfo*>::iterator it=dirFileMap[path].begin (); it != dirFileMap[path].end (); it++)
		if ((*it)->getTarget () == aQI->getTarget ())
		{
			ii = *it;
			break;
		}

	if (!ii)
		return;

	ii->setPriority(aQI->getPriority());
	ii->setStatus(aQI->getStatus());
	ii->setDownloadedBytes(aQI->getDownloadedBytes());
	ii->setTTH(aQI->getTTH());
	for(QueueItemInfo::SourceIter i = ii->getSources().begin(); i != ii->getSources().end(); )
	{
		if(!aQI->isSource(i->getUser()))
			i = ii->getSources().erase(i);
		else
			++i;
	}
	for(QueueItem::Source::Iter j = aQI->getSources().begin(); j != aQI->getSources().end(); ++j)
		if(!ii->isSource((*j)->getUser()))
			ii->getSources().push_back(QueueItemInfo::SourceInfo(*(*j)));
	for(QueueItemInfo::SourceIter i = ii->getBadSources().begin(); i != ii->getBadSources().end(); )
	{
		if(!aQI->isBadSource(i->getUser()))
			i = ii->getBadSources().erase(i);
		else
			++i;
	}
	for(QueueItem::Source::Iter j = aQI->getBadSources().begin(); j != aQI->getBadSources().end(); ++j)
		if(!ii->isBadSource((*j)->getUser()))
			ii->getBadSources().push_back(QueueItemInfo::SourceInfo(*(*j)));
	if (showingDir == path)
		WulforManager::get ()->dispatchGuiFunc (new Func2<DownloadQueue, QueueItemInfo*, bool> (this, &DownloadQueue::updateItem_gui, ii, false));
	WulforManager::get ()->dispatchGuiFunc (new Func0<DownloadQueue> (this, &DownloadQueue::updateStatus_gui));
}
void DownloadQueue::QueueItemInfo::sendPrivateMessage (string text)
{
	for(SourceIter i = sources.begin(); i != sources.end(); ++i)
		if(i->getUser()->getFirstNick() == text)
			WulforManager::get()->addPrivMsg_gui(i->getUser ());
}	
