/*
 * Copyright © 2004-2008 Jens Oknelid, paskharen@gmail.com
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
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include "mainwindow.hh"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <client/FavoriteManager.h>
#include <client/ShareManager.h>
#include <client/Text.h>
#include "downloadqueue.hh"
#include "eggtrayicon.h"
#include "favoritehubs.hh"
#include "finishedtransfers.hh"
#include "func.hh"
#include "hub.hh"
#include "privatemessage.hh"
#include "publichubs.hh"
#include "search.hh"
#include "settingsmanager.hh"
#include "sharebrowser.hh"
#include "UserCommandMenu.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;

MainWindow::MainWindow():
	Entry(Entry::MAIN_WINDOW, "mainwindow.glade"),
	lastUpdate(0),
	lastUp(0),
	lastDown(0),
	minimized(FALSE)
{
	// Configure the dialogs
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("exitDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("connectDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("flistDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	window = GTK_WINDOW(getWidget("mainWindow"));
	gtk_window_set_role(window, getID().c_str());

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
	gtk_about_dialog_set_email_hook((GtkAboutDialogActivateLinkFunc)onAboutDialogActivateLink_gui, (gpointer)this, NULL);
	gtk_about_dialog_set_url_hook((GtkAboutDialogActivateLinkFunc)onAboutDialogActivateLink_gui, (gpointer)this, NULL);
	// This has to be set in code in order to activate the link
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(getWidget("aboutDialog")), "http://linuxdcpp.berlios.de");

	// Set all windows to the default icon
	file = path + "linuxdcpp-icon.png";
	gtk_window_set_icon_from_file(window, file.c_str(), NULL);
	gtk_window_set_default_icon_from_file(file.c_str(), NULL);

	// Disable un-implemented menu items.
	gtk_widget_set_sensitive(getWidget("favoriteUsersMenuItem"), FALSE);
	gtk_widget_set_sensitive(getWidget("addToFavoritesItem"), FALSE);

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
	transferView.insertHiddenColumn("Progress", G_TYPE_INT);
	transferView.insertHiddenColumn("Sort Order", G_TYPE_STRING);
	transferView.insertHiddenColumn("Speed Order", G_TYPE_INT64);
	transferView.insertHiddenColumn("Size Order", G_TYPE_INT64);
	transferView.insertHiddenColumn("CID", G_TYPE_STRING);
	transferView.finalize();
	transferStore = gtk_list_store_newv(transferView.getColCount(), transferView.getGTypes());
	gtk_tree_view_set_model(transferView.get(), GTK_TREE_MODEL(transferStore));
	g_object_unref(transferStore);
	transferSelection = gtk_tree_view_get_selection(transferView.get());
	gtk_tree_selection_set_mode(transferSelection, GTK_SELECTION_MULTIPLE);
	transferView.setSortColumn_gui("User", "Sort Order");
	transferView.setSortColumn_gui("Speed", "Speed Order");
	transferView.setSortColumn_gui("Size", "Size Order");
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(transferStore), transferView.col("Sort Order"), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(transferView.get(), transferView.col("User")), TRUE);
	gtk_tree_view_set_fixed_height_mode(transferView.get(), TRUE);

	// All notebooks created in glade need one page.
	// In our case, this is just a placeholder, so we remove it.
	gtk_notebook_remove_page(GTK_NOTEBOOK(getWidget("book")), -1);
	g_object_set_data(G_OBJECT(getWidget("book")), "page-rotation-list", NULL);
	gtk_widget_set_sensitive(getWidget("closeMenuItem"), FALSE);

	// Initialize the user command menu
	userCommandMenu = new UserCommandMenu(getWidget("userCommandMenu"), ::UserCommand::CONTEXT_CHAT);
	addChild(userCommandMenu);

	// Connect the signals to their callback functions.
	g_signal_connect(window, "delete-event", G_CALLBACK(onCloseWindow_gui), (gpointer)this);
	g_signal_connect(window, "window-state-event", G_CALLBACK(onWindowState_gui), (gpointer)this);
	g_signal_connect(window, "focus-in-event", G_CALLBACK(onFocusIn_gui), (gpointer)this);
	g_signal_connect(window, "key-press-event", G_CALLBACK(onKeyPressed_gui), (gpointer)this);
	g_signal_connect(transferView.get(), "button-press-event", G_CALLBACK(onTransferButtonPressed_gui), (gpointer)this);
	g_signal_connect(transferView.get(), "button-release-event", G_CALLBACK(onTransferButtonReleased_gui), (gpointer)this);
	g_signal_connect(getWidget("book"), "switch-page", G_CALLBACK(onPageSwitched_gui), (gpointer)this);
	g_signal_connect_after(getWidget("pane"), "realize", G_CALLBACK(onPaneRealized_gui), (gpointer)this);
	g_signal_connect(getWidget("connect"), "clicked", G_CALLBACK(onConnectClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("favHubs"), "clicked", G_CALLBACK(onFavoriteHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("publicHubs"), "clicked", G_CALLBACK(onPublicHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("settings"), "clicked", G_CALLBACK(onPreferencesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("hash"), "clicked", G_CALLBACK(onHashClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("search"), "clicked", G_CALLBACK(onSearchClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("queue"), "clicked", G_CALLBACK(onDownloadQueueClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("quit"), "clicked", G_CALLBACK(onQuitClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("finishedDownloads"), "clicked", G_CALLBACK(onFinishedDownloadsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("finishedUploads"), "clicked", G_CALLBACK(onFinishedUploadsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("openFileListMenuItem"), "activate", G_CALLBACK(onOpenFileListClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("openOwnListMenuItem"), "activate", G_CALLBACK(onOpenOwnListClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("refreshFileListMenuItem"), "activate", G_CALLBACK(onRefreshFileListClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("quickConnectMenuItem"), "activate", G_CALLBACK(onConnectClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("reconnectMenuItem"), "activate", G_CALLBACK(onReconnectClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("settingsMenuItem"), "activate", G_CALLBACK(onPreferencesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("closeMenuItem"), "activate", G_CALLBACK(onCloseClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("exitMenuItem"), "activate", G_CALLBACK(onQuitClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("favoriteHubsMenuItem"), "activate", G_CALLBACK(onFavoriteHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("publicHubsMenuItem"), "activate", G_CALLBACK(onPublicHubsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("indexingProgressMenuItem"), "activate", G_CALLBACK(onHashClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchMenuItem"), "activate", G_CALLBACK(onSearchClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("downloadQueueMenuItem"), "activate", G_CALLBACK(onDownloadQueueClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("finishedDownloadsMenuItem"), "activate", G_CALLBACK(onFinishedDownloadsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("finishedUploadsMenuItem"), "activate", G_CALLBACK(onFinishedUploadsClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("previousTabMenuItem"), "activate", G_CALLBACK(onPreviousTabClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("nextTabMenuItem"), "activate", G_CALLBACK(onNextTabClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("aboutMenuItem"), "activate", G_CALLBACK(onAboutClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("getFileListItem"), "activate", G_CALLBACK(onGetFileListClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("matchQueueItem"), "activate", G_CALLBACK(onMatchQueueClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("sendPrivateMessageItem"), "activate", G_CALLBACK(onPrivateMessageClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("addToFavoritesItem"), "activate", G_CALLBACK(onAddFavoriteUserClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("grantExtraSlotItem"), "activate", G_CALLBACK(onGrantExtraSlotClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeUserItem"), "activate", G_CALLBACK(onRemoveUserFromQueueClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("forceAttemptItem"), "activate", G_CALLBACK(onForceAttemptClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("closeConnectionItem"), "activate", G_CALLBACK(onCloseConnectionClicked_gui), (gpointer)this);

	// Load window state and position from settings manager
	gint posX = WGETI("main-window-pos-x");
	gint posY = WGETI("main-window-pos-y");
	gint sizeX = WGETI("main-window-size-x");
	gint sizeY = WGETI("main-window-size-y");

	gtk_window_move(window, posX, posY);
	gtk_window_resize(window, sizeX, sizeY);
	if (WGETI("main-window-maximized"))
		gtk_window_maximize(window);

	GtkWidget *dummy;
	GtkRequisition req;
	dummy = gtk_statusbar_new();
	gtk_widget_size_request(dummy, &req);
	gtk_widget_destroy(dummy);
	emptyStatusWidth = req.width;

	setMainStatus_gui(_("Welcome to LinuxDC++"));

	// Putting this after all the resizing and moving makes the window appear
	// in the correct position instantly, looking slightly more cool
	gtk_widget_show_all(GTK_WIDGET(window));

	setTabPosition_gui(WGETI("tab-position"));
	setToolbarStyle_gui(WGETI("toolbar-style"));

	createTrayIcon_gui();
}

MainWindow::~MainWindow()
{
	QueueManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	LogManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);
	ConnectionManager::getInstance()->removeListener(this);

	GList *list = (GList *)g_object_get_data(G_OBJECT(getWidget("book")), "page-rotation-list");
	g_list_free(list);

	// Save window state and position
	gint posX, posY, sizeX, sizeY, transferPanePosition;
	bool maximized = TRUE;
	GdkWindowState gdkState;

	gtk_window_get_position(window, &posX, &posY);
	gtk_window_get_size(window, &sizeX, &sizeY);
	gdkState = gdk_window_get_state(GTK_WIDGET(window)->window);
	transferPanePosition = sizeY - gtk_paned_get_position(GTK_PANED(getWidget("pane")));

	if (!(gdkState & GDK_WINDOW_STATE_MAXIMIZED))
	{
		maximized = FALSE;
		// The get pos/size functions return junk when window is maximized
		WSET("main-window-pos-x", posX);
		WSET("main-window-pos-y", posY);
		WSET("main-window-size-x", sizeX);
		WSET("main-window-size-y", sizeY);
	}

	WSET("main-window-maximized", maximized);
	if (transferPanePosition > 10)
		WSET("transfer-pane-position", transferPanePosition);

	transferView.saveSettings();

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
}

GtkWidget *MainWindow::getContainer()
{
	return getWidget("mainWindow");
}

void MainWindow::show()
{
	QueueManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	LogManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
	ConnectionManager::getInstance()->addListener(this);

	typedef Func0<MainWindow> F0;
	F0 *f0 = new F0(this, &MainWindow::startSocket_client);
	WulforManager::get()->dispatchClientFunc(f0);

	f0 = new F0(this, &MainWindow::autoConnect_client);
	WulforManager::get()->dispatchClientFunc(f0);

	autoOpen_gui();
}

bool MainWindow::isActive_gui()
{
	return gtk_window_is_active(window);
}

void MainWindow::setUrgent_gui()
{
	gtk_window_set_urgency_hint(window, true);
}

void MainWindow::autoOpen_gui()
{
	if (BOOLSETTING(OPEN_PUBLIC))
		showPublicHubs_gui();
	if (BOOLSETTING(OPEN_QUEUE))
		showDownloadQueue_gui();
	if (BOOLSETTING(OPEN_FAVORITE_HUBS))
		showFavoriteHubs_gui();
	if (BOOLSETTING(OPEN_FINISHED_DOWNLOADS))
		showFinishedDownloads_gui();
	if (BOOLSETTING(OPEN_FINISHED_UPLOADS))
		showFinishedUploads_gui();
}

void MainWindow::addBookEntry_gui(BookEntry *entry)
{
	addChild(entry);

	GtkWidget *page = entry->getContainer();
	GtkWidget *label = entry->getLabelBox();
	GtkWidget *closeButton = entry->getCloseButton();
	GtkWidget *tabMenuItem = entry->getTabMenuItem();

	addTabMenuItem_gui(tabMenuItem, page);

	gtk_notebook_append_page(GTK_NOTEBOOK(getWidget("book")), page, label);

	g_signal_connect(label, "button-release-event", G_CALLBACK(onButtonReleasePage_gui), (gpointer)entry);
	g_signal_connect(closeButton, "button-release-event", G_CALLBACK(onButtonReleasePage_gui), (gpointer)entry);
	g_signal_connect(closeButton, "clicked", G_CALLBACK(onCloseBookEntry_gui), (gpointer)entry);

	gtk_widget_set_sensitive(getWidget("closeMenuItem"), TRUE);

#if GTK_CHECK_VERSION(2, 10, 0)
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(getWidget("book")), page, TRUE);
#endif

	entry->show();
}

GtkWidget *MainWindow::currentPage_gui()
{
	int pageNum = gtk_notebook_get_current_page(GTK_NOTEBOOK(getWidget("book")));

	if (pageNum == -1)
		return NULL;
	else
		return gtk_notebook_get_nth_page(GTK_NOTEBOOK(getWidget("book")), pageNum);
}

void MainWindow::raisePage_gui(GtkWidget *page)
{
	int num = gtk_notebook_page_num(GTK_NOTEBOOK(getWidget("book")), page);

	if (num != -1)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(getWidget("book")), num);
}

void MainWindow::removeBookEntry_gui(BookEntry *entry)
{
	GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));
	GtkWidget *page = entry->getContainer();
	GtkWidget* menuItem = entry->getTabMenuItem();
	int num = gtk_notebook_page_num(book, page);
	removeChild(entry);

	if (num != -1)
	{
		GList *list = (GList *)g_object_get_data(G_OBJECT(book), "page-rotation-list");
		list = g_list_remove(list, (gpointer)page);
		g_object_set_data(G_OBJECT(book), "page-rotation-list", (gpointer)list);

		// if removing the current page, switch to the previous page in the rotation list
		if (num == gtk_notebook_get_current_page(book))
		{
			GList *prev = g_list_first(list);
			if (prev != NULL)
			{
				gint childNum = gtk_notebook_page_num(book, GTK_WIDGET(prev->data));
				gtk_notebook_set_current_page(book, childNum);
			}
		}
		gtk_notebook_remove_page(book, num);

		removeTabMenuItem_gui(menuItem);

		if (gtk_notebook_get_n_pages(book) == 0)
			gtk_widget_set_sensitive(getWidget("closeMenuItem"), FALSE);
	}
}

void MainWindow::previousTab_gui()
{
	GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));

	if (gtk_notebook_get_current_page(book) == 0)
		gtk_notebook_set_current_page(book, -1);
	else
		gtk_notebook_prev_page(book);
}

void MainWindow::nextTab_gui()
{
	GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));

	if (gtk_notebook_get_n_pages(book) - 1 == gtk_notebook_get_current_page(book))
		gtk_notebook_set_current_page(book, 0);
	else
		gtk_notebook_next_page(book);
}

void MainWindow::addTabMenuItem_gui(GtkWidget* menuItem, GtkWidget* page)
{
	g_signal_connect(menuItem, "activate", G_CALLBACK(onRaisePage_gui), (gpointer)page);
	gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("tabsMenu")), menuItem);
	gtk_widget_show_all(getWidget("tabsMenu"));

	gtk_widget_set_sensitive(getWidget("previousTabMenuItem"), TRUE);
	gtk_widget_set_sensitive(getWidget("nextTabMenuItem"), TRUE);
	gtk_widget_set_sensitive(getWidget("tabMenuSeparator"), TRUE);
}

void MainWindow::removeTabMenuItem_gui(GtkWidget *menuItem)
{
	GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));

	gtk_container_remove(GTK_CONTAINER(getWidget("tabsMenu")), menuItem);

	if (gtk_notebook_get_n_pages(book) == 0)
	{
		gtk_widget_set_sensitive(getWidget("previousTabMenuItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("nextTabMenuItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("tabMenuSeparator"), FALSE);
	}
}

/*
 * Create tray icon.
 * @todo: Replace with GtkStatusIcon after GTK+ 2.10 is released and on enough systems.
 */
