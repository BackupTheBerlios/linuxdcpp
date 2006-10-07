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

#include "mainwindow.hh"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <client/FavoriteManager.h>
#include <client/ShareManager.h>
#include "eggtrayicon.h"
#include "sharebrowser.hh"
#include "hub.hh"
#include "wulformanager.hh"

using namespace std;

MainWindow::MainWindow():
	Entry("Main Window", "mainwindow.glade"),
	lastUpdate(0)
{
	createWindow_gui();
	createTrayIcon_gui();
	Func0<MainWindow> *f0 = new Func0<MainWindow>(this, &MainWindow::startSocket_client);
	WulforManager::get()->dispatchClientFunc(f0);

	QueueManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	LogManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
	ConnectionManager::getInstance()->addListener(this);
}

MainWindow::~MainWindow()
{
	QueueManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	LogManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);
	ConnectionManager::getInstance()->removeListener(this);

	// Save window state and position
	int posX, posY, sizeX, sizeY, transferPanePosition;
	int state = 1;
	GdkWindowState gdkState;
	WulforSettingsManager *sm = WulforSettingsManager::get();

	gtk_window_get_position(window, &posX, &posY);
	gtk_window_get_size(window, &sizeX, &sizeY);
	gdkState = gdk_window_get_state(GTK_WIDGET(window)->window);
	transferPanePosition = gtk_paned_get_position(GTK_PANED(getWidget("pane")));

	if (!(gdkState & GDK_WINDOW_STATE_MAXIMIZED))
	{
		state = 0;
		// The get pos/size functions return junk when window is maximized
		sm->set("main-window-pos-x", posX);
		sm->set("main-window-pos-y", posY);
		sm->set("main-window-size-x", sizeX);
		sm->set("main-window-size-y", sizeY);
	}

	sm->set("main-window-maximized", state);
	sm->set("transfer-pane-position", transferPanePosition);

	// Make sure all windows are deallocated
	gtk_widget_destroy(getWidget("connectDialog"));
	gtk_widget_destroy(getWidget("exitDialog"));
	gtk_widget_destroy(getWidget("flistDialog"));
	gtk_widget_destroy(getWidget("aboutDialog"));
	gtk_widget_destroy(GTK_WIDGET(window));
	gtk_widget_destroy(trayIcon);

	// Make sure the pixmaps are freed (using gtk's ref counting).
	g_object_unref(G_OBJECT(uploadPic));
	g_object_unref(G_OBJECT(downloadPic));

	// Free the transferMap
	hash_map<UserID, TransferItem *, HashPair>::iterator it;
	for (it = transferMap.begin(); it != transferMap.end(); ++it)
	{
		gtk_tree_row_reference_free(it->second->rowRef);
		delete it->second;
	}
}

