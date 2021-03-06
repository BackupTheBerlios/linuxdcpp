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

#include "search.hh"

#include <client/ClientManager.h>
#include <client/FavoriteManager.h>
#include <client/QueueManager.h>
#include <client/ShareManager.h>
#include <client/StringTokenizer.h>
#include <client/Text.h>
#include <client/UserCommand.h>
#include "UserCommandMenu.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;

bool Search::onlyOp = FALSE;
GtkTreeModel* Search::searchEntriesModel = NULL;

Search::Search():
	BookEntry(Entry::SEARCH, _("Search: "), "search.glade", generateID())
{
	// Initialize the search entries combo box
	if (searchEntriesModel == NULL)
		searchEntriesModel = gtk_combo_box_get_model(GTK_COMBO_BOX(getWidget("comboboxentrySearch")));
	gtk_combo_box_set_model(GTK_COMBO_BOX(getWidget("comboboxentrySearch")), searchEntriesModel);
	searchEntry = gtk_bin_get_child(GTK_BIN(getWidget("comboboxentrySearch")));
	gtk_widget_grab_focus(getWidget("comboboxentrySearch"));

	// Configure the dialog
	File::ensureDirectory(SETTING(DOWNLOAD_DIRECTORY));
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(getWidget("dirChooserDialog")), Text::fromUtf8(SETTING(DOWNLOAD_DIRECTORY)).c_str());
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("dirChooserDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	// Initialize check button options.
	onlyFree = BOOLSETTING(SEARCH_ONLY_FREE_SLOTS);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonSlots")), onlyFree);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("checkbuttonOp")), onlyOp);

	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxSize")), 1);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxUnit")), 2);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxFile")), 0);

	// Load icons
	iconFile = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
		GTK_STOCK_FILE, 16, (GtkIconLookupFlags)0, NULL);
	iconDirectory = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
		GTK_STOCK_DIRECTORY, 16, (GtkIconLookupFlags)0, NULL);

	// Initialize hub list treeview
	hubView.setView(GTK_TREE_VIEW(getWidget("treeviewHubs")));
	hubView.insertColumn("Search", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
	hubView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
	hubView.insertHiddenColumn("Url", G_TYPE_STRING);
	hubView.insertHiddenColumn("Op", G_TYPE_BOOLEAN);
	hubView.insertHiddenColumn("Editable", G_TYPE_BOOLEAN);
	hubView.finalize();
	hubStore = gtk_list_store_newv(hubView.getColCount(), hubView.getGTypes());
	gtk_tree_view_set_model(hubView.get(), GTK_TREE_MODEL(hubStore));
	g_object_unref(hubStore);
	GtkTreeViewColumn *col = gtk_tree_view_get_column(hubView.get(), hubView.col("Search"));
	GList *list = gtk_tree_view_column_get_cell_renderers(col);
	GtkCellRenderer *renderer = (GtkCellRenderer *)g_list_nth_data(list, 0);
	gtk_tree_view_column_add_attribute(col, renderer, "sensitive", hubView.col("Editable"));
	g_list_free(list);

	// Initialize search result treeview
	resultView.setView(GTK_TREE_VIEW(getWidget("treeviewResult")), TRUE, "search");
	resultView.insertColumn("Filename", G_TYPE_STRING, TreeView::PIXBUF_STRING, 250, "Icon");
	resultView.insertColumn("Nick", G_TYPE_STRING, TreeView::STRING, 100);
	resultView.insertColumn("Type", G_TYPE_STRING, TreeView::STRING, 65);
	resultView.insertColumn("Size", G_TYPE_STRING, TreeView::STRING, 80);
	resultView.insertColumn("Path", G_TYPE_STRING, TreeView::STRING, 100);
	resultView.insertColumn("Slots", G_TYPE_STRING, TreeView::STRING, 50);
	resultView.insertColumn("Connection", G_TYPE_STRING, TreeView::STRING, 90);
	resultView.insertColumn("Hub", G_TYPE_STRING, TreeView::STRING, 150);
	resultView.insertColumn("Exact Size", G_TYPE_STRING, TreeView::STRING, 80);
	resultView.insertColumn("IP", G_TYPE_STRING, TreeView::STRING, 100);
	resultView.insertColumn("TTH", G_TYPE_STRING, TreeView::STRING, 125);
	resultView.insertHiddenColumn("Icon", GDK_TYPE_PIXBUF);
	resultView.insertHiddenColumn("Real Size", G_TYPE_INT64);
	resultView.insertHiddenColumn("Slots Order", G_TYPE_INT);
	resultView.insertHiddenColumn("File Order", G_TYPE_STRING);
	resultView.insertHiddenColumn("SearchResult", G_TYPE_POINTER);
	resultView.insertHiddenColumn("Hub URL", G_TYPE_STRING);
	resultView.insertHiddenColumn("CID", G_TYPE_STRING);
	resultView.insertHiddenColumn("Shared", G_TYPE_BOOLEAN);
	resultView.finalize();
	resultStore = gtk_list_store_newv(resultView.getColCount(), resultView.getGTypes());
	searchFilterModel = gtk_tree_model_filter_new(GTK_TREE_MODEL(resultStore), NULL);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(searchFilterModel), &Search::searchFilterFunc_gui, (gpointer)this, NULL);
	sortedFilterModel = gtk_tree_model_sort_new_with_model(searchFilterModel);
	gtk_tree_view_set_model(resultView.get(), sortedFilterModel);
	g_object_unref(resultStore);
	g_object_unref(searchFilterModel);
	g_object_unref(sortedFilterModel);
	selection = gtk_tree_view_get_selection(resultView.get());
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	resultView.setSortColumn_gui("Size", "Real Size");
	resultView.setSortColumn_gui("Exact Size", "Real Size");
	resultView.setSortColumn_gui("Slots", "Slots Order");
	resultView.setSortColumn_gui("Filename", "File Order");
	gtk_tree_view_set_fixed_height_mode(resultView.get(), TRUE);

	// Initialize the user command menu
	userCommandMenu = new UserCommandMenu(getWidget("usercommandMenu"), ::UserCommand::CONTEXT_SEARCH);
	addChild(userCommandMenu);

	// Connect the signals to their callback functions.
	g_signal_connect(getContainer(), "focus-in-event", G_CALLBACK(onFocusIn_gui), (gpointer)this);
	g_signal_connect(getWidget("checkbuttonFilter"), "toggled", G_CALLBACK(onButtonToggled_gui), (gpointer)this);
	g_signal_connect(getWidget("checkbuttonSlots"), "toggled", G_CALLBACK(onButtonToggled_gui), (gpointer)this);
	g_signal_connect(getWidget("checkbuttonOp"), "toggled", G_CALLBACK(onButtonToggled_gui), (gpointer)this);
	g_signal_connect(getWidget("checkbuttonShared"), "toggled", G_CALLBACK(onButtonToggled_gui), (gpointer)this);
	g_signal_connect(renderer, "toggled", G_CALLBACK(onToggledClicked_gui), (gpointer)this);
	g_signal_connect(resultView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(resultView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(resultView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	g_signal_connect(searchEntry, "key-press-event", G_CALLBACK(onSearchEntryKeyPressed_gui), (gpointer)this);
	g_signal_connect(searchEntry, "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	g_signal_connect(getWidget("entrySize"), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonSearch"), "clicked", G_CALLBACK(onSearchButtonClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("downloadItem"), "activate", G_CALLBACK(onDownloadClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("downloadWholeDirItem"), "activate", G_CALLBACK(onDownloadDirClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchByTTHItem"), "activate", G_CALLBACK(onSearchByTTHClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("copyMagnetItem"), "activate", G_CALLBACK(onCopyMagnetClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("getFileListItem"), "activate", G_CALLBACK(onGetFileListClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("matchQueueItem"), "activate", G_CALLBACK(onMatchQueueClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("sendPrivateMessageItem"), "activate", G_CALLBACK(onPrivateMessageClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("addToFavoritesItem"), "activate", G_CALLBACK(onAddFavoriteUserClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("grantExtraSlotItem"), "activate", G_CALLBACK(onGrantExtraSlotClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeUserFromQueueItem"), "activate", G_CALLBACK(onRemoveUserFromQueueClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("comboboxSize"), "changed", G_CALLBACK(onComboBoxChanged_gui), (gpointer)this);
	g_signal_connect(getWidget("comboboxentrySearch"), "changed", G_CALLBACK(onComboBoxChanged_gui), (gpointer)this);
	g_signal_connect(getWidget("comboboxUnit"), "changed", G_CALLBACK(onComboBoxChanged_gui), (gpointer)this);
	g_signal_connect(getWidget("comboboxFile"), "changed", G_CALLBACK(onComboBoxChanged_gui), (gpointer)this);
}

Search::~Search()
{
	ClientManager::getInstance()->removeListener(this);
	SearchManager::getInstance()->removeListener(this);

	clearList_gui();
	gtk_widget_destroy(getWidget("dirChooserDialog"));

	if (iconFile)
		g_object_unref(iconFile);
	if (iconDirectory)
		g_object_unref(iconDirectory);
}

void Search::show()
{
	initHubs_gui();
	ClientManager::getInstance()->addListener(this);
	SearchManager::getInstance()->addListener(this);
}

void Search::putValue_gui(const string &str, int64_t size, SearchManager::SizeModes mode, SearchManager::TypeModes type)
{
	gtk_entry_set_text(GTK_ENTRY(searchEntry), str.c_str());
	gtk_entry_set_text(GTK_ENTRY(getWidget("entrySize")), Util::toString(size).c_str());
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxSize")), (int)mode);
	gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("comboboxFile")), (int)type);

	search_gui();
}

void Search::initHubs_gui()
{
	ClientManager::getInstance()->lock();

	Client::List& clients = ClientManager::getInstance()->getClients();

	Client *client = NULL;
	for (Client::List::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		client = *it;
		if (client->isConnected())
			addHub_gui(client->getHubName(), client->getHubUrl(), client->getMyIdentity().isOp());
	}

	ClientManager::getInstance()->unlock();
}

void Search::addHub_gui(string name, string url, bool op)
{
	GtkTreeIter iter;
	gtk_list_store_append(hubStore, &iter);
	gtk_list_store_set(hubStore, &iter,
		hubView.col("Search"), TRUE,
		hubView.col("Name"), name.empty() ? url.c_str() : name.c_str(),
		hubView.col("Url"), url.c_str(),
		hubView.col("Op"), op,
		hubView.col("Editable"), onlyOp ? op : TRUE,
		-1);
}

void Search::modifyHub_gui(string name, string url, bool op)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(hubStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (url == hubView.getString(&iter, "Url"))
		{
			gtk_list_store_set(hubStore, &iter,
				hubView.col("Name"), name.empty() ? url.c_str() : name.c_str(),
				hubView.col("Url"), url.c_str(),
				hubView.col("Op"), op,
				hubView.col("Editable"), onlyOp ? op : TRUE,
				-1);
			return;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void Search::removeHub_gui(string url)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(hubStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (url == hubView.getString(&iter, "Url"))
		{
			gtk_list_store_remove(hubStore, &iter);
			return;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void Search::popupMenu_gui()
{
	int count = gtk_tree_selection_count_selected_rows(selection);

	if (count < 1)
		return;
	else if (count == 1)
	{
		gtk_widget_set_sensitive(getWidget("searchByTTHItem"), TRUE);
		gtk_widget_set_sensitive(getWidget("getFileListItem"), TRUE);
		gtk_widget_set_sensitive(getWidget("sendPrivateMessageItem"), TRUE);
		gtk_widget_set_sensitive(getWidget("addToFavoritesItem"), TRUE);
		gtk_widget_set_sensitive(getWidget("matchQueueItem"), TRUE);
		gtk_widget_set_sensitive(getWidget("grantExtraSlotItem"), TRUE);
		gtk_widget_set_sensitive(getWidget("removeUserFromQueueItem"), TRUE);
	}
	else if (count > 1)
	{
		gtk_widget_set_sensitive(getWidget("searchByTTHItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("getFileListItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("sendPrivateMessageItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("addToFavoritesItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("matchQueueItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("grantExtraSlotItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("removeUserFromQueueItem"), FALSE);
	}

	GtkWidget *menuItem;
	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list;
	string tth;
	bool firstTTH;
	bool hasTTH;

	// Clean menus
	gtk_container_foreach(GTK_CONTAINER(getWidget("downloadMenu")), (GtkCallback)gtk_widget_destroy, NULL);
	gtk_container_foreach(GTK_CONTAINER(getWidget("downloadDirMenu")), (GtkCallback)gtk_widget_destroy, NULL);
	userCommandMenu->cleanMenu_gui();

	// Build "Download to..." submenu

	// Add favorite download directories
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if (spl.size() > 0)
	{
		for (StringPairIter i = spl.begin(); i != spl.end(); i++)
		{
			menuItem = gtk_menu_item_new_with_label(i->second.c_str());
			g_object_set_data_full(G_OBJECT(menuItem), "fav", g_strdup(i->first.c_str()), g_free);
			g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadFavoriteClicked_gui), (gpointer)this);
			gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadMenu")), menuItem);
		}
		menuItem = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadMenu")), menuItem);
	}

	// Add Browse item
	menuItem = gtk_menu_item_new_with_label(_("Browse..."));
	g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadToClicked_gui), (gpointer)this);
	gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadMenu")), menuItem);

	// Add search results with the same TTH to menu
	firstTTH = TRUE;
	hasTTH = FALSE;
	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(sortedFilterModel, &iter, path))
		{
			userCommandMenu->addHub(resultView.getString(&iter, "Hub URL"));
			userCommandMenu->addUser(resultView.getString(&iter, "CID"));

			if (firstTTH)
			{
				tth = resultView.getString(&iter, "TTH");
				firstTTH = FALSE;
				hasTTH = TRUE;
			}
			else if (hasTTH)
			{
				if (tth.empty() || tth != resultView.getString(&iter, "TTH"))
					hasTTH = FALSE; // Can't break here since we have to free all the paths
			}
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

	if (hasTTH)
	{
		StringList targets;
		QueueManager::getInstance()->getTargets(TTHValue(tth), targets);

		if (targets.size() > static_cast<size_t>(0))
		{
			menuItem = gtk_separator_menu_item_new();
			gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadMenu")), menuItem);
			for (StringIter i = targets.begin(); i != targets.end(); ++i)
			{
				menuItem = gtk_menu_item_new_with_label(i->c_str());
				g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadToMatchClicked_gui), (gpointer)this);
				gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadMenu")), menuItem);
			}
		}
	}

	// Build "Download whole directory to..." submenu

	spl.clear();
	spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if (spl.size() > 0)
	{
		for (StringPairIter i = spl.begin(); i != spl.end(); i++)
		{
			menuItem = gtk_menu_item_new_with_label(i->second.c_str());
			g_object_set_data_full(G_OBJECT(menuItem), "fav", g_strdup(i->first.c_str()), g_free);
			g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadFavoriteDirClicked_gui), (gpointer)this);
			gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadDirMenu")), menuItem);
		}
		menuItem = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadDirMenu")), menuItem);
	}

	menuItem = gtk_menu_item_new_with_label(_("Browse..."));
	g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadDirToClicked_gui), (gpointer)this);
	gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("downloadDirMenu")), menuItem);

	// Build user command menu
	userCommandMenu->buildMenu_gui();

	gtk_menu_popup(GTK_MENU(getWidget("mainMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	gtk_widget_show_all(getWidget("mainMenu"));
}

void Search::setStatus_gui(string statusBar, string text)
{
	gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusBar)), 0);
	gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusBar)), 0, text.c_str());
}

