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

#include <iostream>
#include <cstring>
#include <string>

#include "publichubs.hh"
#include "hub.hh"
#include "guiproxy.hh"

#include <client/Util.h>

using namespace std;
using namespace Gtk;
using namespace SigC;
using namespace Glib;

int PublicHubs::columnSize[] = { 200, 290, 50, 100 };

PublicHubs::PublicHubs(MainWindow *mw):
	hubs(0),
	filter(""),
	refreshButton("Refresh"),
	connectButton("Connect"),
	filterFrame("Filter"),
	connectFrame("Manual connect address")
{
	this->mw = mw;
	ID = BOOK_PUBLIC_HUBS;
	label.set_text("Public hubs");

	hubStore = ListStore::create(columns);
	hubList.set_model(hubStore);
	hubList.append_column("Name", columns.name);
	hubList.append_column("Description", columns.description);
	hubList.append_column("Users", columns.users);
	hubList.append_column("Address", columns.address);

	for (int i=0;i<columns.size ();i++)
	{
		hubList.get_column (i)->set_sizing (TREE_VIEW_COLUMN_FIXED);
		hubList.get_column (i)->set_resizable (true);
		hubList.get_column (i)->set_fixed_width (columnSize[i]);
	}	

	//BookEntry inherits from VBox
	pack_start(listScroll, PACK_EXPAND_WIDGET);
	pack_start(fieldBox, PACK_SHRINK);
	
	listScroll.add(hubList);
	
	fieldBox.pack_start(filterFrame, PACK_EXPAND_WIDGET);
	fieldBox.pack_start(connectFrame, PACK_EXPAND_WIDGET);
	fieldBox.pack_start(buttonBox, PACK_SHRINK, 2);
	
	filterFrame.add(filterEntry);
	connectFrame.add(connectEntry);
	
	buttonBox.pack_start(refreshButton, PACK_EXPAND_WIDGET, 2);
	buttonBox.pack_start(connectButton, PACK_EXPAND_WIDGET, 2);

	refreshButton.signal_clicked().connect(slot(*this, &PublicHubs::refresh));
	connectButton.signal_clicked().connect(slot(*this, &PublicHubs::connect));

	HubManager *man = HubManager::getInstance();
	GuiProxy::getInstance()->
		addListener<PublicHubs, HubManagerListener>(this, man);

	if (man->isDownloading())
		mw->setStatus("Downloading hub list", STATUS_MAIN);

	hubs = man->getPublicHubs();
	if(hubs.empty()) man->refresh();

	man->save();
	
	updateList();
}

PublicHubs::~PublicHubs() {
	GuiProxy::getInstance()->
		removeListener<PublicHubs>(this);
}

void PublicHubs::updateList() {
	HubEntry::List::const_iterator i;
	TreeModel::iterator it;

	hubs = HubManager::getInstance()->getPublicHubs();
	
	hubStore->clear();
			
	for(i = hubs.begin(); i != hubs.end(); i++) {
		if( filter.getPattern().empty() ||
			filter.match(i->getName()) ||
			filter.match(i->getDescription()) ||
			filter.match(i->getServer()) )
		{
			it = hubStore->append();
			(*it)[columns.name] = WUtil::ConvertToUTF8(i->getName());
			(*it)[columns.description] = WUtil::ConvertToUTF8(i->getDescription());
			(*it)[columns.users] = i->getUsers();
			(*it)[columns.address] = WUtil::ConvertToUTF8(i->getServer());
		}
	}
}

void PublicHubs::on(HubManagerListener::DownloadStarting, 
	const string &file) throw()
{
	string msg;
	msg = "Download starting: " + file;
	mw->setStatus(msg, STATUS_MAIN);
}
	
void PublicHubs::on(HubManagerListener::DownloadFailed, 
	const string &file) throw()
{
	string msg;
	msg = "Download failed: " + file;
	mw->setStatus(msg, STATUS_MAIN);
}

void PublicHubs::on(HubManagerListener::DownloadFinished, 
	const string &file) throw()
{
	string msg;
	msg = "Download finished: " + file;
	mw->setStatus(msg, STATUS_MAIN);
	updateList();
}

void PublicHubs::on(HubManagerListener::FavoriteAdded, 
	const FavoriteHubEntry *fav) throw()
{
	cout << "Added favourite" << endl;
}

void PublicHubs::on(HubManagerListener::FavoriteRemoved, 
	const FavoriteHubEntry *fav) throw()
{
	cout << "Added favourite" << endl;
}

void PublicHubs::on(HubManagerListener::UserAdded, 
	const User::Ptr &user) throw()
{
	cout << "Added user" << endl;
}

void PublicHubs::on(HubManagerListener::UserRemoved, 
	const User::Ptr &user) throw()
{	
	cout << "Added user" << endl;
}

bool PublicHubs::operator== (BookEntry &b) {
	PublicHubs *hub;
	
	hub = dynamic_cast<PublicHubs *>(&b);
	return hub != NULL;
}

void PublicHubs::refresh() {
	filter = WUtil::ConvertFromUTF8(filterEntry.get_text());
	updateList();
}

void PublicHubs::connect()
{
	string address =
		WUtil::ConvertFromUTF8(connectEntry.get_text());
	Hub *hub;
	TreeModel::iterator it;
	RefPtr<TreeSelection> selection;

	if (address == "") {
		selection = hubList.get_selection();
		if (selection->count_selected_rows() == 0) return;
		it = selection->get_selected();
		address = WUtil::ConvertFromUTF8((*it)[columns.address]);
	}
	hub = new Hub(address, mw);
	mw->addPage(hub);
}

void PublicHubs::close() {
	delete this;
}