void MainWindow::createTrayIcon_gui()
{
	string iconPath = WulforManager::get()->getPath() + "/pixmaps/linuxdcpp-icon.png";
	trayToolTip = gtk_tooltips_new();

	trayIcon = GTK_WIDGET(egg_tray_icon_new("LinuxDC++"));
	GtkWidget *trayBox = gtk_event_box_new();
	GtkWidget *trayImage = gtk_image_new_from_file(iconPath.c_str());
	gtk_container_add(GTK_CONTAINER(trayBox), trayImage);
	gtk_container_add(GTK_CONTAINER(trayIcon), trayBox);

	g_signal_connect(getWidget("quitTrayItem"), "activate", G_CALLBACK(onQuitClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("toggleInterfaceItem"), "activate", G_CALLBACK(onToggleWindowVisibility_gui), (gpointer)this);
	g_signal_connect(trayIcon, "button-press-event", G_CALLBACK(onTrayIconClicked_gui), (gpointer)this);

	if (BOOLSETTING(MINIMIZE_TRAY))
		gtk_widget_show_all(trayIcon);
}

void MainWindow::updateTrayToolTip_gui(string download, string upload)
{
	ostringstream toolTip;
	toolTip << "LinuxDC++" << endl << _("Download: ") << download << endl << _("Upload: ") << upload;
	gtk_tooltips_set_tip(trayToolTip, trayIcon, toolTip.str().c_str(), NULL);
}

void MainWindow::setMainStatus_gui(string text, time_t t)
{
	if (!text.empty())
	{
		text = "[" + Util::getShortTimeString(t) + "] " + text;

		setStatus_gui("status1", text);
	}
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

BookEntry* MainWindow::findBookEntry(const EntryType type, const string &id)
{
	Entry *entry = getChild(type, id);
	return dynamic_cast<BookEntry*>(entry);
}

void MainWindow::showDownloadQueue_gui()
{
	BookEntry *entry = findBookEntry(Entry::DOWNLOAD_QUEUE);

	if (entry == NULL)
	{
		entry = new DownloadQueue();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showFavoriteHubs_gui()
{
	BookEntry *entry = findBookEntry(Entry::FAVORITE_HUBS);

	if (entry == NULL)
	{
		entry = new FavoriteHubs();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showFinishedDownloads_gui()
{
	BookEntry *entry = findBookEntry(Entry::FINISHED_DOWNLOADS);

	if (entry == NULL)
	{
		entry = FinishedTransfers::createFinishedDownloads();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showFinishedUploads_gui()
{
	BookEntry *entry = findBookEntry(Entry::FINISHED_UPLOADS);

	if (entry == NULL)
	{
		entry = FinishedTransfers::createFinishedUploads();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showHub_gui(string address, string encoding)
{
	BookEntry *entry = findBookEntry(Entry::HUB, address);

	if (entry == NULL)
	{
		entry = new Hub(address, encoding);
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::addPrivateMessage_gui(string cid, string message, bool useSetting)
{
	BookEntry *entry = findBookEntry(Entry::PRIVATE_MESSAGE, cid);
	bool raise = TRUE;

	// If PM is initiated by another user, use setting except if tab is already open.
	if (useSetting)
		raise = (entry == NULL) ? !BOOLSETTING(POPUNDER_PM) : FALSE;

	if (entry == NULL)
	{
		entry = new PrivateMessage(cid);
		addBookEntry_gui(entry);
	}

	if (!message.empty())
		dynamic_cast<PrivateMessage*>(entry)->addMessage_gui(message);

	if (raise)
		raisePage_gui(entry->getContainer());
}

void MainWindow::showPublicHubs_gui()
{
	BookEntry *entry = findBookEntry(Entry::PUBLIC_HUBS);

	if (entry == NULL)
	{
		entry = new PublicHubs();
		addBookEntry_gui(entry);
	}

	raisePage_gui(entry->getContainer());
}

void MainWindow::showShareBrowser_gui(User::Ptr user, string filename, string dir, bool useSetting)
{
	bool raise = useSetting ? !BOOLSETTING(POPUNDER_FILELIST) : TRUE;
	BookEntry *entry = findBookEntry(Entry::SHARE_BROWSER, user->getCID().toBase32());

	if (entry == NULL)
	{
		entry = new ShareBrowser(user, filename, dir);
		addBookEntry_gui(entry);
	}

	if (raise)
		raisePage_gui(entry->getContainer());
}

Search *MainWindow::addSearch_gui()
{
	Search *entry = new Search();
	addBookEntry_gui(entry);
	raisePage_gui(entry->getContainer());
	return entry;
}

bool MainWindow::findTransfer_gui(const string &cid, bool download, GtkTreeIter *iter)
{
	GtkTreeModel *m = GTK_TREE_MODEL(transferStore);
	bool valid = gtk_tree_model_get_iter_first(m, iter);

	while (valid)
	{
		if (cid == transferView.getString(iter, "CID"))
		{
			if (download && transferView.getValue<GdkPixbuf*>(iter, "Icon") == downloadPic)
				return TRUE;
			if (!download && transferView.getValue<GdkPixbuf*>(iter, "Icon") == uploadPic)
				return TRUE;
		}
		valid = gtk_tree_model_iter_next(m, iter);
	}

	return FALSE;
}

void MainWindow::updateTransfer_gui(StringMap params, bool download)
{
	dcassert(params.find("CID") != params.end());
	GtkTreeIter iter;

	if (!findTransfer_gui(params["CID"], download, &iter))
	{
		gtk_list_store_append(transferStore, &iter);

		if (download)
			gtk_list_store_set(transferStore, &iter, transferView.col("Icon"), downloadPic, -1);
		else
			gtk_list_store_set(transferStore, &iter, transferView.col("Icon"), uploadPic, -1);
	}

	for (StringMap::const_iterator it = params.begin(); it != params.end(); ++it)
	{
		if (it->first == "Size Order" || it->first == "Speed Order")
			gtk_list_store_set(transferStore, &iter, transferView.col(it->first), Util::toInt64(it->second), -1);
		else if (it->first == "Progress")
			gtk_list_store_set(transferStore, &iter, transferView.col(it->first), Util::toInt(it->second), -1);
		else if (!it->second.empty())
			gtk_list_store_set(transferStore, &iter, transferView.col(it->first), it->second.c_str(), -1);
	}
}

void MainWindow::removeTransfer_gui(string cid, bool download)
{
	GtkTreeIter iter;
	if (findTransfer_gui(cid, download, &iter))
		gtk_list_store_remove(transferStore, &iter);
}

void MainWindow::setTabPosition_gui(int position)
{
	GtkPositionType tabPosition;

	switch (position)
	{
		case 0:
			tabPosition = GTK_POS_TOP;
			break;
		case 1:
			tabPosition = GTK_POS_LEFT;
			break;
		case 2:
			tabPosition = GTK_POS_RIGHT;
			break;
		case 3:
			tabPosition = GTK_POS_BOTTOM;
			break;
		default:
			tabPosition = GTK_POS_TOP;
	}

	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(getWidget("book")), tabPosition);
}

void MainWindow::setToolbarStyle_gui(int style)
{
	GtkToolbarStyle toolbarStyle;

	switch (style)
	{
		case 0:
			toolbarStyle = GTK_TOOLBAR_ICONS;
			break;
		case 1:
			toolbarStyle = GTK_TOOLBAR_TEXT;
			break;
		case 2:
			toolbarStyle = GTK_TOOLBAR_BOTH;
			break;
		case 3:
			toolbarStyle = GTK_TOOLBAR_BOTH_HORIZ;
			break;
		case 4:
			gtk_widget_hide(getWidget("toolbar1"));
			break;
		case 5:
			return;
		default:
			toolbarStyle = GTK_TOOLBAR_BOTH;
	}

	if (style != 4)
	{
		gtk_widget_show(getWidget("toolbar1"));
		gtk_toolbar_set_style(GTK_TOOLBAR(getWidget("toolbar1")), toolbarStyle);
	}
}

void MainWindow::popupTransferMenu_gui()
{
	// Build user command menu
	userCommandMenu->cleanMenu_gui();

	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(transferSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(transferStore), &iter, path))
		{
			string cid = transferView.getString(&iter, "CID");
			userCommandMenu->addUser(cid);
			userCommandMenu->addHub(WulforUtil::getHubAddress(CID(cid)));
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

	userCommandMenu->buildMenu_gui();

	gtk_menu_popup(GTK_MENU(getWidget("transferMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	gtk_widget_show_all(getWidget("transferMenu"));
}

bool MainWindow::getUserCommandLines_gui(const string &command, StringMap &ucParams)
{
	string name;
	string label;
	string line;
	StringMap done;
	string::size_type i = 0;
	string::size_type j = 0;
	string text = string("<b>") + _("Enter value for ") + "\'";

	while ((i = command.find("%[line:", i)) != string::npos)
	{
		i += 7;
		j = command.find(']', i);
		if (j == string::npos)
			break;

		name = command.substr(i, j - i);
		if (done.find(name) == done.end())
		{
			line.clear();
			label = text + name + "\'</b>";

			gtk_label_set_label(GTK_LABEL(getWidget("ucLabel")), label.c_str());
			gtk_entry_set_text(GTK_ENTRY(getWidget("ucLineEntry")), "");
			gtk_widget_grab_focus(getWidget("ucLineEntry"));

			gint response = gtk_dialog_run(GTK_DIALOG(getWidget("ucLineDialog")));
			gtk_widget_hide(getWidget("ucLineDialog"));

			if (response == GTK_RESPONSE_OK)
				line = gtk_entry_get_text(GTK_ENTRY(getWidget("ucLineEntry")));

			if (!line.empty())
			{
				ucParams["line:" + name] = line;
				done[name] = line;
			}
			else
				return false;
		}
		i = j + 1;
	}

	return true;
}

void MainWindow::openMagnetDialog_gui(const string &magnet)
{
	string name;
	int64_t size;
	string tth;

	WulforUtil::splitMagnet(magnet, name, size, tth);

	gtk_entry_set_text(GTK_ENTRY(getWidget("magnetEntry")), magnet.c_str());
	gtk_entry_set_text(GTK_ENTRY(getWidget("magnetNameEntry")), name.c_str());
	gtk_entry_set_text(GTK_ENTRY(getWidget("magnetSizeEntry")), Util::formatBytes(size).c_str());
	gtk_entry_set_text(GTK_ENTRY(getWidget("exactSizeEntry")), Util::formatExactSize(size).c_str());
	gtk_entry_set_text(GTK_ENTRY(getWidget("tthEntry")), tth.c_str());

	gtk_dialog_run(GTK_DIALOG(getWidget("magnetDialog")));
	gtk_widget_hide(getWidget("magnetDialog"));
}

gboolean MainWindow::onWindowState_gui(GtkWidget *widget, GdkEventWindowState *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (!mw->minimized && event->new_window_state & (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_WITHDRAWN))
	{
		mw->minimized = TRUE;
		if (BOOLSETTING(SettingsManager::AUTO_AWAY) && !Util::getAway())
			Util::setAway(TRUE);
	}
	else if (mw->minimized && (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED ||
		event->new_window_state == 0))
	{
		mw->minimized = FALSE;
		if (BOOLSETTING(SettingsManager::AUTO_AWAY) && !Util::getManualAway())
			Util::setAway(FALSE);
	}

	return TRUE;
}

gboolean MainWindow::onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *child = mw->currentPage_gui();

	if (child != NULL)
	{
		BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(child), "entry");
		entry->setActive_gui();
	}

	gtk_window_set_urgency_hint(mw->window, FALSE);
	return FALSE;
}

gboolean MainWindow::onCloseWindow_gui(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (!BOOLSETTING(CONFIRM_EXIT))
	{
		WulforManager::get()->deleteMainWindow();
		return FALSE;
	}

	int response = gtk_dialog_run(GTK_DIALOG(mw->getWidget("exitDialog")));
	gtk_widget_hide(mw->getWidget("exitDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		WulforManager::get()->deleteMainWindow();
		return FALSE;
	}

	return TRUE;
}

gboolean MainWindow::onKeyPressed_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (event->state & GDK_CONTROL_MASK)
	{
		if (event->state & GDK_SHIFT_MASK && event->keyval == GDK_ISO_Left_Tab)
		{
			mw->previousTab_gui();
			return TRUE;
		}
		else if (event->keyval == GDK_Tab)
		{
			mw->nextTab_gui();
			return TRUE;
		}
	}

	return FALSE;
}

gboolean MainWindow::onButtonReleasePage_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	gint width, height;
	gdk_drawable_get_size(event->window, &width, &height);

	// If middle mouse button was released when hovering over tab label
	if (event->button == 2 && event->x >= 0 && event->y >= 0
		&& event->x < width && event->y < height)
	{
		BookEntry *entry = (BookEntry *)data;
		WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
		return TRUE;
	}

	return FALSE;
}

gboolean MainWindow::onTransferButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (event->button == 3)
	{
		GtkTreePath *path;
		if (gtk_tree_view_get_path_at_pos(mw->transferView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(mw->transferSelection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}

	return FALSE;
}

gboolean MainWindow::onTransferButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	int count = gtk_tree_selection_count_selected_rows(mw->transferSelection);

	if (count > 0 && event->type == GDK_BUTTON_RELEASE && event->button == 3)
		mw->popupTransferMenu_gui();

	return FALSE;
}

void MainWindow::onRaisePage_gui(GtkMenuItem *item, gpointer data)
{
	WulforManager::get()->getMainWindow()->raisePage_gui((GtkWidget *)data);
}

void MainWindow::onPageSwitched_gui(GtkNotebook *notebook, GtkNotebookPage *page, guint num, gpointer data)
{
	GtkWidget *child = gtk_notebook_get_nth_page(notebook, num);
	BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(child), "entry");

	if (entry)
		entry->setActive_gui();

	GList *list = (GList *)g_object_get_data(G_OBJECT(notebook), "page-rotation-list");
	list = g_list_remove(list, (gpointer)child);
	list = g_list_prepend(list, (gpointer)child);
	g_object_set_data(G_OBJECT(notebook), "page-rotation-list", (gpointer)list);

	// Focus the tab so it will focus its children (e.g. a text entry box)
	gtk_widget_grab_focus(child);
}

void MainWindow::onPaneRealized_gui(GtkWidget *pane, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	gint position = WGETI("transfer-pane-position");

	if (position > 10)
	{
		// @todo: fix get window height when maximized
		gint height;
		gtk_window_get_size(mw->window, NULL, &height);
		gtk_paned_set_position(GTK_PANED(pane), height - position);
	}
}

void MainWindow::onConnectClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	gtk_editable_select_region(GTK_EDITABLE(mw->getWidget("connectEntry")), 0, -1);
	gtk_widget_grab_focus(mw->getWidget("connectEntry"));
	int response = gtk_dialog_run(GTK_DIALOG(mw->getWidget("connectDialog")));
	gtk_widget_hide(mw->getWidget("connectDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		string address = gtk_entry_get_text(GTK_ENTRY(mw->getWidget("connectEntry")));
		mw->showHub_gui(address);
	}
}

void MainWindow::onFavoriteHubsClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showFavoriteHubs_gui();
}

void MainWindow::onPublicHubsClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showPublicHubs_gui();
}

void MainWindow::onPreferencesClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	typedef Func0<MainWindow> F0;

	unsigned short tcpPort = (unsigned short)SETTING(TCP_PORT);
	unsigned short udpPort = (unsigned short)SETTING(UDP_PORT);
	int lastConn = SETTING(INCOMING_CONNECTIONS);
	bool lastShowProgressSetting = BOOLSETTING(SHOW_PROGRESS_BARS);

	gint response = WulforManager::get()->openSettingsDialog_gui();

	if (response == GTK_RESPONSE_OK)
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

		mw->setTabPosition_gui(WGETI("tab-position"));
		mw->setToolbarStyle_gui(WGETI("toolbar-style"));
	}
}

void MainWindow::onHashClicked_gui(GtkWidget *widget, gpointer data)
{
	WulforManager::get()->openHashDialog_gui();
}

void MainWindow::onSearchClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->addSearch_gui();
}

void MainWindow::onDownloadQueueClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showDownloadQueue_gui();
}

void MainWindow::onFinishedDownloadsClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showFinishedDownloads_gui();
}

void MainWindow::onFinishedUploadsClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->showFinishedUploads_gui();
}

void MainWindow::onQuitClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	gboolean retVal; // Not interested in the value, though.
	g_signal_emit_by_name(mw->window, "delete-event", NULL, &retVal);
}

void MainWindow::onOpenFileListClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(mw->getWidget("flistDialog")), Text::fromUtf8(Util::getListPath()).c_str());

 	int ret = gtk_dialog_run(GTK_DIALOG(mw->getWidget("flistDialog")));
	gtk_widget_hide(mw->getWidget("flistDialog"));

	if (ret == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(mw->getWidget("flistDialog")));
		if (temp)
		{
			string path = Text::toUtf8(temp);
			g_free(temp);

			User::Ptr user = DirectoryListing::getUserFromFilename(path);
			if (user)
				mw->showShareBrowser_gui(user, path, "", FALSE);
			else
				mw->setMainStatus_gui(_("Unable to open: Older file list format detected"));
		}
	}
}