void Search::search_gui()
{
	StringList clients;
	GtkTreeIter iter;

	string text = gtk_entry_get_text(GTK_ENTRY(searchEntry));
	if (text.empty())
		return;

	gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(hubStore), &iter);
 	while (valid)
 	{
 		if (hubView.getValue<gboolean>(&iter, "Editable") && hubView.getValue<gboolean>(&iter, "Search"))
 			clients.push_back(hubView.getString(&iter, "Url"));
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(hubStore), &iter);
	}

	if (clients.size() < 1)
		return;

	double lsize = Util::toDouble(gtk_entry_get_text(GTK_ENTRY(getWidget("entrySize"))));

	switch (gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxUnit"))))
	{
		case 1:
			lsize *= 1024.0;
			break;
		case 2:
			lsize *= 1024.0 * 1024.0;
			break;
		case 3:
			lsize *= 1024.0 * 1024.0 * 1024.0;
			break;
	}

	clearList_gui();
	int64_t llsize = static_cast<int64_t>(lsize);
	searchlist = StringTokenizer<string>(text, ' ').getTokens();

	// Strip out terms beginning with -
	text.clear();
	for (StringList::const_iterator si = searchlist.begin(); si != searchlist.end(); ++si)
		if ((*si)[0] != '-')
			text += *si + ' ';
	text = text.substr(0, std::max(text.size(), static_cast<string::size_type>(1)) - 1);

	SearchManager::SizeModes mode((SearchManager::SizeModes)gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxSize"))));
	if (llsize == 0)
		mode = SearchManager::SIZE_DONTCARE;

	int ftype = gtk_combo_box_get_active(GTK_COMBO_BOX(getWidget("comboboxFile")));
	isHash = (ftype == SearchManager::TYPE_TTH);

	// Add new searches to the dropdown list
	GtkListStore *store = GTK_LIST_STORE(searchEntriesModel);
	size_t max = std::max(SETTING(SEARCH_HISTORY) - 1, 0);
	size_t count = 0;
	gchar *entry;
	valid = gtk_tree_model_get_iter_first(searchEntriesModel, &iter);
	while (valid)
	{
		gtk_tree_model_get(searchEntriesModel, &iter, 0, &entry, -1);
		if (text == string(entry) || count >= max)
			valid = gtk_list_store_remove(store, &iter);
		else
			valid = gtk_tree_model_iter_next(searchEntriesModel, &iter);
		count++;
		g_free(entry);
	}

	gtk_list_store_prepend(store, &iter);
	gtk_list_store_set(store, &iter, 0, text.c_str(), -1);

	droppedResult = 0;
	searchHits = 0;
	setStatus_gui("statusbar1", _("Searching for ") + text + " ...");
	setStatus_gui("statusbar2", _("0 items"));
	setStatus_gui("statusbar3", _("0 filtered"));
	setLabel_gui(_("Search: ") + text);

	if (SearchManager::getInstance()->okToSearch())
	{
		SearchManager::getInstance()->search(clients, text, llsize, (SearchManager::TypeModes)ftype, mode, "manual");

		if (BOOLSETTING(CLEAR_SEARCH)) // Only clear if the search was sent.
			gtk_entry_set_text(GTK_ENTRY(searchEntry), "");
	}
	else
	{
		int32_t waitFor = SearchManager::getInstance()->timeToSearch();
		string line = _("Searching too soon, retry in ") + Util::toString(waitFor) + " s";
		setStatus_gui("statusbar1", line);
		setStatus_gui("statusbar2", "");
		setStatus_gui("statusbar3", "");
	}
}

void Search::addResult_gui(SearchResult *result, bool isShared)
{
	if (!result)
		return;

	string filename, fileOrder, path, type, size, exactSize, nick, cid, connection, hubName, hubURL, slots, ip, TTH;
	GdkPixbuf *icon;
	int actualSlots;

	if (result->getType() == SearchResult::TYPE_FILE)
	{
		string file = WulforUtil::linuxSeparator(result->getFile());
		if (file.rfind('/') == tstring::npos)
			filename = file;
		else
		{
			filename = Util::getFileName(file);
			path = Util::getFilePath(file);
		}

		fileOrder = "f" + filename;
		type = Util::getFileExt(filename);
		if (!type.empty() && type[0] == '.')
			type.erase(0, 1);
		size = Util::formatBytes(result->getSize());
		exactSize = Util::formatExactSize(result->getSize());
		icon = iconFile;
	}
	else
	{
		filename = WulforUtil::linuxSeparator(result->getFileName());
		path = WulforUtil::linuxSeparator(result->getFile());
		fileOrder = "d" + filename;
		type = _("Directory");
		icon = iconDirectory;
		if (result->getSize() > 0)
		{
			size = Util::formatBytes(result->getSize());
			exactSize = Util::formatExactSize(result->getSize());
		}
	}

	nick = WulforUtil::getNicks(result->getUser());
	cid = result->getUser()->getCID().toBase32();
	slots = result->getSlotString();
	connection = ClientManager::getInstance()->getConnection(result->getUser()->getCID());
	hubName = result->getHubName().empty() ? result->getHubURL().c_str() : result->getHubName().c_str();
	hubURL = result->getHubURL();
	ip = result->getIP();
	if (result->getType() == SearchResult::TYPE_FILE)
		TTH = result->getTTH().toBase32();

	// assumption: total slots is never above 999
	actualSlots = -1000 * result->getFreeSlots() - result->getSlots();

	// Check that it's not a duplicate
	GtkTreeIter iter;
	SearchResult *result2;
	gboolean valid = gtk_tree_model_get_iter_first(sortedFilterModel, &iter);
	while (valid)
	{
		result2 = resultView.getValue<gpointer, SearchResult *>(&iter, "SearchResult");
		if (result->getUser()->getCID() == result2->getUser()->getCID() && result->getFile() == result2->getFile())
		{
			result->decRef();
			return;
		}

		valid = gtk_tree_model_iter_next(sortedFilterModel, &iter);
	}

	// Have to use insert with values since appending would cause searchFilterFunc to be
	// called with empty row which in turn will cause assert failure in treeview::getString
	gtk_list_store_insert_with_values(resultStore, &iter, searchHits,
		resultView.col("Nick"), nick.c_str(),
		resultView.col("Filename"), filename.c_str(),
		resultView.col("Slots"), slots.c_str(),
		resultView.col("Size"), size.c_str(),
		resultView.col("Path"), path.c_str(),
		resultView.col("Type"), type.c_str(),
		resultView.col("Connection"), connection.c_str(),
		resultView.col("Hub"), hubName.c_str(),
		resultView.col("Exact Size"), exactSize.c_str(),
		resultView.col("IP"), ip.c_str(),
		resultView.col("TTH"), TTH.c_str(),
		resultView.col("Icon"), icon,
		resultView.col("File Order"), fileOrder.c_str(),
		resultView.col("Real Size"), result->getSize(),
		resultView.col("Slots Order"), actualSlots,
		resultView.col("SearchResult"), (gpointer)result,
		resultView.col("Hub URL"), hubURL.c_str(),
		resultView.col("CID"), cid.c_str(),
		resultView.col("Shared"), isShared, 
		-1);

	++searchHits;
	setStatus_gui("statusbar2", Util::toString(searchHits) + _(" items"));

	if (BOOLSETTING(BOLD_SEARCH))
		setBold_gui();
}

void Search::clearList_gui()
{
	GtkTreeIter iter;
	SearchResult *result = NULL;
	GtkTreeModel *m = GTK_TREE_MODEL(resultStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, &iter);
	while (valid)
	{
		result = resultView.getValue<gpointer, SearchResult *>(&iter, "SearchResult", m);
		if (result)
			result->decRef();
		valid = gtk_tree_model_iter_next(m, &iter);
	}
	gtk_list_store_clear(resultStore);
}

gboolean Search::onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	Search *s = (Search *)data;

	gtk_widget_grab_focus(s->getWidget("comboboxentrySearch"));

	return TRUE;
}