void MainWindow::createWindow_gui()
{
	// Configure the dialogs
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("exitDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("connectDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("flistDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	window = GTK_WINDOW(getWidget("mainWindow"));

	// Load icons. We need to do this in the code and not in the .glade file,
	// otherwise we won't always find the images.
	string file, path = WulforManager::get()->getPath() + "/pixmaps/";

	// Set the toolbar and transfer view icons.
	if (!WGETI("use-stock-icons"))
	{
		file = path + "connect.png";
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(getWidget("connect")), gtk_image_new_from_file(file.c_str()));
		file = path + "publichubs.png";
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(getWidget("publicHubs")), gtk_image_new_from_file(file.c_str()));
		file = path + "search.png";
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(getWidget("search")), gtk_image_new_from_file(file.c_str()));
		file = path + "settings.png";
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(getWidget("settings")), gtk_image_new_from_file(file.c_str()));
		file = path + "hash.png";
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(getWidget("hash")), gtk_image_new_from_file(file.c_str()));
		file = path + "FinishedDL.png";
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(getWidget("finishedDownloads")), gtk_image_new_from_file(file.c_str()));
		file = path + "FinishedUL.png";
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(getWidget("finishedUploads")), gtk_image_new_from_file(file.c_str()));
		file = path + "queue.png";
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(getWidget("queue")), gtk_image_new_from_file(file.c_str()));
		file = path + "favhubs.png";
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(getWidget("favHubs")), gtk_image_new_from_file(file.c_str()));
		file = path + "quit.png";
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(getWidget("quit")), gtk_image_new_from_file(file.c_str()));

		file = path + "upload.png";
		uploadPic = gdk_pixbuf_new_from_file(file.c_str(), NULL);
		file = path + "download.png";
		downloadPic = gdk_pixbuf_new_from_file(file.c_str(), NULL);
	}
	else
	{
		uploadPic = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), GTK_STOCK_GO_UP, 16, (GtkIconLookupFlags)0, NULL);
		downloadPic = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), GTK_STOCK_GO_DOWN, 16, (GtkIconLookupFlags)0, NULL);
	}

	// Set the about menu icon
	file = path + "linuxdcpp.png";
	GdkPixbuf *logo = gdk_pixbuf_new_from_file(file.c_str(), NULL);
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(getWidget("aboutDialog")), logo);
	g_object_unref(logo);

	// Set all windows to the default icon
	file = path + "linuxdcpp-icon.png";
	gtk_window_set_icon_from_file(window, file.c_str(), NULL);
	gtk_window_set_default_icon_from_file(file.c_str(), NULL);

	// Disable un-implemented menu items.
	gtk_widget_set_sensitive(getWidget("openDownloadsDirectoryMenuItem"), FALSE);
	gtk_widget_set_sensitive(getWidget("followLastRedirectMenuItem"), FALSE);
	gtk_widget_set_sensitive(getWidget("favoriteUsersMenuItem"), FALSE);
	gtk_widget_set_sensitive(getWidget("adlSearchMenuItem"), FALSE);
	gtk_widget_set_sensitive(getWidget("searchSpyMenuItem"), FALSE);
	gtk_widget_set_sensitive(getWidget("networkStatisticsMenuItem"), FALSE);

	// Initialize transfer treeview
	transferView.setView(GTK_TREE_VIEW(getWidget("transfers")), TRUE, "main");
	transferView.insertColumn("User", G_TYPE_STRING, TreeView::PIXBUF_STRING, 150, "Icon");
	transferView.insertColumn("Hub Name", G_TYPE_STRING, TreeView::STRING, 100);
	if (SETTING(SHOW_PROGRESS_BARS))
		transferView.insertColumn("Status", G_TYPE_STRING, TreeView::PROGRESS, 250, "Progress");
	else
		transferView.insertColumn("Status", G_TYPE_STRING, TreeView::STRING, 250);
	transferView.insertColumn("Time Left", G_TYPE_STRING, TreeView::STRING, 85);
	transferView.insertColumn("Speed", G_TYPE_STRING, TreeView::STRING, 125);
	transferView.insertColumn("Filename", G_TYPE_STRING, TreeView::STRING, 200);
	transferView.insertColumn("Size", G_TYPE_STRING, TreeView::STRING, 125);
	transferView.insertColumn("Path", G_TYPE_STRING, TreeView::STRING, 200);
	transferView.insertColumn("IP", G_TYPE_STRING, TreeView::STRING, 175);
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

	// All notebooks created in glade need one page.
	// In our case, this is just a placeholder, so we remove it.
	gtk_notebook_remove_page(GTK_NOTEBOOK(getWidget("book")), -1);

	// Connect the signals to their callback functions.
	g_signal_connect(window, "delete-event", G_CALLBACK(deleteWindow_gui), (gpointer)this);
	g_signal_connect(window, "key-press-event", G_CALLBACK(onKeyPressed_gui), (gpointer)this);
	g_signal_connect(getWidget("getFileListItem"), "activate", G_CALLBACK(onGetFileListClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("matchQueueItem"), "activate", G_CALLBACK(onMatchQueueClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("sendPrivateMessageItem"), "activate", G_CALLBACK(onPrivateMessageClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("addToFavoritesItem"), "activate", G_CALLBACK(onAddFavoriteUserClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("grantExtraSlotItem"), "activate", G_CALLBACK(onGrantExtraSlotClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeUserItem"), "activate", G_CALLBACK(onRemoveUserFromQueueClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("forceAttemptItem"), "activate", G_CALLBACK(onForceAttemptClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("closeConnectionItem"), "activate", G_CALLBACK(onCloseConnectionClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("connect"), "clicked", G_CALLBACK(connectClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("quickConnectMenuItem"), "activate", G_CALLBACK(connectClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("publicHubs"), "clicked", G_CALLBACK(pubHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("publicHubsMenuItem"), "activate", G_CALLBACK(pubHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("search"), "clicked", G_CALLBACK(searchClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchMenuItem"), "activate", G_CALLBACK(searchClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("settings"), "clicked", G_CALLBACK(settingsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("settingsMenuItem"), "activate", G_CALLBACK(settingsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("hash"), "clicked", G_CALLBACK(hashClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("indexingProgressMenuItem"), "activate", G_CALLBACK(hashClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("queue"), "clicked", G_CALLBACK(dlQueueClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("downloadQueueMenuItem"), "activate", G_CALLBACK(dlQueueClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("finishedDownloads"), "clicked", G_CALLBACK(finishedDLclicked_gui), (gpointer)this);
	g_signal_connect(getWidget("finishedDownloadsMenuItem"), "activate", G_CALLBACK(finishedDLclicked_gui), (gpointer)this);
	g_signal_connect(getWidget("finishedUploads"), "clicked", G_CALLBACK(finishedULclicked_gui), (gpointer)this);
	g_signal_connect(getWidget("finishedUploadsMenuItem"), "activate", G_CALLBACK(finishedULclicked_gui), (gpointer)this);
	g_signal_connect(getWidget("favHubs"), "clicked", G_CALLBACK(favHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("favoriteHubsMenuItem"), "activate", G_CALLBACK(favHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("reconnectMenuItem"), "activate", G_CALLBACK(reconnectClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("closeMenuItem"), "activate", G_CALLBACK(closeClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("exitMenuItem"), "activate", G_CALLBACK(quitClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("quit"), "clicked", G_CALLBACK(quitClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("aboutMenuItem"), "activate", G_CALLBACK(aboutClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("openFileListMenuItem"), "activate", G_CALLBACK(openFileList_gui), (gpointer)this);
	g_signal_connect(getWidget("openOwnListMenuItem"), "activate", G_CALLBACK(openOwnList_gui), (gpointer)this);
	g_signal_connect(getWidget("refreshFileListMenuItem"), "activate", G_CALLBACK(refreshFileList_gui), (gpointer)this);
	g_signal_connect(getWidget("book"), "switch-page", G_CALLBACK(switchPage_gui), (gpointer)this);
	g_signal_connect(transferView.get(), "button-press-event", G_CALLBACK(transferClicked_gui), (gpointer)this);
	g_signal_connect_after(getWidget("pane"), "realize", G_CALLBACK(onPaneRealized_gui), (gpointer)this);

	// Load window state and position from settings manager
	WulforSettingsManager *sm = WulforSettingsManager::get();
	int posX = sm->getInt("main-window-pos-x");
	int posY = sm->getInt("main-window-pos-y");
	int sizeX = sm->getInt("main-window-size-x");
	int sizeY = sm->getInt("main-window-size-y");

	gtk_window_move(window, posX, posY);
	gtk_window_resize(window, sizeX, sizeY);
	if (sm->getInt("main-window-maximized"))
		gtk_window_maximize(window);

	GtkWidget *dummy;
	GtkRequisition req;
	dummy = gtk_statusbar_new();
	gtk_widget_size_request(dummy, &req);
	gtk_widget_destroy(dummy);
	emptyStatusWidth = req.width;

	gtk_statusbar_push(GTK_STATUSBAR(getWidget("status1")), 0, "Welcome to Linux DC++");

	// Putting this after all the resizing and moving makes the window appear
	// in the correct position instantly, looking slightly more cool
	gtk_widget_show_all(GTK_WIDGET(window));
}

/*
 * Create tray icon.
 * @todo: Replace with GtkStatusIcon after GTK+ 2.10 is released and on enough systems.
 */
void MainWindow::createTrayIcon_gui()
{
	string iconPath = WulforManager::get()->getPath() + "/pixmaps/linuxdcpp-icon.png";
	trayToolTip = gtk_tooltips_new();

	trayIcon = GTK_WIDGET(egg_tray_icon_new("Linux DC++"));
	GtkWidget *trayBox = gtk_event_box_new();
	GtkWidget *trayImage = gtk_image_new_from_file(iconPath.c_str());
	gtk_container_add(GTK_CONTAINER(trayBox), trayImage);
	gtk_container_add(GTK_CONTAINER(trayIcon), trayBox);

	g_signal_connect(getWidget("quitTrayItem"), "activate", G_CALLBACK(quitClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("toggleInterfaceItem"), "activate", G_CALLBACK(onToggleWindowVisibility_gui), (gpointer)this);
	g_signal_connect(trayIcon, "button-press-event", G_CALLBACK(onTrayIconClicked_gui), (gpointer)this);

	if (BOOLSETTING(MINIMIZE_TRAY))
		gtk_widget_show_all(trayIcon);
}

GtkWidget *MainWindow::getContainer()
{
	return getWidget("mainWindow");
}

void MainWindow::applyCallback(GCallback closeCallback)
{
	g_signal_connect(window, "delete-event", closeCallback, (gpointer)this);
}

void MainWindow::appendWindowItem(GtkWidget *page, string title)
{
	GtkWidget *menuItem = gtk_menu_item_new_with_label(title.c_str());
	g_signal_connect(menuItem, "activate", G_CALLBACK(onRaisePage_gui), (gpointer)page);
	gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("windowMenu")), menuItem);
	gtk_widget_show_all(getWidget("windowMenu"));
	g_object_set_data(G_OBJECT(page), "menuItem", menuItem);
}

void MainWindow::removeWindowItem(GtkWidget *page)
{
	GtkWidget *menuItem = (GtkWidget *)g_object_get_data(G_OBJECT(page), "menuItem");
	gtk_container_remove(GTK_CONTAINER(getWidget("windowMenu")), menuItem);
}

void MainWindow::modifyWindowItem(GtkWidget *page, string title)
{
	GtkBin *menuItem = (GtkBin *)g_object_get_data(G_OBJECT(page), "menuItem");
	GtkWidget *child = gtk_bin_get_child(menuItem);
	if (child && GTK_IS_LABEL(child))
		gtk_label_set_text(GTK_LABEL(child), title.c_str());
}

void MainWindow::onRaisePage_gui(GtkMenuItem *item, gpointer data)
{
	WulforManager::get()->getMainWindow()->raisePage_gui((GtkWidget *)data);
}

gboolean MainWindow::transferClicked_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		MainWindow *mw = (MainWindow *)data;
		int count = gtk_tree_selection_count_selected_rows(mw->transferSel);

		if (count < 1)
			return FALSE;
		else if (count == 1)
		{
			gtk_widget_set_sensitive(mw->getWidget("getFileListItem"), TRUE);
			gtk_widget_set_sensitive(mw->getWidget("matchQueueItem"), TRUE);
			gtk_widget_set_sensitive(mw->getWidget("sendPrivateMessageItem"), TRUE);
			gtk_widget_set_sensitive(mw->getWidget("grantExtraSlotItem"), TRUE);
			gtk_widget_set_sensitive(mw->getWidget("removeUserItem"), TRUE);
			gtk_widget_set_sensitive(mw->getWidget("addToFavoritesItem"), TRUE);
		}
		else if (count > 1)
		{
			gtk_widget_set_sensitive(mw->getWidget("getFileListItem"), FALSE);
			gtk_widget_set_sensitive(mw->getWidget("matchQueueItem"), FALSE);
			gtk_widget_set_sensitive(mw->getWidget("sendPrivateMessageItem"), FALSE);
			gtk_widget_set_sensitive(mw->getWidget("grantExtraSlotItem"), FALSE);
			gtk_widget_set_sensitive(mw->getWidget("removeUserItem"), FALSE);
			gtk_widget_set_sensitive(mw->getWidget("addToFavoritesItem"), FALSE);
		}
		mw->popup_gui(mw->getWidget("transferMenu"), event);
		return TRUE;
	}
	return FALSE;
}

void MainWindow::popup_gui(GtkWidget *menu, GdkEventButton *event)
{
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
		event ? event->button : 0,
		gdk_event_get_time((GdkEvent*)event));
	gtk_widget_show_all(menu);
}

User::Ptr MainWindow::getSelectedTransfer_gui()
{
	GList *list = gtk_tree_selection_get_selected_rows(transferSel, NULL);
	GtkTreePath *path = (GtkTreePath*)g_list_nth_data(list, 0);
	GtkTreeIter iter;
	User::Ptr user = NULL;

	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(transferStore), &iter, path))
	{
		TransferItem *item = transferView.getValue<gpointer, TransferItem*>(&iter, "TransferItem");
		user = item->user;
	}

	gtk_tree_path_free(path);
	g_list_free(list);

	return user;
}

void MainWindow::onGetFileListClicked_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow*)data;
	User::Ptr user = mw->getSelectedTransfer_gui();
	try
	{
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	}
	catch(const Exception&)
	{
	}
}

void MainWindow::onMatchQueueClicked_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow*)data;
	User::Ptr user = mw->getSelectedTransfer_gui();
	try
	{
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
	}
	catch(const Exception&)
	{
	}
}

void MainWindow::onPrivateMessageClicked_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow*)data;
	User::Ptr user = mw->getSelectedTransfer_gui();
	WulforManager::get()->addPrivMsg_gui(user);
}

void MainWindow::onAddFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow*)data;
	User::Ptr user = mw->getSelectedTransfer_gui();
	FavoriteManager::getInstance()->addFavoriteUser(user);
}

void MainWindow::onGrantExtraSlotClicked_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow*)data;
	User::Ptr user = mw->getSelectedTransfer_gui();
	UploadManager::getInstance()->reserveSlot(user);
}

void MainWindow::onRemoveUserFromQueueClicked_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow*)data;
	User::Ptr user = mw->getSelectedTransfer_gui();
	QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
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
	typedef Func1<MainWindow, const User::Ptr &> F1;
	F1 *func;

	for (int i = 0; i < count; i++)
	{
		path = (GtkTreePath *)g_list_nth_data(list, i);
		if (gtk_tree_model_get_iter(m, &iter, path))
		{
			item = mw->transferView.getValue<gpointer, TransferItem*>(&iter, "TransferItem");
			func = new F1(mw, &MainWindow::forceAttempt_client, item->user);
			WulforManager::get()->dispatchClientFunc(func);
			gtk_list_store_set(mw->transferStore, &iter, mw->transferView.col("Status"), "Connecting (forced)...", -1);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void MainWindow::forceAttempt_client(const User::Ptr &user)
{
	ClientManager::getInstance()->connect(user);
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
			ConnectionManager::getInstance()->disconnect(item->user, item->isDownload);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void MainWindow::connectClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	int response;
	string address;

	response = gtk_dialog_run(GTK_DIALOG(mw->getWidget("connectDialog")));
	gtk_widget_hide(mw->getWidget("connectDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		address = gtk_entry_get_text(GTK_ENTRY(mw->getWidget("connectEntry")));
		WulforManager::get()->addHub_gui(address);
	}
}

void MainWindow::favHubsClicked_gui(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->addFavoriteHubs_gui();
}

void MainWindow::reconnectClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *entryWidget = mw->currentPage_gui();

	if (entryWidget)
	{
		BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(entryWidget), "entry");

		if (entry && entry->getID().substr(0, 5) == "Hub: ")
		{
			Func0<Hub> *func = new Func0<Hub>(dynamic_cast<Hub *>(entry), &Hub::reconnect_client);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void MainWindow::pubHubsClicked_gui(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->addPublicHubs_gui();
}

void MainWindow::hashClicked_gui(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->openHashDialog_gui();
}

void MainWindow::searchClicked_gui(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->addSearch_gui();
}

void MainWindow::dlQueueClicked_gui(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->addDownloadQueue_gui();
}

void MainWindow::finishedDLclicked_gui(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->addFinishedTransfers_gui("Finished Downloads");
}

void MainWindow::finishedULclicked_gui(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->addFinishedTransfers_gui("Finished Uploads");
}

void MainWindow::settingsClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	typedef Func0<MainWindow> F0;

	unsigned short tcpPort = (unsigned short)SETTING(TCP_PORT);
	unsigned short udpPort = (unsigned short)SETTING(UDP_PORT);
	int lastConn = SETTING(INCOMING_CONNECTIONS);
	bool lastShowProgressSetting = BOOLSETTING(SHOW_PROGRESS_BARS);

	if (WulforManager::get()->openSettingsDialog_gui() == GTK_RESPONSE_OK)
	{
		if (SETTING(INCOMING_CONNECTIONS) != lastConn || SETTING(TCP_PORT) != tcpPort || SETTING(UDP_PORT) != udpPort)
		{
			F0 *func = new F0(mw, &MainWindow::startSocket_client);
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

		if (BOOLSETTING(MINIMIZE_TRAY))
			gtk_widget_show_all(mw->trayIcon);
		else
			gtk_widget_hide_all(mw->trayIcon);
	}
}

void MainWindow::closeClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *entryWidget = mw->currentPage_gui();

	if (entryWidget)
	{
		BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(entryWidget), "entry");

		if (entry)
			WulforManager::get()->deleteBookEntry_gui(entry);
	}
}

void MainWindow::quitClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	gboolean retVal; // Not interested in the value, though.
	g_signal_emit_by_name(mw->window, "delete-event", NULL, &retVal);
}

void MainWindow::aboutClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	gtk_dialog_run(GTK_DIALOG(mw->getWidget("aboutDialog")));
	gtk_widget_hide(mw->getWidget("aboutDialog"));
}

