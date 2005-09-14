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
	listFrozen(true),
	client(NULL),
	WIDTH_ICON(20),
	WIDTH_NICK(100),
	WIDTH_SHARED(75),
	WIDTH_DESCRIPTION(75),
	WIDTH_TAG(100),
	WIDTH_CONNECTION(75),
	WIDTH_EMAIL(100)
{
	string file = WulforManager::get()->getPath() + "/glade/hub.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "hubWindow");
	mainBox = glade_xml_get_widget(xml, "hubBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	passwordDialog = GTK_DIALOG(glade_xml_get_widget(xml, "passwordDialog"));
	passwordEntry = GTK_ENTRY(glade_xml_get_widget(xml, "passwordEntry"));
	chatEntry = GTK_ENTRY(glade_xml_get_widget(xml, "chatEntry"));
	chatText = GTK_TEXT_VIEW(glade_xml_get_widget(xml, "chatText"));
	chatScroll = GTK_SCROLLED_WINDOW(glade_xml_get_widget(xml, "chatScroll"));
	nickView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "nickView"));
	mainStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusMain"));
	usersStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusUsers"));
	sharedStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusShared"));
	
	nickStore = gtk_list_store_new(8, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT64);
	gtk_tree_view_set_model(nickView, GTK_TREE_MODEL(nickStore));
	nickSelection = gtk_tree_view_get_selection(nickView);
	TreeViewFactory factory(nickView);
	factory.addColumn_gui(COLUMN_ICON, "", TreeViewFactory::PIXBUF, WIDTH_ICON);
	factory.addColumn_gui(COLUMN_NICK, "Nick", TreeViewFactory::STRING, WIDTH_NICK);
	factory.addColumn_gui(COLUMN_SHARED, "Shared", TreeViewFactory::STRING, WIDTH_SHARED);
	factory.addColumn_gui(COLUMN_DESCRIPTION, "Description", TreeViewFactory::STRING, WIDTH_DESCRIPTION);
	factory.addColumn_gui(COLUMN_TAG, "Tag", TreeViewFactory::STRING, WIDTH_TAG);
	factory.addColumn_gui(COLUMN_CONNECTION, "Connection", TreeViewFactory::STRING, WIDTH_DESCRIPTION);
	factory.addColumn_gui(COLUMN_EMAIL, "eMail", TreeViewFactory::STRING, WIDTH_EMAIL);

	//This makes adding all the users when joining faster
	gtk_tree_view_set_model(nickView, NULL);
	//Another speed optimization
	gtk_tree_view_set_fixed_height_mode(nickView, TRUE);
	
	factory.setSortColumn_gui(COLUMN_SHARED, COLUMN_SHARED_BYTES);

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

	//Connect callbacks
	g_signal_connect(chatEntry, "activate", G_CALLBACK(sendMessage), this);
	g_signal_connect(nickView, "button-release-event", G_CALLBACK(popupNickMenu), this);
	g_signal_connect(browseItem, "activate", G_CALLBACK(browseItemClicked), this);
	g_signal_connect(msgItem, "activate", G_CALLBACK(msgItemClicked), this);
	g_signal_connect(grantItem, "activate", G_CALLBACK(grantItemClicked), this);
	g_signal_connect_after(chatEntry, "key-press-event", G_CALLBACK(doTabCompletion), this);
	g_signal_connect_after(chatEntry, "key-press-event", G_CALLBACK(setChatEntryFocus), this);
}

Hub::~Hub() {
	if (client) {
		client->removeListener(this);
		ClientManager::getInstance()->putClient(client);
	}

	map<string, GdkPixbuf*>::iterator it;
	for (it = userIcons.begin(); it != userIcons.end(); it++)
		g_object_unref(G_OBJECT(it->second));
		
	gtk_widget_destroy(GTK_WIDGET(passwordDialog));
}

GtkWidget *Hub::getWidget() {
	return mainBox;
}

void Hub::connectClient(string address, string nick, string desc, string password)
{
	client = ClientManager::getInstance()->getClient(address);

	if (nick.empty()) client->setNick(SETTING(NICK));
	else client->setNick(nick);
	if (!desc.empty()) client->setDescription(desc);

	client->addListener(this);
	client->setPassword(password);
	client->connect();
}

