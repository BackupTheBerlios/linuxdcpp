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

#include "downloadqueue.hh"
#include "wulformanager.hh"
#include "search.hh"
#include <iostream>
#include <sstream>

using namespace std;

int DownloadQueue::columnSize[] = { 200, 100, 75, 110, 75, 200, 200, 75, 200, 100, 125 };

string DownloadQueue::getTextFromMenu (GtkMenuItem *item)
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

void DownloadQueue::buildStaticMenu_gui ()
{
	dirPriority = GTK_MENU (gtk_menu_new ());
	dirItems["Paused"] = gtk_menu_item_new_with_label ("Paused");
	gtk_menu_shell_append (GTK_MENU_SHELL (dirPriority), dirItems["Paused"]);
	g_signal_connect(G_OBJECT (dirItems["Paused"]), "activate", G_CALLBACK(setDirPriorityClicked_gui), (gpointer)this);
	dirItems["Lowest"] = gtk_menu_item_new_with_label ("Lowest");
	gtk_menu_shell_append (GTK_MENU_SHELL (dirPriority), dirItems["Lowest"]);
	g_signal_connect(G_OBJECT (dirItems["Lowest"]), "activate", G_CALLBACK(setDirPriorityClicked_gui), (gpointer)this);
	dirItems["Low"] = gtk_menu_item_new_with_label ("Low");
	gtk_menu_shell_append (GTK_MENU_SHELL (dirPriority), dirItems["Low"]);
	g_signal_connect(G_OBJECT (dirItems["Low"]), "activate", G_CALLBACK(setDirPriorityClicked_gui), (gpointer)this);
	dirItems["Normal"] = gtk_menu_item_new_with_label ("Normal");
	gtk_menu_shell_append (GTK_MENU_SHELL (dirPriority), dirItems["Normal"]);
	g_signal_connect(G_OBJECT (dirItems["Normal"]), "activate", G_CALLBACK(setDirPriorityClicked_gui), (gpointer)this);
	dirItems["High"] = gtk_menu_item_new_with_label ("High");
	gtk_menu_shell_append (GTK_MENU_SHELL (dirPriority), dirItems["High"]);
	g_signal_connect(G_OBJECT (dirItems["High"]), "activate", G_CALLBACK(setDirPriorityClicked_gui), (gpointer)this);
	dirItems["Highest"] = gtk_menu_item_new_with_label ("Highest");
	gtk_menu_shell_append (GTK_MENU_SHELL (dirPriority), dirItems["Highest"]);
	g_signal_connect(G_OBJECT (dirItems["Highest"]), "activate", G_CALLBACK(setDirPriorityClicked_gui), (gpointer)this);

	dirItems["Set priority"] = gtk_menu_item_new_with_label ("Set priority");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (dirItems["Set priority"]), GTK_WIDGET (dirPriority));

	dirMenu = GTK_MENU (gtk_menu_new ());
	gtk_menu_shell_append (GTK_MENU_SHELL (dirMenu), dirItems["Set priority"]);
	gtk_menu_shell_append (GTK_MENU_SHELL (dirMenu), gtk_separator_menu_item_new ());
	dirItems["Move/Rename"] = gtk_menu_item_new_with_label ("Move/Rename");
	gtk_menu_shell_append (GTK_MENU_SHELL (dirMenu), dirItems["Move/Rename"]);
	gtk_menu_shell_append (GTK_MENU_SHELL (dirMenu), gtk_separator_menu_item_new ());
	dirItems["Remove"] = gtk_menu_item_new_with_label ("Remove");
	gtk_menu_shell_append (GTK_MENU_SHELL (dirMenu), dirItems["Remove"]);
	g_signal_connect(G_OBJECT (dirItems["Remove"]), "activate", G_CALLBACK(removeDirClicked_gui), (gpointer)this);
	gtk_widget_show_all (GTK_WIDGET (dirMenu));
	
	filePriority = GTK_MENU (gtk_menu_new ());
	fileItems["Paused"] = gtk_menu_item_new_with_label ("Paused");
	gtk_menu_shell_append (GTK_MENU_SHELL (filePriority), fileItems["Paused"]);
	g_signal_connect(G_OBJECT (fileItems["Paused"]), "activate", G_CALLBACK(setFilePriorityClicked_gui), (gpointer)this);
	fileItems["Lowest"] = gtk_menu_item_new_with_label ("Lowest");
	gtk_menu_shell_append (GTK_MENU_SHELL (filePriority), fileItems["Lowest"]);
	g_signal_connect(G_OBJECT (fileItems["Lowest"]), "activate", G_CALLBACK(setFilePriorityClicked_gui), (gpointer)this);
	fileItems["Low"] = gtk_menu_item_new_with_label ("Low");
	gtk_menu_shell_append (GTK_MENU_SHELL (filePriority), fileItems["Low"]);
	g_signal_connect(G_OBJECT (fileItems["Low"]), "activate", G_CALLBACK(setFilePriorityClicked_gui), (gpointer)this);
	fileItems["Normal"] = gtk_menu_item_new_with_label ("Normal");
	gtk_menu_shell_append (GTK_MENU_SHELL (filePriority), fileItems["Normal"]);
	g_signal_connect(G_OBJECT (fileItems["Normal"]), "activate", G_CALLBACK(setFilePriorityClicked_gui), (gpointer)this);
	fileItems["High"] = gtk_menu_item_new_with_label ("High");
	gtk_menu_shell_append (GTK_MENU_SHELL (filePriority), fileItems["High"]);
	g_signal_connect(G_OBJECT (fileItems["High"]), "activate", G_CALLBACK(setFilePriorityClicked_gui), (gpointer)this);
	fileItems["Highest"] = gtk_menu_item_new_with_label ("Highest");
	gtk_menu_shell_append (GTK_MENU_SHELL (filePriority), fileItems["Highest"]);
	g_signal_connect(G_OBJECT (fileItems["Highest"]), "activate", G_CALLBACK(setFilePriorityClicked_gui), (gpointer)this);
	fileItems["Set priority"] = gtk_menu_item_new_with_label ("Set priority");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (fileItems["Set priority"]), GTK_WIDGET (filePriority));

	fileMenu = GTK_MENU (gtk_menu_new ());
	browseMenu = GTK_MENU (gtk_menu_new ());
	pmMenu = GTK_MENU (gtk_menu_new ());
	readdMenu = GTK_MENU (gtk_menu_new ());
	removeMenu = GTK_MENU (gtk_menu_new ());
	removeallMenu = GTK_MENU (gtk_menu_new ());
	fileItems["Search for alternates"] = gtk_menu_item_new_with_label ("Search for alternates");
	gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), fileItems["Search for alternates"]);
	g_signal_connect (G_OBJECT (fileItems["Search for alternates"]), "activate", G_CALLBACK (onSearchAlternatesClicked_gui), (gpointer)this);
	fileItems["Search by TTH"] = gtk_menu_item_new_with_label ("Search by TTH");
	gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), fileItems["Search by TTH"]);
	g_signal_connect (G_OBJECT (fileItems["Search by TTH"]), "activate", G_CALLBACK (onSearchByTTHClicked_gui), (gpointer)this);
	gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), gtk_separator_menu_item_new ());
	gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), fileItems["Set priority"]);
	fileItems["Get file list"] = gtk_menu_item_new_with_label ("Get file list");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (fileItems["Get file list"]), GTK_WIDGET (browseMenu));
	gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), fileItems["Get file list"]);
	fileItems["Send private message"] = gtk_menu_item_new_with_label ("Send private message");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (fileItems["Send private message"]), GTK_WIDGET (pmMenu));
	gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), fileItems["Send private message"]);
	fileItems["Re-add source"] = gtk_menu_item_new_with_label ("Re-add source");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (fileItems["Re-add source"]), GTK_WIDGET (readdMenu));
	gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), fileItems["Re-add source"]);
	gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), gtk_separator_menu_item_new ());	
	fileItems["Remove source"] = gtk_menu_item_new_with_label ("Remove source");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (fileItems["Remove source"]), GTK_WIDGET (removeMenu));
	gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), fileItems["Remove source"]);
	fileItems["Remove user from queue"] = gtk_menu_item_new_with_label ("Remove user from queue");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (fileItems["Remove user from queue"]), GTK_WIDGET (removeallMenu));
	gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), fileItems["Remove user from queue"]);
	fileItems["Remove"] = gtk_menu_item_new_with_label ("Remove");
	gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), fileItems["Remove"]);
	g_signal_connect(G_OBJECT (fileItems["Remove"]), "activate", G_CALLBACK(removeFileClicked_gui), (gpointer)this);
	gtk_widget_show_all (GTK_WIDGET (fileMenu));
}