gboolean MainWindow::deleteWindow_gui(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	int response;

	if (!BOOLSETTING(CONFIRM_EXIT))
	{
		mw->transferView.saveSettings();
		return FALSE;
	}

	response = gtk_dialog_run(GTK_DIALOG(mw->getWidget("exitDialog")));
	gtk_widget_hide(mw->getWidget("exitDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		mw->transferView.saveSettings();
		return FALSE;
	}

	return TRUE;
}

void MainWindow::switchPage_gui(GtkNotebook *notebook, GtkNotebookPage *page, guint num, gpointer data)
{
	GtkWidget *child = gtk_notebook_get_nth_page(notebook, num);
	BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(child), "entry");

	if (entry)
		entry->unsetBold_gui();
}

void MainWindow::openHub_gui(string server, string nick, string desc, string password)
{
	WulforManager::get()->addHub_gui(server, nick, desc, password);
}

void MainWindow::autoConnect_client()
{
	FavoriteHubEntry::List &l = FavoriteManager::getInstance()->getFavoriteHubs();
	FavoriteHubEntry::List::const_iterator it;
	FavoriteHubEntry *entry;
	typedef Func4<MainWindow, string, string, string, string> F4;
	F4 *func;
	string nick;

	for (it = l.begin(); it != l.end(); ++it)
	{
		entry = *it;
		if (entry->getConnect() && (!entry->getNick().empty() || !SETTING(NICK).empty()))
		{
			if (entry->getNick().empty())
				nick = SETTING(NICK);
			else
				nick = entry->getNick();

			func = new F4(this, &MainWindow::openHub_gui, entry->getServer(), nick,
				entry->getUserDescription(), entry->getPassword());
			WulforManager::get()->dispatchGuiFunc(func);
		}
	}
}

