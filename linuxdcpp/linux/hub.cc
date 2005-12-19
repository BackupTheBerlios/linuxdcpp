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

#include "hub.hh"
#include "wulformanager.hh"
#include "treeviewfactory.hh"

#include <iostream>
#include <sstream>
#include <gdk/gdkkeysyms.h>

using namespace std;

Hub::Hub(std::string address, GCallback closeCallback):
	BookEntry(WulforManager::HUB, address, address, closeCallback),
	enterCallback(this, &Hub::sendMessage_gui),
	nickListCallback(this, &Hub::popupNickMenu_gui),
	browseCallback(this, &Hub::browseItemClicked_gui),
	msgCallback(this, &Hub::msgItemClicked_gui),
	grantCallback(this, &Hub::grantItemClicked_gui),
	completionCallback(this, &Hub::completion_gui),
	setFocusCallback(this, &Hub::setChatEntryFocus),
	WIDTH_NICK(100),
	WIDTH_SHARED(75),
	WIDTH_DESCRIPTION(75),
	WIDTH_TAG(100),
	WIDTH_CONNECTION(75),
	WIDTH_EMAIL(100),
	lastUpdate(0)
{
	string file = WulforManager::get()->getPath() + "/glade/hub.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "hubWindow");
	mainBox = glade_xml_get_widget(xml, "hubBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	passwordDialog = GTK_DIALOG (glade_xml_get_widget(xml, "passwordDialog"));
	passwordEntry = GTK_ENTRY (glade_xml_get_widget(xml, "entryPassword"));
	chatEntry = GTK_ENTRY(glade_xml_get_widget(xml, "chatEntry"));
	chatText = GTK_TEXT_VIEW(glade_xml_get_widget(xml, "chatText"));
	chatScroll = GTK_SCROLLED_WINDOW(glade_xml_get_widget(xml, "chatScroll"));
	nickView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "nickView"));
	mainStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusMain"));
	usersStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusUsers"));
	sharedStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusShared"));
	
	nickStore = gtk_list_store_new(9, 
		G_TYPE_STRING, 		// COLUMN_NICK
		G_TYPE_STRING, 		// COLUMN_SHARED
		G_TYPE_STRING, 		// COLUMN_DESCRIPTION
		G_TYPE_STRING, 		// COLUMN_TAG
		G_TYPE_STRING, 		// COLUMN_CONNECTION
		G_TYPE_STRING, 		// COLUMN_EMAIL
		G_TYPE_INT64, 		// COLUMN_SHARED_BYTES
		GDK_TYPE_PIXBUF, 	// COLUMN_ICON
		G_TYPE_STRING);		// COLUMN_NICK_ORDER
	gtk_tree_view_set_model(nickView, GTK_TREE_MODEL(nickStore));
	nickSelection = gtk_tree_view_get_selection(nickView);
	TreeViewFactory nickViewFactory(nickView);
	nickViewFactory.addColumn_gui(COLUMN_NICK, "Nick", TreeViewFactory::PIXBUF_STRING, WIDTH_NICK, COLUMN_ICON);
	nickViewFactory.addColumn_gui(COLUMN_SHARED, "Shared", TreeViewFactory::STRING, WIDTH_SHARED);
	nickViewFactory.addColumn_gui(COLUMN_DESCRIPTION, "Description", TreeViewFactory::STRING, WIDTH_DESCRIPTION);
	nickViewFactory.addColumn_gui(COLUMN_TAG, "Tag", TreeViewFactory::STRING, WIDTH_TAG);
	nickViewFactory.addColumn_gui(COLUMN_CONNECTION, "Connection", TreeViewFactory::STRING, WIDTH_DESCRIPTION);
	nickViewFactory.addColumn_gui(COLUMN_EMAIL, "eMail", TreeViewFactory::STRING, WIDTH_EMAIL);
	nickViewFactory.setSortColumn_gui(COLUMN_NICK, COLUMN_NICK_ORDER);
	nickViewFactory.setSortColumn_gui(COLUMN_SHARED, COLUMN_SHARED_BYTES);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), COLUMN_NICK_ORDER, GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(nickView, COLUMN_NICK), true);

	chatBuffer = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(chatText, chatBuffer);
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(chatBuffer, &iter);
	chatMark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, FALSE);
 	
	nickMenu = GTK_MENU(gtk_menu_new());
	browseItem = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Browse files"));
	msgItem = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Private message"));
	grantItem = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Grant slot"));
	gtk_menu_shell_append(GTK_MENU_SHELL(nickMenu), GTK_WIDGET(browseItem));
	gtk_menu_shell_append(GTK_MENU_SHELL(nickMenu), GTK_WIDGET(msgItem));
	gtk_menu_shell_append(GTK_MENU_SHELL(nickMenu), GTK_WIDGET(grantItem));

	pthread_mutex_init(&clientLock, NULL);
	client = NULL;

	//Loading icons for the nick list
	string tmp, path = WulforManager::get()->getPath() + "/pixmaps/";
	tmp = path + "normal.png";
	userIcons["normal"] = gdk_pixbuf_new_from_file(tmp.c_str(), NULL);
	tmp = path + "normal-op.png";
	userIcons["normal-op"] = gdk_pixbuf_new_from_file(tmp.c_str(), NULL);
	tmp = path + "normal-fw.png";
	userIcons["normal-fw"] = gdk_pixbuf_new_from_file(tmp.c_str(), NULL);
	tmp = path + "normal-fw-op.png";
	userIcons["normal-fw-op"] = gdk_pixbuf_new_from_file(tmp.c_str(), NULL);
	tmp = path + "dc++.png";
	userIcons["dc++"] = gdk_pixbuf_new_from_file(tmp.c_str(), NULL);
	tmp = path + "dc++-op.png";
	userIcons["dc++-op"] = gdk_pixbuf_new_from_file(tmp.c_str(), NULL);
	tmp = path + "dc++-fw.png";
	userIcons["dc++-fw"] = gdk_pixbuf_new_from_file(tmp.c_str(), NULL);
	tmp = path + "dc++-fw-op.png";
	userIcons["dc++-fw-op"] = gdk_pixbuf_new_from_file(tmp.c_str(), NULL);
	
	enterCallback.connect(G_OBJECT(chatEntry), "activate", NULL);
	nickListCallback.connect_after(G_OBJECT(nickView), "button-release-event", NULL);
	browseCallback.connect(G_OBJECT(browseItem), "activate", NULL);
	msgCallback.connect(G_OBJECT(msgItem), "activate", NULL);
	grantCallback.connect(G_OBJECT(grantItem), "activate", NULL);
	completionCallback.connect_after(G_OBJECT(chatEntry), "key-press-event", NULL);
	setFocusCallback.connect_after(G_OBJECT(chatEntry), "key-press-event", NULL);
}

