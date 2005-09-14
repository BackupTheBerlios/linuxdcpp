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
#include "treeviewfactory.hh"

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
	lastUpdate(0),

	WIDTH_TYPE(20), 
	WIDTH_USER(150), 
	WIDTH_STATUS(250), 
	WIDTH_TIMELEFT(75),
	WIDTH_SPEED(175), 
	WIDTH_FILENAME(200), 
	WIDTH_SIZE(175), 
	WIDTH_PATH(200)
{
	createWindow();
	autoOpen();
	startSocket();

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

	gtk_widget_destroy(GTK_WIDGET(connectDialog));
	gtk_widget_destroy(GTK_WIDGET(exitDialog));

	//this makes sure the pixmaps are freed (using gtk:s ref counting)
	g_object_unref(G_OBJECT(uploadPic));
	g_object_unref(G_OBJECT(downloadPic));
}

void MainWindow::createWindow()
{
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

	window = GTK_WINDOW(glade_xml_get_widget(xml, "mainWindow"));
	book = GTK_NOTEBOOK(glade_xml_get_widget(xml, "book"));
	transferView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "transfers"));
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

	gtk_widget_set_sensitive(GTK_WIDGET(openDLdir), false);
	gtk_widget_set_sensitive(GTK_WIDGET(followRedirect), false);
	gtk_widget_set_sensitive(GTK_WIDGET(reconnectItem), false);
	gtk_widget_set_sensitive(GTK_WIDGET(favUsersItem), false);
	gtk_widget_set_sensitive(GTK_WIDGET(ADLSearchItem), false);
	gtk_widget_set_sensitive(GTK_WIDGET(searchSpyItem), false);
	gtk_widget_set_sensitive(GTK_WIDGET(networkStatsItem), false);

	transferStore = gtk_list_store_new(10, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, 
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	gtk_tree_view_set_model(transferView, GTK_TREE_MODEL(transferStore));
	transferSel = gtk_tree_view_get_selection(transferView);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(transferView), GTK_SELECTION_MULTIPLE);
	g_signal_connect (G_OBJECT(transferView), "button_press_event", 
		G_CALLBACK(onTransferClicked), (gpointer)this);
	TreeViewFactory factory(transferView);
	factory.addColumn_gui(COLUMN_TYPE, "", TreeViewFactory::PIXBUF, WIDTH_TYPE);
	factory.addColumn_gui(COLUMN_USER, "User", TreeViewFactory::STRING, WIDTH_USER);
	factory.addColumn_gui(COLUMN_STATUS, "Status", TreeViewFactory::STRING, WIDTH_STATUS);
	factory.addColumn_gui(COLUMN_TIMELEFT, "Time Left", TreeViewFactory::STRING, WIDTH_TIMELEFT);
	factory.addColumn_gui(COLUMN_SPEED, "Speed", TreeViewFactory::STRING, WIDTH_SPEED);
	factory.addColumn_gui(COLUMN_FILENAME, "File", TreeViewFactory::STRING, WIDTH_FILENAME);
	factory.addColumn_gui(COLUMN_SIZE, "Size", TreeViewFactory::STRING, WIDTH_SIZE);
	factory.addColumn_gui(COLUMN_PATH, "Path", TreeViewFactory::STRING, WIDTH_PATH);
	gtk_tree_view_insert_column(transferView, gtk_tree_view_column_new(), COLUMN_ID);

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
		
	popupMenu = gtk_menu_new();
	filelist = gtk_menu_item_new_with_label("Get file list");
	GtkMenuShell *pm = GTK_MENU_SHELL(popupMenu);
	gtk_menu_shell_append(pm, filelist);
	g_signal_connect(filelist, "activate", G_CALLBACK(onGetFileListClicked), this);
	matchQueue = gtk_menu_item_new_with_label("Match queue");
	gtk_menu_shell_append(pm, matchQueue);
	g_signal_connect(matchQueue, "activate", G_CALLBACK(onMatchQueueClicked), this);
	privateMessage = gtk_menu_item_new_with_label ("Send private message");
	gtk_menu_shell_append(pm, privateMessage);
	g_signal_connect(privateMessage, "activate", G_CALLBACK(onPrivateMessageClicked), this);
	addToFavorites = gtk_menu_item_new_with_label ("Add to favorites");
	gtk_menu_shell_append(pm, addToFavorites);
	g_signal_connect(addToFavorites, "activate", G_CALLBACK(onAddFavoriteUserClicked), this);
	grantExtraSlot = gtk_menu_item_new_with_label ("Grant extra slot");
	gtk_menu_shell_append(pm, grantExtraSlot);
	g_signal_connect(grantExtraSlot, "activate", G_CALLBACK(onGrantExtraSlotClicked), this);
	removeUser = gtk_menu_item_new_with_label ("Remove user from queue");
	gtk_menu_shell_append(pm, removeUser);
	g_signal_connect(removeUser, "activate", G_CALLBACK(onRemoveUserFromQueueClicked), this);
	gtk_menu_shell_append(pm, gtk_separator_menu_item_new());
	forceAttempt = gtk_menu_item_new_with_label ("Force attempt");
	gtk_menu_shell_append(pm, forceAttempt);
	g_signal_connect(forceAttempt, "activate", G_CALLBACK(onForceAttemptClicked), this);
	gtk_menu_shell_append(pm, gtk_separator_menu_item_new());
	closeConnection = gtk_menu_item_new_with_label ("Close connection");
	g_signal_connect(closeConnection, "activate", G_CALLBACK(onCloseConnectionClicked), this);
	gtk_menu_shell_append(pm, closeConnection);
		
	gtk_widget_show_all(popupMenu);
	gtk_widget_show_all(GTK_WIDGET(window));

	g_signal_connect(connectButton, "clicked", G_CALLBACK(onConnectClicked), this);
	g_signal_connect(quickConnect, "activate", G_CALLBACK(onConnectClicked), this);
	g_signal_connect(pubHubsButton, "clicked", G_CALLBACK(onPubHubsClicked), this);
	g_signal_connect(pubHubsItem, "activate", G_CALLBACK(onPubHubsClicked), this);
	g_signal_connect(queueButton, "clicked", G_CALLBACK(onDlQueueClicked), this);
	g_signal_connect(queueItem, "activate", G_CALLBACK(onDlQueueClicked), this);
	g_signal_connect(favHubsButton, "clicked", G_CALLBACK(onFavHubsClicked), this);
	g_signal_connect(favHubsItem, "activate", G_CALLBACK(onFavHubsClicked), this);
	g_signal_connect(settingsButton, "clicked", G_CALLBACK(onSettingsClicked), this);
	g_signal_connect(settingsItem, "activate", G_CALLBACK(onSettingsClicked), this);
	g_signal_connect(searchButton, "clicked", G_CALLBACK(onSearchClicked), this);
	g_signal_connect(searchItem, "activate", G_CALLBACK(onSearchClicked), this);
	g_signal_connect(hashButton, "clicked", G_CALLBACK(onHashClicked), this);
	g_signal_connect(hashItem, "activate", G_CALLBACK(onHashClicked), this);
	g_signal_connect(quitButton, "clicked", G_CALLBACK(onQuitClicked), this);
	g_signal_connect(quitItem, "activate", G_CALLBACK(onQuitClicked), this);
	g_signal_connect(finishedDL_button, "clicked", G_CALLBACK(onFinishedDLClicked), this);
	g_signal_connect(finishedDL_item, "activate", G_CALLBACK(onFinishedDLClicked), this);
	g_signal_connect(finishedUL_button, "clicked", G_CALLBACK(onFinishedULClicked), this);
	g_signal_connect(finishedUL_item, "activate", G_CALLBACK(onFinishedULClicked), this);
	g_signal_connect(finishedUL_item, "activate", G_CALLBACK(onFinishedULClicked), this);
	g_signal_connect(refreshFList, "activate", G_CALLBACK(onRefreshListClicked), this);
	g_signal_connect(openOwnFList, "activate", G_CALLBACK(onOpenFileListClicked), this);
	g_signal_connect(openFList, "activate", G_CALLBACK(onOpenFileListClicked), this);

	g_signal_connect(window, "delete-event", G_CALLBACK(deleteWindow), this);
	g_signal_connect(window, "button-release-event", G_CALLBACK(onTransferClicked), this);
	
	GtkWidget *dummy;
	GtkRequisition req;
	dummy = gtk_statusbar_new();
	gtk_widget_size_request(dummy, &req);
	gtk_widget_destroy(dummy);
	emptyStatusWidth = req.width;
	
	gtk_statusbar_push(mainStatus, 0, "Welcome to Wulfor - Reloaded");
}

