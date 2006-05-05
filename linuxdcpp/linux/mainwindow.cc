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

	// This makes sure the pixmaps are freed (using gtk's ref counting).
	g_object_unref(G_OBJECT(uploadPic));
	g_object_unref(G_OBJECT(downloadPic));

	// Free the transferMap
	map<UserID, TransferItem *>::iterator it;
	for (it = transferMap.begin(); it != transferMap.end(); it++)
	{
		gtk_tree_row_reference_free(it->second->rowRef);
		delete it->second;
	}
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
	gtk_dialog_set_alternative_button_order(exitDialog, GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(connectDialog, GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(flistDialog, GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

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
	windowMenu = glade_xml_get_widget(xml, "windowMenu");

	gtk_widget_set_sensitive(GTK_WIDGET(openDLdir), false);
	gtk_widget_set_sensitive(GTK_WIDGET(followRedirect), false);
	gtk_widget_set_sensitive(GTK_WIDGET(reconnectItem), false);
	gtk_widget_set_sensitive(GTK_WIDGET(favUsersItem), false);
	gtk_widget_set_sensitive(GTK_WIDGET(ADLSearchItem), false);
	gtk_widget_set_sensitive(GTK_WIDGET(searchSpyItem), false);
	gtk_widget_set_sensitive(GTK_WIDGET(networkStatsItem), false);

	// Initialize transfer treeview
	transferView.setView(GTK_TREE_VIEW(glade_xml_get_widget(xml, "transfers")), true, "main");
	transferView.insertColumn("User", G_TYPE_STRING, TreeView::PIXBUF_STRING, 150, "Icon");
	transferView.insertColumn("Hub Name", G_TYPE_STRING, TreeView::STRING, 100);
	if (SETTING(SHOW_PROGRESS_BARS))
		transferView.insertColumn("Status", G_TYPE_STRING, TreeView::PROGRESS, 250, "Progress");
	else
		transferView.insertColumn("Status", G_TYPE_STRING, TreeView::STRING, 250);
	transferView.insertColumn("Time Left", G_TYPE_STRING, TreeView::STRING, 75);
	transferView.insertColumn("Speed", G_TYPE_STRING, TreeView::STRING, 175);
	transferView.insertColumn("Filename", G_TYPE_STRING, TreeView::STRING, 200);
	transferView.insertColumn("Size", G_TYPE_STRING, TreeView::STRING, 175);
	transferView.insertColumn("Path", G_TYPE_STRING, TreeView::STRING, 200);
	transferView.insertHiddenColumn("Icon", GDK_TYPE_PIXBUF);
	transferView.insertHiddenColumn("TransferItem", G_TYPE_POINTER);
	transferView.insertHiddenColumn("Progress", G_TYPE_INT);
	transferView.insertHiddenColumn("Sort Order", G_TYPE_STRING);
	transferView.insertHiddenColumn("Speed Order", G_TYPE_INT64);
	transferView.insertHiddenColumn("Size Order", G_TYPE_INT64);
	transferView.finalize();
	transferStore = gtk_list_store_newv(transferView.getColCount(), transferView.getGTypes());
	gtk_tree_view_set_model(transferView.get(), GTK_TREE_MODEL(transferStore));
	g_object_unref(transferStore);
	transferSel = gtk_tree_view_get_selection(transferView.get());
	gtk_tree_selection_set_mode(transferSel, GTK_SELECTION_MULTIPLE);
	transferView.setSortColumn_gui("User", "Sort Order");
	transferView.setSortColumn_gui("Speed", "Speed Order");
	transferView.setSortColumn_gui("Size", "Size Order");
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(transferStore), transferView.col("Sort Order"), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(transferView.get(), transferView.col("User")), TRUE);
	gtk_tree_view_set_fixed_height_mode(transferView.get(), TRUE);

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
	gtk_widget_set_sensitive(addToFavorites, FALSE); // Fav user not implemented yet.
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
	settingsCallback.connect(G_OBJECT(settingsButton), "clicked", this);
	settingsCallback.connect(G_OBJECT(settingsItem), "activate", this);
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

void MainWindow::appendWindowItem(GtkWidget *page, string title)
{
	GtkWidget *menuItem = gtk_menu_item_new_with_label(title.c_str());
	g_object_set_data(G_OBJECT(menuItem), "tabWidget", page);
	g_signal_connect(G_OBJECT(menuItem), "activate", G_CALLBACK(raisePage), (gpointer)this);
	gtk_menu_shell_append(GTK_MENU_SHELL(windowMenu), menuItem);
	gtk_widget_show_all(windowMenu);
	windowMenuItems[page] = menuItem;
}

void MainWindow::removeWindowItem(GtkWidget *page)
{
	gtk_container_remove(GTK_CONTAINER(windowMenu), windowMenuItems[page]);
	windowMenuItems.erase(page);
}

void MainWindow::modifyWindowItem(GtkWidget *page, string title)
{
	GtkWidget *child = gtk_bin_get_child(GTK_BIN(windowMenuItems[page]));
	if (child && GTK_IS_LABEL(child))
		gtk_label_set_text(GTK_LABEL(child), title.c_str());
}

void MainWindow::raisePage(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *page = (GtkWidget *)g_object_get_data(G_OBJECT(item), "tabWidget");
	typedef Func1<MainWindow, GtkWidget *> F1;
	F1 *func = new F1(mw, &MainWindow::raisePage_gui, page);
	WulforManager::get()->dispatchGuiFunc(func);
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
	GtkTreeModel *m = GTK_TREE_MODEL(transferStore);
	GList *list = gtk_tree_selection_get_selected_rows(transferSel, &m);
	GtkTreePath *path = (GtkTreePath*)g_list_nth_data(list, 0);
	GtkTreeIter iter;
	TransferItem *item;

	if (!gtk_tree_model_get_iter(m, &iter, path))
		return NULL;

	gtk_tree_path_free(path);
	g_list_free(list);

	item = transferView.getValue<gpointer, TransferItem*>(&iter, "TransferItem");
	return item->user;
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

void MainWindow::onForceAttemptClicked_gui(GtkMenuItem *menuItem, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkTreeModel *m = GTK_TREE_MODEL(mw->transferStore);
	GtkTreeIter iter;
	GtkTreePath *path;
	TransferItem *item;
	GList *list = gtk_tree_selection_get_selected_rows(mw->transferSel, NULL);
	gint count = gtk_tree_selection_count_selected_rows(mw->transferSel);

	for (int i = 0; i < count; i++)
	{
		path = (GtkTreePath *)g_list_nth_data(list, i);
		if (gtk_tree_model_get_iter(m, &iter, path))
		{
			item = mw->transferView.getValue<gpointer, TransferItem*>(&iter, "TransferItem");
			item->user->connect();
			gtk_list_store_set(mw->transferStore, &iter, mw->transferView.col("Status"), "Connecting (forced)...", -1);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void MainWindow::onCloseConnectionClicked_gui(GtkMenuItem *menuItem, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkTreeModel *m = GTK_TREE_MODEL(mw->transferStore);
	GtkTreeIter iter;
	GtkTreePath *path;
	TransferItem *item;
	GList *list = gtk_tree_selection_get_selected_rows(mw->transferSel, NULL);
	gint count = gtk_tree_selection_count_selected_rows(mw->transferSel);

	for (int i = 0; i < count; i++)
	{
		path = (GtkTreePath *)g_list_nth_data(list, i);
		if (gtk_tree_model_get_iter(m, &iter, path))
		{
			item = mw->transferView.getValue<gpointer, TransferItem*>(&iter, "TransferItem");
			ConnectionManager::getInstance()->removeConnection(item->user, item->isDownload);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
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

void MainWindow::settingsClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	Settings *s = WulforManager::get()->openSettingsDialog_gui();
	typedef Func0<MainWindow> F0;
	F0 *func;

	short lastPort = (short)SETTING(IN_PORT);
	int lastConn = SETTING(CONNECTION_TYPE);
	bool lastShowProgressSetting = SETTING(SHOW_PROGRESS_BARS);
	
	if (gtk_dialog_run(GTK_DIALOG(s->getDialog())) == GTK_RESPONSE_OK) {
		s->saveSettings_client();
		SettingsManager::getInstance()->save();

		if (SETTING(CONNECTION_TYPE) != lastConn || SETTING(IN_PORT) != lastPort) {
			Selecter::quit();
			
			func = new F0(this, &MainWindow::startSocket_client);
			WulforManager::get()->dispatchClientFunc(func);
		}

		if (!lastShowProgressSetting && SETTING(SHOW_PROGRESS_BARS))
		{
			GtkTreeViewColumn *col = gtk_tree_view_get_column(mw->transferView.get(), mw->transferView.col("Status"));
			GtkCellRenderer *renderer = gtk_cell_renderer_progress_new();
			gtk_tree_view_column_clear(col);
			gtk_tree_view_column_pack_start(col, renderer, TRUE);
			gtk_tree_view_column_set_attributes(col, renderer, "text", mw->transferView.col("Status"),
				"value", mw->transferView.col("Progress"), NULL);
		}
		else if (lastShowProgressSetting && !SETTING(SHOW_PROGRESS_BARS))
		{
			GtkTreeViewColumn *col = gtk_tree_view_get_column(mw->transferView.get(), mw->transferView.col("Status"));
			GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
			gtk_tree_view_column_clear(col);
			gtk_tree_view_column_pack_start(col, renderer, TRUE);
			gtk_tree_view_column_set_attributes(col, renderer, "text", mw->transferView.col("Status"), NULL);
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
		
	dcassert(pageNum != -1);
	gtk_notebook_remove_page(book, pageNum);
}

void MainWindow::raisePage_gui(GtkWidget *page) {
	int i, pageNum = -1;

	for (i=0; i<gtk_notebook_get_n_pages(book); i++)
		if (page == gtk_notebook_get_nth_page(book, i)) pageNum = i;
		
	dcassert(pageNum != -1);
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

void MainWindow::updateTransfer_gui(TransferItem *item)
{
	dcassert(item);

	GtkTreeIter iter;
	GtkTreePath *treePath;

	treePath = gtk_tree_row_reference_get_path(item->rowRef);
	if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(transferStore), &iter, treePath))
		return;

	if (item->update["file"] && !item->file.empty())
		gtk_list_store_set(transferStore, &iter, transferView.col("Filename"), item->file.c_str(), -1);

	if (item->update["path"] && !item->path.empty())
		gtk_list_store_set(transferStore, &iter, transferView.col("Path"), item->path.c_str(), -1);

	if (item->update["status"] && !item->status.empty())
		gtk_list_store_set(transferStore, &iter, transferView.col("Status"), item->status.c_str(), -1);

	if (item->update["time"] && !item->time.empty())
		gtk_list_store_set(transferStore, &iter, transferView.col("Time Left"), item->time.c_str(), -1);

	if (item->update["sortOrder"] && !item->sortOrder.empty())
		gtk_list_store_set(transferStore, &iter, transferView.col("Sort Order"), item->sortOrder.c_str(), -1);

	if (item->update["size"] && item->size >= 0)
	{
		gtk_list_store_set(transferStore, &iter,
			transferView.col("Size"), Util::formatBytes(item->size).c_str(),
			transferView.col("Size Order"), item->size,
			-1);
	}

	if (item->update["speed"] && item->speed >= 0)
	{
		gtk_list_store_set(transferStore, &iter,
			transferView.col("Speed"),  Util::formatBytes(item->speed).append("/s").c_str(),
			transferView.col("Speed Order"), item->speed,
			-1);
	}

	if (item->update["progress"] && item->progress >= 0 && item->progress <= 100)
		gtk_list_store_set(transferStore, &iter, transferView.col("Progress"), item->progress, -1);
}

void MainWindow::removeTransfer_gui(UserID id)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	TransferItem *item;

	if (transferMap.find(id) != transferMap.end())
	{
		item = transferMap[id];
		path = gtk_tree_row_reference_get_path(item->rowRef);
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(transferStore), &iter, path))
		gtk_list_store_remove(transferStore, &iter);

		gtk_tree_row_reference_free(item->rowRef);
		delete item;
		transferMap.erase(id);
	}
}

MainWindow::TransferItem* MainWindow::getTransferItem(UserID id)
{
	TransferItem *item;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *m = GTK_TREE_MODEL(transferStore);

	if (transferMap.find(id) == transferMap.end())
	{
		transferMap[id] = item = new TransferItem(id.first, id.second);
		gtk_list_store_append(transferStore, &iter);
		path = gtk_tree_model_get_path(m, &iter);
		item->rowRef = gtk_tree_row_reference_new(m, path);
		if (item->isDownload)
			gtk_list_store_set(transferStore, &iter, transferView.col("Icon"), downloadPic, -1);
		else
			gtk_list_store_set(transferStore, &iter, transferView.col("Icon"), uploadPic, -1);
		gtk_list_store_set(transferStore, &iter,
			transferView.col("User"), item->nicks.c_str(),
			transferView.col("TransferItem"), (gpointer)item,
			transferView.col("Hub Name"), item->hubs.c_str(),
			-1);
	}
	else
		item = transferMap[id];

	item->update.clear();
	return item;
}

void MainWindow::transferComplete_client(Transfer *t)
{
	bool isDownload = t->getUserConnection()->isSet(UserConnection::FLAG_DOWNLOAD);
	UserID id(t->getUserConnection()->getUser(), isDownload);
	TransferItem *item = getTransferItem(id);

	if (isDownload)
		item->setStatus("Download finished, idle...");
	else
		item->setStatus("Upload finished, idle...");
	item->setTime("Done");
	item->setProgress(100);
	item->setSortOrder("w" + item->nicks + item->hubs);

	UFunc *func = new UFunc(this, &MainWindow::updateTransfer_gui, item);
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
void MainWindow::on(ConnectionManagerListener::Added, ConnectionQueueItem *cqi) throw()
{
	UserID id(cqi->getUser(), cqi->getDownload());
	TransferItem *item = getTransferItem(id);
	item->setStatus("Connecting...");
	item->setProgress(0);
	item->setSortOrder("w" + item->nicks + item->hubs);

	UFunc *func = new UFunc(this, &MainWindow::updateTransfer_gui, item);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::Removed, ConnectionQueueItem *cqi) throw()
{
	UserID id(cqi->getUser(), cqi->getDownload());

	typedef Func1 <MainWindow, UserID> F1;
	F1 *func = new F1(this, &MainWindow::removeTransfer_gui, id);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::Failed, ConnectionQueueItem *cqi, const string &reason) throw()
{
	UserID id(cqi->getUser(), cqi->getDownload());
	TransferItem *item = getTransferItem(id);
	item->setStatus(reason, true);
	item->setSortOrder("w" + item->nicks + item->hubs);

	UFunc *func = new UFunc(this, &MainWindow::updateTransfer_gui, item);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem *cqi) throw()
{
	UserID id(cqi->getUser(), cqi->getDownload());
	TransferItem *item = getTransferItem(id);

	if (cqi->getState() == ConnectionQueueItem::CONNECTING)
		item->setStatus("Connecting...");
	else
		item->setStatus("Waiting to retry...");

	UFunc *func = new UFunc(this, &MainWindow::updateTransfer_gui, item);
	WulforManager::get()->dispatchGuiFunc(func);
}

//From Download manager
void MainWindow::on(DownloadManagerListener::Starting, Download *dl) throw()
{
	UserID id(dl->getUserConnection()->getUser(), TRUE);
	TransferItem *item = getTransferItem(id);
	string target = Text::acpToUtf8(dl->getTarget());

	if (dl->isSet(Download::FLAG_USER_LIST))
		item->setFile("Filelist");
	else if (dl->isSet(Download::FLAG_TREE_DOWNLOAD))
		item->setFile("TTH: " + Util::getFileName(target));
	else
		item->setFile(Util::getFileName(target));

	item->setPath(Util::getFilePath(target));
	item->setStatus("Download starting...");
	item->setSize(dl->getSize());
	item->setSortOrder("d" + item->nicks + item->hubs);

	UFunc *func = new UFunc(this, &MainWindow::updateTransfer_gui, item);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(DownloadManagerListener::Tick, const Download::List &list) throw()
{
	string status, time, bytes;
	double percent;
	Download* dl;
	Download::List::const_iterator it;
	TransferItem *item;

	for (it = list.begin(); it != list.end(); it++)
	{
		dl = *it;
		ostringstream stream;
		UserID id(dl->getUserConnection()->getUser(), TRUE);
		item = getTransferItem(id);

		bytes = Util::formatBytes(dl->getPos());
		percent = (double)(dl->getPos() * 100.0) / dl->getSize();
		time = Util::formatSeconds((GET_TICK() - dl->getStart()) / 1000);
		stream << setiosflags(ios::fixed) << setprecision(1);
		stream << "Downloaded " << bytes << " (" << percent << "%) in " << time;

		status.clear();
/*		Future flags in DC++ > 0.674
		if (dl->getUserConnection()->isSecure())
			status += "[S]";
		if (dl->isSet(Download::FLAG_TTH_CHECK))
			status += "[T]";
*/		if (dl->isSet(Download::FLAG_ZDOWNLOAD))
			status += "[Z]";
		if (dl->isSet(Download::FLAG_ROLLBACK))
			status += "[R]";
		if (!status.empty())
			status += " ";
		status += stream.str();

		item->setStatus(status);
		item->setTime(Util::formatSeconds(dl->getSecondsLeft()));
		item->setSpeed(dl->getRunningAverage());
		item->setProgress((int)percent);

		UFunc *func = new UFunc(this, &MainWindow::updateTransfer_gui, item);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::on(DownloadManagerListener::Complete, Download *dl) throw()
{
	transferComplete_client(dl);
}

void MainWindow::on(DownloadManagerListener::Failed, Download *dl, const string &reason) throw()
{
	UserID id(dl->getUserConnection()->getUser(), TRUE);
	TransferItem *item = getTransferItem(id);
	string target = Text::acpToUtf8(dl->getTarget());

	if (dl->isSet(Download::FLAG_USER_LIST))
		item->setFile("Filelist");
	else if (dl->isSet(Download::FLAG_TREE_DOWNLOAD))
		item->setFile("TTH: " + Util::getFileName(target));
	else
		item->setFile(Util::getFileName(target));

	item->setPath(Util::getFilePath(target));
	item->setStatus(reason, true);
	item->setSize(dl->getSize());
	item->setSortOrder("w" + item->nicks + item->hubs);

	UFunc *func = new UFunc(this, &MainWindow::updateTransfer_gui, item);
	WulforManager::get()->dispatchGuiFunc(func);
}

//From Upload manager
void MainWindow::on(UploadManagerListener::Starting, Upload *ul) throw()
{
	UserID id(ul->getUserConnection()->getUser(), FALSE);
	TransferItem *item = getTransferItem(id);
	string target = Text::acpToUtf8(ul->getFileName());

	if (ul->isSet(Download::FLAG_USER_LIST))
		item->setFile("Filelist");
	else if (ul->isSet(Download::FLAG_TREE_DOWNLOAD))
		item->setFile("TTH: " + Util::getFileName(target));
	else
		item->setFile(Util::getFileName(target));

	item->setPath(Util::getFilePath(target));
	item->setStatus("Upload starting...");
	item->setSize(ul->getSize());
	item->setSortOrder("u" + item->nicks + item->hubs);

	UFunc *func = new UFunc(this, &MainWindow::updateTransfer_gui, item);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(UploadManagerListener::Tick, const Upload::List &list) throw()
{
	string status, time, bytes;
	double percent;
	Upload* ul;
	Upload::List::const_iterator it;
	TransferItem *item;

	for (it = list.begin(); it != list.end(); it++)
	{
		ul = *it;
		ostringstream stream;
		UserID id(ul->getUserConnection()->getUser(), FALSE);
		item = getTransferItem(id);

		bytes = Util::formatBytes(ul->getPos());
		percent = (double)(ul->getPos() * 100.0) / ul->getSize();
		time = Util::formatSeconds((GET_TICK() - ul->getStart()) / 1000);
		stream << setiosflags(ios::fixed) << setprecision(1);
		stream << "Uploaded " << bytes << " (" << percent << "%) in " << time;

		status.clear();
/*		Future flags in DC++ > 0.674
		if (ul->getUserConnection()->isSecure())
			status += "[S]";
*/		if (ul->isSet(Upload::FLAG_ZUPLOAD))
			status += "[Z]";
		if (!status.empty())
			status += " ";
		status += stream.str();

		item->setStatus(status);
		item->setTime(Util::formatSeconds(ul->getSecondsLeft()));
		item->setSpeed(ul->getRunningAverage());
		item->setProgress((int)percent);

		UFunc *func = new UFunc(this, &MainWindow::updateTransfer_gui, item);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::on(UploadManagerListener::Complete, Upload *ul) throw()
{
	transferComplete_client(ul);
}

//From logmanager
void MainWindow::on(LogManagerListener::Message, const string& str) throw() {
	typedef Func2<MainWindow, GtkStatusbar *, string> F2;
	F2 *func = new F2(this, &MainWindow::setStatus_gui, mainStatus, str);
	WulforManager::get()->dispatchGuiFunc(func);
}