Hub::~Hub() {
	if (client) {
		client->removeListener(this);
		ClientManager::getInstance()->putClient(client);
	}
	pthread_mutex_destroy(&clientLock);

	map<string, GdkPixbuf*>::iterator it;
	for (it = userIcons.begin(); it != userIcons.end(); it++)
		g_object_unref(G_OBJECT(it->second));
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
	typedef Func8<Hub, string, int64_t, string, string, string, string, string, bool> F8;
	F8 * func;
	string icon, nick = user->getNick();
	int64_t shared = user->getBytesShared();

	if (user->isSet(User::DCPLUSPLUS)) icon += "dc++";
	else icon += "normal";
	if (user->isSet(User::PASSIVE)) icon += "-fw";
	if (user->isSet(User::OP)) icon += "-op";
		
	string description = user->getDescription();
	string tag = user->getTag();
	string connection = user->getConnection();
	string email = user->getEmail();
	
	func = new F8(this, &Hub::updateUser_gui, nick, shared, icon, description, tag, connection, email, user->isSet(User::OP));
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
		char *t;
		string text;
		gtk_tree_model_get(GTK_TREE_MODEL(nickStore), iter, COLUMN_NICK, &t, -1);
		text = t;
		g_free(t);
		
		if (nick == text) return;
		gtk_tree_model_iter_next(GTK_TREE_MODEL(nickStore), iter);
	}
}

