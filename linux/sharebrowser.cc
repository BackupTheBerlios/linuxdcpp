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

#include "sharebrowser.hh"
#include "guiproxy.hh"

#include "../client/File.h"
#include "../client/CryptoManager.h"
#include "../client/Exception.h"

#include <iostream>
#include <assert.h>

using namespace std;
using namespace Gtk;
using namespace SigC;
using namespace SigCX;
//can't import whole namespace, messes with client/Exception

ShareBrowser::ShareBrowser(User::Ptr user, MainWindow *mw):
	listing(NULL)
{
	Slot0<void> callback0;
	Slot2<void, const TreeModel::Path&, TreeViewColumn*> callback2;
	QueueManager *qmgr = QueueManager::getInstance();
	GuiProxy *proxy = GuiProxy::getInstance();
	ThreadTunnel *tunnel = proxy->getTunnel();
		
	this->user = user;
	this->mw = mw;

	proxy->addListener<ShareBrowser, QueueManagerListener>(this, qmgr);
	try {
		qmgr->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} catch(const Exception& e) {
		cout << "error: " << e.getError() << endl;
	}

	label.set_text(WUtil::ConvertToUTF8(user->getNick()));
	label.show();

	dirStore = TreeStore::create(dCol);
	dirView.set_model(dirStore);
	dirView.append_column("Directory", dCol.dir);

	fileStore = ListStore::create(fCol);
	fileView.set_model(fileStore);
	fileView.append_column("Filename", fCol.name);
	fileView.append_column("Type", fCol.type);
	fileView.append_column("Size", fCol.size);

	dirScroll.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);
	dirScroll.add(dirView);
	fileScroll.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);
	fileScroll.add(fileView);
	
	pane.add1(dirScroll);
	pane.add2(fileScroll);
	pane.set_position(200);

	callback2 = 
		open_tunnel(tunnel, slot(*this, &ShareBrowser::dirPressed), false);
	dirView.signal_row_activated().connect(callback2);

	pack_start(pane);
	show_all();
}

ShareBrowser::~ShareBrowser() {
	if (listing != NULL) delete listing;
}

void ShareBrowser::on(QueueManagerListener::Finished, 
	QueueItem *item) throw() 
{
	if ( item->isSet(QueueItem::FLAG_CLIENT_VIEW) &&
		item->isSet(QueueItem::FLAG_USER_LIST))
	{
		assert(listing == NULL);
		listing = new DirectoryListing(user);
		listing->loadFile(item->getListName(), false);
		buildList();
		setPosition(item->getSearchString());
	}
}

void ShareBrowser::buildList() {
	DirectoryListing::Directory::List dirs = listing->getRoot()->directories;
	DirectoryListing::Directory::Iter it;
	TreeRow row;

	dirStore->clear();
	dirStore->clear();
	
	for (it = dirs.begin(); it != dirs.end(); it++) {
		row = *(dirStore->append());
		row[dCol.dir] = WUtil::ConvertToUTF8((*it)->getName());
		row[dCol.files] = &((*it)->files);
		processDirectory((*it)->directories, row);
	}
}

void ShareBrowser::processDirectory(DirectoryListing::Directory::List dir, 
	const TreeModel::Row &row)
{
	DirectoryListing::Directory::Iter it;
	TreeRow newRow;

	for (it = dir.begin(); it != dir.end(); it++) {
		newRow = *(dirStore->append(row.children()));
		newRow[dCol.dir] = WUtil::ConvertToUTF8((*it)->getName());
		newRow[dCol.files] = &((*it)->files);
		processDirectory((*it)->directories, newRow);
	}
}

void ShareBrowser::dirPressed (const Gtk::TreeModel::Path& path,
	Gtk::TreeViewColumn* column)
{
	DirectoryListing::File::List * files;
	DirectoryListing::File::Iter it;
	const TreeModel::iterator constIter = dirStore->get_iter(path);
	TreeRow row;
	
	if (!constIter) return;
	fileStore->clear();
	files = (*constIter)[dCol.files];

	for (it = files->begin(); it != files->end(); it++) {
		row = *(fileStore->append());
		row[fCol.name] = WUtil::ConvertToUTF8((*it)->getName());
		row[fCol.type] = WUtil::ConvertToUTF8((*it)->getName());
		row[fCol.size] = WUtil::ConvertToUTF8(Util::formatBytes((*it)->getSize()));
	}
}

void ShareBrowser::setPosition(string pos) {


}

bool ShareBrowser::operator== (BookEntry &b) {
	ShareBrowser *browser = dynamic_cast<ShareBrowser *>(&b);
	
	if (browser == NULL) return false;
	return browser->user->getNick() == user->getNick();
}

