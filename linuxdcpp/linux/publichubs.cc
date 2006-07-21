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

#include "publichubs.hh"

using namespace std;

PublicHubs::PublicHubs():
	BookEntry("Public Hubs"),
	hubs(0),
	filter("")
{
	FavoriteManager *hman = FavoriteManager::getInstance();
	hman->addListener(this);

	GladeXML *xml = getGladeXML("publichubs.glade");

	GtkWidget *window = glade_xml_get_widget(xml, "publicHubsWindow");
	mainBox = glade_xml_get_widget(xml, "publicHubsBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	filterEntry = GTK_ENTRY(glade_xml_get_widget(xml, "filterEntry"));
	comboBox = GTK_COMBO_BOX(glade_xml_get_widget(xml, "hubListBox"));
	configureDialog = GTK_DIALOG(glade_xml_get_widget(xml, "configureDialog"));
	gtk_dialog_set_alternative_button_order(configureDialog, GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	statusMain = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusMain"));
	statusHubs = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusHubs"));
	statusUsers = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusUsers"));

	if (hman->isDownloading())
		setStatus_gui(statusMain, "Downloading hub list");

	// Initialize public hub list treeview
	hubView.setView(GTK_TREE_VIEW(glade_xml_get_widget(xml, "hubView")), true, "publichubs");
	hubView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, 200);
	hubView.insertColumn("Description", G_TYPE_STRING, TreeView::STRING, 350);
	hubView.insertColumn("Users", G_TYPE_INT, TreeView::INT, 75);
	hubView.insertColumn("Address", G_TYPE_STRING, TreeView::STRING, 100);
	hubView.finalize();
	hubStore = gtk_list_store_newv(hubView.getColCount(), hubView.getGTypes());
	gtk_tree_view_set_model(hubView.get(), GTK_TREE_MODEL(hubStore));
	g_object_unref(hubStore);
	hubSelection = gtk_tree_view_get_selection(hubView.get());
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(hubStore), hubView.col("Users"), GTK_SORT_DESCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(hubView.get(), hubView.col("Users")), TRUE);
	gtk_tree_view_set_fixed_height_mode(hubView.get(), TRUE);

	menu = GTK_MENU(glade_xml_get_widget(xml, "menu"));
	GObject *conItem = G_OBJECT(glade_xml_get_widget(xml, "connectMenuItem"));
	GObject *favItem = G_OBJECT(glade_xml_get_widget(xml, "favMenuItem"));

	// Initialize list of public hub lists treeview
	listsView.setView(GTK_TREE_VIEW(glade_xml_get_widget(xml, "listsView")));
	listsView.insertColumn("List", G_TYPE_STRING, TreeView::EDIT_STRING, -1);
	listsView.finalize();
	listsStore = gtk_list_store_newv(listsView.getColCount(), listsView.getGTypes());
	gtk_tree_view_set_model(listsView.get(), GTK_TREE_MODEL(listsStore));
	g_object_unref(listsStore);
	gtk_tree_view_set_headers_visible(listsView.get(), FALSE);
	listsSelection = gtk_tree_view_get_selection(listsView.get());
 	StringList list = hman->getHubLists();
	GtkTreeIter iter;
	for (StringList::iterator it = list.begin(); it != list.end(); it++)
	{
		gtk_list_store_append(listsStore, &iter);
		gtk_list_store_set(listsStore, &iter, 0, it->c_str(), -1);
	}

	// Fill the combo box with hub lists
	gtk_combo_box_set_model(comboBox, GTK_TREE_MODEL(listsStore));
	GtkCellRenderer *cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(comboBox), cell, FALSE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(comboBox), cell, "text", 0);
	gtk_combo_box_set_active(comboBox, hman->getSelectedHubList());

	// Connect the callbacks
	GtkTreeViewColumn *c = gtk_tree_view_get_column(listsView.get(), 0);
	GList *l = gtk_tree_view_column_get_cell_renderers(c);
	GObject *editRenderer = G_OBJECT(g_list_nth_data(l, 0));
	g_list_free(l);
	GObject *connectButton = G_OBJECT(glade_xml_get_widget(xml, "connectButton"));
	GObject *refreshButton = G_OBJECT(glade_xml_get_widget(xml, "refreshButton"));
	GObject *configureButton = G_OBJECT(glade_xml_get_widget(xml, "configureButton"));
	GObject *addButton = G_OBJECT(glade_xml_get_widget(xml, "addButton"));
	GObject *upButton = G_OBJECT(glade_xml_get_widget(xml, "upButton"));
	GObject *downButton = G_OBJECT(glade_xml_get_widget(xml, "downButton"));
	GObject *removeButton = G_OBJECT(glade_xml_get_widget(xml, "removeButton"));

	g_signal_connect(filterEntry, "key-release-event", G_CALLBACK(onFilterHubs_gui), (gpointer)this);
	g_signal_connect(connectButton, "clicked", G_CALLBACK(onConnect_gui), (gpointer)this);
	g_signal_connect(conItem, "activate", G_CALLBACK(onConnect_gui), (gpointer)this);
	g_signal_connect(refreshButton, "clicked", G_CALLBACK(onRefresh_gui), (gpointer)this);
	g_signal_connect(comboBox, "changed", G_CALLBACK(onRefresh_gui), (gpointer)this);
	g_signal_connect(configureButton, "clicked", G_CALLBACK(onConfigure_gui), (gpointer)this);
	g_signal_connect(upButton, "clicked", G_CALLBACK(onMoveUp_gui), (gpointer)this);
	g_signal_connect(downButton, "clicked", G_CALLBACK(onMoveDown_gui), (gpointer)this);
	g_signal_connect(addButton, "clicked", G_CALLBACK(onAdd_gui), (gpointer)this);
	g_signal_connect(removeButton, "clicked", G_CALLBACK(onRemove_gui), (gpointer)this);
	g_signal_connect(editRenderer, "edited", G_CALLBACK(onCellEdited_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(hubView.get()), "button-press-event", G_CALLBACK(onButtonPress_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(hubView.get()), "button-release-event", G_CALLBACK(onButtonRelease_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(hubView.get()), "key-release-event", G_CALLBACK(onKeyRelease_gui), (gpointer)this);
	g_signal_connect(favItem, "activate", G_CALLBACK(onAddFav_gui), (gpointer)this);

	pthread_mutex_init(&hubLock, NULL);
}

PublicHubs::~PublicHubs()
{
	FavoriteManager::getInstance()->removeListener(this);
	pthread_mutex_destroy(&hubLock);
	gtk_widget_destroy(GTK_WIDGET(configureDialog));
}

GtkWidget *PublicHubs::getWidget()
{
	return mainBox;
}

void PublicHubs::downloadList_client()
{
	pthread_mutex_lock(&hubLock);
	hubs = FavoriteManager::getInstance()->getPublicHubs();
	pthread_mutex_unlock(&hubLock);

	if (hubs.empty())
		FavoriteManager::getInstance()->refresh();

	FavoriteManager::getInstance()->save();

	Func0<PublicHubs> *func = new Func0<PublicHubs>(this, &PublicHubs::updateList_gui);
	WulforManager::get()->dispatchGuiFunc(func);
}

void PublicHubs::updateList_gui()
{
	HubEntry::List::const_iterator i;
	GtkTreeIter iter;
	int numHubs = 0;
	int numUsers = 0;
	gint sortColumn;
	GtkSortType sortType;

	gtk_list_store_clear(hubStore);

	gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(hubStore), &sortColumn, &sortType);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(hubStore), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, sortType);

	pthread_mutex_lock(&hubLock);
	for (i = hubs.begin(); i != hubs.end(); i++)
	{
		if (filter.getPattern().empty() || filter.match(i->getName()) ||
			filter.match(i->getDescription()) || filter.match(i->getServer()))
		{
			gtk_list_store_append(hubStore, &iter);
			gtk_list_store_set(hubStore, &iter,
				hubView.col("Name"), i->getName().c_str(),
				hubView.col("Description"), i->getDescription().c_str(),
				hubView.col("Users"), i->getUsers(),
				hubView.col("Address"), i->getServer().c_str(),
				-1);

			numUsers += i->getUsers();
			numHubs++;
		}
	}
	pthread_mutex_unlock(&hubLock);

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(hubStore), sortColumn, sortType);

	setStatus_gui(statusHubs, "Hubs: " + Util::toString(numHubs));
	setStatus_gui(statusUsers, "Users: " + Util::toString(numUsers));
}