void Hub::updateUser(User::Ptr user) {
	string iconFile, nick = user->getNick();
	int64_t shared = user->getBytesShared();

	if (user->isSet(User::DCPLUSPLUS)) iconFile += "dc++";
	else iconFile += "normal";
	if (user->isSet(User::PASSIVE)) iconFile += "-fw";
	if (user->isSet(User::OP)) iconFile += "-op";
		
	string description = user->getDescription();
	string tag = user->getTag();
	string connection = user->getConnection();
	string email = user->getEmail();
	
	GtkTreeIter iter;
	GdkPixbuf *icon;
	string file;

	if (listFrozen) {
		gtk_list_store_append(nickStore, &iter);
	} else {
		findUser(nick, &iter);
		if (!gtk_list_store_iter_is_valid(nickStore, &iter))
			gtk_list_store_append(nickStore, &iter);
	}

	icon = userIcons[iconFile];
	gtk_list_store_set(nickStore, &iter, 
		COLUMN_ICON, icon,
		COLUMN_NICK, nick.c_str(),
		COLUMN_SHARED, Util::formatBytes(shared).c_str(),
		COLUMN_DESCRIPTION, description.c_str(),
		COLUMN_TAG, tag.c_str(),
		COLUMN_CONNECTION, connection.c_str(),
		COLUMN_EMAIL, email.c_str(),
		COLUMN_SHARED_BYTES, shared,
		-1);
}

void Hub::updateCounts() {
	ostringstream userStream;

	if (client) {
		userStream << client->getUserCount() << " User(s)";
		setStatus(usersStatus, userStream.str());
		setStatus(sharedStatus, Util::formatBytes(client->getAvailable()));
	}
}

void Hub::setStatus(GtkStatusbar *status, std::string text) {
	gtk_statusbar_pop(status, 0);
	gtk_statusbar_push(status, 0, text.c_str());
}

void Hub::findUser(string nick, GtkTreeIter *iter) {
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

void Hub::removeUser(string nick) {
	GtkTreeIter iter;
	
	findUser(nick, &iter);
	if (gtk_list_store_iter_is_valid(nickStore, &iter))
		gtk_list_store_remove(nickStore, &iter);
}

void Hub::sendMessage(GtkEntry *entry, gpointer data) {
	Hub *h = (Hub *)data;
	string text = gtk_entry_get_text(h->chatEntry);
	
	if (!text.empty()) {
		gtk_entry_set_text(h->chatEntry, "");
		
		if (h->client) {
			h->client->getMe()->clientMessage(text);
		}
	}
}

gboolean Hub::popupNickMenu(GtkWidget *, GdkEventButton *button, gpointer data)
{
	Hub *h = (Hub *)data;

	//only for right mouse button
	if (button->button != 3) return FALSE;
	//return if no nick is selected
	if (!gtk_tree_selection_get_selected(h->nickSelection, NULL, NULL)) return FALSE;

	gtk_menu_popup(h->nickMenu, NULL, NULL, NULL, NULL, 3, button->time);
	gtk_widget_show_all(GTK_WIDGET(h->nickMenu));
	
	return FALSE;
}

void Hub::browseItemClicked(GtkMenuItem *, gpointer data)
{
	GtkTreeIter iter;
	char *text;
	Hub *h = (Hub *)data;
	User::Ptr user = NULL;
	string nick;
	
	gtk_tree_selection_get_selected(h->nickSelection, NULL, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(h->nickStore), &iter,
		COLUMN_NICK, &text,
		-1);
	nick = text;
	g_free(text);
		
	if (h->client) {
		user = ClientManager::getInstance()->getUser(nick, h->client);
	}

	if (!user) return;

	try	{
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} catch (...) {
		string text = "Could get filelist from: " + user->getNick();
		h->setStatus(h->mainStatus, text);
	}
}

void Hub::msgItemClicked(GtkMenuItem *, gpointer data)
{
	GtkTreeIter iter;
	char *text;
	Hub *h = (Hub *)data;

	gtk_tree_selection_get_selected(h->nickSelection, NULL, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(h->nickStore), &iter,
		COLUMN_NICK, &text,
		-1);

	if (h->client) {
		User::Ptr user = ClientManager::getInstance()->getUser(text, h->client);
		WulforManager::get()->addPrivMsg(user);
	}
	
	g_free(text);
}

void Hub::grantItemClicked(GtkMenuItem *, gpointer data)
{
	GtkTreeIter iter;
	char *text;
	Hub *h = (Hub *)data;

	gtk_tree_selection_get_selected(h->nickSelection, NULL, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(h->nickStore), &iter,
		COLUMN_NICK, &text,
		-1);

	if (h->client) {
		User::Ptr user = ClientManager::getInstance()->getUser(text, h->client);
		if (user) UploadManager::getInstance()->reserveSlot(user);
	}
	
	g_free(text);
}

void Hub::doTabCompletion(GtkWidget *, GdkEventKey *key, gpointer data) {
	if (!BOOLSETTING(TAB_COMPLETION)) return;
	if (key->keyval != GDK_Tab) return;
	
	string nick;
	int countMatches;
	string command;
	Hub *h = (Hub *)data;
 
	nick = gtk_entry_get_text(h->chatEntry);

	//this is needed if we want to use tab completion with user commands
	if (g_strrstr(nick.c_str(), " ")) {
		string::size_type len = nick.length();
		string::size_type pos = nick.find_last_of(" ", len);
		command = nick.substr(0, pos+1);
		nick = nick.erase(0, pos+1);
	}
	else command = "";

	//check if nick is same as previous time.. so we can continue from it
	if (nick == h->prev_nick) {
		gtk_tree_model_iter_next(GTK_TREE_MODEL(h->nickStore), &h->completion_iter);
	}
	//if it's different then start from beginning
	else {
		string::size_type length = nick.length();
		h->nick_completion = g_utf8_strdown(nick.c_str(), length);
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(h->nickStore), &h->completion_iter);
		countMatches = 0; //reset match counter
	}
	while (gtk_list_store_iter_is_valid(h->nickStore, &h->completion_iter)) {
		const char *t;

		gtk_tree_model_get(GTK_TREE_MODEL(h->nickStore), &h->completion_iter, COLUMN_NICK, &t, -1);

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
		if (g_str_has_prefix(name.c_str(), h->nick_completion.c_str())) {
			gtk_entry_set_text(h->chatEntry, t);
			gtk_entry_set_position(h->chatEntry, -1);
			//if we used some commands or words before tab-completion let's prepend them
			gtk_entry_prepend_text(h->chatEntry, command.c_str());
			h->prev_nick = t; //store nick for later use (see beginning of this void)
			countMatches++;
			return;
		}
		
		gtk_tree_model_iter_next(GTK_TREE_MODEL(h->nickStore), &h->completion_iter);

		if (!gtk_list_store_iter_is_valid(h->nickStore, &h->completion_iter)) {
			//if no matches found, don't continue, if we don't do this, client will enter to endless loop
			if (countMatches == 0) return;
			gtk_tree_model_get_iter_first(GTK_TREE_MODEL(h->nickStore), &h->completion_iter);
			//&Hub::doTabCompletion; //if there were matches, we can safely start from beginning
		}	
	}
}

