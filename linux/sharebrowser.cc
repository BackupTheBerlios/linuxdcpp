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
#include "../client/QueueItem.h"

#include <iostream>
#include <assert.h>

using namespace std;
using namespace Gtk;
using namespace SigC;
using namespace SigCX;
//can't import whole namespace, messes with client/Exception

ShareBrowser::ShareBrowser(User::Ptr user, MainWindow *mw):
	listing(NULL),
	downloadItem("Download"),
	dirItem("Download directory"),
	dirItem2("Download directory"),
	viewItem("View as text")

{
	Slot0<void> callback0;
	Slot1<void, GdkEventButton *> callback1;
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
	dirView.append_column("Directory", dCol.name);

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

	//for the popup menu when left-clicking users
	callback1 = open_tunnel(tunnel, slot(*this, &ShareBrowser::buttonPressedDir), true);
	dirView.signal_button_press_event().connect_notify(callback1);
	callback1 = open_tunnel(tunnel, slot(*this, &ShareBrowser::buttonPressedFile), true);
	fileView.signal_button_press_event().connect_notify(callback1);

	callback1 = open_tunnel(tunnel, slot(*this, &ShareBrowser::buttonReleasedDir), true);
	dirView.signal_button_release_event().connect_notify(callback1);
	callback1 = open_tunnel(tunnel, slot(*this, &ShareBrowser::buttonReleasedFile), true);
	fileView.signal_button_release_event().connect_notify(callback1);

	//for popup menu items
	callback0 = open_tunnel(tunnel, slot(*this, &ShareBrowser::downloadClicked), true);
	downloadItem.signal_activate().connect(callback0);
	callback0 = open_tunnel(tunnel, slot(*this, &ShareBrowser::downloadDirClicked), true);
	dirItem.signal_activate().connect(callback0);
	callback0 = open_tunnel(tunnel, slot(*this, &ShareBrowser::viewClicked), true);
	viewItem.signal_activate().connect(callback0);
	callback0 = open_tunnel(tunnel, slot(*this, &ShareBrowser::downloadDirClicked), true);
	dirItem2.signal_activate().connect(callback0);

	fileMenu.append(downloadItem);
	fileMenu.append(dirItem);
	fileMenu.append(viewItem);

	dirMenu.append(dirItem2);

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
		//check so that it's the filelist for the correct user
		if (item->getCurrent()->getUser()->getFullNick() != user->getFullNick())
			return;
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
		row[dCol.name] = WUtil::ConvertToUTF8((*it)->getName());
		row[dCol.dir] = *it;
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
		newRow[dCol.name] = WUtil::ConvertToUTF8((*it)->getName());
		newRow[dCol.dir] = *it;
		processDirectory((*it)->directories, newRow);
	}
}

void ShareBrowser::updateSelection () {
	DirectoryListing::File::List *files;
	DirectoryListing::Directory *dir;
	DirectoryListing::File::Iter it;
	TreeRow row;
	const TreeModel::iterator constIter = 
		dirView.get_selection()->get_selected();

	//make sure we've actually selected a row...
	if (!constIter) return;
	fileStore->clear();
	dir = (*constIter)[dCol.dir];
	files = &(dir->files);

	for (it = files->begin(); it != files->end(); it++) {
		row = *(fileStore->append());
		row[fCol.name] = WUtil::ConvertToUTF8((*it)->getName());
		row[fCol.type] = WUtil::ConvertToUTF8(
			Util::getFileExt((*it)->getName()));
		row[fCol.size] = 
			WUtil::ConvertToUTF8(Util::formatBytes((*it)->getSize()));
		row[fCol.file] = *it;
	}
}

void ShareBrowser::setPosition(string pos) {
	//FIXME - this is when selecting goto dir in search
}

bool ShareBrowser::operator== (BookEntry &b) {
	ShareBrowser *browser = dynamic_cast<ShareBrowser *>(&b);
	
	if (browser == NULL) return false;
	return browser->user->getNick() == user->getNick();
}

void ShareBrowser::downloadClicked() {
	cout << "DL clicked" << endl;
	const TreeModel::iterator constIter = 
		fileView.get_selection()->get_selected();
	string target;

	if (!constIter) return;
	if (!listing) return;

	DirectoryListing::File *file = (*constIter)[fCol.file];
	try {
		//false = prio, TODO: implement changing this
		target = SETTING(DOWNLOAD_DIRECTORY);
		assert(target[target.size() - 1] == PATH_SEPARATOR);
		target += Util::getFileName(file->getName());
		listing->download(file, target, false, false);
	} catch(const Exception& e) {
		cout << "Error: " << e.getError() << endl;
		//TODO: add a statusbar and print this there instead
		//ctrlStatus.SetText(0, e.getError().c_str());
	}
}

void ShareBrowser::downloadDirClicked() {
	cout << "Dir clicked" << endl;
	const TreeModel::iterator constIter = 
		dirView.get_selection()->get_selected();

	if (!constIter) return;
	if (!listing) return;

	DirectoryListing::Directory *dir = (*constIter)[dCol.dir];
	try {
		//false = prio, TODO: implement changing this
		listing->download(dir, SETTING(DOWNLOAD_DIRECTORY), false);
	} catch(const Exception& e) {
		cout << "Error: " << e.getError() << endl;
		//TODO: add a statusbar and print this there instead
		//ctrlStatus.SetText(0, e.getError().c_str());
	}
}

void ShareBrowser::viewClicked() {
	cout << "view clicked - Not fully implemented yet =)" << endl;
	const TreeModel::iterator constIter = 
		fileView.get_selection()->get_selected();
	string target;

	if (!constIter) return;
	if (!listing) return;

	DirectoryListing::File *file = (*constIter)[fCol.file];
	try {
		//true = view file
		//somebody needs to listen to the view though...
		//false = prio, TODO: implement changing this
		target = SETTING(DOWNLOAD_DIRECTORY);
		assert(target[target.size() - 1] == PATH_SEPARATOR);
		target += Util::getFileName(file->getName());
		listing->download(file, target, false, false);
	} catch(const Exception& e) {
		cout << "Error: " << e.getError() << endl;
		//TODO: add a statusbar and print this there instead
		//ctrlStatus.SetText(0, e.getError().c_str());
	}
}

void ShareBrowser::buttonPressedDir(GdkEventButton* event) {
	dirPrevious = event->type;
}

void ShareBrowser::buttonPressedFile(GdkEventButton* event) {
	filePrevious = event->type;
}

void ShareBrowser::buttonReleasedDir(GdkEventButton* event) {
	//single click
	if (dirPrevious == GDK_BUTTON_PRESS) {
		//left button
		if (event->button == 1) {
			updateSelection();
		}

		//right button
		if (event->button == 3) {
			dirMenu.popup(event->button, event->time);
			dirMenu.show_all();
		}
	}

	//double click
	if (dirPrevious == GDK_2BUTTON_PRESS) {
		//left button
		if (event->button == 1) {
			downloadDirClicked();
		}
	}
}

void ShareBrowser::buttonReleasedFile(GdkEventButton* event) {
	//single click
	if (filePrevious == GDK_BUTTON_PRESS) {
		//right button
		if (event->button == 3) {
			fileMenu.popup(event->button, event->time);
			fileMenu.show_all();
		}
	}

	//double click
	if (filePrevious == GDK_2BUTTON_PRESS) {
		//left button
		if (event->button == 1) {
			downloadClicked();
		}
	}
}