void PublicHubs::setStatus_gui(GtkStatusbar *status, string text)
{
	gtk_statusbar_pop(status, 0);
	gtk_statusbar_push(status, 0, text.c_str());
}

gboolean PublicHubs::onButtonPress_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;

	if (event->type == GDK_BUTTON_PRESS || event->type == GDK_2BUTTON_PRESS)
	{
		ph->oldType = event->type;
		ph->oldButton = event->button;
	}

	return FALSE;
}

gboolean PublicHubs::onButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;

	if (ph->oldButton == event->button && gtk_tree_selection_get_selected(ph->hubSelection, NULL, NULL))
	{
		if (event->button == 3 && ph->oldType == GDK_BUTTON_PRESS)
		{
			gtk_menu_popup(ph->menu, NULL, NULL, NULL, NULL, 0, event->time);
			gtk_widget_show_all(GTK_WIDGET(ph->menu));
		}
		else if (event->button == 1 && ph->oldType == GDK_2BUTTON_PRESS)
		{
			ph->onConnect_gui(NULL, data);
		}
	}

	return FALSE;
}

gboolean PublicHubs::onKeyRelease_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;

	if (gtk_tree_selection_get_selected(ph->hubSelection, NULL, NULL))
	{
		if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			gtk_menu_popup(ph->menu, NULL, NULL, NULL, NULL, 0, event->time);
			gtk_widget_show_all(GTK_WIDGET(ph->menu));
		}
		else if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		{
			ph->onConnect_gui(NULL, data);
		}
	}

	return FALSE;
}

