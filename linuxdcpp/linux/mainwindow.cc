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

#include "mainwindow.hh"
#include "wulformanager.hh"
#include "selecter.hh"
#include "settingsdialog.hh"
#include "treeview.hh"
#include "settingsmanager.hh"

#include <client/version.h>
#include <client/Socket.h>
#include <client/Client.h>
#include <client/SettingsManager.h>
#include <client/SearchManager.h>
#include <client/Exception.h>

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

MainWindow::MainWindow():
	connectCallback(this, &MainWindow::connectClicked_gui),
	pubHubsCallback(this, &MainWindow::pubHubsClicked_gui),
	dlQueueCallback(this, &MainWindow::dlQueueClicked_gui),
	settingsCallback(this, &MainWindow::settingsClicked_gui),
	favHubsCallback(this, &MainWindow::favHubsClicked_gui),
	searchCallback(this, &MainWindow::searchClicked_gui),
	hashCallback(this, &MainWindow::hashClicked_gui),
	aboutCallback(this, &MainWindow::aboutClicked_gui),
	quitCallback(this, &MainWindow::quitClicked_gui),
	finishedDL_Callback(this, &MainWindow::finishedDLclicked_gui),
	finishedUL_Callback(this, &MainWindow::finishedULclicked_gui),		
	deleteCallback(this, &MainWindow::deleteWindow_gui),
//	transferCallback(this, &MainWindow::transferClicked_gui),
	switchPageCallback(this, &MainWindow::switchPage_gui),
	openFListCallback(this, &MainWindow::openFList_gui),
	refreshFListCallback(this, &MainWindow::refreshFList_gui),
	lastUpdate(0)
{
	createWindow_gui();
	startSocket_client();

	QueueManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	LogManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
	ConnectionManager::getInstance()->addListener(this);
}

MainWindow::~MainWindow() {
	QueueManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	LogManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);
	ConnectionManager::getInstance()->removeListener(this);

	//Save window state and position
	int posX, posY, sizeX, sizeY, state, transferPanePosition;
	GdkWindowState gdkState;
	WulforSettingsManager *sm = WulforSettingsManager::get();

	gtk_window_get_position(window, &posX, &posY);
	gtk_window_get_size(window, &sizeX, &sizeY);
	gdkState = gdk_window_get_state(GTK_WIDGET(window)->window);
	transferPanePosition = gtk_paned_get_position(transferPane);

	if (gdkState & GDK_WINDOW_STATE_MAXIMIZED) {
		state = 1;
	} else {
		state = 0;
		//The get pos/size functions return junk when window is maximized
		sm->set("main-window-pos-x", posX);
		sm->set("main-window-pos-y", posY);
		sm->set("main-window-size-x", sizeX);
		sm->set("main-window-size-y", sizeY);
	}
	
	sm->set("main-window-maximized", state);
	sm->set("transfer-pane-position", transferPanePosition);

	//Make sure all windows are deallocated (probably not necessary)
	gtk_widget_destroy(GTK_WIDGET(connectDialog));
	gtk_widget_destroy(GTK_WIDGET(exitDialog));
	gtk_widget_destroy(GTK_WIDGET(flistDialog));
	gtk_widget_destroy(GTK_WIDGET(aboutDialog));
	gtk_widget_destroy(GTK_WIDGET(window));

	//this makes sure the pixmaps are freed (using gtk:s ref counting)
	g_object_unref(G_OBJECT(uploadPic));
	g_object_unref(G_OBJECT(downloadPic));
}