void Hub::setChatEntryFocus(GtkWidget *, GdkEventKey *key, gpointer data)
{
	Hub *h = (Hub *)data;

	if (key->keyval != GDK_Tab) return;
	gtk_widget_grab_focus(GTK_WIDGET(h->chatEntry));
	//set "cursor" (the blinking thing) at the end of text. this is because of few things
	//1. "cursor" will be at the beginning of text
	//2. whole text in entry is painted
	gtk_entry_set_position(h->chatEntry, strlen(gtk_entry_get_text(h->chatEntry)));
	return;
}

void Hub::on(ClientListener::Connecting, Client *client) throw() {
	gdk_threads_enter();
	setStatus(mainStatus, "Connecting");
	setStatus(usersStatus, "0 Users");
	setStatus(sharedStatus, "0 B");
	gdk_threads_leave();
}

void Hub::on(ClientListener::Connected, Client *client) throw() {
	gdk_threads_enter();
	setStatus(mainStatus, "Connected");
	gdk_threads_leave();
}

void Hub::on(ClientListener::BadPassword, Client *cl) throw() {
	gdk_threads_enter();
	setStatus(mainStatus, "Wrong password");
	gdk_threads_leave();
	client->setPassword("");
}

void Hub::on(ClientListener::UserUpdated, Client *client, 
	const User::Ptr &user) throw() 
{
	if (!user->isSet(User::HIDDEN)) {
		gdk_threads_enter();
		updateUser(user);
		if (!listFrozen) updateCounts();
		gdk_threads_leave();
	}
}
	
void Hub::on(ClientListener::UsersUpdated, 
	Client *client, const User::List &list) throw()
{
	User::List::const_iterator it;

	gdk_threads_enter();

	for (it = list.begin(); it != list.end(); it++)	{
		if (!(*it)->isSet(User::HIDDEN)) {
			updateUser(*it);
		}
	}
	if (!listFrozen) updateCounts();

	gdk_threads_leave();
}

void Hub::on(ClientListener::UserRemoved, 
	Client *cl, const User::Ptr &user) throw()
{
	gdk_threads_enter();

	::PrivateMessage *privMsg = WulforManager::get()->getPrivMsg(user);
	if (privMsg) {
		privMsg->addMessage(user->getNick() + " left the hub.");
	}

	if (listFrozen)
	{
		listFrozen = false;	
		gtk_tree_view_set_model(nickView, GTK_TREE_MODEL(nickStore));
	}
	removeUser(user->getNick());
	updateCounts();

	gdk_threads_leave();
}