void MainWindow::autoOpen_gui()
{
	if (BOOLSETTING(OPEN_PUBLIC))
		WulforManager::get()->addPublicHubs_gui();
	if (BOOLSETTING(OPEN_QUEUE))
		WulforManager::get()->addDownloadQueue_gui();
	if (BOOLSETTING(OPEN_FAVORITE_HUBS))
		WulforManager::get()->addFavoriteHubs_gui();
	if (BOOLSETTING(OPEN_FINISHED_DOWNLOADS))
		WulforManager::get()->addFinishedTransfers_gui("Finished Downloads");
}

void MainWindow::startSocket_client()
{
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();

	if (ClientManager::getInstance()->isActive())
	{
		try
		{
			ConnectionManager::getInstance()->listen();
		}
		catch (const Exception &e)
		{
			cerr << "StartSocket (tcp): Caught \"" << e.getError() << "\"" << endl;
		}

		try
		{
			SearchManager::getInstance()->listen();
		}
		catch (const Exception &e)
		{
			cerr << "StartSocket (udp): Caught \"" << e.getError() << "\"" << endl;
		}
	}

	ClientManager::getInstance()->infoUpdated();
}

void MainWindow::on(TimerManagerListener::Second, u_int32_t ticks) throw()
{
	if (!ticks) // valgrind complained that ticks was sometimes uninitialized
		return;

	int64_t diff = (int64_t)((lastUpdate == 0) ? ticks - 1000 : ticks - lastUpdate);
	int64_t updiff = Socket::getTotalUp() - lastUp;
	int64_t downdiff = Socket::getTotalDown() - lastDown;
	string status1, status2, status3, status4, status5, status6;

	status1 = "H: " + Client::getCounts();
	status2 = "S: " + Util::toString(SETTING(SLOTS) -
		UploadManager::getInstance()->getRunning()) + '/' +
		Util::toString(SETTING(SLOTS));
	status3 = "D: " + Util::formatBytes(Socket::getTotalDown());
	status4 = "U: " + Util::formatBytes(Socket::getTotalUp());
	if (diff > 0)
	{
		status5 = "D: " + Util::formatBytes((int64_t)(downdiff*1000)/diff) + "/s (" +
			Util::toString(DownloadManager::getInstance()->getDownloadCount()) + ")";
		status6 = "U: " + Util::formatBytes((int64_t)(updiff*1000)/diff) + "/s (" +
			Util::toString(UploadManager::getInstance()->getUploadCount()) + ")";
	}

	lastUpdate = ticks;
	lastUp = Socket::getTotalUp();
	lastDown = Socket::getTotalDown();

	typedef Func6<MainWindow, string, string, string, string, string, string> func_t;
	func_t *func = new func_t(this, &MainWindow::setStats_gui, status1, status2, status3, status4, status5, status6);
	WulforManager::get()->dispatchGuiFunc(func);

	if (BOOLSETTING(MINIMIZE_TRAY))
	{
		string toolTip = status5 + " " + status6;
		typedef Func1<MainWindow, string> F1;
		F1 *f1 = new F1(this, &MainWindow::updateTrayToolTip_gui, toolTip);
		WulforManager::get()->dispatchGuiFunc(f1);
	}
}