void MainWindow::onOpenOwnListClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	typedef Func0<MainWindow> F0;
	F0 *func = new F0(mw, &MainWindow::openOwnList_client);
	WulforManager::get()->dispatchClientFunc(func);

	mw->setMainStatus_gui(_("Loading file list"));
}

void MainWindow::onRefreshFileListClicked_gui(GtkWidget *widget, gpointer data)
{
	typedef Func0<MainWindow> F0;
	F0 *func = new F0((MainWindow *)data, &MainWindow::refreshFileList_client);
	WulforManager::get()->dispatchClientFunc(func);
}

void MainWindow::onReconnectClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *entryWidget = mw->currentPage_gui();

	if (entryWidget)
	{
		BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(entryWidget), "entry");

		if (entry && entry->getType() == Entry::HUB)
		{
			Func0<Hub> *func = new Func0<Hub>(dynamic_cast<Hub *>(entry), &Hub::reconnect_client);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void MainWindow::onCloseClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	GtkWidget *entryWidget = mw->currentPage_gui();

	if (entryWidget)
	{
		BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(entryWidget), "entry");

		if (entry)
			mw->removeBookEntry_gui(entry);
	}
}

void MainWindow::onPreviousTabClicked_gui(GtkWidget* widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->previousTab_gui();
}

