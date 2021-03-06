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
using Glib::ustring;
//can't import whole namespace, messes with client/Exception

int ShareBrowser::fColSize[] = {500, 50, 80};

ShareBrowser::ShareBrowser(QueueItem *item, MainWindow *mw):
	listing(NULL),
	downloadItem("Download"),
	dirItem("Download directory"),
	dirItem2("Download directory"),
	viewItem("View as text")

{
	Slot0<void> callback0;
	Slot1<void, GdkEventButton *> callback1;
	Slot2<void, const TreeModel::Path&, TreeViewColumn*> callback2;
	GuiProxy *proxy = GuiProxy::getInstance();
	ThreadTunnel *tunnel = proxy->getTunnel();
		
	this->user = item->getCurrent()->getUser();
	this->mw = mw;
	this->ID = BOOK_FILE_LIST;

	label.set_text(user->getNick());
	label.show();

	dirStore = TreeStore::create(dCol);
	dirView.set_model(dirStore);
	dirView.append_column("Directory", dCol.name);

	fileStore = ListStore::create(fCol);
	fileView.set_model(fileStore);
	fileView.append_column("Filename", fCol.name);
	fileView.append_column("Type", fCol.type);
    fileView.append_column("Size", fCol.filesize);

    for (int i = 0; i < fCol.size() -1; i++)
    {
    	fileView.get_column(i)->set_sizing(TREE_VIEW_COLUMN_FIXED);
        fileView.get_column(i)->set_resizable(true);
        fileView.get_column(i)->set_fixed_width(fColSize[i]);
	}
	
	dirScroll.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);
	dirScroll.add(dirView);
	fileScroll.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);
	fileScroll.add(fileView);

	add (mainBox);
	
	pane.add1(dirScroll);
	pane.add2(fileScroll);
	pane.set_position(200);

	mainBox.pack_start (pane, PACK_EXPAND_WIDGET, 0);
	mainBox.pack_start (statusBox, PACK_SHRINK, 0);	

	for (int i=0;i<STATUS_LAST;i++)
	{
		statusBar[i].set_has_resize_grip (false);
		if (i==0)
			statusBox.pack_start (statusBar[i], PACK_EXPAND_WIDGET, 2);
		else
			statusBox.pack_start (statusBar[i], PACK_SHRINK, 2);
		
		if (i == 1 || i == 3)	
			statusBar[i].set_size_request (60, -1);
		else
			statusBar[i].set_size_request (100, -1);
	}
	currentItems = 0;
	currentSize = 0;
	
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

	show_all();

	assert(listing == NULL);
	listing = new DirectoryListing(user);
	listing->loadFile(item->getListName (), false);
	buildList();
	updateStatus ();
	setPosition(item->getSearchString());
	
}

void ShareBrowser::setStatus (std::string text, int num)
{
	if (num<0 || num>STATUS_LAST-1) return;

	statusBar[num].pop(1);
	if (num == 0)
		statusBar[num].push ("[" + Util::getShortTimeString() + "] " + text, 1);
	else
		statusBar[num].push (text, 1);
}

void ShareBrowser::updateStatus ()
{
	TreeModel::Row r = *(dirView.get_selection()->get_selected());

	if (!r)
	{
		setStatus ("Items: 0", STATUS_ITEMS);
		setStatus ("Size: 0 B", STATUS_FILE_SIZE);
		setStatus ("File: " + Util::toString (shareItems), STATUS_FILES);
		setStatus ("Size: " + Util::formatBytes (shareSize), STATUS_TOTAL_SIZE);
		return;
	}

	setStatus ("Items: " + Util::toString (currentItems), STATUS_ITEMS);
	setStatus ("Size: " + Util::formatBytes (currentSize), STATUS_FILE_SIZE);
	setStatus ("File: " + Util::toString (shareItems), STATUS_FILES);
	setStatus ("Size: " + Util::formatBytes (shareSize), STATUS_TOTAL_SIZE);
}

ShareBrowser::~ShareBrowser() {
	if (listing != NULL) delete listing;
}

