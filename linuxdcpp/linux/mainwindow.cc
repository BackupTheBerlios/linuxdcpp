#include "mainwindow.hh"
#include "wulformanager.hh"
#include "selecter.hh"

#include <client/Socket.h>
#include <client/Client.h>
#include <client/SettingsManager.h>
#include <client/SearchManager.h>
#include <client/ConnectionManager.h>
#include <client/Exception.h>
#include <client/DownloadManager.h>
#include <client/UploadManager.h>

#include <iostream>

using namespace std;


void MainWindow::quit_callback(GtkWidget *widget, gpointer data) {
	gtk_main_quit();
}

void MainWindow::pubHubs_callback(GtkWidget *widget, gpointer data) {
	//no need to dispatch, already in gui thread
	WulforManager::get()->addPublicHubs_gui();
}

MainWindow::MainWindow() {
	QueueManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);
}

MainWindow::~MainWindow() {
	QueueManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
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

	window = GTK_WINDOW(glade_xml_get_widget(xml, "mainWindow"));
	book = GTK_NOTEBOOK(glade_xml_get_widget(xml, "book"));
	transfers = GTK_TREE_VIEW(glade_xml_get_widget(xml, "transfers"));
	
	gtk_notebook_remove_page(book, -1);

	GObject *o;
	o = G_OBJECT(glade_xml_get_widget(xml, "publicHubs"));
    g_signal_connect(o, "clicked", G_CALLBACK(pubHubs_callback), (gpointer)this);
	o = G_OBJECT(glade_xml_get_widget(xml, "quit"));
    g_signal_connect(o, "clicked", G_CALLBACK(quit_callback), (gpointer)this);
	
	gtk_statusbar_push(mainStatus, 0, "Welcome to Wulfor - Reloaded");
}

GtkWindow *MainWindow::getWindow() {
	return window;
}

void MainWindow::startSocket() {
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();
	if(SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE)
	{
		return;
	}	

	short lastPort = (short)SETTING(IN_PORT);
	short firstPort = lastPort;

	while(true) {
		try {
			ConnectionManager::getInstance()->setPort(lastPort);
			Selecter::WSAASyncSelect(
				ConnectionManager::getInstance()->getServerSocket());

			SearchManager::getInstance()->setPort(lastPort);
			break;
		} catch(const Exception &e) {
			cout << "startSocket caught " << e.getError() << endl;
			short newPort = (short)((lastPort == 32000) ? 1025 : lastPort + 1);
			SettingsManager::getInstance()->
				setDefault(SettingsManager::IN_PORT, newPort);
				
			if(SETTING(IN_PORT) == lastPort || (firstPort == newPort)) {
				// Changing default didn't change port, a fixed port must be in
				// use...(or we tried all ports)
				//cout << STRING(PORT_IS_BUSY) << SETTING(IN_PORT) << endl;
				cout << "Port is busy " << SETTING(IN_PORT) << endl;
				break;
			}
			lastPort = newPort;
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

//QueueManagerListener
void MainWindow::on(QueueManagerListener::Finished, QueueItem *item) throw() {



}

void MainWindow::addPage_gui(GtkWidget *page, GtkWidget *label, bool raise) {
	gtk_notebook_append_page(book, page, label);
	if (raise) gtk_notebook_set_current_page(book, -1);
}

void MainWindow::removePage_gui(GtkWidget *page) {
	int i, pageNum = -1;

	for (i=0; i<gtk_notebook_get_n_pages(book); i++)
		if (page == gtk_notebook_get_nth_page(book, i)) pageNum = i;
		
	assert(pageNum != -1);
	gtk_notebook_remove_page(book, pageNum);
}

void MainWindow::raisePage_gui(GtkWidget *page) {
	int i, pageNum = -1;

	for (i=0; i<gtk_notebook_get_n_pages(book); i++)
		if (page == gtk_notebook_get_nth_page(book, i)) pageNum = i;
		
	assert(pageNum != -1);
	gtk_notebook_set_current_page(book, pageNum);
}
		
void MainWindow::setStatus_gui(std::string status) {
	gtk_statusbar_pop(mainStatus, 0);
	gtk_statusbar_push(mainStatus, 0, status.c_str());
}

void MainWindow::setStats_gui(std::string hub, std::string slot, 
	std::string dTot, std::string uTot, std::string dl, std::string ul)
{
	gtk_statusbar_pop(hubStatus, 0);
	gtk_statusbar_push(hubStatus, 0, hub.c_str());
	gtk_statusbar_pop(slotStatus, 0);
	gtk_statusbar_push(slotStatus, 0, slot.c_str());
	gtk_statusbar_pop(dTotStatus, 0);
	gtk_statusbar_push(dTotStatus, 0, dTot.c_str());
	gtk_statusbar_pop(uTotStatus, 0);
	gtk_statusbar_push(uTotStatus, 0, uTot.c_str());
	gtk_statusbar_pop(ulStatus, 0);
	gtk_statusbar_push(ulStatus, 0, ul.c_str());
	gtk_statusbar_pop(dlStatus, 0);
	gtk_statusbar_push(dlStatus, 0, dl.c_str());
}

