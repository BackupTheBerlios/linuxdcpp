#include "publichubs.hh"
#include "wulformanager.hh"
#include <iostream>
#include <sstream>

using namespace std;

void PublicHubs::filter_callback(GtkWidget *widget, gpointer data) {
	PublicHubs *pubHubs = (PublicHubs *)data;
	pubHubs->filterHubs_gui();
}

void PublicHubs::connect_callback(GtkWidget *widget, gpointer data) {
	PublicHubs *pubHubs = (PublicHubs *)data;
	pubHubs->connect_gui();
}

PublicHubs::PublicHubs(GCallback closeCallback):
	BookEntry(WulforManager::PUBLIC_HUBS, "", "Public Hubs", closeCallback),
	hubs(0),
	filter(""),
	WIDTH_NAME(200),
	WIDTH_DESC(290),
	WIDTH_USERS(50),
	WIDTH_ADDRESS(100)
{
	HubManager::getInstance()->addListener(this);
	if (HubManager::getInstance()->isDownloading())
		setStatus_gui(STATUS_MAIN, "Downloading hub list");
	
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

	//name column
	nameColumn = gtk_tree_view_column_new_with_attributes(
		"Name", gtk_cell_renderer_text_new(), "text", COLUMN_NAME, NULL);
	gtk_tree_view_column_set_sizing(nameColumn, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(nameColumn, WIDTH_NAME);
	gtk_tree_view_column_set_resizable(nameColumn, TRUE);
	gtk_tree_view_insert_column(hubView, nameColumn, COLUMN_NAME);

	//Description column
	descColumn = gtk_tree_view_column_new_with_attributes(
		"Description", gtk_cell_renderer_text_new(), "text", COLUMN_DESC, NULL);
	gtk_tree_view_column_set_sizing(descColumn, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(descColumn, WIDTH_DESC);
	gtk_tree_view_column_set_resizable(descColumn, TRUE);
	gtk_tree_view_insert_column(hubView, descColumn, COLUMN_DESC);

	//Users column
	usersColumn = gtk_tree_view_column_new_with_attributes(
		"Users", gtk_cell_renderer_text_new(), "text", COLUMN_USERS, NULL);
	gtk_tree_view_column_set_sizing(usersColumn, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(usersColumn, WIDTH_USERS);
	gtk_tree_view_column_set_resizable(usersColumn, TRUE);
	gtk_tree_view_insert_column(hubView, usersColumn, COLUMN_USERS);

	//Address column
	addressColumn = gtk_tree_view_column_new_with_attributes(
		"Address", gtk_cell_renderer_text_new(), "text", COLUMN_ADDRESS, NULL);
	gtk_tree_view_column_set_sizing(addressColumn, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(addressColumn, WIDTH_ADDRESS);
	gtk_tree_view_column_set_resizable(addressColumn, TRUE);
	gtk_tree_view_insert_column(hubView, addressColumn, COLUMN_ADDRESS);
	
	GObject *o;
	o = G_OBJECT(glade_xml_get_widget(xml, "filterButton"));
    g_signal_connect(o, "clicked", G_CALLBACK(filter_callback), (gpointer)this);
	o = G_OBJECT(glade_xml_get_widget(xml, "connectButton"));
    g_signal_connect(o, "clicked", G_CALLBACK(connect_callback), (gpointer)this);
	
	pthread_mutex_init(&hubLock, NULL);
	Func0<PublicHubs> *func = new Func0<PublicHubs>(this, &PublicHubs::downloadList_client);
	WulforManager::get()->dispatchClientFunc(func);
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
	setStatus_gui(STATUS_HUBS, hubStream.str());
	setStatus_gui(STATUS_USERS, userStream.str());
}

PublicHubs::~PublicHubs() {
	HubManager::getInstance()->removeListener(this);
	pthread_mutex_destroy(&hubLock);
}

GtkWidget *PublicHubs::getWidget() {
	return mainBox;
}

void PublicHubs::filterHubs_gui() {
	filter = gtk_entry_get_text(filterEntry);
	updateList_gui();
}

void PublicHubs::connect_gui() {


}

void PublicHubs::setStatus_gui(int status, string text) {
	GtkStatusbar *bar;

	switch (status) {
		case STATUS_MAIN:
			bar = statusMain;
			break;
		case STATUS_USERS:
			bar = statusUsers;
			break;
		case STATUS_HUBS:
			bar = statusHubs;
			break;
	}

	gtk_statusbar_pop(bar, 0);
	gtk_statusbar_push(bar, 0, text.c_str());
}

void PublicHubs::on(HubManagerListener::DownloadStarting, 
	const string &file) throw()
{
	string msg = "Download starting: " + file;
	typedef Func2<PublicHubs, int, string> Func;
	Func *func = new Func(this, &PublicHubs::setStatus_gui, STATUS_MAIN, msg);
	WulforManager::get()->dispatchGuiFunc(func);
}
	
void PublicHubs::on(HubManagerListener::DownloadFailed, 
	const string &file) throw()
{
	string msg = "Download failed: " + file;
	typedef Func2<PublicHubs, int, string> Func;
	Func *func = new Func(this, &PublicHubs::setStatus_gui, STATUS_MAIN, msg);
	WulforManager::get()->dispatchGuiFunc(func);
}

void PublicHubs::on(HubManagerListener::DownloadFinished, 
	const string &file) throw()
{
	string msg = "Download finished: " + file;
	typedef Func2<PublicHubs, int, string> Func;
	Func *f2 = new Func(this, &PublicHubs::setStatus_gui, STATUS_MAIN, msg);
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
	cout << "Added favourite" << endl;
}

void PublicHubs::on(HubManagerListener::FavoriteRemoved, 
	const FavoriteHubEntry *fav) throw()
{
	cout << "Removed favourite" << endl;
}

void PublicHubs::on(HubManagerListener::UserAdded, 
	const User::Ptr &user) throw()
{
	cout << "Added user" << endl;
}

void PublicHubs::on(HubManagerListener::UserRemoved, 
	const User::Ptr &user) throw()
{	
	cout << "Removed user" << endl;
}