void MainWindow::onNextTabClicked_gui(GtkWidget* widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	mw->nextTab_gui();
}

void MainWindow::onAboutClicked_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	gtk_dialog_run(GTK_DIALOG(mw->getWidget("aboutDialog")));
	gtk_widget_hide(mw->getWidget("aboutDialog"));
}

void MainWindow::onAboutDialogActivateLink_gui(GtkAboutDialog *dialog, const gchar *link, gpointer data)
{
	WulforUtil::openURI(link);
}

void MainWindow::onCloseBookEntry_gui(GtkWidget *widget, gpointer data)
{
	BookEntry *entry = (BookEntry *)data;
	WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
}

void MainWindow::onGetFileListClicked_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string cid;
	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(mw->transferSelection, NULL);
	typedef Func1<MainWindow, string > F1;
	F1 *func;

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(mw->transferStore), &iter, path))
		{
			cid = mw->transferView.getString(&iter, "CID");
			func = new F1(mw, &MainWindow::getFileList_client, cid);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void MainWindow::onMatchQueueClicked_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string cid;
	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(mw->transferSelection, NULL);
	typedef Func1<MainWindow, string > F1;
	F1 *func;

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(mw->transferStore), &iter, path))
		{
			cid = mw->transferView.getString(&iter, "CID");
			func = new F1(mw, &MainWindow::matchQueue_client, cid);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void MainWindow::onPrivateMessageClicked_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string cid;
	User::Ptr user;
	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(mw->transferSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(mw->transferStore), &iter, path))
		{
			cid = mw->transferView.getString(&iter, "CID");
			mw->addPrivateMessage_gui(cid);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void MainWindow::onAddFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string cid;
	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(mw->transferSelection, NULL);
	typedef Func1<MainWindow, string > F1;
	F1 *func;

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(mw->transferStore), &iter, path))
		{
			cid = mw->transferView.getString(&iter, "CID");
			func = new F1(mw, &MainWindow::addFavoriteUser_client, cid);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void MainWindow::onGrantExtraSlotClicked_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string cid;
	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(mw->transferSelection, NULL);
	typedef Func1<MainWindow, string > F1;
	F1 *func;

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(mw->transferStore), &iter, path))
		{
			cid = mw->transferView.getString(&iter, "CID");
			func = new F1(mw, &MainWindow::grantExtraSlot_client, cid);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void MainWindow::onRemoveUserFromQueueClicked_gui(GtkMenuItem *item, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string cid;
	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(mw->transferSelection, NULL);
	typedef Func1<MainWindow, string > F1;
	F1 *func;

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(mw->transferStore), &iter, path))
		{
			cid = mw->transferView.getString(&iter, "CID");
			func = new F1(mw, &MainWindow::removeUserFromQueue_client, cid);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void MainWindow::onForceAttemptClicked_gui(GtkMenuItem *menuItem, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string cid;
	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(mw->transferSelection, NULL);
	typedef Func1<MainWindow, string> F1;
	F1 *func;

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(mw->transferStore), &iter, path))
		{
			cid = mw->transferView.getString(&iter, "CID");
			gtk_list_store_set(mw->transferStore, &iter, mw->transferView.col("Status"), _("Connecting (forced)..."), -1);

			func = new F1(mw, &MainWindow::forceAttempt_client, cid);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void MainWindow::onCloseConnectionClicked_gui(GtkMenuItem *menuItem, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;
	string cid;
	GtkTreeIter iter;
	GtkTreePath *path;
	bool download;
	GList *list = gtk_tree_selection_get_selected_rows(mw->transferSelection, NULL);
	typedef Func2<MainWindow, string, bool> F2;
	F2 *func;

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(mw->transferStore), &iter, path))
		{
			cid = mw->transferView.getString(&iter, "CID");
			gtk_list_store_set(mw->transferStore, &iter, mw->transferView.col("Status"), _("Closing connection..."), -1);
			if (mw->transferView.getValue<GdkPixbuf*>(&iter,"Icon") == mw->downloadPic)
				download = TRUE;
			else
				download = FALSE;

			func = new F2(mw, &MainWindow::closeConnection_client, cid, download);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void MainWindow::onTrayIconClicked_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	MainWindow *mw = (MainWindow *)data;

	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
		onToggleWindowVisibility_gui(NULL, data);
	else if (event->type == GDK_BUTTON_PRESS && event->button == 3)
		gtk_menu_popup(GTK_MENU(mw->getWidget("trayMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
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
		gtk_widget_hide(GTK_WIDGET(win));
	}
	else
	{
		gtk_window_move(win, x, y);
		if (isMaximized) gtk_window_maximize(win);
		if (isIconified) gtk_window_iconify(win);
		gtk_widget_show(GTK_WIDGET(win));
	}
}

void MainWindow::autoConnect_client()
{
	FavoriteHubEntry *hub;
	FavoriteHubEntry::List &l = FavoriteManager::getInstance()->getFavoriteHubs();
	typedef Func2<MainWindow, string, string> F2;
	F2 *func;

	for (FavoriteHubEntry::List::const_iterator it = l.begin(); it != l.end(); ++it)
	{
		hub = *it;

		if (hub->getConnect())
		{
			func = new F2(this, &MainWindow::showHub_gui, hub->getServer(), hub->getEncoding());
			WulforManager::get()->dispatchGuiFunc(func);
		}
	}
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

void MainWindow::refreshFileList_client()
{
	try
	{
		ShareManager::getInstance()->setDirty();
		ShareManager::getInstance()->refresh(TRUE, TRUE, FALSE);
	}
	catch (const ShareException&)
	{
	}
}

void MainWindow::openOwnList_client()
{
	User::Ptr user = ClientManager::getInstance()->getMe();
	string path = ShareManager::getInstance()->getOwnListFile();

	typedef Func4<MainWindow, User::Ptr, string, string, bool> F4;
	F4 *func = new F4(this, &MainWindow::showShareBrowser_gui, user, path, "", FALSE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::getFileList_client(string cid)
{
	try
	{
		if (!cid.empty())
		{
			User::Ptr user = ClientManager::getInstance()->getUser(CID(cid));
			QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
		}
	}
	catch (const Exception&)
	{
	}
}

void MainWindow::matchQueue_client(string cid)
{
	try
	{
		if (!cid.empty())
		{
			User::Ptr user = ClientManager::getInstance()->getUser(CID(cid));
			QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
		}
	}
	catch (const Exception&)
	{
	}
}

void MainWindow::addFavoriteUser_client(string cid)
{
	if (!cid.empty())
	{
		User::Ptr user = ClientManager::getInstance()->getUser(CID(cid));
		FavoriteManager::getInstance()->addFavoriteUser(user);
	}
}

void MainWindow::grantExtraSlot_client(string cid)
{
	if (!cid.empty())
	{
		User::Ptr user = ClientManager::getInstance()->getUser(CID(cid));
		UploadManager::getInstance()->reserveSlot(user);
	}
}

void MainWindow::removeUserFromQueue_client(string cid)
{
	if (!cid.empty())
	{
		User::Ptr user = ClientManager::getInstance()->getUser(CID(cid));
		QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
	}
}

void MainWindow::forceAttempt_client(string cid)
{
	if (!cid.empty())
	{
		User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
		ClientManager::getInstance()->connect(user);
	}
}

void MainWindow::closeConnection_client(string cid, bool download)
{
	if (!cid.empty())
	{
		User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
		ConnectionManager::getInstance()->disconnect(user, download);
	}
}

void MainWindow::transferComplete_client(Transfer *t)
{
	bool download;
	StringMap params;
	User::Ptr user = t->getUserConnection().getUser();

	params["CID"] = user->getCID().toBase32();
	params["Progress"] = "100";
	params["Time Left"] = _("Done");
	params["Speed"] = " ";
	params["Speed Order"] = "0";
	params["Sort Order"] = "w" + WulforUtil::getNicks(user) + WulforUtil::getHubNames(user);

	if (t->getUserConnection().isSet(UserConnection::FLAG_DOWNLOAD))
	{
		params["Status"] = _("Download finished, idle...");
		download = TRUE;
	}
	else
	{
		params["Status"] = _("Upload finished, idle...");
		download = FALSE;
	}

	typedef Func2<MainWindow, StringMap, bool> F2;
	F2 *func = new F2(this, &MainWindow::updateTransfer_gui, params, download);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::Added, ConnectionQueueItem *cqi) throw()
{
	StringMap params;

	params["CID"] = cqi->getUser()->getCID().toBase32();
	params["User"] = WulforUtil::getNicks(cqi->getUser());
	params["Hub Name"] = WulforUtil::getHubNames(cqi->getUser());
	params["Status"] = _("Connecting...");
	params["Progress"] = "0";
	params["Time Left"] = " ";
	params["Speed"] = " ";
	params["Speed Order"] = "0";
	params["Sort Order"] = "w" + params["User"] + params["Hub Name"];

	typedef Func2<MainWindow, StringMap, bool> F2;
	F2 *func = new F2(this, &MainWindow::updateTransfer_gui, params, cqi->getDownload());
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::Removed, ConnectionQueueItem *cqi) throw()
{
	string cid = cqi->getUser()->getCID().toBase32();

	typedef Func2 <MainWindow, string, bool> F2;
	F2 *func = new F2(this, &MainWindow::removeTransfer_gui, cid, cqi->getDownload());
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::Failed, ConnectionQueueItem *cqi, const string &reason) throw()
{
	StringMap params;
	User::Ptr user = cqi->getUser();

	params["CID"] = user->getCID().toBase32();
	params["Status"] = reason;
	params["Time Left"] = " ";
	params["Speed"] = " ";
	params["Speed Order"] = "0";
	params["Sort Order"] = "w" + WulforUtil::getNicks(user) + WulforUtil::getHubNames(user);

	typedef Func2<MainWindow, StringMap, bool> F2;
	F2 *func = new F2(this, &MainWindow::updateTransfer_gui, params, cqi->getDownload());
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem *cqi) throw()
{
	StringMap params;

	params["CID"] = cqi->getUser()->getCID().toBase32();
	params["Time Left"] = " ";
	params["Speed"] = " ";
	params["Speed Order"] = "0";

	if (cqi->getState() == ConnectionQueueItem::CONNECTING)
		params["Status"] = _("Connecting...");
	else
		params["Status"] = _("Waiting to retry...");

	typedef Func2<MainWindow, StringMap, bool> F2;
	F2 *func = new F2(this, &MainWindow::updateTransfer_gui, params, cqi->getDownload());
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(DownloadManagerListener::Starting, Download *dl) throw()
{
	StringMap params;
	User::Ptr user = dl->getUserConnection().getUser();

	if (dl->isSet(Download::FLAG_USER_LIST))
		params["Filename"] = _("Filelist");
	else if (dl->isSet(Download::FLAG_TREE_DOWNLOAD))
		params["Filename"] = "TTH: " + Util::getFileName(dl->getTarget());
	else
		params["Filename"] = Util::getFileName(dl->getTarget());

	params["CID"] = user->getCID().toBase32();
	params["Path"] = Util::getFilePath(dl->getTarget());
	params["Status"] = _("Download starting...");
	params["Size"] = Util::formatBytes(dl->getSize());
	params["Size Order"] = Util::toString(dl->getSize());
	params["Sort Order"] = "d" + WulforUtil::getNicks(user) + WulforUtil::getHubNames(user);
	params["IP"] = dl->getUserConnection().getRemoteIp();

	typedef Func2<MainWindow, StringMap, bool> F2;
	F2 *func = new F2(this, &MainWindow::updateTransfer_gui, params, TRUE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(DownloadManagerListener::Tick, const Download::List &list) throw()
{
	Download *dl;
	StringMap params;
	string status;
	double percent;
	typedef Func2<MainWindow, StringMap, bool> F2;
	F2 *func;

	for (Download::List::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		ostringstream stream;
		params.clear();
		status.clear();
		dl = *it;
		percent = 0.0;

		if (dl->getUserConnection().isSecure())
		{
			if (dl->getUserConnection().isTrusted())
				status += "[S]";
			else
				status += "[U]";
		}
		if (dl->isSet(Download::FLAG_TTH_CHECK))
			status += "[T]";
		if (dl->isSet(Download::FLAG_ZDOWNLOAD))
			status += "[Z]";
		if (dl->isSet(Download::FLAG_ROLLBACK))
			status += "[R]";
		if (!status.empty())
			status += " ";

		if (dl->getSize() > 0)
			percent = (double)(dl->getPos() * 100.0) / dl->getSize();

		stream << setiosflags(ios::fixed) << setprecision(1);
		stream << _("Downloaded ") << Util::formatBytes((dl->getPos())) << " (" << percent;
		stream << "%) in " << Util::formatSeconds((GET_TICK() - dl->getStart()) / 1000);

		params["CID"] = dl->getUserConnection().getUser()->getCID().toBase32();
		params["Status"] = status + stream.str();
		params["Time Left"] = Util::formatSeconds(dl->getSecondsLeft());
		params["Progress"] = Util::toString((int)percent);
		params["Speed"] = Util::formatBytes(dl->getRunningAverage()).append("/s");
		params["Speed Order"] = Util::toString(dl->getRunningAverage());

		func = new F2(this, &MainWindow::updateTransfer_gui, params, TRUE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::on(DownloadManagerListener::Complete, Download *dl) throw()
{
	transferComplete_client(dl);
}

void MainWindow::on(DownloadManagerListener::Failed, Download *dl, const string &reason) throw()
{
	StringMap params;
	User::Ptr user = dl->getUserConnection().getUser();

	if (dl->isSet(Download::FLAG_USER_LIST))
		params["Filename"] = _("Filelist");
	else if (dl->isSet(Download::FLAG_TREE_DOWNLOAD))
		params["Filename"] = "TTH: " + Util::getFileName(dl->getTarget());
	else
		params["Filename"] = Util::getFileName(dl->getTarget());

	params["CID"] = user->getCID().toBase32();
	params["Path"] = Util::getFilePath(dl->getTarget());
	params["Status"] = reason;
	params["Time Left"] = " ";
	params["Speed"] = " ";
	params["Speed Order"] = "0";
	params["Size"] = Util::formatBytes(dl->getSize());
	params["Size Order"] = Util::toString(dl->getSize());
	params["Sort Order"] = "w" + WulforUtil::getNicks(user) + WulforUtil::getHubNames(user);

	typedef Func2<MainWindow, StringMap, bool> F2;
	F2 *func = new F2(this, &MainWindow::updateTransfer_gui, params, TRUE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(UploadManagerListener::Starting, Upload *ul) throw()
{
	StringMap params;
	User::Ptr user = ul->getUser();

	if (ul->isSet(Upload::FLAG_USER_LIST))
		params["Filename"] = _("Filelist");
	else if (ul->isSet(Download::FLAG_TREE_DOWNLOAD))
		params["Filename"] = "TTH: " + Util::getFileName(ul->getSourceFile());
	else
		params["Filename"] = Util::getFileName(ul->getSourceFile());

	params["CID"] = user->getCID().toBase32();
	params["Path"] = Util::getFilePath(ul->getSourceFile());
	params["Status"] = _("Upload starting...");
	params["Size"] = Util::formatBytes(ul->getSize());
	params["Size Order"] = Util::toString(ul->getSize());
	params["Sort Order"] = "u" + WulforUtil::getNicks(user) + WulforUtil::getHubNames(user);
	params["IP"] = ul->getUserConnection().getRemoteIp();

	typedef Func2<MainWindow, StringMap, bool> F2;
	F2 *func = new F2(this, &MainWindow::updateTransfer_gui, params, FALSE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(UploadManagerListener::Tick, const Upload::List &list) throw()
{
	Upload *ul;
	StringMap params;
	string status;
	double percent;
	typedef Func2<MainWindow, StringMap, bool> F2;
	F2 *func;

	for (Upload::List::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		ostringstream stream;
		params.clear();
		status.clear();
		ul = *it;
		percent = 0.0;

		if (ul->getUserConnection().isSecure())
		{
			if (ul->getUserConnection().isTrusted())
				status += "[S]";
			else
				status += "[U]";
		}
		if (ul->isSet(Upload::FLAG_ZUPLOAD))
			status += "[Z]";
		if (!status.empty())
			status += " ";

		if (ul->getSize() > 0)
			percent = (double)(ul->getPos() * 100.0) / ul->getSize();

		stream << setiosflags(ios::fixed) << setprecision(1);
		stream << _("Uploaded ") << Util::formatBytes((ul->getPos())) << " (" << percent;
		stream << _("%) in ") << Util::formatSeconds((GET_TICK() - ul->getStart()) / 1000);

		params["CID"] = ul->getUser()->getCID().toBase32();
		params["Status"] = status + stream.str();
		params["Time Left"] = Util::formatSeconds(ul->getSecondsLeft());
		params["Progress"] = Util::toString((int)percent);
		params["Speed"] = Util::formatBytes(ul->getRunningAverage()).append("/s");
		params["Speed Order"] = Util::toString(ul->getRunningAverage());

		func = new F2(this, &MainWindow::updateTransfer_gui, params, FALSE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::on(UploadManagerListener::Complete, Upload *ul) throw()
{
	transferComplete_client(ul);
}

void MainWindow::on(LogManagerListener::Message, time_t t, const string &message) throw()
{
	typedef Func2<MainWindow, string, time_t> F2;
	F2 *func = new F2(this, &MainWindow::setMainStatus_gui, message, t);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(QueueManagerListener::Finished, QueueItem *item, const string& dir, int64_t avSpeed) throw()
{
	if (item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST))
	{
		User::Ptr user = item->getCurrent();
		string listName = item->getListName();

		typedef Func4<MainWindow, User::Ptr, string, string, bool> F4;
		F4 *func = new F4(this, &MainWindow::showShareBrowser_gui, user, listName, dir, TRUE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::on(TimerManagerListener::Second, uint32_t ticks) throw()
{
	// Avoid calculating status update if it's not needed
	if (!BOOLSETTING(MINIMIZE_TRAY) && minimized)
		return;

	string status1, status2, status3, status4, status5, status6;
	int64_t diff = (int64_t)((lastUpdate == 0) ? ticks - 1000 : ticks - lastUpdate);
	int64_t updiff = Socket::getTotalUp() - lastUp;
	int64_t downdiff = Socket::getTotalDown() - lastDown;

	status1 = _("H: ") + Client::getCounts();
	status2 = _("S: ") + Util::toString(SETTING(SLOTS) -
		UploadManager::getInstance()->getRunning()) + '/' +
		Util::toString(SETTING(SLOTS));
	status3 = _("D: ") + Util::formatBytes(Socket::getTotalDown());
	status4 = _("U: ") + Util::formatBytes(Socket::getTotalUp());
	if (diff > 0)
	{
		status5 = Util::formatBytes((int64_t)((downdiff*1000)/diff)) + "/s (" +
			Util::toString(DownloadManager::getInstance()->getDownloadCount()) + ")";
		status6 = Util::formatBytes((int64_t)((updiff*1000)/diff)) + "/s (" +
			Util::toString(UploadManager::getInstance()->getUploadCount()) + ")";
	}

	lastUpdate = ticks;
	lastUp = Socket::getTotalUp();
	lastDown = Socket::getTotalDown();

	typedef Func6<MainWindow, string, string, string, string, string, string> func_t;
	func_t *func = new func_t(this, &MainWindow::setStats_gui, status1, status2, status3, status4, status5, status6);
	WulforManager::get()->dispatchGuiFunc(func);

	if (BOOLSETTING(MINIMIZE_TRAY) && !status5.empty() && !status6.empty())
	{
		typedef Func2<MainWindow, string, string> F2;
		F2 *f2 = new F2(this, &MainWindow::updateTrayToolTip_gui, status5, status6);
		WulforManager::get()->dispatchGuiFunc(f2);
	}
}

