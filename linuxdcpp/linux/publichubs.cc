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

#include "publichubs.hh"
#include "wulformanager.hh"
#include "treeviewfactory.hh"
#include <iostream>
#include <sstream>

using namespace std;

PublicHubs::PublicHubs(GCallback closeCallback):
	BookEntry(WulforManager::PUBLIC_HUBS, "", "Public Hubs", closeCallback),
	filterCallback(this, &PublicHubs::filterHubs_gui),
	connectCallback(this, &PublicHubs::connect_gui),
	refreshCallback(this, &PublicHubs::refresh_gui),
	configureCallback(this, &PublicHubs::configure_gui),
	upCallback(this, &PublicHubs::moveUp_gui),
	downCallback(this, &PublicHubs::moveDown_gui),
	addCallback(this, &PublicHubs::add_gui),
	removeCallback(this, &PublicHubs::remove_gui),
	editCallback(this, &PublicHubs::cellEdited_gui),
	mouseButtonCallback(this, &PublicHubs::buttonEvent_gui),
	addFavCallback(this, &PublicHubs::addFav_gui),
	hubs(0),
	filter(""),
	WIDTH_NAME(200),
	WIDTH_DESC(350),
	WIDTH_USERS(50),
	WIDTH_ADDRESS(100)
{
	HubManager *hman = HubManager::getInstance();
	GObject *o;

	hman->addListener(this);
	if (hman->isDownloading())
		setStatus_gui(statusMain, "Downloading hub list");
	
	string file = WulforManager::get()->getPath() + "/glade/publichubs.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "publicHubsWindow");
	mainBox = glade_xml_get_widget(xml, "publicHubsBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	filterEntry = GTK_ENTRY(glade_xml_get_widget(xml, "filterEntry"));
	hubView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "hubView"));
	listsView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "listsView"));
	combo = GTK_COMBO_BOX(glade_xml_get_widget(xml, "hubListBox"));
	configureDialog = GTK_DIALOG(glade_xml_get_widget(xml, "configureDialog"));

	statusMain = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusMain"));
	statusHubs = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusHubs"));
	statusUsers = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusUsers"));
	
	hubStore = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
	gtk_tree_view_set_model(hubView, GTK_TREE_MODEL(hubStore));

	TreeViewFactory factory(hubView);
	factory.addColumn_gui(COLUMN_NAME, "Name", TreeViewFactory::STRING, WIDTH_NAME);
	factory.addColumn_gui(COLUMN_DESC, "Description", TreeViewFactory::STRING, WIDTH_DESC);
	factory.addColumn_gui(COLUMN_USERS, "Users", TreeViewFactory::INT, WIDTH_USERS);
	factory.addColumn_gui(COLUMN_ADDRESS, "Address", TreeViewFactory::STRING, WIDTH_ADDRESS);

	menu = GTK_MENU(gtk_menu_new()); 
	conItem = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Connect"));
	favItem = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Add to favourites"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(conItem));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(favItem));

	//Fill the combo box with hub lists
	comboStore = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_combo_box_set_model(combo, GTK_TREE_MODEL(comboStore));
	GtkCellRenderer *cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), cell, FALSE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(combo), cell, "text", 0);
 	StringList list = hman->getHubLists();
	StringList::iterator it;
	GtkTreeIter iter, current;
	for (it = list.begin(); it != list.end(); it++) {
		gtk_list_store_append(comboStore, &iter);
		gtk_list_store_set(comboStore, &iter,
			0, it->c_str(),
			-1);
	}
	gtk_combo_box_set_active(combo, hman->getSelectedHubList());

	gtk_tree_view_set_headers_visible(listsView, FALSE);
	listsStore = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(listsView, GTK_TREE_MODEL(listsStore));
	TreeViewFactory factory2(listsView);
	factory2.addColumn_gui(0, "", TreeViewFactory::EDIT_STRING, -1);

	GtkTreeViewColumn *c = gtk_tree_view_get_column(listsView, 0);
	GList *l = gtk_tree_view_column_get_cell_renderers(c);
	o = G_OBJECT(l->data);
	editCallback.connect(o, "edited", NULL);
	g_list_free(l);
			
	o = G_OBJECT(glade_xml_get_widget(xml, "filterButton"));
	filterCallback.connect(o, "clicked", NULL);
	o = G_OBJECT(glade_xml_get_widget(xml, "connectButton"));
	connectCallback.connect(o, "clicked", NULL);
	o = G_OBJECT(glade_xml_get_widget(xml, "refreshButton"));
	refreshCallback.connect(o, "clicked", NULL);
	o = G_OBJECT(glade_xml_get_widget(xml, "configureButton"));
	configureCallback.connect(o, "clicked", NULL);

	o = G_OBJECT(glade_xml_get_widget(xml, "addButton"));
	addCallback.connect(o, "clicked", NULL);
	o = G_OBJECT(glade_xml_get_widget(xml, "upButton"));
	upCallback.connect(o, "clicked", NULL);
	o = G_OBJECT(glade_xml_get_widget(xml, "downButton"));
	downCallback.connect(o, "clicked", NULL);
	o = G_OBJECT(glade_xml_get_widget(xml, "removeButton"));
	removeCallback.connect(o, "clicked", NULL);

	addFavCallback.connect(G_OBJECT(favItem), "activate", NULL);
	connectCallback.connect(G_OBJECT(conItem), "activate", NULL);
	mouseButtonCallback.connect(G_OBJECT(hubView), "button-press-event", NULL);
	mouseButtonCallback.connect(G_OBJECT(hubView), "button-release-event", NULL);

	filterCallback.connect(G_OBJECT(filterEntry), "activate", NULL);
	refreshCallback.connect(G_OBJECT(combo), "changed", NULL);

	pthread_mutex_init(&hubLock, NULL);
}

