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

void MainWindow::quit_callback(GtkWidget *widget, gpointer data) {
	MainWindow *m = (MainWindow*)data;
	gtk_widget_show_all (GTK_WIDGET (m->exitDialog));
	gint response = gtk_dialog_run (m->exitDialog);
	gtk_widget_hide (GTK_WIDGET (m->exitDialog));
	if (response == GTK_RESPONSE_OK)	
		gtk_main_quit();
}

void MainWindow::exit_callback(GtkWidget *widget, gpointer data) {
	gtk_main_quit();
}

gboolean MainWindow::delete_callback(GtkWidget *widget, GdkEvent *event, gpointer data) {
	MainWindow *m = (MainWindow*)data;
	gtk_widget_show_all (GTK_WIDGET (m->exitDialog));
	gint response = gtk_dialog_run (m->exitDialog);
	gtk_widget_hide (GTK_WIDGET (m->exitDialog));
	if (response == GTK_RESPONSE_OK)
		return FALSE;
	else
		return TRUE;
}

void MainWindow::pubHubs_callback(GtkWidget *widget, gpointer data) {
	//no need to dispatch, already in gui thread
	WulforManager::get()->addPublicHubs_gui();
}

void MainWindow::dlQueue_callback(GtkWidget *widget, gpointer data) {
	WulforManager::get()->addDownloadQueue_gui();
}

void MainWindow::favHubs_callback(GtkWidget *widget, gpointer data) {
	WulforManager::get()->addFavoriteHubs_gui();
}

void MainWindow::search_callback(GtkWidget *widget, gpointer data) {
	WulforManager::get()->addSearch_gui();
}

void MainWindow::settings_callback(GtkWidget *widget, gpointer data) {
	Settings *s = WulforManager::get()->openSettingsDialog_gui();

	short lastPort = (short)SETTING(IN_PORT);
	int lastConn = SETTING(CONNECTION_TYPE);	
	
	if (gtk_dialog_run(GTK_DIALOG (s->getDialog ())) == GTK_RESPONSE_OK)
	{
		s->saveSettings_client ();
		SettingsManager::getInstance()->save();
		if (SETTING(CONNECTION_TYPE) != lastConn || SETTING(IN_PORT) != lastPort) 
		{
			Selecter::quit();
			((MainWindow*)data)->startSocket_client();
		}		
	}

	gtk_widget_hide (s->getDialog());
}

void MainWindow::hash_callback (GtkWidget *widget, gpointer data) {
	/*Hash *h = WulforManager::get ()->openHashDialog_gui();
	
	if (gtk_dialog_run (GTK_DIALOG (h->getDialog ())) == GTK_RESPONSE_OK)
		cout << "yay" << endl;
		
	gtk_widget_hide (h->getDialog ());*/
}

MainWindow::MainWindow():
	WIDTH_TYPE(20), 
	WIDTH_USER(150), 
	WIDTH_STATUS(250), 
	WIDTH_TIMELEFT(75),
	WIDTH_SPEED(175), 
	WIDTH_FILENAME(200), 
	WIDTH_SIZE(175), 
	WIDTH_PATH(200)
{
	QueueManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
	ConnectionManager::getInstance()->addListener(this);
	
	startSocket_client();
}

MainWindow::~MainWindow() {
	QueueManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);
	ConnectionManager::getInstance()->removeListener(this);

	//this makes sure the pixmaps are freed (using gtk:s ref counting)
	g_object_unref(G_OBJECT(uploadPic));
	g_object_unref(G_OBJECT(downloadPic));
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

	pubHubsButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "publicHubs"));
	searchButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "search"));
	settingsButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "settings"));
	hashButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "hash"));
	queueButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "queue"));
	favHubsButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "favHubs"));
	quitButton = GTK_TOOL_BUTTON(glade_xml_get_widget(xml, "quit"));

	window = GTK_WINDOW(glade_xml_get_widget(xml, "mainWindow"));
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (exit_callback), (gpointer)this);
	g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (delete_callback), (gpointer)this);
	exitDialog = GTK_DIALOG (glade_xml_get_widget (xml, "exitDialog"));
	book = GTK_NOTEBOOK(glade_xml_get_widget(xml, "book"));
	transferView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "transfers"));

	transferStore = gtk_list_store_new(9, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, 
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(transferView, GTK_TREE_MODEL(transferStore));
	transferSel = gtk_tree_view_get_selection(transferView);
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

	file = WulforManager::get()->getPath() + "/pixmaps/queue.png";
	gtk_tool_button_set_icon_widget(queueButton, 
		gtk_image_new_from_file(file.c_str()));
	file = WulforManager::get()->getPath() + "/pixmaps/favhubs.png";
	gtk_tool_button_set_icon_widget(favHubsButton, 
		gtk_image_new_from_file(file.c_str()));
		
	gtk_widget_show_all(GTK_WIDGET(window));

   	g_signal_connect(G_OBJECT(pubHubsButton), 
		"clicked", G_CALLBACK(pubHubs_callback), (gpointer)this);
   	g_signal_connect(G_OBJECT (queueButton), 
		"clicked", G_CALLBACK(dlQueue_callback), (gpointer)this);
	g_signal_connect(G_OBJECT (favHubsButton), 
		"clicked", G_CALLBACK(favHubs_callback), (gpointer)this);
	g_signal_connect(G_OBJECT (settingsButton),
		"clicked", G_CALLBACK(settings_callback), (gpointer)this);
	g_signal_connect(G_OBJECT (searchButton),
		"clicked", G_CALLBACK(search_callback), (gpointer)this);
	g_signal_connect(G_OBJECT (hashButton),
		"clicked", G_CALLBACK(hash_callback), (gpointer)this);    	
	g_signal_connect(G_OBJECT(quitButton), 
		"clicked", G_CALLBACK(quit_callback), (gpointer)this);

	GtkWidget *dummy;
	GtkRequisition req;
	dummy = gtk_statusbar_new();
	gtk_widget_size_request(dummy, &req);
	gtk_widget_destroy(dummy);
	emptyStatusWidth = req.width;
	
	gtk_statusbar_push(mainStatus, 0, "Welcome to Wulfor - Reloaded");
}