gboolean PublicHubs::onFilterHubs_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	StringSearch pattern(gtk_entry_get_text(ph->filterEntry));

	if (!(pattern == ph->filter))
	{
		ph->filter = pattern;
		ph->updateList_gui();
	}

	return FALSE;
}

void PublicHubs::onConnect_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(ph->hubSelection, NULL, &iter))
	{
		string address = ph->hubView.getString(&iter, "Address");
		WulforManager::get()->addHub_gui(address);
	}
}

void PublicHubs::onRefresh_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	int pos = gtk_combo_box_get_active(ph->comboBox);
	FavoriteManager::getInstance()->setHubList(pos);
	Func0<PublicHubs> *f = new Func0<PublicHubs>(ph, &PublicHubs::refresh_client);
	WulforManager::get()->dispatchClientFunc(f);
}

void PublicHubs::onAddFav_gui(GtkMenuItem *item, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	FavoriteHubEntry entry;
	string name, address, description;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(ph->hubSelection, NULL, &iter))
	{
		name = ph->hubView.getString(&iter, "Name");
		description = ph->hubView.getString(&iter, "Description");
		address = ph->hubView.getString(&iter, "Address");

		entry.setName(name);
		entry.setServer(address);
		entry.setDescription(description);
		entry.setNick(SETTING(NICK));
		entry.setPassword("");
		entry.setUserDescription(SETTING(DESCRIPTION));

		typedef Func1<PublicHubs, FavoriteHubEntry> F1;
		F1 *func = new F1(ph, &PublicHubs::addFav_client, entry);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void PublicHubs::onConfigure_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;

	// Have to get active here since temp could be NULL after dialog is closed
	gchar *temp = gtk_combo_box_get_active_text(ph->comboBox);
	string active = string(temp);
	g_free(temp);

	gtk_widget_show_all(GTK_WIDGET(ph->configureDialog));
	gint ret = gtk_dialog_run(ph->configureDialog);
	gtk_widget_hide(GTK_WIDGET(ph->configureDialog));

	if (ret == GTK_RESPONSE_OK)
	{
		string lists, url;
		GtkTreeIter iter;

		GtkTreeModel *m = GTK_TREE_MODEL(ph->listsStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, &iter);
		while (valid)
		{
			url = ph->listsView.getString(&iter, "List");
			lists += url + ";";
			if (url == active)
				gtk_combo_box_set_active_iter(ph->comboBox, &iter);
			valid = gtk_tree_model_iter_next(m, &iter);
		}

		if (gtk_combo_box_get_active(ph->comboBox) < 0)
			gtk_combo_box_set_active(ph->comboBox, 0);

		if (!lists.empty())
			lists.erase(lists.size() - 1);

		SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, lists);
	}
}