PublicHubs::~PublicHubs() {
	HubManager::getInstance()->removeListener(this);
	pthread_mutex_destroy(&hubLock);
	gtk_widget_destroy(GTK_WIDGET(configureDialog));
}

void PublicHubs::downloadList_client() {
	HubManager *man = HubManager::getInstance();
	
	pthread_mutex_lock(&hubLock);
	hubs = man->getPublicHubs();
	pthread_mutex_unlock(&hubLock);
	
	if (hubs.empty()) man->refresh();
	man->save();

	Func0<PublicHubs> *func = new Func0<PublicHubs>(this, &PublicHubs::updateList_gui);
	WulforManager::get()->dispatchGuiFunc(func);
}

void PublicHubs::updateList_gui() {
	HubEntry::List::const_iterator i;
	GtkTreeIter iter;
	ostringstream hubStream, userStream;
	int numHubs = 0, numUsers = 0;

	gtk_list_store_clear(hubStore);

	pthread_mutex_lock(&hubLock);
	for (i = hubs.begin(); i != hubs.end(); i++) {
		if( filter.getPattern().empty() ||
			filter.match(i->getName()) ||
			filter.match(i->getDescription()) ||
			filter.match(i->getServer()) )
		{
			gtk_list_store_append(hubStore, &iter);
			gtk_list_store_set(hubStore, &iter, 
				COLUMN_NAME, i->getName().c_str(),
				COLUMN_DESC, i->getDescription().c_str(),
				COLUMN_USERS, i->getUsers(),
				COLUMN_ADDRESS, i->getServer().c_str(),
				-1);
				
			numUsers += i->getUsers();
			numHubs++;
		}
	}
	pthread_mutex_unlock(&hubLock);
	
	hubStream << "Hubs: " << numHubs;
	userStream << "Users: " << numUsers;
	setStatus_gui(statusHubs, hubStream.str());
	setStatus_gui(statusUsers, userStream.str());
}

GtkWidget *PublicHubs::getWidget() {
	return mainBox;
}

gboolean PublicHubs::buttonEvent_gui(
	GtkWidget *widget, GdkEventButton *event, gpointer)
{
	static int oldButton, oldType;
	GtkTreeSelection *sel;
	
	if (event->type == GDK_BUTTON_PRESS || event->type == GDK_2BUTTON_PRESS) {
		oldType = event->type;
		oldButton = event->button;
		return FALSE;	
	}
	
	if (oldButton != event->button) return FALSE;

	sel = gtk_tree_view_get_selection(hubView);
	if (!gtk_tree_selection_get_selected(sel, NULL, NULL)) return FALSE;

	//single click right button
	if (event->button == 3 && oldType == GDK_BUTTON_PRESS) {
		gtk_menu_popup(menu, NULL, NULL, NULL, NULL, 3, event->time);
		gtk_widget_show_all(GTK_WIDGET(menu));
	}

	//double click left button
	if (event->button == 1 && oldType == GDK_2BUTTON_PRESS) {
		connect_gui(NULL, NULL);	
	}

	return FALSE;
}