void MainWindow::createWindow_gui() {
	string file = WulforManager::get()->getPath() + "/glade/mainwindow.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	mainStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "status1"));
	hubStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "status2"));
	slotStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "status3"));
	dTotStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "status4"));
	uTotStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "status5"));
	ulStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "status6"));
	dlStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "status7"));

	connectButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "connect"));
	pubHubsButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "publicHubs"));
	searchButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "search"));
	settingsButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "settings"));
	hashButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "hash"));
	queueButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "queue"));
	finishedDL_button = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "finishedDownloads"));
	finishedUL_button = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "finishedUploads"));
	favHubsButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "favHubs"));
	quitButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "quit"));

	exitDialog = GTK_DIALOG(glade_xml_get_widget(xml, "exitDialog"));
	connectDialog = GTK_DIALOG(glade_xml_get_widget(xml, "connectDialog"));
	flistDialog = GTK_DIALOG(glade_xml_get_widget(xml, "flistDialog"));
	aboutDialog = GTK_DIALOG(glade_xml_get_widget(xml, "aboutDialog"));

	window = GTK_WINDOW(glade_xml_get_widget(xml, "mainWindow"));
	transferPane = GTK_PANED(glade_xml_get_widget(xml, "pane"));
	book = GTK_NOTEBOOK(glade_xml_get_widget(xml, "book"));
	connectEntry = GTK_ENTRY(glade_xml_get_widget(xml, "connectEntry"));

	openFList = GTK_MENU_ITEM(glade_xml_get_widget(xml, "open_file_list_1"));
	openOwnFList = GTK_MENU_ITEM(glade_xml_get_widget(xml, "open_own_list1"));
	refreshFList = GTK_MENU_ITEM(glade_xml_get_widget(xml, "refresh_file_list1"));
	openDLdir = GTK_MENU_ITEM(glade_xml_get_widget(xml, "open_downloads_directory1"));
	quickConnect = GTK_MENU_ITEM(glade_xml_get_widget(xml, "quick_connect1"));
	followRedirect = GTK_MENU_ITEM(glade_xml_get_widget(xml, "follow_last_redirect1"));
	reconnectItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "reconnect1"));
	settingsItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "settings1"));
	quitItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "exit1"));
	
	pubHubsItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "public_hubs1"));
	queueItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "download_queue1"));
	finishedDL_item = GTK_MENU_ITEM(glade_xml_get_widget(xml, "finished_downloads1"));
	finishedUL_item = GTK_MENU_ITEM(glade_xml_get_widget(xml, "finished_uploads1"));
	favHubsItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "favorite_hubs1"));
	favUsersItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "favorite_users1"));
	searchItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "search1"));
	ADLSearchItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "adl_search1"));
	searchSpyItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "search_spy1"));
	networkStatsItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "network_statistics1"));
	hashItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "indexing_progress1"));
	aboutItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "about"));

	gtk_widget_set_sensitive(GTK_WIDGET(openDLdir), false);
	gtk_widget_set_sensitive(GTK_WIDGET(followRedirect), false);
	gtk_widget_set_sensitive(GTK_WIDGET(reconnectItem), false);
	gtk_widget_set_sensitive(GTK_WIDGET(favUsersItem), false);
	gtk_widget_set_sensitive(GTK_WIDGET(ADLSearchItem), false);
	gtk_widget_set_sensitive(GTK_WIDGET(searchSpyItem), false);
	gtk_widget_set_sensitive(GTK_WIDGET(networkStatsItem), false);

	// Initialize transfer treeview
	transferView.setView(GTK_TREE_VIEW(glade_xml_get_widget(xml, "transfers")), true, "main");
	// column for transfer type icon; didn't need a title displayed so a space was used
	transferView.insertColumn(" ", GDK_TYPE_PIXBUF, TreeView::PIXBUF, 20);
	transferView.insertColumn("User", G_TYPE_STRING, TreeView::STRING, 150);
    if (SETTING(SHOW_PROGRESS_BARS))
		transferView.insertColumn("Status", G_TYPE_STRING, TreeView::PROGRESS, 250, "Progress");
	else
		transferView.insertColumn("Status", G_TYPE_STRING, TreeView::STRING, 250);
	transferView.insertColumn("Time Left", G_TYPE_STRING, TreeView::STRING, 75);
	transferView.insertColumn("Speed", G_TYPE_STRING, TreeView::STRING, 175);
	transferView.insertColumn("Filename", G_TYPE_STRING, TreeView::STRING, 200);
	transferView.insertColumn("Size", G_TYPE_STRING, TreeView::STRING, 175);
	transferView.insertColumn("Path", G_TYPE_STRING, TreeView::STRING, 200);
	transferView.insertHiddenColumn("ID", G_TYPE_STRING);
	transferView.insertHiddenColumn("User Ptr", G_TYPE_POINTER);
	if (SETTING(SHOW_PROGRESS_BARS))
		transferView.insertHiddenColumn("Progress", G_TYPE_INT);
	transferView.finalize();
	transferStore = gtk_list_store_newv(transferView.getColCount(), transferView.getGTypes());
	gtk_tree_view_set_model(transferView.get(), GTK_TREE_MODEL(transferStore));
	transferSel = gtk_tree_view_get_selection(transferView.get());
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(transferView.get()), GTK_SELECTION_MULTIPLE);

	g_signal_connect(G_OBJECT(transferView.get()), "button_press_event", G_CALLBACK(transferClicked_gui), (gpointer)this);

	file = WulforManager::get()->getPath() + "/pixmaps/upload.png";
	uploadPic = gdk_pixbuf_new_from_file(file.c_str(), NULL);
	file = WulforManager::get()->getPath() + "/pixmaps/download.png";
	downloadPic = gdk_pixbuf_new_from_file(file.c_str(), NULL);

	//All notebooks created in glade need one page.
	//In our case, this is just a placeholder, so we remove it.
	gtk_notebook_remove_page(book, -1);

	//We need to do this in the code and not in the .glade file,
	//otherwise we won't always find the images using binreloc
	file = WulforManager::get()->getPath() + "/pixmaps/publichubs.png";
	gtk_tool_button_set_icon_widget(pubHubsButton, 
		gtk_image_new_from_file(file.c_str()));
	file = WulforManager::get()->getPath() + "/pixmaps/search.png";
	gtk_tool_button_set_icon_widget(searchButton, 
		gtk_image_new_from_file(file.c_str()));
	file = WulforManager::get()->getPath() + "/pixmaps/settings.png";
	gtk_tool_button_set_icon_widget(settingsButton, 
		gtk_image_new_from_file(file.c_str()));
	file = WulforManager::get()->getPath() + "/pixmaps/FinishedDL.png";
	gtk_tool_button_set_icon_widget(finishedDL_button, 
		gtk_image_new_from_file(file.c_str()));
	file = WulforManager::get()->getPath() + "/pixmaps/FinishedUL.png";
	gtk_tool_button_set_icon_widget(finishedUL_button, 
		gtk_image_new_from_file(file.c_str()));

	file = WulforManager::get()->getPath() + "/pixmaps/queue.png";
	gtk_tool_button_set_icon_widget(queueButton, 
		gtk_image_new_from_file(file.c_str()));
	file = WulforManager::get()->getPath() + "/pixmaps/favhubs.png";
	gtk_tool_button_set_icon_widget(favHubsButton, 
		gtk_image_new_from_file(file.c_str()));
		
	popupMenu = gtk_menu_new ();
	filelist = gtk_menu_item_new_with_label ("Get file list");
	gtk_menu_shell_append (GTK_MENU_SHELL (popupMenu), filelist);
	g_signal_connect (G_OBJECT (filelist), "activate", G_CALLBACK (onGetFileListClicked_gui), (gpointer)this);
	matchQueue = gtk_menu_item_new_with_label ("Match queue");
	gtk_menu_shell_append (GTK_MENU_SHELL (popupMenu), matchQueue);
	g_signal_connect (G_OBJECT (matchQueue), "activate", G_CALLBACK (onMatchQueueClicked_gui), (gpointer)this);
	privateMessage = gtk_menu_item_new_with_label ("Send private message");
	gtk_menu_shell_append (GTK_MENU_SHELL (popupMenu), privateMessage);
	g_signal_connect (G_OBJECT (privateMessage), "activate", G_CALLBACK (onPrivateMessageClicked_gui), (gpointer)this);
	addToFavorites = gtk_menu_item_new_with_label ("Add to favorites");
	gtk_menu_shell_append (GTK_MENU_SHELL (popupMenu), addToFavorites);
	g_signal_connect (G_OBJECT (addToFavorites), "activate", G_CALLBACK (onAddFavoriteUserClicked_gui), (gpointer)this);
	grantExtraSlot = gtk_menu_item_new_with_label ("Grant extra slot");
	gtk_menu_shell_append (GTK_MENU_SHELL (popupMenu), grantExtraSlot);
	g_signal_connect (G_OBJECT (grantExtraSlot), "activate", G_CALLBACK (onGrantExtraSlotClicked_gui), (gpointer)this);
	removeUser = gtk_menu_item_new_with_label ("Remove user from queue");
	gtk_menu_shell_append (GTK_MENU_SHELL (popupMenu), removeUser);
	g_signal_connect (G_OBJECT (removeUser), "activate", G_CALLBACK (onRemoveUserFromQueueClicked_gui), (gpointer)this);
	gtk_menu_shell_append (GTK_MENU_SHELL (popupMenu), gtk_separator_menu_item_new ());
	forceAttempt = gtk_menu_item_new_with_label ("Force attempt");
	gtk_menu_shell_append (GTK_MENU_SHELL (popupMenu), forceAttempt);
	g_signal_connect (G_OBJECT (forceAttempt), "activate", G_CALLBACK (onForceAttemptClicked_gui), (gpointer)this);
	gtk_menu_shell_append (GTK_MENU_SHELL (popupMenu), gtk_separator_menu_item_new ());
	closeConnection = gtk_menu_item_new_with_label ("Close connection");
	g_signal_connect (G_OBJECT (closeConnection), "activate", G_CALLBACK (onCloseConnectionClicked_gui), (gpointer)this);
	gtk_menu_shell_append (GTK_MENU_SHELL (popupMenu), closeConnection);
		
	gtk_widget_show_all (popupMenu);
		
	gtk_widget_show_all(GTK_WIDGET(window));

   	connectCallback.connect(G_OBJECT(connectButton), "clicked", NULL);
	connectCallback.connect(G_OBJECT(quickConnect), "activate", NULL);
   	pubHubsCallback.connect(G_OBJECT(pubHubsButton), "clicked", NULL);
	pubHubsCallback.connect(G_OBJECT(pubHubsItem), "activate", NULL);
   	dlQueueCallback.connect(G_OBJECT(queueButton), "clicked", NULL);
	dlQueueCallback.connect(G_OBJECT(queueItem), "activate", NULL);
	favHubsCallback.connect(G_OBJECT(favHubsButton), "clicked", NULL);
	favHubsCallback.connect(G_OBJECT(favHubsItem), "activate", NULL);
	settingsCallback.connect(G_OBJECT(settingsButton), "clicked", NULL);
	settingsCallback.connect(G_OBJECT(settingsItem), "activate", NULL);
	searchCallback.connect(G_OBJECT(searchButton), "clicked", NULL);
	searchCallback.connect(G_OBJECT(searchItem), "activate", NULL);
	hashCallback.connect(G_OBJECT(hashButton), "clicked", NULL);
	hashCallback.connect(G_OBJECT(hashItem), "activate", NULL);
	aboutCallback.connect(G_OBJECT(aboutItem), "activate", NULL);
	quitCallback.connect(G_OBJECT(quitItem), "activate", NULL);
	quitCallback.connect(G_OBJECT(quitButton), "clicked", NULL);
	aboutItem = GTK_MENU_ITEM(glade_xml_get_widget(xml, "about"));
	finishedDL_Callback.connect(G_OBJECT(finishedDL_button), "clicked", NULL);
	finishedDL_Callback.connect(G_OBJECT(finishedDL_item), "activate", NULL);
	finishedUL_Callback.connect(G_OBJECT(finishedUL_button), "clicked", NULL);
	finishedUL_Callback.connect(G_OBJECT(finishedUL_item), "activate", NULL);
	openFListCallback.connect(G_OBJECT(openFList), "activate", NULL);
	openFListCallback.connect(G_OBJECT(openOwnFList), "activate", NULL);
	refreshFListCallback.connect(G_OBJECT(refreshFList), "activate", NULL);
	
	deleteCallback.connect(G_OBJECT(window), "delete-event", NULL);