void Hub::updateUser_gui(string nick, int64_t shared, string iconFile,
		string description, string tag, string connection, string email, bool opstatus) {

	GtkTreeIter iter;
	GdkPixbuf *icon;
	string file, nick_order = (opstatus ? "o" : "u") + nick;
	ostringstream userStream;

	findUser_gui(nick, &iter);

	NicksMapIter curNick;
	curNick = nicks.find(nick);

	if(curNick == nicks.end()) {
		gtk_list_store_append(nickStore, &iter);
		nicks[nick] = nick;
	}

	icon = userIcons[iconFile];
	gtk_list_store_set(nickStore, &iter, 
		COLUMN_NICK, Text::acpToUtf8(nick).c_str(),
		COLUMN_SHARED, Util::formatBytes(shared).c_str(),
		COLUMN_DESCRIPTION, description.c_str(),
		COLUMN_TAG, tag.c_str(),
		COLUMN_CONNECTION, connection.c_str(),
		COLUMN_EMAIL, email.c_str(),
		COLUMN_SHARED_BYTES, shared,
		COLUMN_ICON, icon,
		COLUMN_NICK_ORDER, nick_order.c_str(),
		-1);

	u_int32_t ticks = GET_TICK();	
	if (client && ticks > lastUpdate + 1000) {
		lastUpdate = ticks;

		pthread_mutex_lock(&clientLock);
		{
			userStream << client->getUserCount() << " User(s)";
			setStatus_gui(usersStatus, userStream.str());
			setStatus_gui(sharedStatus, Util::formatBytes(client->getAvailable()));

//        	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), COLUMN_NICK_ORDER, GTK_SORT_ASCENDING);
//	        gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(nickView, COLUMN_NICK), TRUE);
		}
		pthread_mutex_unlock(&clientLock);
	}
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
	GtkWidget *entry;
	string password;
	int ret;

	GtkWidget *dialog = gtk_dialog_new_with_buttons ("Enter password",
		mainWin->getWindow(),
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		NULL);

	entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), entry, TRUE, TRUE, 5);
	gtk_widget_show_all(dialog);

	ret = gtk_dialog_run(GTK_DIALOG(dialog));
	password = gtk_entry_get_text(GTK_ENTRY(entry));
	gtk_widget_destroy(dialog);
	if (ret != GTK_RESPONSE_ACCEPT) return;

	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::setPassword_client, password);
	WulforManager::get()->dispatchClientFunc(func);
}

void Hub::addMessage_gui(string msg) {
	GtkTextIter iter;
	
	//this function is sometimes triggered with an empty msg
	if (msg.empty()) return;
	
	string text = "[" + Util::getShortTimeString() + "] " + msg + "\n";
	GtkAdjustment *adj;
	bool setBottom;

	adj = gtk_scrolled_window_get_vadjustment(chatScroll);
	setBottom = gtk_adjustment_get_value(adj) >= (adj->upper - adj->page_size);
	
	gtk_text_buffer_get_end_iter(chatBuffer, &iter);
	gtk_text_buffer_insert(chatBuffer, &iter, text.c_str(), text.size());

	if (setBottom) {
		gtk_text_buffer_get_end_iter(chatBuffer, &iter);
		gtk_text_buffer_move_mark(chatBuffer, chatMark, &iter);
		gtk_text_view_scroll_to_mark(chatText, chatMark, 0, FALSE, 0, 0);
	}
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
	
	g_free(text);
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
	
	g_free(text);
}

void Hub::grantItemClicked_gui(GtkMenuItem *, gpointer) {
	GtkTreeIter iter;
	char *text;
	gtk_tree_selection_get_selected(nickSelection, NULL, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(nickStore), &iter,
		COLUMN_NICK, &text,
		-1);

	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::grantSlot_client, string(text));
	WulforManager::get()->dispatchClientFunc(func);
	
	g_free(text);
}