gboolean MainWindow::onTransferClicked (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		if (event->button == 3)
		{
			MainWindow *mw = (MainWindow*)user_data;
			GtkTreeSelection *selection;
			GtkTreeModel *m = GTK_TREE_MODEL(mw->transferStore);
			selection = mw->transferSel;
			int count = gtk_tree_selection_count_selected_rows(selection);
						
			if (count < 1)
				return FALSE;
			else if (count == 1)
			{
				gtk_widget_set_sensitive(mw->filelist, true);
				gtk_widget_set_sensitive(mw->matchQueue, true);
				gtk_widget_set_sensitive(mw->privateMessage, true);
				gtk_widget_set_sensitive(mw->grantExtraSlot, true);
				gtk_widget_set_sensitive(mw->removeUser, true);
			}	
			else if (count > 1)
			{
				gtk_widget_set_sensitive(mw->filelist, false);
				gtk_widget_set_sensitive(mw->matchQueue, false);
				gtk_widget_set_sensitive(mw->privateMessage, false);
				gtk_widget_set_sensitive(mw->grantExtraSlot, false);
				gtk_widget_set_sensitive(mw->removeUser, false);			
			}

			gtk_menu_popup(GTK_MENU(mw->popupMenu), NULL, NULL, NULL, NULL,
				(event != NULL) ? 1 : 0,
				gdk_event_get_time((GdkEvent*)event));	

			return TRUE;
		}
	}
	return FALSE;
}

