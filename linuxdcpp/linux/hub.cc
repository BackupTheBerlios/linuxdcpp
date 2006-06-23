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

#include "hub.hh"

Hub::Hub(std::string address):
	BookEntry("Hub: " + address),
	enterCallback(this, &Hub::sendMessage_gui),
	nickListCallback(this, &Hub::popupNickMenu_gui),
	browseCallback(this, &Hub::browseItemClicked_gui),
	msgCallback(this, &Hub::msgItemClicked_gui),
	grantCallback(this, &Hub::grantItemClicked_gui),
	completionCallback(this, &Hub::completion_gui),
	setFocusCallback(this, &Hub::setChatEntryFocus),
	lastUpdate(0)
{
	GladeXML *xml = getGladeXML("hub.glade");

	GtkWidget *window = glade_xml_get_widget(xml, "hubWindow");
	mainBox = glade_xml_get_widget(xml, "hubBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	chatText = GTK_TEXT_VIEW(glade_xml_get_widget(xml, "chatText"));
	if (SETTING(USE_OEM_MONOFONT))
	{
		PangoFontDescription *font_desc;
		font_desc = pango_font_description_from_string("Mono 10");
		gtk_widget_modify_font(GTK_WIDGET(chatText), font_desc);
		pango_font_description_free(font_desc);
	}

	nickPane = GTK_PANED(glade_xml_get_widget(xml, "pane"));
	passwordDialog = GTK_DIALOG (glade_xml_get_widget(xml, "passwordDialog"));
	passwordEntry = GTK_ENTRY (glade_xml_get_widget(xml, "entryPassword"));
	chatEntry = GTK_ENTRY(glade_xml_get_widget(xml, "chatEntry"));
	chatText = GTK_TEXT_VIEW(glade_xml_get_widget(xml, "chatText"));
	chatScroll = GTK_SCROLLED_WINDOW(glade_xml_get_widget(xml, "chatScroll"));
	mainStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusMain"));
	usersStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusUsers"));
	sharedStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusShared"));

	// Initialize nick treeview
	nickView.setView(GTK_TREE_VIEW(glade_xml_get_widget(xml, "nickView")), true, "hub");
	nickView.insertColumn("Nick", G_TYPE_STRING, TreeView::PIXBUF_STRING, 100, "Icon");
	nickView.insertColumn("Shared", G_TYPE_STRING, TreeView::STRING, 75);
	nickView.insertColumn("Description", G_TYPE_STRING, TreeView::STRING, 75);
	nickView.insertColumn("Tag", G_TYPE_STRING, TreeView::STRING, 100);
	nickView.insertColumn("Connection", G_TYPE_STRING, TreeView::STRING, 75);
	nickView.insertColumn("eMail", G_TYPE_STRING, TreeView::STRING, 100);
	nickView.insertHiddenColumn("Shared Bytes", G_TYPE_INT64);
	nickView.insertHiddenColumn("Icon", GDK_TYPE_PIXBUF);
	nickView.insertHiddenColumn("Nick Order", G_TYPE_STRING);
	nickView.finalize();
	nickStore = gtk_list_store_newv(nickView.getColCount(), nickView.getGTypes());
	gtk_tree_view_set_model(nickView.get(), GTK_TREE_MODEL(nickStore));
	g_object_unref(nickStore);
	nickSelection = gtk_tree_view_get_selection(nickView.get());
	nickView.setSortColumn_gui("Nick", "Nick Order");
	nickView.setSortColumn_gui("Shared", "Shared Bytes");

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
	nickListCallback.connect_after(G_OBJECT(nickView.get()), "button-release-event", NULL);
	browseCallback.connect(G_OBJECT(browseItem), "activate", NULL);
	msgCallback.connect(G_OBJECT(msgItem), "activate", NULL);
	grantCallback.connect(G_OBJECT(grantItem), "activate", NULL);
	completionCallback.connect_after(G_OBJECT(chatEntry), "key-press-event", NULL);
	setFocusCallback.connect_after(G_OBJECT(chatEntry), "key-press-event", NULL);

	int nickPanePosition = WulforSettingsManager::get()->getInt("nick-pane-position");
	gtk_paned_set_position(nickPane, nickPanePosition);
}

Hub::~Hub()
{
	if (client)
	{
		client->removeListener(this);
		ClientManager::getInstance()->putClient(client);
	}
	pthread_mutex_destroy(&clientLock);

	hash_map<string, GdkPixbuf *>::iterator it;
	for (it = userIcons.begin(); it != userIcons.end(); it++)
		g_object_unref(G_OBJECT(it->second));

	int nickPanePosition = gtk_paned_get_position(nickPane);
	WulforSettingsManager::get()->set("nick-pane-position", nickPanePosition);
}

GtkWidget *Hub::getWidget()
{
	return mainBox;
}

void Hub::onRedirect_gui(Client* cl, string address)
{
	dcassert(cl);
        // the client is dead, long live the client!
	pthread_mutex_lock(&clientLock);
	client->removeListener(this);
	ClientManager::getInstance()->putClient(client);
	clearNickList_gui();
	client = ClientManager::getInstance()->getClient(address);
	client->addListener(this);
	
	client->connect();
	pthread_mutex_unlock(&clientLock);
}

void Hub::connectClient_client(string address, string nick, string desc, string password) {
	pthread_mutex_lock(&clientLock);
	client = ClientManager::getInstance()->getClient(address);

	if (nick.empty()) client->getMyIdentity().setNick(SETTING(NICK));
	else client->getMyIdentity().setNick(nick);
	if (!desc.empty()) client->getMyIdentity().setDescription(desc);

	client->addListener(this);
	client->setPassword(password);
	client->connect();
	pthread_mutex_unlock(&clientLock);
}

void Hub::updateUser_client(const OnlineUser& user) {
	Identity id = user.getIdentity();
	string icon, nick = id.getNick();
	int64_t shared = id.getBytesShared();

	if (user.getUser()->isSet(User::DCPLUSPLUS)) icon += "dc++";
	else icon += "normal";
	if (user.getUser()->isSet(User::PASSIVE)) icon += "-fw";

	bool op = id.isOp();
	if (op) icon += "-op";

	string description = id.getDescription();
	string tag = id.getTag();
	string connection = id.getConnection();
	string email = id.getEmail();

	typedef Func9<Hub, string, int64_t, string, string, string, string, string, Identity, bool> F9;
	F9 * func = new F9(this, &Hub::updateUser_gui, nick, shared, icon, description, tag, connection, email, id, op);
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

	GtkTreeModel *m = GTK_TREE_MODEL(nickStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, iter);

	while (valid)
	{
		if (nick == nickView.getString(iter, "Nick"))
			return;
		valid = gtk_tree_model_iter_next(m, iter);
	}
}

Identity Hub::getUserID(std::string nick)
{
	dcassert(idMap.find(nick) != idMap.end());
	return idMap[nick];
}

void Hub::updateUser_gui(string nick, int64_t shared, string iconFile,
		string description, string tag, string connection, string email, Identity id, bool opstatus) {

	GtkTreeIter iter;
	GdkPixbuf *icon;
	string file, nick_order = (opstatus ? "o" : "u") + nick;
	string users;

	findUser_gui(nick, &iter);

	NicksMapIter curNick;
	curNick = nicks.find(nick);

	if(curNick == nicks.end()) {
		gtk_list_store_append(nickStore, &iter);
		nicks[nick] = nick;
		idMap[nick] = id;
	}

	icon = userIcons[iconFile];
	gtk_list_store_set(nickStore, &iter, 
		nickView.col("Nick"), nick.c_str(),
		nickView.col("Shared"), Util::formatBytes(shared).c_str(),
		nickView.col("Description"), description.c_str(),
		nickView.col("Tag"), tag.c_str(),
		nickView.col("Connection"), connection.c_str(),
		nickView.col("eMail"), email.c_str(),
		nickView.col("Shared Bytes"), shared,
		nickView.col("Icon"), icon,
		nickView.col("Nick Order"), nick_order.c_str(),
		-1);

	u_int32_t ticks = GET_TICK();	
	if (client && ticks > lastUpdate + 1000) {
		lastUpdate = ticks;

		pthread_mutex_lock(&clientLock);
		{
			users = Util::toString(client->getUserCount()) + " User(s)";
			setStatus_gui(usersStatus, users);
			setStatus_gui(sharedStatus, Util::formatBytes(client->getAvailable()));
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
		idMap.erase(nick);
	}
}

void Hub::clearNickList_gui() {
	gtk_list_store_clear(nickStore);
	nicks.clear();
	idMap.clear();
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
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

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

	if (BOOLSETTING(LOG_MAIN_CHAT)) {
		StringMap params;
		params["message"] = Text::acpToUtf8(text);
		client->getHubIdentity().getParams(params, "hub", false);
		params["hubURL"] = client->getHubUrl();
		client->getMyIdentity().getParams(params, "my", true);
		LOG(LogManager::CHAT, params);
	}

	if (BOOLSETTING(BOLD_HUB))
		setBold_gui();
}

void Hub::addPrivateMessage_gui(User::Ptr tabUser, User::Ptr msgUser, std::string msg)
{
	::PrivateMessage *privMsg = WulforManager::get()->addPrivMsg_gui(tabUser); 
	privMsg->addMessage_gui(msgUser, msg);
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

void Hub::browseItemClicked_gui(GtkMenuItem *, gpointer data) {
	GtkTreeIter iter;
	string nick;
	gtk_tree_selection_get_selected(nickSelection, NULL, &iter);
	nick = nickView.getString(&iter, "Nick");

	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::getFileList_client, nick);
	WulforManager::get()->dispatchClientFunc(func);
}

void Hub::msgItemClicked_gui(GtkMenuItem *, gpointer data) {
	GtkTreeIter iter;
	string nick;
	gtk_tree_selection_get_selected(nickSelection, NULL, &iter);
	nick = nickView.getString(&iter, "Nick");

	pthread_mutex_lock(&clientLock);
	if (client) {
		string addr = client->getAddress();
		if (client->getPort() != 411)
			addr += ":" + Util::toString(client->getPort());
		User::Ptr user = idMap[nick].getUser();
		WulforManager::get()->addPrivMsg_gui(user);
	}
	pthread_mutex_unlock(&clientLock);
}

void Hub::grantItemClicked_gui(GtkMenuItem *, gpointer data) {
	GtkTreeIter iter;
	string nick;
	gtk_tree_selection_get_selected(nickSelection, NULL, &iter);
	nick = nickView.getString(&iter, "Nick");

	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::grantSlot_client, nick);
	WulforManager::get()->dispatchClientFunc(func);
}

void Hub::grantSlot_client(string userName) {
	pthread_mutex_lock(&clientLock);
	if (client) {
		User::Ptr user = idMap[userName].getUser();
		if (user) UploadManager::getInstance()->reserveSlot(user);
	}
	pthread_mutex_unlock(&clientLock);
}

void Hub::getFileList_client(string nick) {
	User::Ptr user = NULL;

	pthread_mutex_lock(&clientLock);
	if (client) {
		string addr = client->getAddress();
		if (client->getPort() != 411)
			addr += ":" + Util::toString(client->getPort());
		user = idMap[nick].getUser();
	}
	pthread_mutex_unlock(&clientLock);

	if (!user) return;

	try	{
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} catch (...) {
		string text = "Couldn't get filelist from: " + nick;
		typedef Func2<Hub, GtkStatusbar *, string> F2;
		F2 *func = new F2(this, &Hub::setStatus_gui, mainStatus, text);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::sendMessage_client(string message) {
	pthread_mutex_lock(&clientLock);
	if (client) {
		client->hubMessage(message);
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

void Hub::on(ClientListener::UserUpdated, Client *client, const OnlineUser& user) throw()
{
	if (!user.getIdentity().isHidden()) {
		updateUser_client(user);
	}
}

void Hub::on(ClientListener::UsersUpdated,
             Client *client, const OnlineUser::List &list) throw()
{
	OnlineUser::List::const_iterator it;
	for (it = list.begin(); it != list.end(); it++)	{
		if (!(*it)->getIdentity().isHidden()) {
			updateUser_client(*(*it));
		}
	}
}

void Hub::on(ClientListener::UserRemoved, Client *cl, const OnlineUser &user) throw()
{
	pthread_mutex_lock(&clientLock);
	size_t userCount = cl->getUserCount();
	int64_t available = cl->getAvailable();
	pthread_mutex_unlock(&clientLock);

	typedef Func3<Hub, string, size_t, int64_t> F3;
	F3 *f3 = new F3(this, &Hub::onUserRemoved_gui, user.getIdentity().getNick(), userCount, available);
	WulforManager::get()->dispatchGuiFunc(f3);
}

void Hub::onUserRemoved_gui(string nick, size_t userCount, int64_t available)
{
	string count = userCount + " User(s)";
	removeUser_gui(nick);

	setStatus_gui(usersStatus, count);
	setStatus_gui(sharedStatus, Util::formatBytes(available));
}

void Hub::on(ClientListener::Redirect, 
	Client *cl, const string &address) throw()
{
	dcassert(cl);
	///@todo implement AUTO_FOLLOW
	if (!address.empty()) {
		if (ClientManager::getInstance()->isConnected(address)) {
			Func2<Hub, GtkStatusbar*, string> *func = new Func2<Hub, GtkStatusbar*, string> (
				this, &Hub::setStatus_gui, mainStatus, "Redirecting to already connected hub");
			WulforManager::get()->dispatchGuiFunc(func);
			return;
		}

		typedef Func2<Hub, Client*, string> F2;
		F2 *func = new F2(this, &Hub::onRedirect_gui, cl, address);
		WulforManager::get()->dispatchGuiFunc(func);
		client = ClientManager::getInstance()->getClient(Text::fromT(address));
		client->addListener(this);
		client->connect();
		pthread_mutex_unlock(&clientLock);
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
	typedef Func1<Hub, string> F1;
	typedef Func2<MainWindow, GtkWidget *, string> F2;
	string hubName = "Hub: ";

	if (client->getHubName().empty())
		hubName += client->getAddress() + ":" + Util::toString(client->getPort());
	else
		hubName += client->getHubName();

	F1 *func1 = new F1(this, &BookEntry::setLabel_gui, hubName);
	WulforManager::get()->dispatchGuiFunc(func1);

	F2 *func2 = new F2(WulforManager::get()->getMainWindow(), &MainWindow::modifyWindowItem, getWidget(), hubName);
	WulforManager::get()->dispatchGuiFunc(func2);
}

void Hub::on(ClientListener::StatusMessage, Client* client, const string &msg) throw()
{
	Func1<Hub, string> *func = new Func1<Hub, string> (this, &Hub::addMessage_gui, msg);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::Message, const OnlineUser& from, const string &msg) throw()
{
	string line = "<" + from.getIdentity().getNick() + "> " + msg;
	Func1<Hub, string> *func = new Func1<Hub, string>(this, &Hub::addMessage_gui, line);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::PrivateMessage,	Client *client, const OnlineUser &from,
	const OnlineUser& to, const OnlineUser& replyTo, const string &msg) throw()
{
	const User::Ptr& user = (replyTo.getUser() == ClientManager::getInstance()->getMe()) ? to.getUser() : replyTo.getUser();

	typedef Func3<Hub, User::Ptr, User::Ptr, string> F3;
	F3 *func = new F3(this, &Hub::addPrivateMessage_gui, user, from.getUser(), msg);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::UserCommand, Client *client,
	int int1, int int2, const string& string1, const string& string2) throw()
{
	///@todo figure out what this is supposed to do =)
}

void Hub::on(ClientListener::HubFull, Client *client) throw() {
	typedef Func2<Hub, GtkStatusbar*, string> F2;
	F2 *func = new F2(this, &Hub::setStatus_gui, mainStatus, "Sorry, hub full");
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::NickTaken, Client *cl) throw() {
	pthread_mutex_lock(&clientLock);
	client->removeListener(this);
	client->disconnect(false);
	client = NULL;
	pthread_mutex_unlock(&clientLock);

	typedef Func2<Hub, GtkStatusbar*, string> F2;
	F2 *func = new F2(this, &Hub::setStatus_gui, mainStatus, "Nick already taken");
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
	///@todo figure out what to do here...
}

void Hub::completion_gui(GtkWidget *, GdkEventKey *key, gpointer) {
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
		string t = nickView.getString(&completion_iter, "Nick");

		int length = t.size(); //length is needed for few things
		string name = g_utf8_strdown(t.c_str(), length);

		//check for [ or ( prefix and remove [] or () with text inside them
		while (g_str_has_prefix(name.c_str(), "[") || g_str_has_prefix(name.c_str(), "(")) {
			if (g_str_has_prefix(name.c_str(), "[")) {
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
			gtk_entry_set_text(chatEntry, t.c_str());
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