//	transferCallback.connect(G_OBJECT(window), "button-release-event", this);
	switchPageCallback.connect(G_OBJECT(book), "switch-page", NULL);

	//Load window state and position from settings manager
	WulforSettingsManager *sm = WulforSettingsManager::get();
	int posX =  sm->getInt("main-window-pos-x");
	int posY = sm->getInt("main-window-pos-y");
	int sizeX = sm->getInt("main-window-size-x");
	int sizeY = sm->getInt("main-window-size-y");
	int transferPanePosition = sm->getInt("transfer-pane-position");
 	
	gtk_window_move(window, posX, posY);
	gtk_window_resize(window, sizeX, sizeY);
	if (sm->getInt("main-window-maximized"))
		gtk_window_maximize(window);
	gtk_paned_set_position(transferPane, transferPanePosition);

	//Create text in about window
	GtkLabel *al = GTK_LABEL(glade_xml_get_widget(xml, "aboutLabel"));
	string text =	string("<big>Linux DC++</big>\n") +
					string("Bringing DC++ to Linux!\n") +
					string("<b>Version:</b> 0.1-rc1\n") +
					string("<b>Core version:</b> ") + string(VERSIONSTRING) + string("\n") +
					string("<b>Developers:</b>\n ") +
					string("Jens Oknelid (paskharen)\n") +
					string("Alexander Nordfelth (phase)\n") +
					string("Dyluck\n") +
					string("s4kk3\n") +
					string("Trent Lloyd\n") +
					string("Kristian Berg/Ixan\n") +
					string("luusl\n") +
					string("Rikard Bj\303\266rklind\n") +
					string("clairvoyant\n") +
					string("obi\n") +
					string("John Armstrong\n") +
					string("Naga");
	//gtk_label_set_markup(al, Text::acpToUtf8(text).c_str());
	
	GtkWidget *dummy;
	GtkRequisition req;
	dummy = gtk_statusbar_new();
	gtk_widget_size_request(dummy, &req);
	gtk_widget_destroy(dummy);
	emptyStatusWidth = req.width;
	
	gtk_statusbar_push(mainStatus, 0, "Welcome to Linux DC++");
}

