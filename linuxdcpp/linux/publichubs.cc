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
	hubs(0),
	filter(""),
	WIDTH_NAME(200),
	WIDTH_DESC(350),
	WIDTH_USERS(50),
	WIDTH_ADDRESS(100)
{
	HubManager::getInstance()->addListener(this);
	if (HubManager::getInstance()->isDownloading())
		setStatus_gui(statusMain, "Downloading hub list");
	
	string file = WulforManager::get()->getPath() + "/glade/publichubs.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "publicHubsWindow");
	mainBox = glade_xml_get_widget(xml, "publicHubsBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	filterEntry = GTK_ENTRY(glade_xml_get_widget(xml, "filterEntry"));
	connectEntry = GTK_ENTRY(glade_xml_get_widget(xml, "connectEntry"));
	hubView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "hubView"));
	
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

	GObject *o;
	o = G_OBJECT(glade_xml_get_widget(xml, "filterButton"));
	filterCallback.connect(o, "clicked", NULL);
	o = G_OBJECT(glade_xml_get_widget(xml, "connectButton"));
	connectCallback.connect(o, "clicked", NULL);
	
	pthread_mutex_init(&hubLock, NULL);
}

PublicHubs::~PublicHubs() {
	HubManager::getInstance()->removeListener(this);
	pthread_mutex_destroy(&hubLock);
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
	for(i = hubs.begin(); i != hubs.end(); i++) {
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

void PublicHubs::filterHubs_gui(GtkWidget *e, gpointer d) {
	filter = gtk_entry_get_text(filterEntry);
	updateList_gui();
}

void PublicHubs::connect_gui(GtkWidget *e, gpointer d) {
	string address = gtk_entry_get_text(connectEntry);
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	char *text;
	
	if (!address.empty()) {
		WulforManager::get()->addHub_gui(address);
		return;
	}
	
	selection = gtk_tree_view_get_selection(hubView);
	if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
		gtk_tree_model_get(GTK_TREE_MODEL(hubStore), &iter, COLUMN_ADDRESS, &text, -1);
		address = text;
		WulforManager::get()->addHub_gui(address);
	}
}

void PublicHubs::setStatus_gui(GtkStatusbar *status, string text) {
	gtk_statusbar_pop(status, 0);
	gtk_statusbar_push(status, 0, text.c_str());
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

void PublicHubs::on(HubManagerListener::FavoriteAdded, 
	const FavoriteHubEntry *fav) throw()
{
	//cout << "Added favourite" << endl;
}

void PublicHubs::on(HubManagerListener::FavoriteRemoved, 
	const FavoriteHubEntry *fav) throw()
{
	//cout << "Removed favourite" << endl;
}

void PublicHubs::on(HubManagerListener::UserAdded, 
	const User::Ptr &user) throw()
{
	//cout << "Added user" << endl;
}

void PublicHubs::on(HubManagerListener::UserRemoved, 
	const User::Ptr &user) throw()
{	
	//cout << "Removed user" << endl;
}