void DownloadQueue::buildDynamicMenu_gui ()
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (fileStore);
	selection = gtk_tree_view_get_selection(fileView->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;	

	QueueItemInfo *i = TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO);

	if (i->getTTH ())
		gtk_widget_set_sensitive (fileItems["Search by TTH"], true);
	else
		gtk_widget_set_sensitive (fileItems["Search by TTH"], false);
	
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
	for (QueueItemInfo::SourceIter it=i->getSources ().begin (); it != i->getSources ().end (); it++)
	{
		browseItems.push_back (gtk_menu_item_new_with_label (string (it->getUser()->getFullNick()).c_str ()));
		gtk_menu_shell_append (GTK_MENU_SHELL (browseMenu), browseItems.back ());
		g_signal_connect (G_OBJECT (browseItems.back ()), "activate", G_CALLBACK (onGetFileListClicked_gui), (gpointer)this);

		pmItems.push_back (gtk_menu_item_new_with_label (string (it->getUser()->getFullNick()).c_str ()));
		gtk_menu_shell_append (GTK_MENU_SHELL (pmMenu), pmItems.back ());
		g_signal_connect (G_OBJECT (pmItems.back ()), "activate", G_CALLBACK (onSendPrivateMessageClicked_gui), (gpointer)this);

		readdItems.push_back (gtk_menu_item_new_with_label (string (it->getUser()->getFullNick()).c_str ()));
		gtk_menu_shell_append (GTK_MENU_SHELL (readdMenu), readdItems.back ());
		g_signal_connect (G_OBJECT (readdItems.back ()), "activate", G_CALLBACK (onReAddSourceClicked_gui), (gpointer)this);

		removeItems.push_back (gtk_menu_item_new_with_label (string (it->getUser()->getFullNick()).c_str ()));
		gtk_menu_shell_append (GTK_MENU_SHELL (removeMenu), removeItems.back ());
		g_signal_connect (G_OBJECT (removeItems.back ()), "activate", G_CALLBACK (onRemoveSourceClicked_gui), (gpointer)this);

		removeallItems.push_back (gtk_menu_item_new_with_label (string (it->getUser()->getFullNick()).c_str ()));
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
	//Lock l(cs);
	//pthread_mutex_lock (&queueLock);
	QueueManager::getInstance ()->setPriority(target, p);
	//pthread_mutex_unlock (&queueLock);
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
	GtkTreeView *v = q->dirView->get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	selection = gtk_tree_view_get_selection (v);

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
	if (item == GTK_MENU_ITEM (q->dirItems["Paused"]))
		q->setDirPriority_gui (TreeViewFactory::getValue<gchar*,string>(m, &iter, DIRCOLUMN_REALPATH), QueueItem::PAUSED);
	else if (item == GTK_MENU_ITEM (q->dirItems["Lowest"]))
		q->setDirPriority_gui (TreeViewFactory::getValue<gchar*,string>(m, &iter, DIRCOLUMN_REALPATH), QueueItem::LOWEST);
	else if (item == GTK_MENU_ITEM (q->dirItems["Low"]))
		q->setDirPriority_gui (TreeViewFactory::getValue<gchar*,string>(m, &iter, DIRCOLUMN_REALPATH), QueueItem::LOW);
	else if (item == GTK_MENU_ITEM (q->dirItems["Normal"]))
		q->setDirPriority_gui (TreeViewFactory::getValue<gchar*,string>(m, &iter, DIRCOLUMN_REALPATH), QueueItem::NORMAL);
	else if (item == GTK_MENU_ITEM (q->dirItems["High"]))
		q->setDirPriority_gui (TreeViewFactory::getValue<gchar*,string>(m, &iter, DIRCOLUMN_REALPATH), QueueItem::HIGH);
	else if (item == GTK_MENU_ITEM (q->dirItems["Highest"]))
		q->setDirPriority_gui (TreeViewFactory::getValue<gchar*,string>(m, &iter, DIRCOLUMN_REALPATH), QueueItem::HIGHEST);
	
}
void DownloadQueue::setFilePriorityClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView->get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	selection = gtk_tree_view_get_selection (v);
	
	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	if (item == GTK_MENU_ITEM (q->fileItems["Paused"]))
		q->setPriority_client (TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO)->getTarget (), QueueItem::PAUSED);
	else if (item == GTK_MENU_ITEM (q->fileItems["Lowest"]))
		q->setPriority_client (TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO)->getTarget (), QueueItem::LOWEST);
	else if (item == GTK_MENU_ITEM (q->fileItems["Low"]))
		q->setPriority_client (TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO)->getTarget (), QueueItem::LOW);
	else if (item == GTK_MENU_ITEM (q->fileItems["Normal"]))
		q->setPriority_client (TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO)->getTarget (), QueueItem::NORMAL);
	else if (item == GTK_MENU_ITEM (q->fileItems["High"]))
		q->setPriority_client (TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO)->getTarget (), QueueItem::HIGH);
	else if (item == GTK_MENU_ITEM (q->fileItems["Highest"]))
		q->setPriority_client (TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO)->getTarget (), QueueItem::HIGHEST);
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
	selection = gtk_tree_view_get_selection(((DownloadQueue*)user_data)->dirView->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;	

	vector<string> iters;
	((DownloadQueue*)user_data)->getChildren (TreeViewFactory::getValue<gchar*,string>(m, &iter, DIRCOLUMN_REALPATH), &iters);
	for (int i=iters.size ()-1; i>=0; i--)
	{	
		 ((DownloadQueue*)user_data)->remove_client(iters[i]);
	}
}
void DownloadQueue::removeFileClicked_gui (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (((DownloadQueue*)user_data)->fileStore);
	selection = gtk_tree_view_get_selection(((DownloadQueue*)user_data)->fileView->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;	

	 ((DownloadQueue*)user_data)->remove_client((TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO))->getTarget ());
}
DownloadQueue::DownloadQueue(GCallback closeCallback):
	BookEntry(WulforManager::DOWNLOAD_QUEUE, "", "Download Queue", closeCallback)
{
	QueueManager::getInstance()->addListener(this);
	
	string file = WulforManager::get()->getPath() + "/glade/downloadqueue.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "downloadQueueWindow");
	mainBox = glade_xml_get_widget(xml, "downloadQueueBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	GtkTreeView *tdir = GTK_TREE_VIEW(glade_xml_get_widget(xml, "dirView"));
	GtkTreeView *tfile = GTK_TREE_VIEW(glade_xml_get_widget(xml, "fileView"));
	statusbar.push_back (glade_xml_get_widget (xml, "statusMain"));
	statusbar.push_back (glade_xml_get_widget (xml, "statusItems"));
	statusbar.push_back (glade_xml_get_widget (xml, "statusFileSize"));
	statusbar.push_back (glade_xml_get_widget (xml, "statusFiles"));
	statusbar.push_back (glade_xml_get_widget (xml, "statusTotalSize"));
	
	dirStore = gtk_tree_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model (tdir, GTK_TREE_MODEL (dirStore));

	gtk_widget_set_events (GTK_WIDGET (tdir), GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
 	g_signal_connect(G_OBJECT (tdir), "button_press_event", G_CALLBACK(dir_onButtonPressed_gui), (gpointer)this);
   	g_signal_connect(G_OBJECT (tdir), "button_release_event", G_CALLBACK(dir_onButtonReleased_gui), (gpointer)this);
    	g_signal_connect(G_OBJECT (tdir), "popup_menu", G_CALLBACK(dir_onPopupMenu_gui), (gpointer)this);
	dirView = new TreeViewFactory (tdir);
	dirView->addColumn_gui (DIRCOLUMN_DIR, "Directory", TreeViewFactory::STRING, -1);


	fileStore = gtk_list_store_new (12, 	G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
						G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	gtk_tree_view_set_model(tfile, GTK_TREE_MODEL(fileStore));
    	g_signal_connect(G_OBJECT (tfile), "button_press_event", G_CALLBACK(file_onButtonPressed_gui), (gpointer)this);
    	g_signal_connect(G_OBJECT (tfile), "button_release_event", G_CALLBACK(file_onButtonReleased_gui), (gpointer)this);
    	g_signal_connect(G_OBJECT (tfile), "popup_menu", G_CALLBACK(file_onPopupMenu_gui), (gpointer)this);
   	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(tfile), GTK_SELECTION_SINGLE);
	fileView = new TreeViewFactory (tfile);
	fileView->addColumn_gui(COLUMN_TARGET, "Filename", TreeViewFactory::STRING, columnSize[COLUMN_TARGET]);
	fileView->addColumn_gui(COLUMN_STATUS, "Status", TreeViewFactory::STRING, columnSize[COLUMN_STATUS]);
	fileView->addColumn_gui(COLUMN_SIZE, "Size", TreeViewFactory::STRING, columnSize[COLUMN_SIZE]);
	fileView->addColumn_gui(COLUMN_DOWNLOADED, "Downloaded", TreeViewFactory::STRING, columnSize[COLUMN_DOWNLOADED]);
	fileView->addColumn_gui(COLUMN_PRIORITY, "Priority", TreeViewFactory::STRING, columnSize[COLUMN_PRIORITY]);
	fileView->addColumn_gui(COLUMN_USERS, "Users", TreeViewFactory::STRING, columnSize[COLUMN_USERS]);
	fileView->addColumn_gui(COLUMN_PATH, "Path", TreeViewFactory::STRING, columnSize[COLUMN_PATH]);
	fileView->addColumn_gui(COLUMN_EXACT_SIZE, "Exact size", TreeViewFactory::STRING, columnSize[COLUMN_EXACT_SIZE]);
	fileView->addColumn_gui(COLUMN_ERRORS, "Error", TreeViewFactory::STRING, columnSize[COLUMN_ERRORS]);
	fileView->addColumn_gui(COLUMN_ADDED, "Added", TreeViewFactory::STRING, columnSize[COLUMN_ADDED]);
	fileView->addColumn_gui(COLUMN_TTH, "TTH", TreeViewFactory::STRING, columnSize[COLUMN_TTH]);

	buildStaticMenu_gui ();
	pthread_mutex_init(&queueLock, NULL);
}

DownloadQueue::~DownloadQueue() 
{
	QueueManager::getInstance()->removeListener(this);
	pthread_mutex_destroy(&queueLock);
}

GtkWidget *DownloadQueue::getWidget() 
{
	return mainBox;
}

void DownloadQueue::onSearchAlternatesClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView->get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	selection = gtk_tree_view_get_selection (v);
	
	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
	QueueItemInfo *ii = TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO);
	string target = TreeViewFactory::getValue<gchar*,string>(m, &iter, COLUMN_TARGET);
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
	GtkTreeView *v = q->fileView->get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	selection = gtk_tree_view_get_selection (v);
	
	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
	QueueItemInfo *ii = TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO);
		
	Search *s = WulforManager::get ()->addSearch_gui ();
	s->putValue (ii->getTTH ()->toBase32(), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
}
void DownloadQueue::onGetFileListClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView->get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	selection = gtk_tree_view_get_selection (v);
	
	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
	QueueItemInfo *ii = TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO);	
	
	ii->getUserList (getTextFromMenu (item));
}
void DownloadQueue::onSendPrivateMessageClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView->get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	selection = gtk_tree_view_get_selection (v);
	
	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
	QueueItemInfo *ii = TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO);	
	
	ii->sendPrivateMessage (getTextFromMenu (item));
}
void DownloadQueue::onReAddSourceClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView->get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	selection = gtk_tree_view_get_selection (v);
	
	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
	QueueItemInfo *ii = TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO);	
	
	ii->reAddSource (getTextFromMenu (item), q);
}
void DownloadQueue::onRemoveSourceClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView->get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	selection = gtk_tree_view_get_selection (v);
	
	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
	QueueItemInfo *ii = TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO);	
	
	ii->removeSource (getTextFromMenu (item));
}
void DownloadQueue::onRemoveUserFromQueueClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	DownloadQueue *q = (DownloadQueue*)user_data;
	GtkTreeView *v = q->fileView->get ();
	GtkTreeModel *m = gtk_tree_view_get_model (v);
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	selection = gtk_tree_view_get_selection (v);
	
	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
	QueueItemInfo *ii = TreeViewFactory::getValue<gpointer,QueueItemInfo*>(m, &iter, COLUMN_INFO);	
	
	ii->removeSources (getTextFromMenu (item));
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
			((DownloadQueue*)user_data)->update_gui ();
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
	((DownloadQueue*)user_data)->filePrevious = event->type;
	return FALSE;
}
gboolean DownloadQueue::file_onButtonReleased_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if (((DownloadQueue*)user_data)->filePrevious == GDK_BUTTON_PRESS)
    	{
		if (event->button == 3)
		{
        		GtkTreeSelection *selection;

        		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
		        if (gtk_tree_selection_count_selected_rows(selection)  == 1)
	  		{
				((DownloadQueue*)user_data)->buildDynamicMenu_gui ();
				((DownloadQueue*)user_data)->file_popup_menu_gui(event, user_data);			
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
	Lock l(cs);
	const QueueItem::StringMap &ll = QueueManager::getInstance ()->lockQueue ();
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
						DIRCOLUMN_DIR, getNextSubDir (Util::getFilePath(it->second->getTarget())).c_str (),
						DIRCOLUMN_REALPATH, realpath.c_str (),
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
	gtk_tree_view_expand_all (dirView->get ());
	QueueManager::getInstance()->unlockQueue();
	pthread_mutex_unlock (&queueLock);
}
void DownloadQueue::addDir_gui (string path, GtkTreeIter *row, string &current)
{
	GtkTreeIter newRow;
	string tmp = getNextSubDir (path);
	GtkTreeModel *m = GTK_TREE_MODEL (dirStore);
	string rowdata = TreeViewFactory::getValue<gchar*,string> (m, row, DIRCOLUMN_REALPATH);
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
					DIRCOLUMN_DIR, tmp.c_str (),
					DIRCOLUMN_REALPATH, realpath.c_str (),
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
	GtkTreeModel *m = gtk_tree_view_get_model (dirView->get ());
	GtkTreeIter it;
	if (!gtk_tree_model_iter_children (m, &it, &dirMap[path]))
		return;
	while (1)
	{
		iter->push_back (it);
		getChildren (TreeViewFactory::getValue<gchar*,string> (m, &it, DIRCOLUMN_REALPATH), iter);
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
	GtkTreeModel *m = gtk_tree_view_get_model (dirView->get ());
	GtkTreeIter it;
	if (!gtk_tree_model_iter_children (m, &it, &dirMap[path]))
		return;
	while (1)
	{
		string lp = TreeViewFactory::getValue<gchar*,string> (m, &it, DIRCOLUMN_REALPATH);
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
			if (TreeViewFactory::getValue<gpointer,QueueItemInfo*>(GTK_TREE_MODEL (dq->fileStore), &it, COLUMN_INFO) == this)
			{
				found = true;
				break;	
			}
			if (!gtk_tree_model_iter_next (GTK_TREE_MODEL (dq->fileStore), &it))
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
	
					tmp += j->getUser()->getFullNick();
			}
			gtk_list_store_set (dq->fileStore, &it, COLUMN_USERS, string (tmp.empty() ? "No users" : tmp).c_str (), -1);
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
					gtk_list_store_set (dq->fileStore, &it, COLUMN_STATUS, buf, -1);
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
					gtk_list_store_set (dq->fileStore, &it, COLUMN_STATUS, buf, -1);
				}
			}
			else if(getStatus() == QueueItem::STATUS_RUNNING)
				gtk_list_store_set (dq->fileStore, &it, COLUMN_STATUS, "Running...", -1);
		}
		// Size
		{
			gtk_list_store_set (dq->fileStore, &it,  COLUMN_SIZE, (getSize() == -1) ? "Unkown" : Util::formatBytes(getSize()).c_str (), 
								COLUMN_EXACT_SIZE, (getSize() == -1) ? "Unkown" : Util::formatExactSize(getSize()).c_str (), -1);
		}
		// Downloaded
		{
			if(getSize() > 0)
				gtk_list_store_set (dq->fileStore, &it, COLUMN_DOWNLOADED, string (Util::formatBytes(getDownloadedBytes()) + " (" + Util::toString((double)getDownloadedBytes()*100.0/(double)getSize()) + "%)").c_str (), -1);
			else
				gtk_list_store_set (dq->fileStore, &it, COLUMN_DOWNLOADED, "", -1);
		}
		// Priority
		{
			switch(getPriority())
			{
				case QueueItem::PAUSED: gtk_list_store_set (dq->fileStore, &it, COLUMN_PRIORITY, "Paused", -1); break;
				case QueueItem::LOWEST: gtk_list_store_set (dq->fileStore, &it, COLUMN_PRIORITY, "Lowest", -1); break;
				case QueueItem::LOW: gtk_list_store_set (dq->fileStore, &it, COLUMN_PRIORITY, "Low", -1); break;
				case QueueItem::NORMAL: gtk_list_store_set (dq->fileStore, &it, COLUMN_PRIORITY, "Normal", -1); break;
				case QueueItem::HIGH: gtk_list_store_set (dq->fileStore, &it, COLUMN_PRIORITY, "High", -1); break;
				case QueueItem::HIGHEST: gtk_list_store_set (dq->fileStore, &it, COLUMN_PRIORITY, "Highest", -1); break;
			}
		}
		// Path
		{
			gtk_list_store_set (dq->fileStore, &it, COLUMN_PATH, Util::getFilePath (getTarget ()).c_str (), -1);
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
					tmp += j->getUser()->getNick();
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
			gtk_list_store_set (dq->fileStore, &it, COLUMN_ERRORS, string (tmp.empty() ? "No errors" : tmp).c_str (), -1);
		}	
		// Added
		{
			gtk_list_store_set (dq->fileStore, &it, COLUMN_ADDED, Util::formatTime("%Y-%m-%d %H:%M", getAdded()).c_str ());
		}		
		// TTH
		{
			if (getTTH () != NULL)
				gtk_list_store_set (dq->fileStore, &it, COLUMN_TTH, string (getTTH ()->toBase32 ()).c_str (), -1);
		}		
	}
	else
	{
		GtkTreeIter it;
		gtk_list_store_append (dq->fileStore, &it);
		// Item
		gtk_list_store_set (dq->fileStore, &it, COLUMN_INFO, (gpointer)this, -1);
		gtk_list_store_set (dq->fileStore, &it, COLUMN_TARGET, Util::getFileName (getTarget()).c_str (), -1);
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
	
					tmp += j->getUser()->getFullNick();
			}
			gtk_list_store_set (dq->fileStore, &it, COLUMN_USERS, string (tmp.empty() ? "No users" : tmp).c_str (), -1);
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
					gtk_list_store_set (dq->fileStore, &it, COLUMN_STATUS, buf, -1);
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
					gtk_list_store_set (dq->fileStore, &it, COLUMN_STATUS, buf, -1);
				}
			}
			else if(getStatus() == QueueItem::STATUS_RUNNING)
				gtk_list_store_set (dq->fileStore, &it, COLUMN_STATUS, "Running...", -1);
		}
		// Size
		{
			gtk_list_store_set (dq->fileStore, &it,  COLUMN_SIZE, (getSize() == -1) ? "Unkown" : Util::formatBytes(getSize()).c_str (), 
								COLUMN_EXACT_SIZE, (getSize() == -1) ? "Unkown" : Util::formatExactSize(getSize()).c_str (), -1);
		}
		// Downloaded
		{
			if(getSize() > 0)
				gtk_list_store_set (dq->fileStore, &it, COLUMN_DOWNLOADED, string (Util::formatBytes(getDownloadedBytes()) + " (" + Util::toString((double)getDownloadedBytes()*100.0/(double)getSize()) + "%)").c_str (), -1);
			else
				gtk_list_store_set (dq->fileStore, &it, COLUMN_DOWNLOADED, "", -1);
		}
		// Priority
		{
			switch(getPriority())
			{
				case QueueItem::PAUSED: gtk_list_store_set (dq->fileStore, &it, COLUMN_PRIORITY, "Paused", -1); break;
				case QueueItem::LOWEST: gtk_list_store_set (dq->fileStore, &it, COLUMN_PRIORITY, "Lowest", -1); break;
				case QueueItem::LOW: gtk_list_store_set (dq->fileStore, &it, COLUMN_PRIORITY, "Low", -1); break;
				case QueueItem::NORMAL: gtk_list_store_set (dq->fileStore, &it, COLUMN_PRIORITY, "Normal", -1); break;
				case QueueItem::HIGH: gtk_list_store_set (dq->fileStore, &it, COLUMN_PRIORITY, "High", -1); break;
				case QueueItem::HIGHEST: gtk_list_store_set (dq->fileStore, &it, COLUMN_PRIORITY, "Highest", -1); break;
			}
		}
		// Path
		{
			gtk_list_store_set (dq->fileStore, &it, COLUMN_PATH, Util::getFilePath (getTarget ()).c_str (), -1);
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
					tmp += j->getUser()->getNick();
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
			gtk_list_store_set (dq->fileStore, &it, COLUMN_ERRORS, string (tmp.empty() ? "No errors" : tmp).c_str (), -1);
		}	
		// Added
		{
			gtk_list_store_set (dq->fileStore, &it, COLUMN_ADDED, Util::formatTime("%Y-%m-%d %H:%M", getAdded()).c_str ());
		}	
		// TTH
		{
			if (getTTH () != NULL)
				gtk_list_store_set (dq->fileStore, &it, COLUMN_TTH, string (getTTH ()->toBase32 ()).c_str (), -1);
		}
	}
}
void DownloadQueue::updateStatus_gui ()
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (dirStore);
	selection = gtk_tree_view_get_selection(dirView->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
	{
		setStatus_gui ("Items: 0", STATUS_ITEMS);
		setStatus_gui ("Size: 0 B", STATUS_FILE_SIZE);
		setStatus_gui ("Files: " + Util::toString (queueItems), STATUS_FILES);
		setStatus_gui ("Size: " + Util::formatBytes (queueSize), STATUS_TOTAL_SIZE);
		return;
	}
	string path = TreeViewFactory::getValue<gchar*,string>(GTK_TREE_MODEL(dirStore), &iter, DIRCOLUMN_REALPATH);
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
	selection = gtk_tree_view_get_selection(dirView->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;


	showingDir = TreeViewFactory::getValue<gchar*,string>(m, &iter, DIRCOLUMN_REALPATH);
	gtk_list_store_clear (fileStore);
	if (dirFileMap.find (showingDir) == dirFileMap.end ())
	{
		updateStatus_gui ();
		return;
	}
	for (vector<QueueItemInfo*>::iterator it=dirFileMap[showingDir].begin (); it != dirFileMap[showingDir].end (); it++)
	{
		(*it)->update (this, true);			
	}
	updateStatus_gui ();
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
		string rp = TreeViewFactory::getValue<gchar*,string>(GTK_TREE_MODEL (dirStore), &iter[i], DIRCOLUMN_REALPATH);
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
	update_gui ();
}

void DownloadQueue::on(QueueManagerListener::Added, QueueItem* aQI) throw()
{
	cout << aQI->getTarget () << endl;
	GtkTreeIter row;
	GtkTreeModel *m = GTK_TREE_MODEL (dirStore);
	QueueItemInfo *i = new QueueItemInfo (aQI);
	string realpath = "/" + getNextSubDir (Util::getFilePath(aQI->getTarget())) + "/";
	if (dirMap.find (realpath) == dirMap.end ())
	{
		gtk_tree_store_append (dirStore, &row, NULL);
		gtk_tree_store_set (	dirStore, &row, 
					DIRCOLUMN_DIR, getNextSubDir (Util::getFilePath(aQI->getTarget())).c_str (),
					DIRCOLUMN_REALPATH, realpath.c_str (),
					-1);
		dirMap[realpath] = row;
		string tmp;
		addDir_gui (getRemainingDir (Util::getFilePath(aQI->getTarget())), &row, tmp);
		fileMap[aQI->getTarget ()] = aQI;
		addFile_gui (i, tmp);
	}
	else
	{
		string tmp;
		addDir_gui (getRemainingDir (Util::getFilePath(aQI->getTarget())), &dirMap[realpath], tmp);
		fileMap[aQI->getTarget ()] = aQI;
		addFile_gui (i, tmp);
	}
	gtk_tree_view_expand_all (dirView->get ());
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	selection = gtk_tree_view_get_selection(dirView->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
	
	if (showingDir == TreeViewFactory::getValue<gchar*,string>(m, &iter, DIRCOLUMN_REALPATH))
		i->update (this, true);
}

void DownloadQueue::on(QueueManagerListener::Removed, QueueItem* aQI) throw()
{
	removeFile_gui (aQI->getTarget ());
}
void DownloadQueue::on(QueueManagerListener::Finished, QueueItem* aQI) throw()
{

}
void DownloadQueue::on(QueueManagerListener::Moved, QueueItem* aQI) throw()
{
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
	ii->update (this, false);
	updateStatus_gui ();
}
void DownloadQueue::QueueItemInfo::sendPrivateMessage (string text)
{
	for(SourceIter i = sources.begin(); i != sources.end(); ++i)
		if(i->getUser()->getFullNick () == text)			
			WulforManager::get()->addPrivMsg_gui(i->getUser ());
}	
