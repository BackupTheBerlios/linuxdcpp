/* 
* Copyright (C) 2001-2003 Jens Oknelid, paskharen@spray.se
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
#include "privatemsg.hh"
#include "sharebrowser.hh"
#include "guiproxy.hh"

#include "../client/Util.h"
#include "../client/SettingsManager.h"

using namespace std;
using namespace Gtk;
using namespace SigC;
using namespace SigCX;
using namespace Glib;

int Hub::columnSize[] = {100, 50 };

Hub::Hub(string address, MainWindow *mw):
	browseItem("Browse files"),
	pmItem("Personal message"),
	favItem("Add to favourites")
{
	Slot1<void, GdkEventButton*> callback1;
	Slot0<void> callback0;
	GuiProxy *proxy = GuiProxy::getInstance();
	ThreadTunnel *tunnel = proxy->getTunnel();
	Adjustment *adj = chatScroll.get_vadjustment();
		
	this->address = address;
	this->mw = mw;
	
	this->ID = BOOK_HUB;
	add (mainBox);
	
	pane.set_position(600);
	chat.set_editable(false);
	chat.set_cursor_visible(false);
	
	nickStore = ListStore::create(columns);
	nickView.set_model(nickStore);
	nickView.append_column("Nick", columns.nick);
	nickView.append_column("Shared", columns.shared);

	for (int i=0;i<columns.size ();i++)
	{
		nickView.get_column (i)->set_sizing (TREE_VIEW_COLUMN_FIXED);
		nickView.get_column (i)->set_resizable (true);
		nickView.get_column (i)->set_fixed_width (columnSize[i]);
	}

	nickScroll.add(nickView);
	chatScroll.add(chat);
	
	chatBox.pack_start(chatScroll, PACK_EXPAND_WIDGET, 2);
	chatBox.pack_start(chatEntry, PACK_SHRINK, 2);

	pane.add1(chatBox);
	pane.add2(nickScroll);

	mainBox.pack_start (pane, PACK_EXPAND_WIDGET, 0);
	mainBox.pack_start (statusBox, PACK_SHRINK, 0);
	
	for (int i=0;i<HUB_NUM_MESSAGES;i++)
	{
		statusBar[i].set_has_resize_grip (false);
		if (i==0)
			statusBox.pack_start (statusBar[i], PACK_EXPAND_WIDGET, 2);
		else
			statusBox.pack_start (statusBar[i], PACK_SHRINK, 2);
		
		if (i>0)	
			statusBar[i].set_size_request (75, -1);
	}

	label.set_text (address);
	label.show();

	//for the popup menu when left-clicking users
	callback1 = open_tunnel(tunnel, slot(*this, &Hub::showPopupMenu), true);
	nickView.signal_button_press_event().connect_notify(callback1);

	//for popup menu items
	callback0 = open_tunnel(tunnel, slot(*this, &Hub::browseClicked), true);
	browseItem.signal_activate().connect(callback0);
	callback0 = open_tunnel(tunnel, slot(*this, &Hub::pmClicked), true);
	pmItem.signal_activate().connect(callback0);
	callback0 = open_tunnel(tunnel, slot(*this, &Hub::favClicked), true);
	favItem.signal_activate().connect(callback0);
	
	//For enter in the chat entry
	callback0 = open_tunnel(tunnel, slot(*this, &Hub::enterPressed), true);
	chatEntry.signal_activate().connect(callback0);

	//To set the chatScroll to bottom position when text is inserted
	callback0 = open_tunnel(tunnel, slot(*this, &Hub::somethingChanged), true);
	adj->signal_changed().connect(callback0);
	callback0 = open_tunnel(tunnel, slot(*this, &Hub::valueChanged), true);
	adj->signal_value_changed().connect(callback0);
	scrollVal = 0;	
		
	popupMenu.append(browseItem);
	popupMenu.append(pmItem);
	popupMenu.append(favItem);

	client = ClientManager::getInstance()->getClient(address);
	proxy->addListener<Hub, ClientListener>(this, client);
	client->setNick(SETTING(NICK));
	client->connect();
}

Hub::~Hub() {
	ClientManager::getInstance()->putClient(client);
	GuiProxy::getInstance()->removeListener<Hub>(this);
}
		
bool Hub::operator== (BookEntry &b) {
	Hub *hub;
	
	hub = dynamic_cast<Hub *>(&b);
	if (!hub) return false;
	
	return hub->address == address;
}

void Hub::on(ClientListener::Connecting, Client *client) throw() {
	setStatus("Connecting", HUB_STATUS_MAIN);
	setStatus("0 Users", HUB_STATUS_USERS);
	setStatus("0 B", HUB_STATUS_SHARED);
}

void Hub::on(ClientListener::Connected, Client *client) throw() {
	setStatus("Connected", HUB_STATUS_MAIN);
}

void Hub::on(ClientListener::BadPassword, Client *client) throw() {
	setStatus ("Sorry, wrong password", HUB_STATUS_MAIN);
	client->setPassword("");
}

void Hub::updateUser (const User::Ptr &user)
{
	TreeModel::iterator it = findUser (user->getNick());
	if (it == nickStore->children ().end ()) {
		it = nickStore->append ();
		//insert the nick in the map
		userMap[user->getNick()] = it;
	}

	TreeModel::Row row = *it;		

	row[columns.nick] = user->getNick();
	row[columns.shared] = Util::formatBytes (user->getBytesShared ());

	char buffer[32];
	sprintf (buffer, "%d User(s)", client->getUserCount ());
	setStatus (buffer, HUB_STATUS_USERS);
	setStatus (Util::formatBytes(client->getAvailable ()), HUB_STATUS_SHARED);
}

void Hub::on(ClientListener::UserUpdated, Client *client, 
	const User::Ptr &user) throw() 
{
	if(!user->isSet(User::HIDDEN))
		updateUser (user);
}
	
void Hub::on(ClientListener::UsersUpdated, 
	Client *client, const User::List &list) throw()
{
	User::List::const_iterator it;
	for (it = list.begin (); it != list.begin (); it++)
	{
		if(!(*it)->isSet(User::HIDDEN))
			updateUser (*it);
	}
}

void Hub::on(ClientListener::UserRemoved, 
	Client *client, const User::Ptr &user) throw()
{
	PrivateMsg temp(user, mw);
	BookEntry *e = mw->getPage(&temp);
	if (e) {
		PrivateMsg *msg = dynamic_cast<PrivateMsg*>(e);
		msg->addMsg(user->getNick() + " left the hub.");
	}

	TreeModel::iterator it = findUser(user->getNick());
	if (it == nickStore->children().end()) {
		return;
	} else {
		nickStore->erase(it);
		//remove from userMap
		userMap.erase(user->getNick());
	}
	
	char buffer[32];
	sprintf (buffer, "%d User(s)", client->getUserCount ());
	setStatus (buffer, HUB_STATUS_USERS);
	setStatus (Util::formatBytes(client->getAvailable ()), HUB_STATUS_SHARED);
}

void Hub::on(ClientListener::Redirect, 
	Client *client, const string &msg) throw()
{
	//some junk from the old client... (0.2 something)
	//probably easier to check the windows side of things for better code...
			/*
			{
				string s, f;
				short p = 411;
				Util::decodeUrl(line, s, p, f);
				if(ClientManager::getInstance()->isConnected(s, p)) {
					speak(ADD_STATUS_LINE, STRING(REDIRECT_ALREADY_CONNECTED));
					return;
				}
			}
			redirect = line;
			if(BOOLSETTING(AUTO_FOLLOW)) {
				PostMessage(WM_COMMAND, IDC_FOLLOW, 0);
			} else {
				speak(ADD_STATUS_LINE, STRING(PRESS_FOLLOW) + line);
			}
			*/
}