User::Ptr MainWindow::getSelectedTransfer()
{
	GtkTreeSelection *selection;
	GtkTreeModel *model = GTK_TREE_MODEL(transferStore);
	selection = transferSel;

	GList *list = gtk_tree_selection_get_selected_rows(selection, &model);
	GList *first = g_list_first(list);
	GtkTreeIter iter;
	
	if (!first)	return NULL;
	if (!gtk_tree_model_get_iter(model, &iter, (GtkTreePath*)first->data)) 
		return NULL;
	
	ConnectionQueueItem *qi;
	gtk_tree_model_get (model, &iter, COLUMN_USERPTR, &qi, -1);
	return qi->getUser();
}

GtkWindow *MainWindow::getWindow() {
	return window;
}

void MainWindow::onGetFileListClicked(GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	User::Ptr user = mw->getSelectedTransfer();
	try 
	{
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} 
	catch(const Exception&) 
	{
	}
}

void MainWindow::onMatchQueueClicked(GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	User::Ptr user = mw->getSelectedTransfer();
	try 
	{
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
	} 
	catch(const Exception&) 
	{
	}
}

void MainWindow::onPrivateMessageClicked(GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	User::Ptr user = mw->getSelectedTransfer();
	WulforManager::get()->addPrivMsg(user);
}

void MainWindow::onAddFavoriteUserClicked(GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	User::Ptr user = mw->getSelectedTransfer();
	HubManager::getInstance()->addFavoriteUser(user);
}