void ShareBrowser::buildList() {
	DirectoryListing::Directory::List dirs = listing->getRoot()->directories;
	DirectoryListing::Directory::Iter it;
	DirectoryListing::File::Iter file;
	TreeRow row;

	dirStore->clear();

	shareSize = 0;
	shareItems = 0;
	
	for (it = dirs.begin(); it != dirs.end(); it++) {
		row = *(dirStore->append());
		
		//add the name and check if the name is in UTF8
		if (listing->getUtf8()) {
			row[dCol.name] = (*it)->getName();
		} else {
			row[dCol.name] = WUtil::ConvertToUTF8((*it)->getName());
		}
		
		row[dCol.dir] = *it;
		for (file = (*it)->files.begin(); file != (*it)->files.end(); file++)
		{
			shareItems++;
			shareSize += (*file)->getSize ();
		}		
		processDirectory((*it)->directories, row);
	}
}

void ShareBrowser::processDirectory(DirectoryListing::Directory::List dir, 
	const TreeModel::Row &row)
{
	DirectoryListing::Directory::Iter it;
	DirectoryListing::File::Iter file;
	TreeRow newRow;

	for (it = dir.begin(); it != dir.end(); it++) {
		newRow = *(dirStore->append(row.children()));

		//add the name and check if the name is in UTF8
		if (listing->getUtf8()) {
			newRow[dCol.name] = (*it)->getName();
		} else {
			newRow[dCol.name] = WUtil::ConvertToUTF8((*it)->getName());
		}

		newRow[dCol.dir] = *it;
		for (file = (*it)->files.begin(); file != (*it)->files.end(); file++)
		{
			shareItems++;
			shareSize += (*file)->getSize ();
		}
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

	currentSize = 0;
	currentItems = 0;
	for (it = files->begin(); it != files->end(); it++) {
		row = *(fileStore->append());
		//data needs to be converted to utf8 if it's not in that form
		if (listing->getUtf8()) {
			row[fCol.name] = (*it)->getName();
			row[fCol.type] = Util::getFileExt((*it)->getName());
		} else {
			row[fCol.name] = WUtil::ConvertToUTF8((*it)->getName());
			row[fCol.type] = 
				WUtil::ConvertToUTF8(Util::getFileExt((*it)->getName()));
		}

		row[fCol.filesize] = Util::formatBytes((*it)->getSize());
		row[fCol.file] = *it;
		currentSize += (*it)->getSize ();
		currentItems++;
	}
	updateStatus ();
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
	const TreeModel::iterator constIter = 
		fileView.get_selection()->get_selected();
	string target;

	if (!constIter) return;
	if (!listing) return;

	DirectoryListing::File *file = (*constIter)[fCol.file];
	try {
		//false = prio
		target = WUtil::ConvertFromUTF8(SETTING(DOWNLOAD_DIRECTORY));
		assert(target[target.size() - 1] == PATH_SEPARATOR);
		
		if (listing->getUtf8()) {
			target += WUtil::ConvertFromUTF8(Util::getFileName(file->getName()));
		} else {
			target += Util::getFileName(file->getName());
		}

		listing->download(file, target, false, false);
	} catch(const Exception& e) {
		setStatus (e.getError(), 0);
	}
}

void ShareBrowser::downloadDirClicked() {
	const TreeModel::iterator constIter = 
		dirView.get_selection()->get_selected();
	string target;

	if (!constIter) return;
	if (!listing) return;

	DirectoryListing::Directory *dir = (*constIter)[dCol.dir];
	try {
		//false = prio
		target = SETTING(DOWNLOAD_DIRECTORY);
		listing->download(dir, WUtil::ConvertFromUTF8(target), false);
	} catch(const Exception& e) {
		setStatus(e.getError(), 0);
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
		//true = view file, somebody needs to listen to the view though...
		//false = prio
		target = WUtil::ConvertFromUTF8(SETTING(DOWNLOAD_DIRECTORY));
		assert(target[target.size() - 1] == PATH_SEPARATOR);
		
		if (listing->getUtf8()) {
			target += WUtil::ConvertFromUTF8(Util::getFileName(file->getName()));
		} else {
			target += Util::getFileName(file->getName());
		}

		listing->download(file, target, true, false);
	} catch(const Exception& e) {
		setStatus(e.getError(), 0);
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

void ShareBrowser::close()
{
	getParent ()->remove_page (*this);
	delete this;
}