gboolean Search::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Search *s = (Search *)data;
	s->oldEventType = event->type;

	if (event->button == 3)
	{
		GtkTreePath *path;
		if (gtk_tree_view_get_path_at_pos(s->resultView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(s->selection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}
	return FALSE;
}

gboolean Search::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Search *s = (Search *)data;
	gint count = gtk_tree_selection_count_selected_rows(s->selection);

	if (count > 0 && event->type == GDK_BUTTON_RELEASE && event->button == 3)
		s->popupMenu_gui();
	else if (count == 1 && s->oldEventType == GDK_2BUTTON_PRESS && event->button == 1)
		s->onDownloadClicked_gui(NULL, data);

	return FALSE;
}

gboolean Search::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	Search *s = (Search *)data;
	if (widget == GTK_WIDGET(s->resultView.get()))
	{
		gint count = gtk_tree_selection_count_selected_rows(s->selection);

		if (count > 0)
		{
			if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
				s->onDownloadClicked_gui(NULL, data);
			else if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
				s->onRemoveClicked_gui(NULL, data);
			else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
				s->popupMenu_gui();
		}
	}
	else
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonFilter"))))
			gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(s->searchFilterModel));
	}

	return FALSE;
}

gboolean Search::onSearchEntryKeyPressed_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	Search *s = (Search *)data;

	if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
	{
		s->search_gui();
	}
	else if (event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
	{
		gtk_combo_box_popup(GTK_COMBO_BOX(s->getWidget("comboboxentrySearch")));
		return TRUE;
	}

	return FALSE;
}