void MainWindow::onGrantExtraSlotClicked(GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	User::Ptr user = mw->getSelectedTransfer();
	UploadManager::getInstance()->reserveSlot(user);
}

void MainWindow::onRemoveUserFromQueueClicked(GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	User::Ptr user = mw->getSelectedTransfer();
	QueueManager::getInstance()->removeSources(user, QueueItem::Source::FLAG_REMOVED);
}

void MainWindow::onForceAttemptClicked(GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *m = GTK_TREE_MODEL(mw->transferStore);
	selection = mw->transferSel;

	GList *list = gtk_tree_selection_get_selected_rows(selection, &m);
	GList *tmp = g_list_first(list);
	vector<User::Ptr> user;
	GtkTreeIter tmpiter;
	
	if (!tmp)
		return;
	
	while (1)
	{
		if (gtk_tree_model_get_iter(m, &tmpiter, (GtkTreePath*)tmp->data))
		{
			user.push_back(TreeViewFactory::getValue<gpointer,ConnectionQueueItem*>(m, &tmpiter, COLUMN_USERPTR)->getUser());
			gtk_list_store_set(mw->transferStore, &tmpiter, COLUMN_STATUS, "Connecting (forced)...", -1);
		}
		
		tmp = g_list_next(tmp);
		if (!tmp)
			break;
	}	
	
	for (int i=0;i<user.size ();i++)
		user[i]->connect();
}

void MainWindow::onCloseConnectionClicked(GtkMenuItem *item, gpointer user_data)
{
	MainWindow *mw = (MainWindow*)user_data;
	GtkTreeSelection *selection;
	GtkTreeModel *model = GTK_TREE_MODEL(mw->transferStore);
	selection = mw->transferSel;

	vector<User::Ptr> users;
	vector<bool> isDownload;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(selection, &model);
	
	if (!g_list_first(list)) return;
	
	for (list = g_list_first(list); list != NULL; list = g_list_next(list))
	{
		if (gtk_tree_model_get_iter(model, &iter, (GtkTreePath*)list->data))
		{
			ConnectionQueueItem *qi;
			gtk_tree_model_get(model, &iter, COLUMN_USERPTR, &qi, -1);

			if (qi->getConnection())
			{
				if (qi->getConnection()->isSet(UserConnection::FLAG_UPLOAD))
					isDownload.push_back(false);
				else
					isDownload.push_back(true);
					
				users.push_back (qi->getUser());
			}
		}
	}	

	for (int i=0; i<users.size(); i++)
		ConnectionManager::getInstance()->removeConnection(users[i], isDownload[i]);
}

void MainWindow::onConnectClicked(GtkWidget *widget, gpointer data) {
	int response;
	string address;
	MainWindow *mw = (MainWindow *)data;
	
	gtk_widget_show_all(GTK_WIDGET(mw->connectDialog));
	response = gtk_dialog_run(mw->connectDialog);
	gtk_widget_hide(GTK_WIDGET(mw->connectDialog));
	address = gtk_entry_get_text(mw->connectEntry);
	
	if (response == GTK_RESPONSE_OK) {
		WulforManager::get()->addHub(address);
	}
}

void MainWindow::onPubHubsClicked(GtkWidget *widget, gpointer data) {
	WulforManager::get()->addPublicHubs();
}

void MainWindow::onSearchClicked(GtkWidget *widget, gpointer data) {
	WulforManager::get()->addSearch();
}

void MainWindow::onHashClicked(GtkWidget *widget, gpointer data) {
	Hash h;
	h.run();
}

void MainWindow::onDlQueueClicked(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->addDownloadQueue();
}

void MainWindow::onFavHubsClicked(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->addFavoriteHubs();
}

void MainWindow::onFinishedDLClicked(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->addFinishedDownloads();
}

void MainWindow::onFinishedULClicked(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->addFinishedUploads();
}