GtkWindow *MainWindow::getWindow() {
	return window;
}

gboolean MainWindow::transferClicked_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		if (event->button == 3)
		{
			MainWindow *mw = (MainWindow*)user_data;
			GtkTreeSelection *selection;
			GtkTreeModel *m = GTK_TREE_MODEL (mw->transferStore);
			selection = mw->transferSel;
			int count = gtk_tree_selection_count_selected_rows (selection);
						
			if (count < 1)
				return FALSE;
			else if (count == 1)
			{
				gtk_widget_set_sensitive (mw->filelist, true);
				gtk_widget_set_sensitive (mw->matchQueue, true);
				gtk_widget_set_sensitive (mw->privateMessage, true);
				gtk_widget_set_sensitive (mw->grantExtraSlot, true);
				gtk_widget_set_sensitive (mw->removeUser, true);
			}	
			else if (count > 1)
			{
				gtk_widget_set_sensitive (mw->filelist, false);
				gtk_widget_set_sensitive (mw->matchQueue, false);
				gtk_widget_set_sensitive (mw->privateMessage, false);
				gtk_widget_set_sensitive (mw->grantExtraSlot, false);
				gtk_widget_set_sensitive (mw->removeUser, false);			
			}
			((MainWindow*)user_data)->popup (event, user_data);
			return TRUE;
		}
	}
	return FALSE;
}

void MainWindow::popup (GdkEventButton *event, gpointer user_data)
{
    gtk_menu_popup(GTK_MENU (popupMenu), NULL, NULL, NULL, NULL,
		(event != NULL) ? 1 : 0,
		gdk_event_get_time((GdkEvent*)event));	
}

User::Ptr MainWindow::getSelectedTransfer_gui()
{
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (transferStore);
	selection = transferSel;

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	GtkTreeIter iter;
	
	if (!tmp)
		return NULL;
	if (!gtk_tree_model_get_iter (m, &iter, (GtkTreePath*)tmp->data))
		return NULL;	
	return transferView.getValue<gpointer,ConnectionQueueItem*>(&iter, "User Ptr")->getUser();
}

void MainWindow::onGetFileListClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	User::Ptr user = mw->getSelectedTransfer_gui ();
	try 
	{
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} 
	catch(const Exception&) 
	{
	}
}

void MainWindow::onMatchQueueClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	User::Ptr user = mw->getSelectedTransfer_gui ();
	try 
	{
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
	} 
	catch(const Exception&) 
	{
	}
}

void MainWindow::onPrivateMessageClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	User::Ptr user = mw->getSelectedTransfer_gui ();
	WulforManager::get()->addPrivMsg_gui(user);
}

void MainWindow::onAddFavoriteUserClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	User::Ptr user = mw->getSelectedTransfer_gui ();
	HubManager::getInstance()->addFavoriteUser(user);
}

void MainWindow::onGrantExtraSlotClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	User::Ptr user = mw->getSelectedTransfer_gui ();
	UploadManager::getInstance()->reserveSlot(user);
}

void MainWindow::onRemoveUserFromQueueClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	User::Ptr user = mw->getSelectedTransfer_gui ();
	QueueManager::getInstance()->removeSources(user, QueueItem::Source::FLAG_REMOVED);
}