void PublicHubs::addFav_gui(GtkMenuItem *i, gpointer d) {
	FavoriteHubEntry entry;
	char *name, *address, *description;
	GtkTreeSelection *selection;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection(hubView);
	if (!gtk_tree_selection_get_selected(selection, NULL, &iter)) return;
	gtk_tree_model_get(GTK_TREE_MODEL(hubStore), &iter, 
		COLUMN_NAME, &name,
		COLUMN_DESC, &description,
		COLUMN_ADDRESS, &address,
		-1);

	entry.setName(name);
	entry.setServer(address);
	entry.setDescription(description);
	entry.setNick(SETTING(NICK));
	entry.setPassword("");
	entry.setUserDescription(SETTING(DESCRIPTION));

	g_free(name);
	g_free(address);
	g_free(description);

	typedef Func1<PublicHubs, FavoriteHubEntry> F1;
	F1 *func = new F1(this, &PublicHubs::addFav_client, entry);
	WulforManager::get()->dispatchClientFunc(func);
}

void PublicHubs::addFav_client(FavoriteHubEntry entry) {
	HubManager::getInstance()->addFavorite(entry);
}

void PublicHubs::filterHubs_gui(GtkWidget *w, gpointer d) {
	filter = gtk_entry_get_text(filterEntry);
	updateList_gui();
}

void PublicHubs::connect_gui(GtkWidget *w, gpointer d) {
	string address;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	char *text;
	
	selection = gtk_tree_view_get_selection(hubView);
	if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
		gtk_tree_model_get(GTK_TREE_MODEL(hubStore), &iter, COLUMN_ADDRESS, &text, -1);
		address = text;
		WulforManager::get()->addHub_gui(address);
	}
}

void PublicHubs::refresh_gui(GtkWidget *widget, gpointer data) {
	int pos = gtk_combo_box_get_active(combo);
	HubManager::getInstance()->setHubList(pos);
	Func0<PublicHubs> *f = new Func0<PublicHubs>(this, &PublicHubs::refresh_client);
	WulforManager::get()->dispatchClientFunc(f);
}

void PublicHubs::configure_gui(GtkWidget *widget, gpointer data) {
	int ret;
	GtkTreeIter it1, it2;
	char *tmp;

	gtk_list_store_clear(listsStore);
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(comboStore), &it1);
	while (gtk_list_store_iter_is_valid(comboStore, &it1)) {
		gtk_tree_model_get(GTK_TREE_MODEL(comboStore), &it1, 0, &tmp, -1);
		gtk_tree_model_iter_next(GTK_TREE_MODEL(comboStore), &it1);
		gtk_list_store_append(listsStore, &it2);
		gtk_list_store_set(listsStore, &it2, 0, tmp, -1);
	}
	
	gtk_widget_show_all(GTK_WIDGET(configureDialog));
	ret = gtk_dialog_run(configureDialog);
	gtk_widget_hide(GTK_WIDGET(configureDialog));

	if (ret == GTK_RESPONSE_OK) {
		string lists;
		int pos = HubManager::getInstance()->getSelectedHubList();

		gtk_list_store_clear(comboStore);
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listsStore), &it1);
		while (gtk_list_store_iter_is_valid(listsStore, &it1)) {
			gtk_tree_model_get(GTK_TREE_MODEL(listsStore), &it1, 0, &tmp, -1);
			gtk_tree_model_iter_next(GTK_TREE_MODEL(listsStore), &it1);
			gtk_list_store_append(comboStore, &it2);
			gtk_list_store_set(comboStore, &it2, 0, tmp, -1);

			lists += string(tmp) + ";";
			g_free(tmp);
		}

		if (!lists.empty()) lists.erase(lists.size() - 1);
		SettingsManager::getInstance()->set(
			SettingsManager::HUBLIST_SERVERS, lists);
		gtk_combo_box_set_active(combo, pos);
	}
}