void MainWindow::onSettingsClicked(GtkWidget *widget, gpointer data)
{
	Settings s;
	short lastPort = (short)SETTING(IN_PORT);
	int lastConn = SETTING(CONNECTION_TYPE);	
	MainWindow *mw = (MainWindow *)data;

	if (s.run() == GTK_RESPONSE_OK)
	{
		s.saveSettings_client();
		SettingsManager::getInstance()->save();

		if (SETTING(CONNECTION_TYPE) != lastConn || SETTING(IN_PORT) != lastPort)
		{
			Selecter::quit();
			mw->startSocket();
		}		
	}
}

void MainWindow::onRefreshListClicked(GtkWidget *widget, gpointer data)
{
	ShareManager::getInstance()->refresh(true, true, false);
}

void MainWindow::onOpenFileListClicked(GtkWidget *widget, gpointer data)
{
	string name = "Own List";
	string path = Util::getDataPath() + "MyList.DcLst";
	MainWindow *mw = (MainWindow *)data;

	if (widget == GTK_WIDGET(mw->openFList))
	{
		GtkWidget *listSelection = gtk_file_chooser_dialog_new( "Select filelist to browse",
			NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(listSelection), 
			Text::toT(Util::getDataPath() + "FileLists/").c_str());
	
 		int ret = gtk_dialog_run (GTK_DIALOG(listSelection));
		path = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(listSelection) );
		gtk_widget_destroy(listSelection);

		if (ret != GTK_RESPONSE_OK) return;

		name = g_path_get_basename(path.c_str());
	}

	//Pointer is ref counted, should be no need to delete
	User::Ptr dummy = new User(name);
	WulforManager::get()->addShareBrowser(dummy, path);
}

void MainWindow::onQuitClicked(GtkWidget *widget, gpointer data) {
	gboolean retVal;		// Not interested in the value though.
	MainWindow *mw = (MainWindow *)data;
	g_signal_emit_by_name(G_OBJECT(mw->window), "delete-event", data, &retVal);
}

gboolean MainWindow::deleteWindow(
	GtkWidget *widget, GdkEvent *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	int response;
	bool quit = false;
	
	if (BOOLSETTING(CONFIRM_EXIT)) {
		gtk_widget_show_all(GTK_WIDGET(mw->exitDialog));
		response = gtk_dialog_run(mw->exitDialog);
		gtk_widget_hide(GTK_WIDGET(mw->exitDialog));
		if (response == GTK_RESPONSE_OK) quit = true;
	} else {
		quit = true;
	}

	if (quit) {
		gtk_main_quit();
		return FALSE;
	} else {
		return TRUE;
	}
}

/*
void MainWindow::switchPage_gui(GtkNotebook *notebook, 
	GtkNotebookPage *page, guint page_num, gpointer user_data)
{
	BookEntry *b = WulforManager::get()->getBookEntry_gui(page_num);
	if (b)
		b->switchedPage();
}
*/

void MainWindow::autoConnect()
{
	FavoriteHubEntry::List &l = HubManager::getInstance()->getFavoriteHubs();
	FavoriteHubEntry::List::const_iterator it;

	for (it = l.begin(); it != l.end(); it++) {
		FavoriteHubEntry *entry = *it;
		if (entry->getConnect())
			if (!entry->getNick().empty () || !SETTING(NICK).empty()) {
				string nick;
			
				if (entry->getNick().empty())
					nick = SETTING(NICK);
				else
					nick = entry->getNick();
			
				WulforManager::get()->addHub(entry->getServer(), nick, 
					entry->getUserDescription(), entry->getPassword());
			}
	}
}

void MainWindow::autoOpen()
{
	if (SETTING(OPEN_PUBLIC))
		WulforManager::get()->addPublicHubs();
	if (SETTING(OPEN_QUEUE))
		WulforManager::get()->addDownloadQueue();
	if (SETTING(OPEN_FAVORITE_HUBS))
		WulforManager::get()->addFavoriteHubs();
	if(BOOLSETTING(OPEN_FINISHED_DOWNLOADS))
		WulforManager::get()->addFinishedDownloads(); 
}