void MainWindow::onForceAttemptClicked_gui (GtkMenuItem *item, gpointer user_data)
{

	MainWindow *mw = (MainWindow*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (mw->transferStore);
	selection = mw->transferSel;

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<User::Ptr> user;
	GtkTreeIter tmpiter;
	
	if (!tmp)
		return;
	
	while (1)
	{
		if (gtk_tree_model_get_iter (m, &tmpiter, (GtkTreePath*)tmp->data))
		{
			user.push_back(mw->transferView.getValue<gpointer,ConnectionQueueItem*>(&tmpiter, "User Ptr")->getUser ());
			gtk_list_store_set(mw->transferStore, &tmpiter, mw->transferView.col("Status"), "Connecting (forced)...", -1);
		}
		
		tmp = g_list_next (tmp);
		if (!tmp)
			break;
	}	
	
	for (int i=0;i<user.size ();i++)
		user[i]->connect ();
}

void MainWindow::onCloseConnectionClicked_gui (GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL (mw->transferStore);
	selection = mw->transferSel;

	GList *list = gtk_tree_selection_get_selected_rows (selection, &m);
	GList *tmp = g_list_first (list);
	vector<User::Ptr> user;
	vector<bool> isDownload;
	GtkTreeIter tmpiter;
	
	if (!tmp)
		return;
	
	while (1)
	{
		if (gtk_tree_model_get_iter (m, &tmpiter, (GtkTreePath*)tmp->data))
		{
			ConnectionQueueItem *qi = mw->transferView.getValue<gpointer,ConnectionQueueItem*>(&tmpiter, "User Ptr");
			if (qi->getConnection())
			{
				if (qi->getConnection()->isSet(UserConnection::FLAG_UPLOAD))
					isDownload.push_back(false);
				else
					isDownload.push_back(true);
				user.push_back (qi->getUser ());
			}
		}
		
		tmp = g_list_next (tmp);
		if (!tmp)
			break;
	}	

	for (int i=0;i<user.size ();i++)
		ConnectionManager::getInstance()->removeConnection(user[i], isDownload[i]);
}

void MainWindow::connectClicked_gui(GtkWidget *widget, gpointer data) {
	int response;
	string address;
	
	gtk_widget_show_all(GTK_WIDGET(connectDialog));
	response = gtk_dialog_run(connectDialog);
	gtk_widget_hide(GTK_WIDGET(connectDialog));
	
	if (response == GTK_RESPONSE_OK) {
		address = gtk_entry_get_text(connectEntry);
		WulforManager::get()->addHub_gui(address);
	}
}

void MainWindow::pubHubsClicked_gui(GtkWidget *widget, gpointer data) {
	WulforManager::get()->addPublicHubs_gui();
}

void MainWindow::searchClicked_gui(GtkWidget *widget, gpointer data) {
	WulforManager::get()->addSearch_gui();
}

void MainWindow::hashClicked_gui(GtkWidget *widget, gpointer data) {
	Hash *h = WulforManager::get()->openHashDialog_gui();
	gtk_dialog_run(GTK_DIALOG(h->getDialog()));
	WulforManager::get()->deleteDialogEntry_gui();
}

void MainWindow::dlQueueClicked_gui(GtkWidget *widget, gpointer data) {
	WulforManager::get()->addDownloadQueue_gui();
}

void MainWindow::favHubsClicked_gui(GtkWidget *widget, gpointer data) {
	WulforManager::get()->addFavoriteHubs_gui();
}

void MainWindow::finishedDLclicked_gui(GtkWidget *widget, gpointer data)
{
	WulforManager *wm;
	wm->get()->addFinishedTransfers_gui(wm->get()->FINISHED_DOWNLOADS, "Finished Downloads"); 
}

void MainWindow::finishedULclicked_gui(GtkWidget *widget, gpointer data)
{
	WulforManager *wm;
	wm->get()->addFinishedTransfers_gui(wm->get()->FINISHED_UPLOADS, "Finished Uploads");
}

void MainWindow::settingsClicked_gui(GtkWidget *widget, gpointer data) {
	Settings *s = WulforManager::get()->openSettingsDialog_gui();
	typedef Func0<MainWindow> F0;
	F0 *func;

	short lastPort = (short)SETTING(IN_PORT);
	int lastConn = SETTING(CONNECTION_TYPE);	
	
	if (gtk_dialog_run(GTK_DIALOG(s->getDialog())) == GTK_RESPONSE_OK) {
		s->saveSettings_client();
		SettingsManager::getInstance()->save();

		if (SETTING(CONNECTION_TYPE) != lastConn || SETTING(IN_PORT) != lastPort) {
			Selecter::quit();
			
			func = new F0(this, &MainWindow::startSocket_client);
			WulforManager::get()->dispatchClientFunc(func);
		}		
	}

	gtk_widget_destroy(s->getDialog());
	delete s;
}

void MainWindow::quitClicked_gui(GtkWidget *widget, gpointer data) {
	gboolean retVal;		// Not interested in the value though.
	g_signal_emit_by_name(G_OBJECT(window), "delete-event", NULL, &retVal);
}

void MainWindow::aboutClicked_gui(GtkWidget *widget, gpointer data) {
	gtk_widget_show_all(GTK_WIDGET(aboutDialog));
	gtk_dialog_run(aboutDialog);
	gtk_widget_hide(GTK_WIDGET(aboutDialog));
}

gboolean MainWindow::deleteWindow_gui(
	GtkWidget *widget, GdkEvent *event, gpointer data)
{
	int response;

	if (!BOOLSETTING(CONFIRM_EXIT))
	{
		transferView.saveSettings();
		WulforManager::get()->deleteAllBookEntries();
		gtk_main_quit();
		return TRUE;
	}
	
	gtk_widget_show_all(GTK_WIDGET(exitDialog));
	response = gtk_dialog_run(exitDialog);
	gtk_widget_hide(GTK_WIDGET(exitDialog));

	if (response == GTK_RESPONSE_OK)
	{
		transferView.saveSettings();
		WulforManager::get()->deleteAllBookEntries();
		gtk_main_quit();
		return TRUE;
	}

	return TRUE;
}

void MainWindow::switchPage_gui(GtkNotebook *notebook, 
	GtkNotebookPage *page, guint page_num, gpointer user_data)
{
	BookEntry *b = WulforManager::get()->getBookEntry_gui(page_num);
	if (b)
		b->switchedPage();
}

void MainWindow::openHub_gui(
	string server, string nick, string desc, string password)
{
	WulforManager::get()->addHub_gui(server, nick, desc, password);
}

void MainWindow::autoConnect_client() {
	FavoriteHubEntry::List &l = HubManager::getInstance()->getFavoriteHubs();
	FavoriteHubEntry::List::const_iterator it;
	typedef Func4<MainWindow, string, string, string, string> F4;
	F4 *func;

	for (it = l.begin(); it != l.end(); it++) {
		FavoriteHubEntry *entry = *it;
		if (entry->getConnect())
			if (!entry->getNick().empty () || !SETTING(NICK).empty()) {
				string nick;
			
				if (entry->getNick().empty())
					nick =  SETTING(NICK);
				else
					nick = entry->getNick();
			
				func = new F4(this, &MainWindow::openHub_gui,
					entry->getServer(),
					nick,
					entry->getUserDescription(),
					entry->getPassword());
				WulforManager::get()->dispatchGuiFunc(func);
			}
	}
}

void MainWindow::autoOpen_gui() {
	if (SETTING(OPEN_PUBLIC))
		WulforManager::get()->addPublicHubs_gui();
	if (SETTING(OPEN_QUEUE))
		WulforManager::get()->addDownloadQueue_gui();
	if (SETTING(OPEN_FAVORITE_HUBS))
		WulforManager::get()->addFavoriteHubs_gui();
	if (BOOLSETTING(OPEN_FINISHED_DOWNLOADS)) {
		WulforManager::get()->addFinishedTransfers_gui(
			WulforManager::FINISHED_DOWNLOADS, "Finished Downloads"); 
	}
}

void MainWindow::startSocket_client() {
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();

	if (SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE)
		return;

	short port = (short)SETTING(IN_PORT);

	while(true) {
		try {
			ConnectionManager::getInstance()->setPort(port);
			Selecter::WSAASyncSelect(
				ConnectionManager::getInstance()->getServerSocket());
			break;
		} catch(const Exception& e) {
			cout << "StartSocket (tcp): Caught \"" << e.getError() << "\""<< endl;
			port++;
			if (port > 32000)
			{
				cout << "StartSocket (tcp): Can't find a good port" << endl;
				break;
			}
		}
	}

	port = (short)SETTING(UDP_PORT);

	while(true) {
		try {
			SearchManager::getInstance()->setPort(port);
			break;
		} catch(const Exception& e) {
			cout << "StartSocket (udp): Caught \"" << e.getError() << "\""<< endl;
			port++;
			if (port > 32000)
			{
				cout << "StartSocket (udp): Can't find a good port" << endl;
				break;
			}
		}
	}
}

//TimerManagerListener
void MainWindow::on(TimerManagerListener::Second, u_int32_t ticks) throw() {
	int64_t diff = (int64_t)((lastUpdate == 0) ? ticks - 1000 : 
		ticks - lastUpdate);
	int64_t updiff = Socket::getTotalUp() - lastUp;
	int64_t downdiff = Socket::getTotalDown() - lastDown;
	string status1, status2, status3, status4, status5, status6;

	status1 = "H: " + Client::getCounts();
	status2 = "S: " + Util::toString(SETTING(SLOTS) -  
		UploadManager::getInstance()->getRunning()) + '/' +
		Util::toString(SETTING(SLOTS));
	status3 = "D: " + Util::formatBytes(Socket::getTotalDown());
	status4 = "U: " + Util::formatBytes(Socket::getTotalUp());
	status5 = "D: " + Util::formatBytes((int64_t)(downdiff*1000)/diff) + "/s (" +
		Util::toString(DownloadManager::getInstance()->getDownloadCount()) + ")";
	status6 = "U: " + Util::formatBytes((int64_t)(updiff*1000)/diff) + "/s (" +
		Util::toString(UploadManager::getInstance()->getUploadCount()) + ")";

	lastUpdate = ticks;
	lastUp = Socket::getTotalUp();
	lastDown = Socket::getTotalDown();

	typedef Func6<MainWindow, string, string, string, string, string, string> func_t;
	func_t *func = new func_t(this, &MainWindow::setStats_gui, status1, status2, status3, status4, status5, status6);
	WulforManager::get()->dispatchGuiFunc(func);
}

//From QueueManagerListener
void MainWindow::on(QueueManagerListener::Finished, QueueItem *item) throw() {
	if (!item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST)) 
		return;

	User::Ptr user = item->getCurrent()->getUser();
	string searchString = item->getSearchString();
	string listName = item->getListName();

	if (item->isSet(QueueItem::FLAG_CLIENT_VIEW) && 
		item->isSet(QueueItem::FLAG_USER_LIST))
	{
		typedef Func3<MainWindow, User::Ptr, string, string> F3;
		F3 *func = new F3(this, &MainWindow::addShareBrowser_gui, 
			user, searchString, listName);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::addShareBrowser_gui(User::Ptr user, string searchString, string listName) {
	ShareBrowser *browser;
	browser = WulforManager::get()->addShareBrowser_gui(user, listName);
	browser->setPosition_gui(searchString);
}

void MainWindow::addPage_gui(GtkWidget *page, GtkWidget *label, bool raise) {
	gtk_notebook_append_page(book, page, label);
	if (raise) gtk_notebook_set_current_page(book, -1);
}

void MainWindow::removePage_gui(GtkWidget *page) {
	int i, pageNum = -1;

	for (i=0; i<gtk_notebook_get_n_pages(book); i++)
		if (page == gtk_notebook_get_nth_page(book, i)) pageNum = i;
		
	assert(pageNum != -1);
	gtk_notebook_remove_page(book, pageNum);
}

void MainWindow::raisePage_gui(GtkWidget *page) {
	int i, pageNum = -1;

	for (i=0; i<gtk_notebook_get_n_pages(book); i++)
		if (page == gtk_notebook_get_nth_page(book, i)) pageNum = i;
		
	assert(pageNum != -1);
	gtk_notebook_set_current_page(book, pageNum);
}

GtkWidget *MainWindow::currentPage_gui() {
	int pageNum = gtk_notebook_get_current_page(book);

	if (pageNum == -1)
		return NULL;
	else 
		return gtk_notebook_get_nth_page(book, pageNum);
}

void MainWindow::setStatus_gui(GtkStatusbar *status, std::string text) {
	// Apparently if (!status) ... crashes for some people. Strange.
	if (status == NULL) return; 

	if (status != mainStatus) {
		PangoLayout *pango;
		int width;
		GtkRequisition req;

		pango = gtk_widget_create_pango_layout(GTK_WIDGET(window), text.c_str());
		pango_layout_get_pixel_size(pango, &width, NULL);
		g_object_unref(G_OBJECT(pango));
		gtk_widget_size_request(GTK_WIDGET(status), &req);
		if (width > req.width - emptyStatusWidth)
			gtk_widget_set_size_request(GTK_WIDGET(status), 
				width + emptyStatusWidth, -1);
	}

	gtk_statusbar_pop(status, 0);
	gtk_statusbar_push(status, 0, text.c_str());
}

void MainWindow::setStats_gui(std::string hub, std::string slot, 
	std::string dTot, std::string uTot, std::string dl, std::string ul)
{
	setStatus_gui(hubStatus, hub);
	setStatus_gui(slotStatus, slot);
	setStatus_gui(dTotStatus, dTot);
	setStatus_gui(uTotStatus, uTot);
	setStatus_gui(ulStatus, ul);
	setStatus_gui(dlStatus, dl);
}

void MainWindow::updateTransfer_gui(string id, connection_t type, ConnectionQueueItem *item, 
	string status, string time, string speed, string file, string size, string path, int progress)
{
	GtkTreeIter iter;
	findId_gui(id, &iter);
	
	if (!gtk_list_store_iter_is_valid(transferStore, &iter)) {
		gtk_list_store_append(transferStore, &iter);
		gtk_list_store_set(transferStore, &iter, transferView.col("ID"), id.c_str(), -1);
	}

	if (type == CONNECTION_UL) {
		gtk_list_store_set(transferStore, &iter, transferView.col(" "), uploadPic, -1);
	}
	if (type == CONNECTION_DL) {
		gtk_list_store_set(transferStore, &iter, transferView.col(" "), downloadPic, -1);
	}
	if (item) {
		gtk_list_store_set(transferStore, &iter,
			transferView.col("User"), item->getUser()->getNick().c_str(),
			transferView.col("User Ptr"), (gpointer)item,
			-1);
	}
	if (SETTING(SHOW_PROGRESS_BARS) && status != "") {
		gtk_list_store_set(transferStore, &iter, transferView.col("Status"), status.c_str(), transferView.col("Progress"), progress, -1);
	}
	else if (status != ""){
		gtk_list_store_set(transferStore, &iter, transferView.col("Status"), status.c_str(), -1);
	}
	if (time != "") {
		gtk_list_store_set(transferStore, &iter, transferView.col("Time Left"), time.c_str(), -1);
	}
	if (speed != "") {
		gtk_list_store_set(transferStore, &iter, transferView.col("Speed"), speed.c_str(), -1);
	}
	if (file != "") {
		gtk_list_store_set(transferStore, &iter, transferView.col("Filename"), file.c_str(), -1);
	}
	if (size != "") {
		gtk_list_store_set(transferStore, &iter, transferView.col("Size"), size.c_str(), -1);
	}
	if (path != "") {
		gtk_list_store_set(transferStore, &iter, transferView.col("Path"), path.c_str(), -1);
	}
}

void MainWindow::removeTransfer_gui(string id) {
	GtkTreeIter iter;
	findId_gui(id, &iter);
	
	if (gtk_list_store_iter_is_valid(transferStore, &iter)) {
		gtk_list_store_remove(transferStore, &iter);
	}
}

void MainWindow::findId_gui(string id, GtkTreeIter *iter) {
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(transferStore), iter);

	while (gtk_list_store_iter_is_valid(transferStore, iter)) {
		char *t;
		string text;
		gtk_tree_model_get(GTK_TREE_MODEL(transferStore), iter, 
			transferView.col("ID"), &t, -1);
		text = t;
		g_free(t);

		if (id == text) return;

		//When the connection is just created we don't know the type.
		//Thus we have a special "connecting" id that matches any other id
		//from the same user, as well as separate upload and download ids.
		if (text.find("$Connecting", 0) != string::npos || 
			id.find("$Connecting", 0) != string::npos)
		{
			if (text.substr(0, text.find('$', 0)) == 
				id.substr(0, id.find('$', 0)))
			{
				gtk_list_store_set(transferStore, iter, 
					transferView.col("ID"), id.c_str(), -1);
				return;
			}
		}

		gtk_tree_model_iter_next(GTK_TREE_MODEL(transferStore), iter);
	}
}

string MainWindow::getId_client(ConnectionQueueItem *item) {
	string ret = item->getUser()->getNick() + "$" + 
		item->getUser()->getLastHubAddress();

	//The $ is a special char in DC that can't be used in nicks.
	//Thus nobody can make an evil nick to mess with this list.
	if (item->getConnection()) {
		if (item->getConnection()->isSet(UserConnection::FLAG_UPLOAD)) {
			ret += "$Upload";
		} else {
			ret += "$Download";
		}
	} else {
		ret += "$Connecting";
	}

	return ret;
}

string MainWindow::getId_client(Transfer *t) {
	assert (t->getUserConnection());
	assert (t->getUserConnection()->getCQI());

	return getId_client(t->getUserConnection()->getCQI());
}

void MainWindow::transferComplete_client(Transfer *t) {
	string status, id = getId_client(t);

	if (t->getUserConnection()->isSet(UserConnection::FLAG_UPLOAD)) {
		status = "Upload finished, idle...";
	} else {
		status = "Download finished, idle...";
	}

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, NULL, status,
		"Done", " ", "", "", "", 100);
	WulforManager::get()->dispatchGuiFunc(func);
}


