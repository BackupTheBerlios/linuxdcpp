#include "hub.hh"
#include "wulformanager.hh"

#include <iostream>
#include <sstream>

using namespace std;

Hub::Hub(std::string address, GCallback closeCallback, string nick, string desc, string password):
	BookEntry(WulforManager::HUB, address, address, closeCallback),
	WIDTH_ICON(16),
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
	TreeView treeView(nickView);
	treeView.addColumn_gui(COLUMN_ICON, "", TreeView::PIXBUF, WIDTH_ICON);
	treeView.addColumn_gui(COLUMN_NICK, "Nick", TreeView::STRING, WIDTH_NICK);
	treeView.addColumn_gui(COLUMN_SHARED, "Shared", TreeView::STRING, WIDTH_SHARED);

	chatBuffer = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(chatText, chatBuffer);

	client = NULL;

	typedef Func4<Hub, string, string, string, string> F4;
	F4 *func = new F4(this, &Hub::connectClient_client, address, nick, desc, password);
	WulforManager::get()->dispatchClientFunc(func);
}

Hub::~Hub() {
	if (client) {
		client->removeListener(this);
		ClientManager::getInstance()->putClient(client);
	}
}

GtkWidget *Hub::getWidget() {
	return mainBox;
}

void Hub::connectClient_client(string address, string nick, string desc, string password) {
	client = ClientManager::getInstance()->getClient(address);

	if (nick.empty()) client->setNick(SETTING(NICK));
	else client->setNick(nick);
	if (!desc.empty()) client->setDescription(desc);

	client->addListener(this);
	client->setPassword(password);
	client->connect();
}

void Hub::setStatus_gui(GtkStatusbar *status, std::string text) {
	gtk_statusbar_pop(status, 0);
	gtk_statusbar_push(status, 0, text.c_str());
}

void Hub::findUser_gui(string nick, GtkTreeIter *iter) {
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(nickStore), iter);
	
	while (gtk_list_store_iter_is_valid(nickStore, iter)) {
		char *text;
		gtk_tree_model_get(GTK_TREE_MODEL(nickStore), iter, COLUMN_NICK, &text, -1);
		if (nick == text) return;
		gtk_tree_model_iter_next(GTK_TREE_MODEL(nickStore), iter);
	}
}

void Hub::updateUser_gui(const User::Ptr &user) {
	GtkTreeIter iter;
	GdkPixbuf *icon;
	string file;
	ostringstream userStream;

	findUser_gui(user->getNick(), &iter);
	if (!gtk_list_store_iter_is_valid(nickStore, &iter)) {
		gtk_list_store_append(nickStore, &iter);
	}

	file = WulforManager::get()->getPath() + "/pixmaps/normal.png";
	icon = gdk_pixbuf_new_from_file(file.c_str(), NULL);
	gtk_list_store_set(nickStore, &iter, 
		COLUMN_ICON, icon,
		COLUMN_NICK, user->getNick().c_str(),
		COLUMN_SHARED, Util::formatBytes(user->getBytesShared()).c_str(),
		-1);
	g_object_unref(G_OBJECT(icon));

	userStream << client->getUserCount() << " User(s)";
	setStatus_gui(usersStatus, userStream.str());
	setStatus_gui(sharedStatus, Util::formatBytes(client->getAvailable()));
}

void Hub::removeUser_gui(const User::Ptr &user) {
	GtkTreeIter iter;
	
	findUser_gui(user->getNick(), &iter);
	if (gtk_list_store_iter_is_valid(nickStore, &iter)) {
		gtk_list_store_remove(nickStore, &iter);
	}
}

void Hub::clearNickList_gui() {
	gtk_list_store_clear(nickStore);
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

void Hub::setPassword_client(string password) {
	client->setPassword(password);
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

void Hub::on(ClientListener::BadPassword, Client *client) throw() {
	Func2<Hub, GtkStatusbar*, string> *func = new Func2<Hub, GtkStatusbar*, string>
		(this, &Hub::setStatus_gui, mainStatus, "Sorry, wrong password");
	WulforManager::get()->dispatchGuiFunc(func);
	client->setPassword("");
}

void Hub::on(ClientListener::UserUpdated, Client *client, 
	const User::Ptr &user) throw() 
{
	if (!user->isSet(User::HIDDEN)) {
		typedef Func1<Hub, const User::Ptr&> F1;
		F1 *func = new F1(this, &Hub::updateUser_gui, user);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}
	
void Hub::on(ClientListener::UsersUpdated, 
	Client *client, const User::List &list) throw()
{
	User::List::const_iterator it;
	for (it = list.begin (); it != list.begin (); it++)	{
		if (!(*it)->isSet(User::HIDDEN)) {
			typedef Func1<Hub, const User::Ptr&> F1;
			F1 *func = new F1(this, &Hub::updateUser_gui, *it);
			WulforManager::get()->dispatchGuiFunc(func);
		}
	}
}

void Hub::on(ClientListener::UserRemoved, 
	Client *client, const User::Ptr &user) throw()
{
	/*
	PrivateMsg temp(user, mw);
	BookEntry *e = mw->getPage(&temp);
	if (e) {
		PrivateMsg *msg = dynamic_cast<PrivateMsg*>(e);
		msg->addMsg(user->getNick() + " left the hub.");
	}
	*/
	typedef Func1<Hub, const User::Ptr&> F1;
	F1 *remove = new F1(this, &Hub::removeUser_gui, user);
	WulforManager::get()->dispatchGuiFunc(remove);

	ostringstream userStream;
	typedef Func2<Hub, GtkStatusbar*, string> F2;
	F2 *status;
	userStream << client->getUserCount() << " User(s)";
	
	status = new F2(this, &Hub::setStatus_gui, usersStatus, userStream.str());
	WulforManager::get()->dispatchGuiFunc(status);
	status = new F2(this, &Hub::setStatus_gui, sharedStatus, Util::formatBytes(client->getAvailable()));
	WulforManager::get()->dispatchGuiFunc(status);
}

void Hub::on(ClientListener::Redirect, 
	Client *client, const string &address) throw()
{
	if(!address.empty()) {
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
		client->removeListener(this);
		ClientManager::getInstance()->putClient(client);
		Func0<Hub> *func = new Func0<Hub>(this, &Hub::clearNickList_gui);
		WulforManager::get()->dispatchGuiFunc(func);
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
	typedef Func2<Hub, GtkStatusbar*, string> F2;
	F2 *func = new F2(this, &Hub::setStatus_gui, mainStatus, "Connect failed: " + reason);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::GetPassword, Client *client) throw() {
	if (!client->getPassword().empty ()) {
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
	/*
	PrivateMsg privMsg(user, mw);
	BookEntry *pos;

	pos = mw->getPage(new PrivateMsg(user, mw));
	if (!pos) {
		mw->addPage(new PrivateMsg(user, mw));
		pos = mw->getPage(new PrivateMsg(user, mw));
	}

	dynamic_cast<PrivateMsg *>(pos)->addMsg(msg);
	*/
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

void Hub::on(ClientListener::NickTaken, Client *client) throw() {
	client->removeListener(this);
	client->disconnect();

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