void MainWindow::startSocket()
{
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
			if (port > 32000) {
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
			if (port > 32000) {
				cout << "StartSocket (udp): Can't find a good port" << endl;
				break;
			}
		}
	}
}

void MainWindow::addPage(GtkWidget *page, GtkWidget *label, bool raise) {
	gtk_notebook_append_page(book, page, label);
	if (raise) gtk_notebook_set_current_page(book, -1);
}

void MainWindow::removePage(GtkWidget *page)
{
	int i, pageNum = -1;

	for (i=0; i<gtk_notebook_get_n_pages(book); i++)
		if (page == gtk_notebook_get_nth_page(book, i)) pageNum = i;
		
	assert(pageNum != -1);
	gtk_notebook_remove_page(book, pageNum);
}

void MainWindow::raisePage(GtkWidget *page)
{
	int i, pageNum = -1;

	for (i=0; i<gtk_notebook_get_n_pages(book); i++)
		if (page == gtk_notebook_get_nth_page(book, i)) pageNum = i;
		
	assert(pageNum != -1);
	gtk_notebook_set_current_page(book, pageNum);
}

GtkWidget *MainWindow::getCurrentPage()
{
	GtkWidget *page;
	int pageNum = gtk_notebook_get_current_page(book);
	if (pageNum == -1) page = NULL;
	else page = gtk_notebook_get_nth_page(book, pageNum);
	
	return page;	
}

void MainWindow::setStatus(GtkStatusbar *status, std::string text)
{
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

void MainWindow::updateTransfer(string id, connection_t type, ConnectionQueueItem *item, 
	string status, string time, string speed, string file, string size, string path)
{
	GtkTreeIter iter;
	findTransferId(id, &iter);
	
	if (!gtk_list_store_iter_is_valid(transferStore, &iter)) {
		gtk_list_store_append(transferStore, &iter);
		gtk_list_store_set(transferStore, &iter, COLUMN_ID, id.c_str(), -1);
	}

	if (type == CONNECTION_UL) {
		gtk_list_store_set(transferStore, &iter, COLUMN_TYPE, uploadPic, -1);
	}
	if (type == CONNECTION_DL) {
		gtk_list_store_set(transferStore, &iter, COLUMN_TYPE, downloadPic, -1);
	}
	if (item) {
		gtk_list_store_set(transferStore, &iter, 
			COLUMN_USER, item->getUser()->getNick().c_str(), 
			COLUMN_USERPTR, (gpointer)item, 
			-1);
	}
	if (status != "") {
		gtk_list_store_set(transferStore, &iter, COLUMN_STATUS, status.c_str(), -1);
	}
	if (time != "") {
		gtk_list_store_set(transferStore, &iter, COLUMN_TIMELEFT, time.c_str(), -1);
	}
	if (speed != "") {
		gtk_list_store_set(transferStore, &iter, COLUMN_SPEED, speed.c_str(), -1);
	}
	if (file != "") {
		gtk_list_store_set(transferStore, &iter, COLUMN_FILENAME, file.c_str(), -1);
	}
	if (size != "") {
		gtk_list_store_set(transferStore, &iter, COLUMN_SIZE, size.c_str(), -1);
	}
	if (path != "") {
		gtk_list_store_set(transferStore, &iter, COLUMN_PATH, path.c_str(), -1);
	}
}

void MainWindow::removeTransfer(string id)
{
	GtkTreeIter iter;
	findTransferId(id, &iter);

	if (gtk_list_store_iter_is_valid(transferStore, &iter)) {
		gtk_list_store_remove(transferStore, &iter);
	}
}

void MainWindow::findTransferId(string id, GtkTreeIter *iter)
{
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(transferStore), iter);

	while (gtk_list_store_iter_is_valid(transferStore, iter)) {
		char *t;
		string text;
		gtk_tree_model_get(GTK_TREE_MODEL(transferStore), iter, 
			COLUMN_ID, &t, -1);
		text = t;
		g_free(t);

		if (id == text) break;

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
					COLUMN_ID, id.c_str(), -1);
				break;
			}
		}

		gtk_tree_model_iter_next(GTK_TREE_MODEL(transferStore), iter);
	}
}