void MainWindow::openFList_gui(GtkWidget *widget, gpointer data)
{
	User::Ptr user;
	string path, filename;

	if (widget == GTK_WIDGET(openFList))
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(flistDialog), 
			Text::toT(Util::getDataPath() + "FileLists/").c_str());
		gtk_widget_show_all(GTK_WIDGET(flistDialog));
 		int ret = gtk_dialog_run(flistDialog);
		gtk_widget_hide(GTK_WIDGET(flistDialog));
	
		if (ret != GTK_RESPONSE_OK && ret != GTK_RESPONSE_ACCEPT) return;
	
		path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(flistDialog));
		filename = g_path_get_basename(path.c_str());
		if (filename.substr(filename.length()-8, 8) != ".xml.bz2" && Util::getFileExt(filename) != ".DcLst")
			return;
		user = new User(filename);
	}
	else
	{
		user = new User("My List");
		path = Util::getDataPath() + "files.xml.bz2";
		try
		{
			// Test if file list already exists
			::File myFileList(path, ::File::READ, ::File::OPEN);
			myFileList.close();
		}
		catch (const FileException&)
		{
			// No existing file list; create one instead
			ShareManager::getInstance()->getOwnListFile();
		}
	}

	WulforManager::get()->addShareBrowser_gui(user, path);
}