void Hub::grantSlot_client(string userName) {
	pthread_mutex_lock(&clientLock);
	if (client) {
		User::Ptr user = ClientManager::getInstance()->getUser(userName, client);
		if (user) UploadManager::getInstance()->reserveSlot(user);
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
		client->password(client->getPassword());
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
	for (it = list.begin(); it != list.end(); it++)	{
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
		u_int16_t p = 411;
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
	Func1<BookEntry, string> *func;
	
	//name is never set ???
	if (client->getName().empty()) {
		string hubName = client->getAddress() + ":" + client->getAddressPort();
		func = new Func1<BookEntry, string>(this, &BookEntry::setLabel_gui, hubName);
	} else {
		func = new Func1<BookEntry, string>(this, &BookEntry::setLabel_gui, client->getName());	
	}
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

void Hub::completion_gui(GtkWidget *, GdkEventKey *key, gpointer) {
	if (!BOOLSETTING(TAB_COMPLETION)) return;
	if (key->keyval != GDK_Tab) return;
	
	string nick;
	int countMatches;
	string command;
 
	nick = gtk_entry_get_text(chatEntry);

	//this is needed if we want to use tab completion with user commands
	if (g_strrstr(nick.c_str(), " ")) {
		string::size_type len = nick.length();
		string::size_type pos = nick.find_last_of(" ", len);
		command = nick.substr(0, pos+1);
		nick = nick.erase(0, pos+1);
	}
	else command = "";

	//check if nick is same as previous time.. so we can continue from it
	if (nick == prev_nick) {
		gtk_tree_model_iter_next(GTK_TREE_MODEL(nickStore), &completion_iter);
	}
	//if it's different then start from beginning
	else {
		string::size_type length = nick.length();
		nick_completion = g_utf8_strdown(nick.c_str(), length);
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(nickStore), &completion_iter);
		countMatches = 0; //reset match counter
	}
	while (gtk_list_store_iter_is_valid(nickStore, &completion_iter)) {
		const char *t;

		gtk_tree_model_get(GTK_TREE_MODEL(nickStore), &completion_iter, COLUMN_NICK, &t, -1);

		int length = strlen(t); //length is needed for few things
		string name = g_utf8_strdown(t, length);

		//check for [ or ( prefix and remove [] or () with text inside them
		while(g_str_has_prefix(name.c_str(), "[") || g_str_has_prefix(name.c_str(), "(")){
			if(g_str_has_prefix(name.c_str(), "[")){
				string::size_type start = name.find_first_of("[", 0);
				string::size_type end = name.find_first_of("]", start);
				if (end == string::npos) end = start; //if end == string::npos it looks like there isn't closing bracket so we can safely remove [ and continue completion.. or can we???
				name = name.erase(start, end+1);
			}
			else {
				string::size_type start = name.find_first_of("(", 0);
				string::size_type end = name.find_first_of(")", start);
				if (end == string::npos) end = start; //same as previous
				name = name.erase(start, end+1);
			}
		}

		//if we found a match let's show it
		if (g_str_has_prefix(name.c_str(), nick_completion.c_str())) {
			gtk_entry_set_text(chatEntry, t);
			gtk_entry_set_position(chatEntry, -1);
			//if we used some commands or words before tab-completion let's prepend them
			gtk_entry_prepend_text(chatEntry, command.c_str());
			prev_nick = t; //store nick for later use (see beginning of this void)
			countMatches++;
			return;
		}
		
		gtk_tree_model_iter_next(GTK_TREE_MODEL(nickStore), &completion_iter);

		if (!gtk_list_store_iter_is_valid(nickStore, &completion_iter)) {
			//if no matches found, don't continue, if we don't do this, client will enter to endless loop
			if(countMatches == 0) return;
			gtk_tree_model_get_iter_first(GTK_TREE_MODEL(nickStore), &completion_iter);
			&Hub::completion_gui; //if there were matches, we can safely start from beginning
		}	
	}
}

void Hub::setChatEntryFocus(GtkWidget *, GdkEventKey *key, gpointer)
{
	if (key->keyval != GDK_Tab) return;
	gtk_widget_grab_focus(GTK_WIDGET(chatEntry));
	//set "cursor" (the blinking thing) at the end of text. this is because of few things
	//1. "cursor" will be at the beginning of text
	//2. whole text in entry is painted
	gtk_entry_set_position(chatEntry, strlen(gtk_entry_get_text(chatEntry)));
	return;
}