string MainWindow::getTransferId(ConnectionQueueItem *item)
{
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

string MainWindow::getTransferId(Transfer *t)
{
	assert (t->getUserConnection());
	assert (t->getUserConnection()->getCQI());
	return getTransferId(t->getUserConnection()->getCQI());
}

void MainWindow::transferComplete(Transfer *t)
{
	string status, id = getTransferId(t);

	if (t->getUserConnection()->isSet(UserConnection::FLAG_UPLOAD)) {
		status = "Upload finished, idle...";
	} else {
		status = "Download finished, idle...";
	}

	updateTransfer(id, CONNECTION_NA, NULL, status,	"Done", " ", "", "", "");
}

//TimerManagerListener
void MainWindow::on(TimerManagerListener::Second, u_int32_t ticks) throw()
{
	int64_t diff = (int64_t)((lastUpdate == 0) ? ticks - 1000 : 
		ticks - lastUpdate);
	int64_t updiff = Socket::getTotalUp() - lastUp;
	int64_t downdiff = Socket::getTotalDown() - lastDown;
	string hub, slot, dTot, uTot, ul, dl;

	hub = "H: " + Client::getCounts();
	slot = "S: " + Util::toString(SETTING(SLOTS) -  
		UploadManager::getInstance()->getRunning()) + '/' +
		Util::toString(SETTING(SLOTS));
	dTot = "D: " + Util::formatBytes(Socket::getTotalDown());
	uTot = "U: " + Util::formatBytes(Socket::getTotalUp());
	ul = "D: " + Util::formatBytes((int64_t)(downdiff*1000)/diff) + "/s (" +
		Util::toString(DownloadManager::getInstance()->getDownloadCount()) + ")";
	dl = "U: " + Util::formatBytes((int64_t)(updiff*1000)/diff) + "/s (" +
		Util::toString(UploadManager::getInstance()->getUploadCount()) + ")";

	lastUpdate = ticks;
	lastUp = Socket::getTotalUp();
	lastDown = Socket::getTotalDown();

	gdk_threads_enter();
	setStatus(hubStatus, hub);
	setStatus(slotStatus, slot);
	setStatus(dTotStatus, dTot);
	setStatus(uTotStatus, uTot);
	setStatus(ulStatus, ul);
	setStatus(dlStatus, dl);
	gdk_threads_leave();
}

//From Connection manager
void MainWindow::on(ConnectionManagerListener::Added, ConnectionQueueItem *item) throw()
{
	string status = "Connecting...";
	string id = getTransferId(item);
	gdk_threads_enter();
	updateTransfer(id, CONNECTION_NA, item, status, "", "", "", "", "");
	gdk_threads_leave();
}

void MainWindow::on(ConnectionManagerListener::Removed, ConnectionQueueItem *item) throw()
{
	string id = getTransferId(item);
	gdk_threads_enter();
	removeTransfer(id);
	gdk_threads_leave();
}

void MainWindow::on(ConnectionManagerListener::Failed, ConnectionQueueItem *item, const string &reason) throw()
{
	string status = reason;
	string id = getTransferId(item);
	gdk_threads_enter();
	updateTransfer(id, CONNECTION_NA, NULL, status, "", "", "", "", "");
	gdk_threads_leave();
}

void MainWindow::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem *item) throw()
{
	string status;
	string id = getTransferId(item);

	if (item->getState() == ConnectionQueueItem::CONNECTING) {
		status = "Connecting...";
	} else {
		status = "Waiting to retry...";
	}

	gdk_threads_enter();
	updateTransfer(id, CONNECTION_NA, NULL, status,	"", "", "", "", "");
	gdk_threads_leave();
}