void MainWindow::on(QueueManagerListener::Finished, QueueItem *item, int64_t avSpeed) throw()
{
	if (item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST))
	{
		User::Ptr user = item->getCurrent()->getUser();
		string searchString = item->getSearchString();
		string listName = item->getListName();

		typedef Func4<MainWindow, User::Ptr, string, string, bool> F4;
		F4 *func = new F4(this, &MainWindow::addShareBrowser_gui, user, listName, searchString, TRUE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::addShareBrowser_gui(User::Ptr user, string listName, string searchString, bool useSetting)
{
	bool raise = useSetting ? !BOOLSETTING(POPUNDER_FILELIST) : TRUE;
	WulforManager::get()->addShareBrowser_gui(user, listName, raise);
}

void MainWindow::addPage_gui(GtkWidget *page, GtkWidget *label, bool raise)
{
	gtk_notebook_append_page(GTK_NOTEBOOK(getWidget("book")), page, label);
	g_signal_connect(label, "button-press-event", G_CALLBACK(onButtonPressPage_gui), (gpointer)this);

	if (raise)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(getWidget("book")), -1);

#if GTK_CHECK_VERSION(2, 10, 0)
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(getWidget("book")), page, TRUE);
#endif
}

void MainWindow::removePage_gui(GtkWidget *page)
{
	int num = gtk_notebook_page_num(GTK_NOTEBOOK(getWidget("book")), page);

	if (num != -1)
		gtk_notebook_remove_page(GTK_NOTEBOOK(getWidget("book")), num);
}