void Search::onSearchButtonClicked_gui(GtkWidget *widget, gpointer data)
{
	Search *s = (Search *)data;
	s->search_gui();
}

void Search::onButtonToggled_gui(GtkToggleButton *button, gpointer data)
{
	Search *s = (Search *)data;

	if (button == GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonSlots")))
	{
		s->onlyFree = gtk_toggle_button_get_active(button);
		if (s->onlyFree != BOOLSETTING(SEARCH_ONLY_FREE_SLOTS))
			SettingsManager::getInstance()->set(SettingsManager::SEARCH_ONLY_FREE_SLOTS, s->onlyFree);
	}
	else if (button == GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonOp")))
	{
		onlyOp = gtk_toggle_button_get_active(button);
		GtkTreeIter iter;
		GtkTreeModel *m = GTK_TREE_MODEL(s->hubStore);
		bool valid = gtk_tree_model_get_iter_first(m, &iter);
		while (valid)
		{
			if (!s->hubView.getValue<gboolean>(&iter, "Op"))
				gtk_list_store_set(s->hubStore, &iter, s->hubView.col("Editable"), !onlyOp, -1);

			valid = gtk_tree_model_iter_next(m, &iter);
		}
	}

	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(s->searchFilterModel));
}

void Search::onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	Search *s = (Search *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->hubStore), &iter, path))
	{
		if (s->hubView.getValue<gboolean>(&iter, "Editable"))
		{
			gboolean toggled = s->hubView.getValue<gboolean>(&iter, "Search");
			gtk_list_store_set(s->hubStore, &iter, s->hubView.col("Search"), !toggled, -1);
		}
	}

	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(s->searchFilterModel));
}