void Hub::on(ClientListener::Redirect, 
	Client *cl, const string &address) throw()
{
	if (!address.empty()) {
		string s, f;
		u_int16_t p = 411;
		Util::decodeUrl(Text::fromT(address), s, p, f);
		if (ClientManager::getInstance()->isConnected(s, p)) {
			gdk_threads_enter();
			setStatus(mainStatus, "Redirecting to alredy connected hub");
			gdk_threads_leave();
			return;
		}
		
		// the client is dead, long live the client!
		client->removeListener(this);
		ClientManager::getInstance()->putClient(client);

		gdk_threads_enter();
		gtk_list_store_clear(nickStore);
		gdk_threads_leave();

		client = ClientManager::getInstance()->getClient(Text::fromT(address));
		client->addListener(this);
		client->connect();

		//for bookentry, when WulforManager searches for pages
		id = address;
	}
}

void Hub::on(ClientListener::Failed, 
	Client *client, const string &reason) throw()
{
	gdk_threads_enter();
	setStatus(mainStatus, "Connect failed: " + reason);
	gdk_threads_leave();
}

void Hub::on(ClientListener::GetPassword, Client *client) throw() {
	if (!client->getPassword().empty())
	{
		client->password(client->getPassword());
	}
	else
	{
		MainWindow *mainWin = WulforManager::get()->getMainWindow();
		string password;
		int ret;

		gdk_threads_enter();
		gtk_widget_show_all(GTK_WIDGET(passwordDialog));
		ret = gtk_dialog_run(passwordDialog);
		password = gtk_entry_get_text(passwordEntry);
		gdk_threads_leave();

		if (ret == GTK_RESPONSE_ACCEPT && client) {
			client->setPassword(password);
			client->password(client->getPassword());
		}
	}
}

void Hub::on(ClientListener::HubUpdated, Client *client) throw() {
	string hubName;
	
	//name is never set ???
	if (client->getName().empty()) {
		hubName = client->getAddress() + ":" + client->getAddressPort();
	} else {
		hubName = client->getName();
	}
	
	gdk_threads_enter();
	setBookLabel(hubName);	
	gdk_threads_leave();
}

void Hub::on(ClientListener::Message, 
	Client *client, const string &msg) throw()
{
	GtkTextIter iter;
	
	//this function is sometimes triggered with an empty msg
	if (msg.empty()) return;
	
	string text = "[" + Util::getShortTimeString() + "] " + msg + "\n";
	GtkAdjustment *adj;
	bool setBottom;

	gdk_threads_enter();
	adj = gtk_scrolled_window_get_vadjustment(chatScroll);
	setBottom = gtk_adjustment_get_value(adj) >= (adj->upper - adj->page_size);
	
	gtk_text_buffer_get_end_iter(chatBuffer, &iter);
	gtk_text_buffer_insert(chatBuffer, &iter, text.c_str(), text.size());

	if (setBottom) {
		gtk_text_buffer_get_end_iter(chatBuffer, &iter);
		gtk_text_buffer_move_mark(chatBuffer, chatMark, &iter);
		gtk_text_view_scroll_to_mark(chatText, chatMark, 0, FALSE, 0, 0);
	}
	gdk_threads_leave();
}

void Hub::on(ClientListener::PrivateMessage, 
	Client *client, const User::Ptr &user, const string &msg) throw()
{
	gdk_threads_enter();
	::PrivateMessage *privMsg = 
		WulforManager::get()->getPrivMsg(user);
		
	if (!privMsg)
		privMsg = WulforManager::get()->addPrivMsg(user);
	
	privMsg->addMessage(msg);
	gdk_threads_leave();
}

void Hub::on(ClientListener::UserCommand, Client *client, 
	int, int, const string&, const string&) throw()
{
	//TODO: figure out what this is supposed to do =)
}

void Hub::on(ClientListener::HubFull, Client *client) throw() {
	gdk_threads_enter();
	setStatus(mainStatus, "The hub is full");
	gdk_threads_leave();
}

void Hub::on(ClientListener::NickTaken, Client *cl) throw() {
	client->removeListener(this);
	client->disconnect();
	client = NULL;

	gdk_threads_enter();
	setStatus(mainStatus, "Your nick is alredy taken");
	gdk_threads_leave();
}

void Hub::on(ClientListener::SearchFlood, Client *client, 
	const string &msg) throw() 
{
	gdk_threads_enter();
	setStatus(mainStatus, "Search spam from " + msg);
	gdk_threads_leave();
}
	
void Hub::on(ClientListener::NmdcSearch, Client *client, const string&, 
	int, int64_t, int, const string&) throw()
{
	//TODO: figure out what to do here...
}