void MainWindow::raisePage_gui(GtkWidget *page)
{
	int num = gtk_notebook_page_num(GTK_NOTEBOOK(getWidget("book")), page);

	if (num != -1)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(getWidget("book")), num);
}

GtkWidget *MainWindow::currentPage_gui()
{
	int pageNum = gtk_notebook_get_current_page(GTK_NOTEBOOK(getWidget("book")));

	if (pageNum == -1)
		return NULL;
	else
		return gtk_notebook_get_nth_page(GTK_NOTEBOOK(getWidget("book")), pageNum);
}

void MainWindow::setStatus_gui(string statusBar, std::string text)
{
	if (statusBar != "status1")
	{
		PangoLayout *pango;
		int width;
		GtkRequisition req;

		pango = gtk_widget_create_pango_layout(GTK_WIDGET(window), text.c_str());
		pango_layout_get_pixel_size(pango, &width, NULL);
		g_object_unref(G_OBJECT(pango));
		gtk_widget_size_request(getWidget(statusBar), &req);
		if (width > req.width - emptyStatusWidth)
			gtk_widget_set_size_request(getWidget(statusBar), width + emptyStatusWidth, -1);
	}

	gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusBar)), 0);
	gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusBar)), 0, text.c_str());
}

void MainWindow::setStats_gui(std::string hub, std::string slot,
	std::string dTot, std::string uTot, std::string dl, std::string ul)
{
	setStatus_gui("status2", hub);
	setStatus_gui("status3", slot);
	setStatus_gui("status4", dTot);
	setStatus_gui("status5", uTot);
	setStatus_gui("status6", ul);
	setStatus_gui("status7", dl);
}

void MainWindow::openFileList_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string path = Text::toT(Util::getListPath());

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(mw->getWidget("flistDialog")), path.c_str());

 	int ret = gtk_dialog_run(GTK_DIALOG(mw->getWidget("flistDialog")));
	gtk_widget_hide(mw->getWidget("flistDialog"));

	if (ret == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(mw->getWidget("flistDialog")));
		path = string(temp);
		g_free(temp);

		User::Ptr user = DirectoryListing::getUserFromFilename(path);
		if (user)
			WulforManager::get()->addShareBrowser_gui(user, path);
		else
			mw->setStatus_gui("status1", "Unable to open: Older file list format detected");
	}
}