void Search::onDownloadClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string fav;
		SearchResult *result;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func2<Search, string, SearchResult *> F2;
		F2 *func;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				result = s->resultView.getValue<gpointer, SearchResult *>(&iter, "SearchResult");
				if (result)
				{
					func = new F2(s, &Search::download_client, SETTING(DOWNLOAD_DIRECTORY), result);
					WulforManager::get()->dispatchClientFunc(func);
				}
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onDownloadFavoriteClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string fav;
		SearchResult *result;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func2<Search, string, SearchResult *> F2;
		F2 *func;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				result = s->resultView.getValue<gpointer, SearchResult *>(&iter, "SearchResult");
				fav = string((gchar *)g_object_get_data(G_OBJECT(item), "fav"));
				if (result && !fav.empty())
				{
					func = new F2(s, &Search::download_client, fav, result);
					WulforManager::get()->dispatchClientFunc(func);
				}
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onDownloadToClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	int response = gtk_dialog_run(GTK_DIALOG(s->getWidget("dirChooserDialog")));
	gtk_widget_hide(s->getWidget("dirChooserDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		int count = gtk_tree_selection_count_selected_rows(s->selection);
		gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->getWidget("dirChooserDialog")));

		if (temp && count > 0)
		{
			string target = Text::toUtf8(temp);
			g_free(temp);

			if (target[target.length() - 1] != PATH_SEPARATOR)
				target += PATH_SEPARATOR;

			SearchResult *result;
			GtkTreeIter iter;
			GtkTreePath *path;
			GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
			typedef Func2<Search, string, SearchResult *> F2;
			F2 *func;

			for (GList *i = list; i; i = i->next)
			{
				path = (GtkTreePath *)i->data;
				if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
				{
					result = s->resultView.getValue<gpointer, SearchResult *>(&iter, "SearchResult");
					if (result)
					{
						func = new F2(s, &Search::download_client, target, result);
						WulforManager::get()->dispatchClientFunc(func);
					}
				}
				gtk_tree_path_free(path);
			}
			g_list_free(list);
		}
	}
}