void Hub::on(ClientListener::Failed, 
	Client *client, const string &reason) throw()
{
	setStatus("Connect failed: " + reason, HUB_STATUS_MAIN);
}

void Hub::on(ClientListener::GetPassword, Client *client) throw() {
	if (client->getPassword () > 0)
		client->password (client->getPassword());
	else
	{
		showPasswordDialog ();
		client->password (client->getPassword ());	
	}
}

void Hub::on(ClientListener::HubUpdated, Client *client) throw() {
	label.set_text(client->getName());
}

void Hub::on(ClientListener::Message, 
	Client *client, const string &msg) throw()
{
	RefPtr<TextBuffer> buffer;

	buffer = chat.get_buffer();
	buffer->insert (buffer->end(), "[" + Util::getShortTimeString() + "] " + msg + "\n");
}

void Hub::on(ClientListener::PrivateMessage, 
	Client *client, const User::Ptr &user, const string &msg) throw()
{
	PrivateMsg privMsg(user, mw);
	BookEntry *pos;

	pos = mw->getPage(new PrivateMsg(user, mw));
	if (!pos) {
		mw->addPage(new PrivateMsg(user, mw));
		pos = mw->getPage(new PrivateMsg(user, mw));
	}

	dynamic_cast<PrivateMsg *>(pos)->addMsg(msg);
}

