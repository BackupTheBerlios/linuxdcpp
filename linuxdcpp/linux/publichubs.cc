#include "publichubs.hh"
#include "wulformanager.hh"
#include <iostream>

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
	BookEntry(WulforManager::PUBLIC_HUBS, "", "Public Hubs", closeCallback)
{
	HubManager::getInstance()->addListener(this);
	
	string file = WulforManager::get()->getPath() + "/glade/publichubs.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "publicHubsWindow");
	mainBox = glade_xml_get_widget(xml, "publicHubsBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	filterEntry = GTK_ENTRY(glade_xml_get_widget(xml, "filterEntry"));
	connectEntry = GTK_ENTRY(glade_xml_get_widget(xml, "connectEntry"));
	//filterButton = GTK_BUTTON(glade_xml_get_widget(xml, "filterButton"));
	//connectButton = GTK_BUTTON(glade_xml_get_widget(xml, "connectButton"));
	hubView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "hubView"));
	
	GObject *o;
	o = G_OBJECT(glade_xml_get_widget(xml, "filterButton"));
    g_signal_connect(o, "clicked", G_CALLBACK(filter_callback), (gpointer)this);
	o = G_OBJECT(glade_xml_get_widget(xml, "connectButton"));
    g_signal_connect(o, "clicked", G_CALLBACK(connect_callback), (gpointer)this);
}

PublicHubs::~PublicHubs() {
	HubManager::getInstance()->removeListener(this);
}

GtkWidget *PublicHubs::getWidget() {
	return mainBox;
}

void PublicHubs::filterHubs_gui() {

}

void PublicHubs::connect_gui() {


}