void Search::onDownloadToMatchClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string fileName = WulforUtil::getTextFromMenu(item);
		SearchResult *result;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func2<Search, string, SearchResult *> F2;
		F2 *func;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				result = s->resultView.getValue<gpointer, SearchResult *>(&iter, "SearchResult");
				if (result)
				{
					func = new F2(s, &Search::addSource_client, fileName, result);
					WulforManager::get()->dispatchClientFunc(func);
				}
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onDownloadDirClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		SearchResult *result;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func2<Search, string, SearchResult *> F2;
		F2 *func;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				result = s->resultView.getValue<gpointer, SearchResult *>(&iter, "SearchResult");
				if (result)
				{
					func = new F2(s, &Search::downloadDir_client, SETTING(DOWNLOAD_DIRECTORY), result);
					WulforManager::get()->dispatchClientFunc(func);
				}
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onDownloadFavoriteDirClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string fav;
		SearchResult *result;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func2<Search, string, SearchResult *> F2;
		F2 *func;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				result = s->resultView.getValue<gpointer, SearchResult *>(&iter, "SearchResult");
				fav = (gchar *)g_object_get_data(G_OBJECT(item), "fav");

				if (result && !fav.empty())
				{
					func = new F2(s, &Search::downloadDir_client, fav, result);
					WulforManager::get()->dispatchClientFunc(func);
				}
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onDownloadDirToClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	gint response = gtk_dialog_run(GTK_DIALOG(s->getWidget("dirChooserDialog")));
	gtk_widget_hide(s->getWidget("dirChooserDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		int count = gtk_tree_selection_count_selected_rows(s->selection);
		gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->getWidget("dirChooserDialog")));

		if (temp && count > 0)
		{
			string target = Text::toUtf8(temp);
			g_free(temp);

			if (target[target.length() - 1] != PATH_SEPARATOR)
				target += PATH_SEPARATOR;

			SearchResult *result;
			GtkTreeIter iter;
			GtkTreePath *path;
			GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
			typedef Func2<Search, string, SearchResult *> F2;
			F2 *func;

			for (GList *i = list; i; i = i->next)
			{
				path = (GtkTreePath *)i->data;
				if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
				{
					result = s->resultView.getValue<gpointer, SearchResult *>(&iter, "SearchResult");
					if (result)
					{
						func = new F2(s, &Search::downloadDir_client, target, result);
						WulforManager::get()->dispatchClientFunc(func);
					}
				}
				gtk_tree_path_free(path);
			}
			g_list_free(list);
		}
	}
}