void Hub::on(ClientListener::UserCommand, Client *client, 
	int, int, const string&, const string&) throw()
{
	//TODO: figure out what this is supposed to do =)
}

void Hub::on(ClientListener::HubFull, Client *client) throw() {
	setStatus("Sorry, hub full", HUB_STATUS_MAIN);
}

void Hub::on(ClientListener::NickTaken, Client *client) throw() {
	client->removeListener(this);
	client->disconnect();
	setStatus("Nick taken", HUB_STATUS_MAIN);
}

void Hub::on(ClientListener::SearchFlood, Client *client, 
	const string &msg) throw() 
{
	setStatus("Search spam from: " + msg, HUB_STATUS_MAIN);
}
	
void Hub::on(ClientListener::NmdcSearch, Client *client, const string&, 
	int, int64_t, int, const string&) throw()
{

}

TreeModel::iterator Hub::findUser(ustring nick) {
	if (userMap.find(nick) != userMap.end()) return userMap[nick];
	else return nickStore->children().end();
}

void Hub::showPopupMenu(GdkEventButton* event) {
	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3)) {
		popupMenu.popup(event->button, event->time);
		popupMenu.show_all();
	}
}

void Hub::browseClicked() {
	ShareBrowser *b;
	User::Ptr user;
	RefPtr<TreeSelection> sel = nickView.get_selection();
	TreeModel::iterator it = sel->get_selected();
	ustring tmp;	
	ClientManager *man = ClientManager::getInstance();
	
	if (!it) return;
	tmp = (*it)[columns.nick];
	user = man->getUser(tmp.raw(), client);

	try
	{
		QueueManager::getInstance ()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	}
	catch(...)
	{
		cout << "Couldn't add filelist." << endl;
	}	
}

void Hub::pmClicked() {
	PrivateMsg *privMsg;
	User::Ptr user;
	RefPtr<TreeSelection> sel = nickView.get_selection();
	TreeModel::iterator it = sel->get_selected();
	ustring tmp;
	
	if (!it) return;
	
	tmp = (*it)[columns.nick];
	user = new User(tmp.raw());
	user->setClient(client);
	privMsg = new PrivateMsg(user, mw);
	mw->addPage(privMsg);
}

void Hub::favClicked() {
	cout << "add user to favourites" << endl;
}

void Hub::enterPressed() {
	client->getMe()->clientMessage(chatEntry.get_text());
	chatEntry.set_text("");
}

void Hub::somethingChanged() {
	Adjustment *adj = chatScroll.get_vadjustment();
	VScrollbar *scroll = chatScroll.get_vscrollbar();

	//if value has been updated, do nothing (otherwise the user cant scroll!)
	//otherwise bounds have changed or something => update
	if (scrollVal == scroll->get_value())
		scroll->set_value(adj->get_upper());
}

void Hub::valueChanged() {
	scrollVal = chatScroll.get_vscrollbar()->get_value();
}

void Hub::setStatus(std::string text, int num) 
{
	if (num<0 || num>HUB_NUM_MESSAGES-1) return;

	statusBar[num].pop(1);
	if (num == 0)
		statusBar[num].push ("[" + Util::getShortTimeString() + "] " + text, 1);
	else
		statusBar[num].push(text, 1);
}

void Hub::close ()
{
	if (client->isConnected ())
		cout << "Disconnected from " << client->getName () << endl;
	client->disconnect ();
	getParent ()->remove_page (*this);
	delete this;
}

void Hub::showPasswordDialog ()
{
	Gtk::Dialog window ("Password", *mw, true) ;
	Gtk::Entry entry;

	window.set_default_size (200, 50);

	entry.set_invisible_char('*');
	entry.set_visibility(false);
	window.get_vbox ()->pack_start (entry, PACK_EXPAND_WIDGET);
	window.add_button (GTK_STOCK_APPLY, 1);
		
	window.show_all ();
		
	window.run ();

	client->setPassword (entry.get_text ().raw ());
}