GtkWindow *MainWindow::getWindow() {
	return window;
}

void MainWindow::autoConnect_client() {
	FavoriteHubEntry::List &l = HubManager::getInstance()->getFavoriteHubs();
	FavoriteHubEntry::List::const_iterator it;
	
	for (it = l.begin(); it != l.end(); it++) {
		FavoriteHubEntry *entry = *it;
		if (entry->getConnect())
			if (!entry->getNick().empty () || !SETTING(NICK).empty()) {
				string nick;
			
				if (entry->getNick().empty())
					nick =  SETTING(NICK);
				else
					nick = entry->getNick();
			
				WulforManager::get()->addHub_gui(
					entry->getServer(),
					nick,
					entry->getUserDescription(),
					entry->getPassword());
			}
	}
}

void MainWindow::startSocket_client() {
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();

	if(SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE)
		return;

	short lastPort = (short)SETTING(IN_PORT);
	short firstPort = lastPort;

	while (true) {
		try {
			ConnectionManager::getInstance()->setPort(lastPort);
			Selecter::WSAASyncSelect(
				ConnectionManager::getInstance()->getServerSocket());

			SearchManager::getInstance()->setPort(lastPort);
			break;
		} catch (const Exception &e) {
			cout << "startSocket caught " << e.getError() << endl;
			short newPort = (short)((lastPort == 32000) ? 1025 : lastPort + 1);
			SettingsManager::getInstance()->
				setDefault(SettingsManager::IN_PORT, newPort);
				
			if (SETTING(IN_PORT) == lastPort || (firstPort == newPort)) {
				// Changing default didn't change port, a fixed port must be in
				// use...(or we tried all ports)
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

//From QueueManagerListener
void MainWindow::on(QueueManagerListener::Finished, QueueItem *item) throw() {
	if (!item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST)) 
		return;

	User::Ptr user = item->getCurrent()->getUser();
	string searchString = item->getSearchString();
	string listName = item->getListName();

	if (item->isSet(QueueItem::FLAG_CLIENT_VIEW) && 
		item->isSet(QueueItem::FLAG_USER_LIST))
	{
		typedef Func3<MainWindow, User::Ptr, string, string> F3;
		F3 *func = new F3(this, &MainWindow::addShareBrowser_gui, 
			user, searchString, listName);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::addShareBrowser_gui(User::Ptr user, string searchString, string listName) {
	ShareBrowser *browser;
	browser = WulforManager::get()->addShareBrowser_gui(user, listName);
	browser->setPosition_gui(searchString);
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
		
void MainWindow::setStatus_gui(GtkStatusbar *status, std::string text) {
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

void MainWindow::setStats_gui(std::string hub, std::string slot, 
	std::string dTot, std::string uTot, std::string dl, std::string ul)
{
	setStatus_gui(hubStatus, hub);
	setStatus_gui(slotStatus, slot);
	setStatus_gui(dTotStatus, dTot);
	setStatus_gui(uTotStatus, uTot);
	setStatus_gui(ulStatus, ul);
	setStatus_gui(dlStatus, dl);
}

void MainWindow::updateTransfer_gui(string id, connection_t type, string user, 
	string status, string time, string speed, string file, string size, string path)
{
	GtkTreeIter iter;
	findId_gui(id, &iter);
	
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
	if (user != "") {
		gtk_list_store_set(transferStore, &iter, COLUMN_USER, user.c_str(), -1);
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

void MainWindow::removeTransfer_gui(string id) {
	GtkTreeIter iter;
	findId_gui(id, &iter);
	
	if (gtk_list_store_iter_is_valid(transferStore, &iter)) {
		gtk_list_store_remove(transferStore, &iter);
	}
}

void MainWindow::findId_gui(string id, GtkTreeIter *iter) {
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(transferStore), iter);

	while (gtk_list_store_iter_is_valid(transferStore, iter)) {
		char *t;
		string text;
		gtk_tree_model_get(GTK_TREE_MODEL(transferStore), iter, 
			COLUMN_ID, &t, -1);
		text = t;
		delete[] t;

		if (id == text) return;

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
				return;
			}
		}

		gtk_tree_model_iter_next(GTK_TREE_MODEL(transferStore), iter);
	}
}

string MainWindow::getId_client(ConnectionQueueItem *item) {
	string ret = item->getUser()->getFullNick();

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

string MainWindow::getId_client(Transfer *t) {
	assert (t->getUserConnection());
	assert (t->getUserConnection()->getCQI());

	return getId_client(t->getUserConnection()->getCQI());
}

void MainWindow::transferComplete_client(Transfer *t) {
	string status, id = getId_client(t);

	if (t->getUserConnection()->isSet(UserConnection::FLAG_UPLOAD)) {
		status = "Upload finished, idle...";
	} else {
		status = "Download finished, idle...";
	}

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, "", status,
		"Done", " ", "", "", "");
	WulforManager::get()->dispatchGuiFunc(func);
}

//From Connection manager
void MainWindow::on(ConnectionManagerListener::Added, ConnectionQueueItem *item) throw() {
	string status = "Connecting...";
	string user = item->getUser()->getNick();
	string id = getId_client(item);

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, 
		user, status, "", "", "", "", "");
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::Removed, ConnectionQueueItem *item) throw() {
	string id = getId_client(item);

	typedef Func1 <MainWindow, string> F1;
	F1 *func = new F1(this, &MainWindow::removeTransfer_gui, id);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::Failed, ConnectionQueueItem *item, const string &reason) throw() {
	string status = reason;
	string id = getId_client(item);

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, "", status,
		"", "", "", "", "");
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem *item) throw() {
	string status;
	string id = getId_client(item);

	if (item->getState() == ConnectionQueueItem::CONNECTING) {
		status = "Connecting...";
	} else {
		status = "Waiting to retry...";
	}

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, "", status,
		"", "", "", "", "");
	WulforManager::get()->dispatchGuiFunc(func);
}

//From Download manager
void MainWindow::on(DownloadManagerListener::Starting, Download *dl) throw() {
	string status, size, path, file;
	string id = getId_client(dl);

	size = Util::formatBytes(dl->getSize());
	file = Util::getFileName(dl->getTarget());
	path = Util::getFilePath(dl->getTarget());
	status = "Download starting...";

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_DL, "", status,
		"", "", file, size, path);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(DownloadManagerListener::Tick, const Download::List &list) throw() {
	string id, status, timeLeft, speed;
	Download::List::const_iterator it;
	
	for (it = list.begin(); it != list.end(); it++) {
		Download* dl = *it;
		ostringstream stream;

		id = getId_client(dl); 

		string bytes = Util::formatBytes(dl->getPos());
		double percent = (double)(dl->getPos() * 100.0) / dl->getSize();
		string time = Util::formatSeconds((GET_TICK() - dl->getStart()) / 1000);
		stream << setiosflags(ios::fixed) << setprecision(1);
		stream << "Downloaded " << bytes << " (" << percent << "%) in " << time;

		timeLeft = Util::formatSeconds(dl->getSecondsLeft());
		speed = Util::formatBytes(dl->getRunningAverage()) + "/s";

		if (dl->isSet(Download::FLAG_ZDOWNLOAD)) {
			status = "* " + stream.str();
		} else {
			status = stream.str();
		}

		UFunc *func;
		func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, "", 
			status,	timeLeft, speed, "", "", "");
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::on(DownloadManagerListener::Complete, Download *dl) throw() {
	transferComplete_client(dl);
}

void MainWindow::on(DownloadManagerListener::Failed, Download *dl, const string &reason) throw() {
	string status, size, file, path;
	string id = getId_client(dl); 

	status = reason;
	size = Util::formatBytes(dl->getSize());
	file = Util::getFileName(dl->getTarget());
	path = Util::getFilePath(dl->getTarget());

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, "", 
		status,	"", "", file, size, path);
	WulforManager::get()->dispatchGuiFunc(func);
}

//From Upload manager
void MainWindow::on(UploadManagerListener::Starting, Upload *ul) throw() {
	string status, size, path, file;
	string id = getId_client(ul);

	size = Util::formatBytes(ul->getSize());
	file = Util::getFileName(ul->getFileName());
	path = Util::getFilePath(ul->getFileName());
	status = "Upload starting...";

	UFunc *func;
	func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_UL, "", status,
		"", "", file, size, path);
	WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(UploadManagerListener::Tick, const Upload::List &list) throw() {
	string id, status, timeLeft, speed;
	Upload::List::const_iterator it;
	
	for (it = list.begin(); it != list.end(); it++) {
		Upload* ul = *it;
		ostringstream stream;

		id = getId_client(ul); 

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

		UFunc *func;
		func = new UFunc(this, &MainWindow::updateTransfer_gui, id, CONNECTION_NA, "", 
			status,	timeLeft, speed, "", "", "");
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void MainWindow::on(UploadManagerListener::Complete, Upload *ul) throw() {
	transferComplete_client(ul);
}