void MainWindow::openOwnList_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	typedef Func0<MainWindow> F0;
	F0 *f0 = new F0(mw, &MainWindow::openOwnList_client);
	WulforManager::get()->dispatchClientFunc(f0);

	mw->setStatus_gui("status1", "Loading file list");
}

void MainWindow::openOwnList_client()
{
	User::Ptr user = ClientManager::getInstance()->getMe();
	string path = ShareManager::getInstance()->getOwnListFile();

	// Have to use MainWindow::addShareBrowser_gui since WulforManager's has a return type
	typedef Func4<MainWindow, User::Ptr, string, string, bool> F4;
	F4 *f4 = new F4(this, &MainWindow::addShareBrowser_gui, user, path, "", FALSE);
	WulforManager::get()->dispatchGuiFunc(f4);

	typedef Func2<MainWindow, string, string> F2;
	F2 *f2 = new F2(this, &MainWindow::setStatus_gui, "status1", "File list loaded");
	WulforManager::get()->dispatchGuiFunc(f2);
}

void MainWindow::refreshFileList_gui(GtkWidget *widget, gpointer data)
{
	typedef Func0<MainWindow> F0;
	F0 *f0 = new F0((MainWindow *)data, &MainWindow::refreshFileList_client);
	WulforManager::get()->dispatchClientFunc(f0);

}

void MainWindow::refreshFileList_client()
{
	try
	{
		ShareManager::getInstance()->setDirty();
		ShareManager::getInstance()->refresh(TRUE, TRUE, FALSE);
	}
	catch (const ShareException& e)
	{
	}
}

void MainWindow::updateTransfer_gui(TransferItem *item)
{
	dcassert(item);

	GtkTreeIter iter;
	GtkTreePath *path;

	path = gtk_tree_row_reference_get_path(item->rowRef);
	if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(transferStore), &iter, path))
	{
		gtk_tree_path_free(path);
		return;
	}
	gtk_tree_path_free(path);

	if (item->isSet(TransferItem::MASK_FILE) && !item->file.empty())
		gtk_list_store_set(transferStore, &iter, transferView.col("Filename"), item->file.c_str(), -1);

	if (item->isSet(TransferItem::MASK_PATH) && !item->path.empty())
		gtk_list_store_set(transferStore, &iter, transferView.col("Path"), item->path.c_str(), -1);

	if (item->isSet(TransferItem::MASK_STATUS) && !item->status.empty())
		gtk_list_store_set(transferStore, &iter, transferView.col("Status"), item->status.c_str(), -1);

	if (item->isSet(TransferItem::MASK_TIME) && !item->time.empty())
		gtk_list_store_set(transferStore, &iter, transferView.col("Time Left"), item->time.c_str(), -1);

	if (item->isSet(TransferItem::MASK_SORT_ORDER) && !item->sortOrder.empty())
		gtk_list_store_set(transferStore, &iter, transferView.col("Sort Order"), item->sortOrder.c_str(), -1);

	if (item->isSet(TransferItem::MASK_SIZE) && item->size >= 0)
	{
		gtk_list_store_set(transferStore, &iter,
			transferView.col("Size"), Util::formatBytes(item->size).c_str(),
			transferView.col("Size Order"), item->size,
			-1);
	}

	if (item->isSet(TransferItem::MASK_SPEED) && item->speed >= 0)
	{
		gtk_list_store_set(transferStore, &iter,
			transferView.col("Speed"), Util::formatBytes(item->speed).append("/s").c_str(),
			transferView.col("Speed Order"), item->speed,
			-1);
	}

	if (item->isSet(TransferItem::MASK_PROGRESS) && item->progress >= 0 && item->progress <= 100)
		gtk_list_store_set(transferStore, &iter, transferView.col("Progress"), item->progress, -1);

	if (item->isSet(TransferItem::MASK_IP) && !item->ip.empty())
		gtk_list_store_set(transferStore, &iter, transferView.col("IP"), item->ip.c_str(), -1);
}

void MainWindow::removeTransfer_gui(TransferItem *item)
{
	GtkTreeIter iter;
	GtkTreePath *path;

	path = gtk_tree_row_reference_get_path(item->rowRef);
	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(transferStore), &iter, path))
		gtk_list_store_remove(transferStore, &iter);

	gtk_tree_path_free(path);
	gtk_tree_row_reference_free(item->rowRef);
	delete item;
	item = NULL;
}

MainWindow::TransferItem* MainWindow::getTransferItem(UserID id)
{
	TransferItem *item;

	if (transferMap.find(id) == transferMap.end())
	{
		transferMap[id] = item = new TransferItem(id.first, id.second);
		typedef Func1<MainWindow, TransferItem *> F1;
		F1 *f1 = new F1(this, &MainWindow::insertTransfer_gui, item);
		WulforManager::get()->dispatchGuiFunc(f1);
	}
	else
		item = transferMap[id];

	item->updateMask = 0;
	return item;
}