//From QueueManagerListener
void MainWindow::on(QueueManagerListener::Finished, QueueItem *item) throw()
{
	if (!item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST)) 
		return;

	User::Ptr user = item->getCurrent()->getUser();
	string searchString = item->getSearchString();
	string listName = item->getListName();

	if (item->isSet(QueueItem::FLAG_CLIENT_VIEW) && 
		item->isSet(QueueItem::FLAG_USER_LIST))
	{
		ShareBrowser *browser;
		gdk_threads_enter();
		browser = WulforManager::get()->addShareBrowser(user, listName);
		browser->setPosition(searchString);
		gdk_threads_leave();
	}
}

//From Download manager
void MainWindow::on(DownloadManagerListener::Starting, Download *dl) throw()
{
	string status, size, path, file, target;
	string id = getTransferId(dl);
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

	gdk_threads_enter();
	updateTransfer(id, CONNECTION_DL, NULL, status, "", "", file, size, path);
	gdk_threads_leave();
}

void MainWindow::on(DownloadManagerListener::Tick, const Download::List &list) throw()
{
	string id, status, timeLeft, speed;
	Download::List::const_iterator it;
	
	for (it = list.begin(); it != list.end(); it++)
	{
		Download* dl = *it;
		ostringstream stream;
		id = getTransferId(dl); 

		string bytes = Util::formatBytes(dl->getPos());
		double percent = (double)(dl->getPos() * 100.0) / dl->getSize();
		string time = Util::formatSeconds((GET_TICK() - dl->getStart()) / 1000);
		stream << setiosflags(ios::fixed) << setprecision(1);
		stream << "Downloaded " << bytes << " (" << percent << "%) in " << time;

		timeLeft = Util::formatSeconds(dl->getSecondsLeft());
		speed = Util::formatBytes(dl->getRunningAverage()) + "/s";

		if (dl->isSet(Download::FLAG_ZDOWNLOAD))
			status = "* " + stream.str();
		else
			status = stream.str();

		gdk_threads_enter();
		updateTransfer(id, CONNECTION_NA, NULL, status, timeLeft, speed, "", "", "");
		gdk_threads_leave();
	}
}

void MainWindow::on(DownloadManagerListener::Complete, Download *dl) throw()
{
	gdk_threads_enter();
	transferComplete(dl);
	gdk_threads_leave();
}

void MainWindow::on(DownloadManagerListener::Failed, Download *dl, const string &reason) throw()
{
	string status, size, file, path, target;
	string id = getTransferId(dl); 

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

	gdk_threads_enter();
	updateTransfer(id, CONNECTION_NA, NULL, status,	"", "", file, size, path);
	gdk_threads_leave();
}

//From Upload manager
void MainWindow::on(UploadManagerListener::Starting, Upload *ul) throw()
{
	string status, size, path, file, target;
	string id = getTransferId(ul);

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

	gdk_threads_enter();
	updateTransfer(id, CONNECTION_UL, NULL, status,	"", "", file, size, path);
	gdk_threads_leave();
}

void MainWindow::on(UploadManagerListener::Tick, const Upload::List &list) throw()
{
	string id, status, timeLeft, speed;
	Upload::List::const_iterator it;
	
	for (it = list.begin(); it != list.end(); it++) {
		Upload* ul = *it;
		ostringstream stream;
		id = getTransferId(ul); 

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

		gdk_threads_enter();
		updateTransfer(id, CONNECTION_NA, NULL, status,	timeLeft, speed, "", "", "");
		gdk_threads_leave();
	}
}

void MainWindow::on(UploadManagerListener::Complete, Upload *ul) throw()
{
	gdk_threads_enter();
	transferComplete(ul);
	gdk_threads_leave();
}

//From logmanager
void MainWindow::on(LogManagerListener::Message, const string& str) throw()
{
	gdk_threads_enter();
	setStatus(mainStatus, str);
	gdk_threads_leave();
}