void PublicHubs::add_gui(GtkWidget *widget, gpointer data) {
	GtkTreeIter it;
	GtkTreePath *p;
	char *s;
		
	gtk_list_store_append(listsStore, &it);
	gtk_list_store_set(listsStore, &it, 0, "New list", -1);
	s = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(listsStore), &it);
	p = gtk_tree_path_new_from_string(s);
	gtk_tree_view_set_cursor(listsView, p, 
		gtk_tree_view_get_column(listsView, 0), TRUE);
	
	g_free(s);
	gtk_tree_path_free(p);
}

void PublicHubs::moveUp_gui(GtkWidget *widget, gpointer data) {
	GtkTreeSelection *sel = gtk_tree_view_get_selection(listsView);
	GtkTreeIter cur, prev, next;
	
	if (!gtk_tree_selection_get_selected(sel, NULL, &cur)) return;

	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listsStore), &next);
	while (gtk_list_store_iter_is_valid(listsStore, &next)) {
		prev = next;
		gtk_tree_model_iter_next(GTK_TREE_MODEL(listsStore), &next);
		
		if (next.stamp == cur.stamp &&
			next.user_data == cur.user_data &&
			next.user_data2 == cur.user_data2 &&
			next.user_data3 == cur.user_data3)
		{
			gtk_list_store_swap(listsStore, &cur, &prev);
			return;
		}
	}
}

void PublicHubs::moveDown_gui(GtkWidget *widget, gpointer data) {
	GtkTreeSelection *sel = gtk_tree_view_get_selection(listsView);
	GtkTreeIter it, next;

	if (!gtk_tree_selection_get_selected(sel, NULL, &next)) return;
	it = next;
	if (!gtk_tree_model_iter_next(GTK_TREE_MODEL(listsStore), &next)) return;
	gtk_list_store_swap(listsStore, &it, &next);
}

void PublicHubs::remove_gui(GtkWidget *widget, gpointer data) {
	GtkTreeSelection *sel = gtk_tree_view_get_selection(listsView);
	GtkTreeIter cur;
	if (!gtk_tree_selection_get_selected(sel, NULL, &cur)) return;
	gtk_list_store_remove(listsStore, &cur);
}

void PublicHubs::cellEdited_gui(GtkCellRendererText *cell, 
	char *path, char *text, gpointer data)
{
	GtkTreeIter it;
	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(listsStore), &it, path);
	gtk_list_store_set(listsStore, &it, 0, text, -1);
}

void PublicHubs::setStatus_gui(GtkStatusbar *status, string text) {
	gtk_statusbar_pop(status, 0);
	gtk_statusbar_push(status, 0, text.c_str());
}

void PublicHubs::refresh_client() {
	HubManager::getInstance()->refresh();
}

void PublicHubs::on(HubManagerListener::DownloadStarting, 
	const string &file) throw()
{
	string msg = "Download starting: " + file;
	typedef Func2<PublicHubs, GtkStatusbar*, string> Func;
	Func *func = new Func(this, &PublicHubs::setStatus_gui, statusMain, msg);
	WulforManager::get()->dispatchGuiFunc(func);
}
	
void PublicHubs::on(HubManagerListener::DownloadFailed, 
	const string &file) throw()
{
	string msg = "Download failed: " + file;
	typedef Func2<PublicHubs, GtkStatusbar*, string> Func;
	Func *func = new Func(this, &PublicHubs::setStatus_gui, statusMain, msg);
	WulforManager::get()->dispatchGuiFunc(func);
}

void PublicHubs::on(HubManagerListener::DownloadFinished, 
	const string &file) throw()
{
	string msg = "Download finished: " + file;
	typedef Func2<PublicHubs, GtkStatusbar*, string> Func;
	Func *f2 = new Func(this, &PublicHubs::setStatus_gui, statusMain, msg);
	WulforManager::get()->dispatchGuiFunc(f2);

	pthread_mutex_lock(&hubLock);
	hubs = HubManager::getInstance()->getPublicHubs();
	pthread_mutex_unlock(&hubLock);

	Func0<PublicHubs> *f0 = new Func0<PublicHubs>(this, &PublicHubs::updateList_gui);
	WulforManager::get()->dispatchGuiFunc(f0);
}