void Search::onSearchByTTHClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		SearchResult *result;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				result = s->resultView.getValue<gpointer, SearchResult *>(&iter, "SearchResult");
				if (result && result->getType() == SearchResult::TYPE_FILE)
					s->putValue_gui(result->getTTH().toBase32(), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onGetFileListClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string cid, dir;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func3<Search, string, string, bool> F3;
		F3 *func;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				cid = s->resultView.getString(&iter, "CID");
				dir = s->resultView.getString(&iter, "Path");
				func = new F3(s, &Search::getFileList_client, cid, dir, FALSE);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onMatchQueueClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func3<Search, string, string, bool> F3;
		F3 *func;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				cid = s->resultView.getString(&iter, "CID");
				func = new F3(s, &Search::getFileList_client, cid, "", TRUE);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onPrivateMessageClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				cid = s->resultView.getString(&iter, "CID");
				WulforManager::get()->getMainWindow()->addPrivateMessage_gui(cid);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onAddFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func1<Search, string> F1;
		F1 *func;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				cid = s->resultView.getString(&iter, "CID");
				func = new F1(s, &Search::addFavUser_client, cid);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onGrantExtraSlotClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func1<Search, string> F1;
		F1 *func;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				cid = s->resultView.getString(&iter, "CID");
				func = new F1(s, &Search::grantSlot_client, cid);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onRemoveUserFromQueueClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);
		typedef Func1<Search, string> F1;
		F1 *func;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				cid = s->resultView.getString(&iter, "CID");
				func = new F1(s, &Search::removeSource_client, cid);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onRemoveClicked_gui(GtkMenuItem *item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		SearchResult *result;
		GtkTreeIter iter, filterIter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				result = s->resultView.getValue<gpointer, SearchResult *>(&iter, "SearchResult");
				if (result)
					result->decRef();
				gtk_tree_model_sort_convert_iter_to_child_iter(GTK_TREE_MODEL_SORT(s->sortedFilterModel), &filterIter, &iter);
				gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(s->searchFilterModel), &iter, &filterIter);
				gtk_list_store_remove(s->resultStore, &iter);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Search::onCopyMagnetClicked_gui(GtkMenuItem* item, gpointer data)
{
	Search *s = (Search *)data;

	if (gtk_tree_selection_count_selected_rows(s->selection) > 0)
	{
		int64_t size;
		string magnets, magnet, filename, tth;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(s->selection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(s->sortedFilterModel, &iter, path))
			{
				filename = s->resultView.getString(&iter, "Filename");
				size = s->resultView.getValue<int64_t>(&iter, "Real Size");
				tth = s->resultView.getString(&iter, "TTH");
				magnet = WulforUtil::makeMagnet(filename, size, tth);

				if (!magnet.empty())
				{
					if (!magnets.empty())
						magnets += '\n';
					magnets += magnet;
				}
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		if (!magnets.empty())
			gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), magnets.c_str(), magnets.length());
	}
}

void Search::download_client(string target, SearchResult *result)
{
	try
	{
		if (result->getType() == SearchResult::TYPE_FILE)
		{
			string subdir = Util::getFileName(WulforUtil::linuxSeparator(result->getFile()));
			QueueManager::getInstance()->add(target + subdir, result->getSize(), result->getTTH(), result->getUser());
		}
		else
		{
			QueueManager::getInstance()->addDirectory(result->getFile(), result->getUser(), target);
		}
	}
	catch (const Exception&)
	{
	}
}

void Search::downloadDir_client(string target, SearchResult *result)
{
	try
	{
		if (result->getType() == SearchResult::TYPE_FILE)
		{
			string path = WulforUtil::windowsSeparator(Util::getFilePath(WulforUtil::linuxSeparator(result->getFile())));
			QueueManager::getInstance()->addDirectory(path, result->getUser(), target);
		}
		else
		{
			QueueManager::getInstance()->addDirectory(result->getFile(), result->getUser(), target);
		}
	}
	catch (const Exception&)
	{
	}
}

void Search::addSource_client(string source, SearchResult *result)
{
	try
	{
		if (result->getType() == SearchResult::TYPE_FILE)
		{
			QueueManager::getInstance()->add(source, result->getSize(), result->getTTH(), result->getUser());
		}
	}
	catch (const Exception&)
	{
	}
}

void Search::getFileList_client(string cid, string dir, bool match)
{
	if (!cid.empty())
	{
		try
		{
			User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
			if (user)
			{
				QueueItem::FileFlags flags;
				if (match)
					flags = QueueItem::FLAG_MATCH_QUEUE;
				else
					flags = QueueItem::FLAG_CLIENT_VIEW;

				QueueManager::getInstance()->addList(user, flags, dir);
			}
		}
		catch (const Exception&)
		{
		}
	}
}

void Search::grantSlot_client(string cid)
{
	if (!cid.empty())
	{
		User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			UploadManager::getInstance()->reserveSlot(user);
	}
}

void Search::addFavUser_client(string cid)
{
	if (!cid.empty())
	{
		User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			FavoriteManager::getInstance()->addFavoriteUser(user);
	}
}

void Search::removeSource_client(string cid)
{
	if (!cid.empty())
	{
		User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
	}
}