void MainWindow::refreshFList_gui(GtkWidget *widget, gpointer data)
{
	ShareManager::getInstance()->setDirty();
	// Function would simply not work when put into GUI queue.
	ShareManager::getInstance()->refresh(true, true, false);
	ShareManager::getInstance()->getOwnListFile();
}

//From Connection manager
void MainWindow::on(ConnectionManagerListener::Added, ConnectionQueueItem *item) throw() {
	string status = "Connecting...";
	string id = getId_client(item);

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, 
		item, status, "", "", "", "", "", 0);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::Removed, ConnectionQueueItem *item) throw() {
	string id = getId_client(item);

	typedef Func1 <MainWindow, string> F1;
	F1 *func = new F1(this, &MainWindow::removeTransfer_gui, id);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::Failed, ConnectionQueueItem *item, const string &reason) throw() {
	string status = reason;
	string id = getId_client(item);

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, NULL, status,
		"", "", "", "", "", 0);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem *item) throw() {
	string status;
	string id = getId_client(item);

	if (item->getState() == ConnectionQueueItem::CONNECTING) {
		status = "Connecting...";
	} else {
		status = "Waiting to retry...";
	}

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, NULL, status,
		"", "", "", "", "", 0);
	WulforManager::get()->dispatchGuiFunc(func);
}