void PublicHubs::onAdd_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeViewColumn *col;

	gtk_list_store_append(ph->listsStore, &iter);
	gtk_list_store_set(ph->listsStore, &iter, ph->listsView.col("List"), "New list", -1);
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(ph->listsStore), &iter);
	col = gtk_tree_view_get_column(ph->listsView.get(), 0);
	gtk_tree_view_set_cursor(ph->listsView.get(), path, col, TRUE);
	gtk_tree_path_free(path);
}

void PublicHubs::onMoveUp_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter prev, current;
	GtkTreeModel *m = GTK_TREE_MODEL(ph->listsStore);

	if (gtk_tree_selection_get_selected(ph->listsSelection, NULL, &current))
	{
		GtkTreePath *path = gtk_tree_model_get_path(m, &current);

		if (gtk_tree_path_prev(path) && gtk_tree_model_get_iter(m, &prev, path))
		{
			gtk_list_store_swap(ph->listsStore, &current, &prev);
		}
		gtk_tree_path_free(path);
	}
}

void PublicHubs::onMoveDown_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter current, next;

	if (gtk_tree_selection_get_selected(ph->listsSelection, NULL, &current))
	{
		next = current;
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(ph->listsStore), &next))
			gtk_list_store_swap(ph->listsStore, &current, &next);
	}
}

void PublicHubs::onRemove_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter current;

	if (gtk_tree_selection_get_selected(ph->listsSelection, NULL, &current))
		gtk_list_store_remove(ph->listsStore, &current);
}

void PublicHubs::onCellEdited_gui(GtkCellRendererText *cell, char *path, char *text, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter it;

	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(ph->listsStore), &it, path);
	gtk_list_store_set(ph->listsStore, &it, ph->listsView.col("List"), text, -1);
}

void PublicHubs::refresh_client()
{
	FavoriteManager::getInstance()->refresh();
}

void PublicHubs::addFav_client(FavoriteHubEntry entry)
{
	FavoriteManager::getInstance()->addFavorite(entry);
}

void PublicHubs::on(FavoriteManagerListener::DownloadStarting, const string &file) throw()
{
	string msg = "Download starting: " + file;
	typedef Func2<PublicHubs, GtkStatusbar*, string> Func;
	Func *func = new Func(this, &PublicHubs::setStatus_gui, statusMain, msg);
	WulforManager::get()->dispatchGuiFunc(func);
}

void PublicHubs::on(FavoriteManagerListener::DownloadFailed, const string &file) throw()
{
	string msg = "Download failed: " + file;
	typedef Func2<PublicHubs, GtkStatusbar*, string> Func;
	Func *func = new Func(this, &PublicHubs::setStatus_gui, statusMain, msg);
	WulforManager::get()->dispatchGuiFunc(func);
}

void PublicHubs::on(FavoriteManagerListener::DownloadFinished, const string &file) throw()
{
	string msg = "Download finished: " + file;
	typedef Func2<PublicHubs, GtkStatusbar*, string> Func;
	Func *f2 = new Func(this, &PublicHubs::setStatus_gui, statusMain, msg);
	WulforManager::get()->dispatchGuiFunc(f2);

	pthread_mutex_lock(&hubLock);
	hubs = FavoriteManager::getInstance()->getPublicHubs();
	pthread_mutex_unlock(&hubLock);

	Func0<PublicHubs> *f0 = new Func0<PublicHubs>(this, &PublicHubs::updateList_gui);
	WulforManager::get()->dispatchGuiFunc(f0);
}