void Search::on(ClientManagerListener::ClientConnected, Client *client) throw()
{
	if (client)
	{
		typedef Func3<Search, string, string, bool> F3;
		F3 *func = new F3(this, &Search::addHub_gui, client->getHubName(), client->getHubUrl(), client->getMyIdentity().isOp());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Search::on(ClientManagerListener::ClientUpdated, Client *client) throw()
{
	if (client)
	{
		typedef Func3<Search, string, string, bool> F3;
		F3 *func = new F3(this, &Search::modifyHub_gui, client->getHubName(), client->getHubUrl(), client->getMyIdentity().isOp());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Search::on(ClientManagerListener::ClientDisconnected, Client *client) throw()
{
	if (client)
	{
		typedef Func1<Search, string> F1;
		F1 *func = new F1(this, &Search::removeHub_gui, client->getHubUrl());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Search::on(SearchManagerListener::SR, SearchResult *result) throw()
{
	bool isShared = false;
	if (searchlist.empty() || result == NULL)
		return;

	typedef Func2<Search, string, string> F2;
	F2 *func;

	if (isHash)
	{
		if (result->getType() != SearchResult::TYPE_FILE || TTHValue(searchlist[0]) != result->getTTH())
		{
			++droppedResult;
			func = new F2(this, &Search::setStatus_gui, "statusbar3", Util::toString(droppedResult) + _(" filtered"));
			WulforManager::get()->dispatchGuiFunc(func);
			return;
		}
	}
	else
	{
		for (TStringIter i = searchlist.begin(); i != searchlist.end(); ++i)
		{
			if ((*i->begin() != '-' && Util::findSubString(result->getFile(), *i) == (string::size_type)-1) ||
			    (*i->begin() == '-' && i->size() != 1 && Util::findSubString(result->getFile(), i->substr(1)) != (string::size_type)-1))
			{
				++droppedResult;
				func = new F2(this, &Search::setStatus_gui, "statusbar3", Util::toString(droppedResult) + _(" filtered"));
				WulforManager::get()->dispatchGuiFunc(func);
				return;
			}
		}
	}

	// Reject results without free slots
	if (onlyFree && result->getFreeSlots() < 1)
	{
		++droppedResult;
		func = new F2(this, &Search::setStatus_gui, "statusbar3", Util::toString(droppedResult) + _(" filtered"));
		WulforManager::get()->dispatchGuiFunc(func);
		return;
	}

	if (result->getType() != SearchResult::TYPE_DIRECTORY)
		isShared = ShareManager::getInstance()->isTTHShared(result->getTTH());

	result->incRef();

	typedef Func2<Search, SearchResult *, bool> F2_2;
	F2_2 *func2 = new F2_2(this, &Search::addResult_gui, result, isShared);
	WulforManager::get()->dispatchGuiFunc(func2);
}


gboolean Search::searchFilterFunc_gui(GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	Search *s = (Search *)data;
	dcassert(model == GTK_TREE_MODEL(s->resultStore));

	// Filter based on selected hubs.
	SearchResult *result = s->resultView.getValue<gpointer, SearchResult *>(iter, "SearchResult", model);
	string hub = result->getHubURL();
	GtkTreeIter hubIter;
	bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(s->hubStore), &hubIter);
	while (valid)
	{
		if (hub == s->hubView.getString(&hubIter, "Url"))
		{
			if (!s->hubView.getValue<gboolean>(&hubIter, "Editable"))
				return FALSE;
			else if (!s->hubView.getValue<gboolean>(&hubIter, "Search"))
				return FALSE;
			else
				break;
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(s->hubStore), &hubIter);
	}

	// Filter based on free slots.
	if (s->onlyFree && result->getFreeSlots() < 1)
		return FALSE;

	// Hide results already in share
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonShared"))) &&
		s->resultView.getValue<gboolean>(iter, "Shared", model) == TRUE)
		return FALSE;

	// Search within local results.
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonFilter"))))
		return TRUE;

	// Filter based on search terms.
	string filter = Text::toLower(gtk_entry_get_text(GTK_ENTRY(s->searchEntry)));
	TStringList filterList = StringTokenizer<tstring>(filter, ' ').getTokens();
	string filename = Text::toLower(s->resultView.getString(iter, "Filename", model));
	string path = Text::toLower(s->resultView.getString(iter, "Path", model));
	for (TStringList::const_iterator term = filterList.begin(); term != filterList.end(); ++term)
	{
		if ((*term)[0] == '-')
		{
			if (filename.find((*term).substr(1)) != string::npos)
				return FALSE;
			else if (path.find((*term).substr(1)) != string::npos)
				return FALSE;
		}
		else if (filename.find(*term) == string::npos && path.find(*term) == string::npos)
			return FALSE;
	}

	// Filter based on file size.
	double filterSize = Util::toDouble(gtk_entry_get_text(GTK_ENTRY(s->getWidget("entrySize"))));
	if (filterSize > 0)
	{
		switch (gtk_combo_box_get_active(GTK_COMBO_BOX(s->getWidget("comboboxUnit"))))
		{
			case 1:
				filterSize *= 1024.0;
				break;
			case 2:
				filterSize *= 1024.0 * 1024.0;
				break;
			case 3:
				filterSize *= 1024.0 * 1024.0 * 1024.0;
				break;
		}

		switch (gtk_combo_box_get_active(GTK_COMBO_BOX(s->getWidget("comboboxSize"))))
		{
			case 0:
				if (result->getSize() != filterSize)
					return FALSE;
				break;
			case 1:
				if (result->getSize() < filterSize)
					return FALSE;
				break;
			case 2:
				if (result->getSize() > filterSize)
					return FALSE;
		}
	}

	int type = gtk_combo_box_get_active(GTK_COMBO_BOX(s->getWidget("comboboxFile")));
	if (type != SearchManager::TYPE_ANY && type != ShareManager::getInstance()->getType(filename))
		return FALSE;

	return TRUE;
}

void Search::onComboBoxChanged_gui(GtkWidget* widget, gpointer data)
{
	Search *s = (Search *)data;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("checkbuttonFilter"))))
		gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(s->searchFilterModel));
}
