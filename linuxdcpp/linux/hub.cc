/* 
* Copyright (C) 2004 Jens Oknelid, paskharen@spray.se
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

#include "hub.hh"
#include "wulformanager.hh"
#include "treeviewfactory.hh"

#include <iostream>
#include <sstream>

using namespace std;

Hub::Hub(std::string address, GCallback closeCallback):
	BookEntry(WulforManager::HUB, address, address, closeCallback),
	enterCallback(this, &Hub::sendMessage_gui),
	nickListCallback(this, &Hub::popupNickMenu_gui),
	browseCallback(this, &Hub::browseItemClicked_gui),
	msgCallback(this, &Hub::msgItemClicked_gui),
	WIDTH_ICON(20),
	WIDTH_NICK(100),
	WIDTH_SHARED(50)
{
	string file = WulforManager::get()->getPath() + "/glade/hub.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "hubWindow");
	mainBox = glade_xml_get_widget(xml, "hubBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	chatEntry = GTK_ENTRY(glade_xml_get_widget(xml, "chatEntry"));
	chatText = GTK_TEXT_VIEW(glade_xml_get_widget(xml, "chatText"));
	nickView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "nickView"));
	mainStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusMain"));
	usersStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusUsers"));
	sharedStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusShared"));
	
	nickStore = gtk_list_store_new(3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(nickView, GTK_TREE_MODEL(nickStore));
	nickSelection = gtk_tree_view_get_selection(nickView);
	TreeViewFactory factory(nickView);
	factory.addColumn_gui(COLUMN_ICON, "", TreeViewFactory::PIXBUF, WIDTH_ICON);
	factory.addColumn_gui(COLUMN_NICK, "Nick", TreeViewFactory::STRING, WIDTH_NICK);
	factory.addColumn_gui(COLUMN_SHARED, "Shared", TreeViewFactory::STRING, WIDTH_SHARED);

	chatBuffer = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(chatText, chatBuffer);

	nickMenu = GTK_MENU(gtk_menu_new());
	browseItem = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Browse files"));
	msgItem = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Private message"));
	gtk_menu_shell_append(GTK_MENU_SHELL(nickMenu), GTK_WIDGET(browseItem));
	gtk_menu_shell_append(GTK_MENU_SHELL(nickMenu), GTK_WIDGET(msgItem));

	pthread_mutex_init(&clientLock, NULL);
	client = NULL;

	enterCallback.connect(G_OBJECT(chatEntry), "activate", NULL);
	nickListCallback.connect_after(G_OBJECT(nickView), "button-release-event", NULL);
	browseCallback.connect(G_OBJECT(browseItem), "activate", NULL);
	msgCallback.connect(G_OBJECT(msgItem), "activate", NULL);
}

Hub::~Hub() {
	if (client) {
		client->removeListener(this);
		ClientManager::getInstance()->putClient(client);
	}
	pthread_mutex_destroy(&clientLock);
}

GtkWidget *Hub::getWidget() {
	return mainBox;
}

void Hub::connectClient_client(string address, string nick, string desc, string password) {
	pthread_mutex_lock(&clientLock);
	client = ClientManager::getInstance()->getClient(address);

	if (nick.empty()) client->setNick(SETTING(NICK));
	else client->setNick(nick);
	if (!desc.empty()) client->setDescription(desc);

	client->addListener(this);
	client->setPassword(password);
	client->connect();
	pthread_mutex_unlock(&clientLock);
}

void Hub::updateUser_client(User::Ptr user) {
	typedef Func3<Hub, string, int64_t, string> F3;
	F3 * func;
	string nick = user->getNick();
	int64_t shared = user->getBytesShared();

	string icon = WulforManager::get()->getPath() + "/pixmaps/";
	if (user->isSet(User::DCPLUSPLUS)) icon += "dc++";
	else icon += "normal";
	if (user->isSet(User::PASSIVE)) icon += "-fw";
	if (user->isSet(User::OP)) icon += "-op";
	icon += ".png";

	func = new F3(this, &Hub::updateUser_gui, nick, shared, icon);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::setStatus_gui(GtkStatusbar *status, std::string text) {
	gtk_statusbar_pop(status, 0);
	gtk_statusbar_push(status, 0, text.c_str());
}

void Hub::findUser_gui(string nick, GtkTreeIter *iter) {
	if (nicks.find(nick) == nicks.end()) {
		iter = NULL;
		return;
	}

	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(nickStore), iter);
	
	while (gtk_list_store_iter_is_valid(nickStore, iter)) {
		char *text;
		gtk_tree_model_get(GTK_TREE_MODEL(nickStore), iter, COLUMN_NICK, &text, -1);
		if (nick == text) return;
		gtk_tree_model_iter_next(GTK_TREE_MODEL(nickStore), iter);
	}
}

void Hub::updateUser_gui(string nick, int64_t shared, string iconFile) {
	GtkTreeIter iter;
	GdkPixbuf *icon;
	string file;
	ostringstream userStream;

	findUser_gui(nick, &iter);
	if (!gtk_list_store_iter_is_valid(nickStore, &iter)) {
		gtk_list_store_append(nickStore, &iter);
		nicks.insert(nick);
	}

	icon = gdk_pixbuf_new_from_file(iconFile.c_str(), NULL);
	gtk_list_store_set(nickStore, &iter, 
		COLUMN_ICON, icon,
		COLUMN_NICK, nick.c_str(),
		COLUMN_SHARED, 	Util::formatBytes(shared).c_str(),
		-1);
	g_object_unref(G_OBJECT(icon));
	
	pthread_mutex_lock(&clientLock);
	if (client) {
		userStream << client->getUserCount() << " User(s)";
		setStatus_gui(usersStatus, userStream.str());
		setStatus_gui(sharedStatus, Util::formatBytes(client->getAvailable()));
	}
	pthread_mutex_unlock(&clientLock);
}

void Hub::removeUser_gui(string nick) {
	GtkTreeIter iter;
	
	findUser_gui(nick, &iter);
	if (gtk_list_store_iter_is_valid(nickStore, &iter)) {
		gtk_list_store_remove(nickStore, &iter);
		nicks.erase(nick);
	}
}

void Hub::clearNickList_gui() {
	gtk_list_store_clear(nickStore);
	nicks.clear();
}

void Hub::getPassword_gui() {
	MainWindow *mainWin = WulforManager::get()->getMainWindow();
	GtkEntry *entry;
	string password;

	GtkWidget *dialog = gtk_dialog_new_with_buttons ("Enter password",
		mainWin->getWindow(),
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_OK,
		GTK_RESPONSE_ACCEPT,
		NULL);

	entry = GTK_ENTRY(gtk_entry_new());
	gtk_entry_set_visibility(entry, FALSE);
	gtk_container_add(GTK_CONTAINER(dialog), GTK_WIDGET(entry));

	gtk_dialog_run(GTK_DIALOG(dialog));
	password = gtk_entry_get_text(entry);

	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::setPassword_client, password);
	WulforManager::get()->dispatchClientFunc(func);
}

void Hub::addMessage_gui(string msg) {
	GtkTextIter iter;
	string text = "[" + Util::getShortTimeString() + "] " + msg + "\n";
	
	gtk_text_buffer_get_end_iter(chatBuffer, &iter);
	gtk_text_buffer_insert(chatBuffer, &iter, text.c_str(), text.size());
}

void Hub::addPrivateMessage_gui(User::Ptr user, std::string msg) {
	::PrivateMessage *privMsg = 
		WulforManager::get()->getPrivMsg_gui(user);
		
	if (!privMsg)
		privMsg = WulforManager::get()->addPrivMsg_gui(user);
	
	privMsg->addMessage_gui(msg);
}

void Hub::sendMessage_gui(GtkEntry *entry, gpointer data) {
	string text = gtk_entry_get_text(chatEntry);
	typedef Func1<Hub, string> F1;
	F1 *func;
	
	if (!text.empty()) {
		gtk_entry_set_text(chatEntry, "");
		func = new F1(this, &Hub::sendMessage_client, text);
		WulforManager::get()->dispatchClientFunc(func);		
	}
}

void Hub::popupNickMenu_gui(GtkWidget *, GdkEventButton *button, gpointer) {
	//only for right mouse button
	if (button->button != 3) return;
	//return if no nick is selected
	if (!gtk_tree_selection_get_selected(nickSelection, NULL, NULL)) return;

	gtk_menu_popup(nickMenu, NULL, NULL, NULL, NULL, 3, button->time);
	gtk_widget_show_all(GTK_WIDGET(nickMenu));
}

void Hub::browseItemClicked_gui(GtkMenuItem *, gpointer) {
	GtkTreeIter iter;
	char *text;
	gtk_tree_selection_get_selected(nickSelection, NULL, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(nickStore), &iter,
		COLUMN_NICK, &text,
		-1);
		
	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::getFileList_client, string(text));
	WulforManager::get()->dispatchClientFunc(func);
}

void Hub::msgItemClicked_gui(GtkMenuItem *, gpointer) {
	GtkTreeIter iter;
	char *text;
	gtk_tree_selection_get_selected(nickSelection, NULL, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(nickStore), &iter,
		COLUMN_NICK, &text,
		-1);

	pthread_mutex_lock(&clientLock);
	if (client) {
		User::Ptr user = ClientManager::getInstance()->getUser(text, client);
		WulforManager::get()->addPrivMsg_gui(user);
	}
	pthread_mutex_unlock(&clientLock);
}

void Hub::getFileList_client(string nick) {
	User::Ptr user = NULL;

	pthread_mutex_lock(&clientLock);
	if (client) {
		user = ClientManager::getInstance()->getUser(nick, client);
	}
	pthread_mutex_unlock(&clientLock);

	if (!user) return;

	try	{
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} catch (...) {
		string text = "Could get filelist from: " + nick;
		typedef Func2<Hub, GtkStatusbar *, string> F2;
		F2 *func = new F2(this, &Hub::setStatus_gui, mainStatus, text);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::sendMessage_client(string message) {
	pthread_mutex_lock(&clientLock);
	if (client) {
		client->getMe()->clientMessage(message);
	}
	pthread_mutex_unlock(&clientLock);
}

void Hub::setPassword_client(string password) {
	pthread_mutex_lock(&clientLock);
	if (client) {
		client->setPassword(password);
	}
	pthread_mutex_unlock(&clientLock);
}

void Hub::on(ClientListener::Connecting, Client *client) throw() {
	typedef Func2<Hub, GtkStatusbar*, string> F2;
	F2 *func;

	func = new F2(this, &Hub::setStatus_gui, mainStatus, "Connecting");
	WulforManager::get()->dispatchGuiFunc(func);
	func = new F2(this, &Hub::setStatus_gui, usersStatus, "0 Users");
	WulforManager::get()->dispatchGuiFunc(func);
	func = new F2(this, &Hub::setStatus_gui, sharedStatus, "0 B");
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::Connected, Client *client) throw() {
	Func2<Hub, GtkStatusbar*, string> *func = new Func2<Hub, GtkStatusbar*, string>
		(this, &Hub::setStatus_gui, mainStatus, "Connected");
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::BadPassword, Client *cl) throw() {
	Func2<Hub, GtkStatusbar*, string> *func = new Func2<Hub, GtkStatusbar*, string>
		(this, &Hub::setStatus_gui, mainStatus, "Sorry, wrong password");
	WulforManager::get()->dispatchGuiFunc(func);

	pthread_mutex_lock(&clientLock);
	client->setPassword("");
	pthread_mutex_unlock(&clientLock);
}

void Hub::on(ClientListener::UserUpdated, Client *client, 
	const User::Ptr &user) throw() 
{
	if (!user->isSet(User::HIDDEN)) {
		updateUser_client(user);
	}
}
	
void Hub::on(ClientListener::UsersUpdated, 
	Client *client, const User::List &list) throw()
{
	User::List::const_iterator it;
	for (it = list.begin (); it != list.begin (); it++)	{
		if (!(*it)->isSet(User::HIDDEN)) {
			updateUser_client(*it);
		}
	}
}

void Hub::on(ClientListener::UserRemoved, 
	Client *cl, const User::Ptr &user) throw()
{
	::PrivateMessage *privMsg = 
		WulforManager::get()->getPrivMsg_client(user);
	if (privMsg) {
		typedef Func1< ::PrivateMessage, string> F1;
		F1 *func;
		func = new F1(privMsg, &::PrivateMessage::addMessage_gui,
			user->getNick() + " left the hub.");
		WulforManager::get()->dispatchGuiFunc(func);
	}

	typedef Func1<Hub, string> F1;
	F1 *remove = new F1(this, &Hub::removeUser_gui, user->getNick());
	WulforManager::get()->dispatchGuiFunc(remove);

	pthread_mutex_lock(&clientLock);
	ostringstream userStream;
	typedef Func2<Hub, GtkStatusbar*, string> F2;
	F2 *status;
	userStream << client->getUserCount() << " User(s)";
	
	status = new F2(this, &Hub::setStatus_gui, usersStatus, userStream.str());
	WulforManager::get()->dispatchGuiFunc(status);
	status = new F2(this, &Hub::setStatus_gui, sharedStatus, Util::formatBytes(client->getAvailable()));
	WulforManager::get()->dispatchGuiFunc(status);
	pthread_mutex_unlock(&clientLock);
}

void Hub::on(ClientListener::Redirect, 
	Client *cl, const string &address) throw()
{
	if (!address.empty()) {
		string s, f;
		short p = 411;
		Util::decodeUrl(Text::fromT(address), s, p, f);
		if (ClientManager::getInstance()->isConnected(s, p)) {
			Func2<Hub, GtkStatusbar*, string> *func = new Func2<Hub, GtkStatusbar*, string> (
				this, &Hub::setStatus_gui, mainStatus, "Redirecting to alredy connected hub");
			WulforManager::get()->dispatchGuiFunc(func);
			return;
		}
		
		// the client is dead, long live the client!
		pthread_mutex_lock(&clientLock);
		client->removeListener(this);
		ClientManager::getInstance()->putClient(client);
		Func0<Hub> *func = new Func0<Hub>(this, &Hub::clearNickList_gui);
		WulforManager::get()->dispatchGuiFunc(func);
		client = ClientManager::getInstance()->getClient(Text::fromT(address));
		client->addListener(this);
		client->connect();
		pthread_mutex_unlock(&clientLock);

		//for bookentry, when WulforManager searches for pages
		id = address;
	}
}

void Hub::on(ClientListener::Failed, 
	Client *client, const string &reason) throw()
{
	typedef Func2<Hub, GtkStatusbar*, string> F2;
	F2 *func = new F2(this, &Hub::setStatus_gui, mainStatus, "Connect failed: " + reason);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::GetPassword, Client *client) throw() {
	if (!client->getPassword().empty()) {
		client->password(client->getPassword());
	} else {
		Func0<Hub> *func = new Func0<Hub>(this, &Hub::getPassword_gui);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::HubUpdated, Client *client) throw() {
	Func1<BookEntry, string> *func = new Func1<BookEntry, string> (
		this, &BookEntry::setLabel_gui, client->getName());
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::Message, 
	Client *client, const string &msg) throw()
{
	Func1<Hub, string> *func = new Func1<Hub, string> (
		this, &Hub::addMessage_gui, msg);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::PrivateMessage, 
	Client *client, const User::Ptr &user, const string &msg) throw()
{
	typedef Func2<Hub, User::Ptr, string> F2;
	F2 *func = new F2(this, &Hub::addPrivateMessage_gui, user, msg);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::UserCommand, Client *client, 
	int, int, const string&, const string&) throw()
{
	//TODO: figure out what this is supposed to do =)
}

void Hub::on(ClientListener::HubFull, Client *client) throw() {
	typedef Func2<Hub, GtkStatusbar*, string> F2;
	F2 *func = new F2(this, &Hub::setStatus_gui, mainStatus, "Sorry, hub full");
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::NickTaken, Client *cl) throw() {
	pthread_mutex_lock(&clientLock);
	client->removeListener(this);
	client->disconnect();
	client = NULL;
	pthread_mutex_unlock(&clientLock);

	typedef Func2<Hub, GtkStatusbar*, string> F2;
	F2 *func = new F2(this, &Hub::setStatus_gui, mainStatus, "Nick alredy taken");
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::SearchFlood, Client *client, 
	const string &msg) throw() 
{
	typedef Func2<Hub, GtkStatusbar*, string> F2;
	F2 *func = new F2(this, &Hub::setStatus_gui, mainStatus, "Search spam from " + msg);
	WulforManager::get()->dispatchGuiFunc(func);
}
	
void Hub::on(ClientListener::NmdcSearch, Client *client, const string&, 
	int, int64_t, int, const string&) throw()
{
	//TODO: figure out what to do here...
}