//From Download manager
void MainWindow::on(DownloadManagerListener::Starting, Download *dl) throw() {
	string status, size, path, file, target;
	string id = getId_client(dl);

	size = Util::formatBytes(dl->getSize());
	target = Text::acpToUtf8(dl->getTarget());
	status = "Download starting...";

	if (dl->isSet(Download::FLAG_USER_LIST))
	{
		file = "Filelist";
		path = "";
	}
	else
	{
		file = Util::getFileName(target);
		path = Util::getFilePath(target);
	}

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_DL, NULL, status,
		"", "", file, size, path, 0);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(DownloadManagerListener::Tick, const Download::List &list) throw() {
	string id, status, timeLeft, speed;
	Download::List::const_iterator it;
	
	for (it = list.begin(); it != list.end(); it++) {
		Download* dl = *it;
		ostringstream stream;

		id = getId_client(dl); 

		string bytes = Util::formatBytes(dl->getPos());
		double percent = (double)(dl->getPos() * 100.0) / dl->getSize();
		string time = Util::formatSeconds((GET_TICK() - dl->getStart()) / 1000);
		stream << setiosflags(ios::fixed) << setprecision(1);
		stream << "Downloaded " << bytes << " (" << percent << "%) in " << time;

		timeLeft = Util::formatSeconds(dl->getSecondsLeft());
		speed = Util::formatBytes(dl->getRunningAverage()) + "/s";

		if (dl->isSet(Download::FLAG_ZDOWNLOAD)) {
			status = "* " + stream.str();
		} else {
			status = stream.str();
		}

		UFunc *func;
		func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, NULL, 
			status,	timeLeft, speed, "", "", "", (int)percent);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::on(DownloadManagerListener::Complete, Download *dl) throw() {
	transferComplete_client(dl);
}

void MainWindow::on(DownloadManagerListener::Failed, Download *dl, const string &reason) throw() {
	string status, size, file, path, target;
	string id = getId_client(dl); 

	status = reason;
	size = Util::formatBytes(dl->getSize());
	target = Text::acpToUtf8(dl->getTarget());

	if (dl->isSet(Download::FLAG_USER_LIST))
	{
		file = "Filelist";
		path = "";
	}
	else
	{
		file = Util::getFileName(target);
		path = Util::getFilePath(target);
	}

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, NULL, 
		status,	"", "", file, size, path, 0);
	WulforManager::get()->dispatchGuiFunc(func);
}

//From Upload manager
void MainWindow::on(UploadManagerListener::Starting, Upload *ul) throw() {
	string status, size, path, file, target;
	string id = getId_client(ul);

	size = Util::formatBytes(ul->getSize());
	target = Text::acpToUtf8(ul->getFileName());
	status = "Upload starting...";

	if (ul->isSet(Upload::FLAG_USER_LIST))
	{
		file = "Filelist";
		path = "";
	}
	else
	{
		file = Util::getFileName(target);
		path = Util::getFilePath(target);
	}

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_UL, NULL, status,
		"", "", file, size, path, 0);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(UploadManagerListener::Tick, const Upload::List &list) throw() {
	string id, status, timeLeft, speed;
	Upload::List::const_iterator it;
	
	for (it = list.begin(); it != list.end(); it++) {
		Upload* ul = *it;
		ostringstream stream;

		id = getId_client(ul); 

		string bytes = Util::formatBytes(ul->getPos());
		double percent = (double)(ul->getPos() * 100.0) / ul->getSize();
		string time = Util::formatSeconds((GET_TICK() - ul->getStart()) / 1000);
		stream << setiosflags(ios::fixed) << setprecision(1);
		stream << "Uploaded " << bytes << " (" << percent << "%) in " << time;

		timeLeft = Util::formatSeconds(ul->getSecondsLeft());
		speed = Util::formatBytes(ul->getRunningAverage()) + "/s";

		if (ul->isSet(Download::FLAG_ZDOWNLOAD)) {
			status = "* " + stream.str();
		} else {
			status = stream.str();
		}

		UFunc *func;
		func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, NULL, 
			status,	timeLeft, speed, "", "", "", (int)percent);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::on(UploadManagerListener::Complete, Upload *ul) throw() {
	transferComplete_client(ul);
}

//From logmanager
void MainWindow::on(LogManagerListener::Message, const string& str) throw() {
	typedef Func2<MainWindow, GtkStatusbar *, string> F2;
	F2 *func = new F2(this, &MainWindow::setStatus_gui, mainStatus, str);
	WulforManager::get()->dispatchGuiFunc(func);
}