void MainWindow::insertTransfer_gui(TransferItem *item)
{
	dcassert(item);

	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *m = GTK_TREE_MODEL(transferStore);

	gtk_list_store_append(transferStore, &iter);
	path = gtk_tree_model_get_path(m, &iter);
	item->rowRef = gtk_tree_row_reference_new(m, path);
	gtk_tree_path_free(path);

	if (item->isDownload)
		gtk_list_store_set(transferStore, &iter, transferView.col("Icon"), downloadPic, -1);
	else
		gtk_list_store_set(transferStore, &iter, transferView.col("Icon"), uploadPic, -1);

	gtk_list_store_set(transferStore, &iter,
		transferView.col("User"), item->nicks.c_str(),
		transferView.col("Hub Name"), item->hubs.c_str(),
		transferView.col("TransferItem"), (gpointer)item,
		-1);
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

	if (transferMap.find(id) != transferMap.end())
	{
		TransferItem *item = transferMap[id];
		transferMap.erase(id);
		typedef Func1 <MainWindow, TransferItem *> F1;
		F1 *func = new F1(this, &MainWindow::removeTransfer_gui, item);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::on(ConnectionManagerListener::Failed, ConnectionQueueItem *cqi, const string &reason) throw()
{
	UserID id(cqi->getUser(), cqi->getDownload());
	TransferItem *item = getTransferItem(id);
	item->setStatus(reason, TRUE);
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
	item->setIp(dl->getUserConnection()->getRemoteIp());

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

	for (it = list.begin(); it != list.end(); ++it)
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
		if (dl->getUserConnection()->isSecure())
			status += "[S]";
		if (dl->isSet(Download::FLAG_TTH_CHECK))
			status += "[T]";
		if (dl->isSet(Download::FLAG_ZDOWNLOAD))
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
	item->setStatus(reason, TRUE);
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
	item->setIp(ul->getUserConnection()->getRemoteIp());

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

	for (it = list.begin(); it != list.end(); ++it)
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
		if (ul->getUserConnection()->isSecure())
			status += "[S]";
		if (ul->isSet(Upload::FLAG_ZUPLOAD))
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
void MainWindow::on(LogManagerListener::Message, time_t t, const string& str) throw()
{
	string message = "[" + Util::getShortTimeString(t) + "] " + str;
	typedef Func2<MainWindow, string, string> F2;
	F2 *func = new F2(this, &MainWindow::setStatus_gui, "status1", message);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::onTrayIconClicked_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
		onToggleWindowVisibility_gui(NULL, data);
	else if (event->type == GDK_BUTTON_PRESS && event->button == 3)
		mw->popup_gui(mw->getWidget("trayMenu"), event);
}

void MainWindow::onToggleWindowVisibility_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWindow *win = mw->window;
	static int x, y;
	static bool isMaximized, isIconified;

	if (GTK_WIDGET_VISIBLE(win))
	{
		GdkWindowState state;
		gtk_window_get_position(win, &x, &y);
		state = gdk_window_get_state(GTK_WIDGET(win)->window);
		isMaximized = (state & GDK_WINDOW_STATE_MAXIMIZED);
		isIconified = (state & GDK_WINDOW_STATE_ICONIFIED);
		gtk_widget_hide_all(GTK_WIDGET(win));
	}
	else
	{
		gtk_window_move(win, x, y);
		if (isMaximized) gtk_window_maximize(win);
		if (isIconified) gtk_window_iconify(win);
		gtk_widget_show_all(GTK_WIDGET(win));
	}
}

void MainWindow::updateTrayToolTip_gui(string toolTip)
{
	gtk_tooltips_set_tip(trayToolTip, trayIcon, toolTip.c_str(), NULL);
}

gboolean MainWindow::onKeyPressed_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkNotebook *book = GTK_NOTEBOOK(mw->getWidget("book"));

	if (event->state & GDK_CONTROL_MASK)
	{
		if ((event->state & GDK_SHIFT_MASK && event->keyval == GDK_ISO_Left_Tab) || event->keyval == GDK_Left)
		{
			if (gtk_notebook_get_current_page(book) == 0)
				gtk_notebook_set_current_page(book, -1);
			else
				gtk_notebook_prev_page(book);

			return TRUE;
		}
		else if (event->keyval == GDK_Tab || event->keyval == GDK_Right)
		{
			if (gtk_notebook_get_n_pages(book) - 1 == gtk_notebook_get_current_page(book))
				gtk_notebook_set_current_page(book, 0);
			else
				gtk_notebook_next_page(book);

			return TRUE;
		}
	}

	return FALSE;
}

gboolean MainWindow::onButtonPressPage_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (event->button == 2)
	{
		GtkNotebook *book = GTK_NOTEBOOK(mw->getWidget("book"));
		GtkWidget *entryWidget;

		for (int i = 0; i < gtk_notebook_get_n_pages(book); i++)
		{
			entryWidget = gtk_notebook_get_nth_page(book, i);
			if (gtk_notebook_get_tab_label(book, entryWidget) == widget)
			{
				BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(entryWidget), "entry");

				if (entry)
					WulforManager::get()->deleteBookEntry_gui(entry);

				break;
			}
		}
		return TRUE;
	}

	return FALSE;
}

void MainWindow::onPaneRealized_gui(GtkWidget *pane, gpointer data)
{
	gtk_paned_set_position(GTK_PANED(pane), WGETI("transfer-pane-position"));
}
