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

#include "mainwindow.hh"
#include "publichubs.hh"
#include "search.hh"
#include "settingsdialog.hh"
#include "selecter.hh"
#include "guiproxy.hh"
#include "sharebrowser.hh"

#include "../client/Util.h"
#include "../client/Client.h"
#include "../client/SettingsManager.h"
#include "../client/SearchManager.h"
#include "../client/ConnectionManager.h"
#include "../client/Exception.h"
#include "queue.hh"

#include <iostream>
#include <assert.h>

using namespace Gtk;
using namespace Gdk;
using namespace SigCX;
using namespace std;
using namespace Toolbar_Helpers;
using namespace Notebook_Helpers;

//cant include Glib, conflicts with Exception
using Glib::ustring;

MainWindow::MainWindow():
	Window(),
	pubHubsIcon("test.xpm"),
	searchIcon("test.xpm"),
	settingsIcon("test.xpm"),
	exitIcon("test.xpm"),
	hashIcon("test.xpm"),
	queueIcon("test.xpm")
{
	int i;
	Slot0<void> callback;
	GuiProxy *proxy = GuiProxy::getInstance();
	ThreadTunnel *tunnel = proxy->getTunnel();

	set_title(WUtil::ConvertToUTF8("Wülfor 0.1"));
	set_default_size(800, 600);

	hashProgress = new HashDialog (this);
	
	callback = 
		open_tunnel(tunnel, slot(*this, &MainWindow::pubHubsClicked), false);
	leftBar.tools().push_back(ButtonElem("Public Hubs", pubHubsIcon, callback));
	callback = 
		open_tunnel(tunnel, slot(*this, &MainWindow::searchClicked), false);
	leftBar.tools().push_back(ButtonElem("Search", searchIcon, callback));
	callback = 
		open_tunnel(tunnel, slot(*this, &MainWindow::settingsClicked), false);
	leftBar.tools().push_back(ButtonElem("Settings", settingsIcon, callback));
	callback =
		open_tunnel(tunnel, slot(*this, &MainWindow::hashClicked), false);
	leftBar.tools().push_back(ButtonElem("Hash", hashIcon, callback));
	callback = 
		open_tunnel(tunnel, slot(*this, &MainWindow::queueClicked), false);	
	leftBar.tools().push_back(ButtonElem("Queue", queueIcon, callback));
	callback =
		open_tunnel(tunnel, slot(*this, &MainWindow::exitClicked), false);	
	rightBar.tools().push_back(ButtonElem("Exit", exitIcon, callback));

	add(mainBox);
	
	pane.add1(book);
	pane.add2(CTransfer::getInstance ()->getTransferScroll ());
	pane.set_position(360);
	
	mainBox.pack_start(toolBox, PACK_SHRINK, 0);
	mainBox.pack_start(pane, PACK_EXPAND_WIDGET, 0);
	mainBox.pack_start(statusBox, PACK_SHRINK, 2);
	
	toolBox.pack_start(leftBar, PACK_SHRINK, 0);
	toolBox.pack_end(rightBar, PACK_SHRINK, 0);
	
	for (i=0; i<NUM_STATUS; i++) {
		status[i].set_has_resize_grip(false);
		if (i == 0) statusBox.pack_start(status[i], PACK_EXPAND_WIDGET, 2);
		else statusBox.pack_start(status[i], PACK_SHRINK, 2);


		switch (i) {
			case 1:
				status[i].set_size_request(70, -1); break;
			case 2:
				status[i].set_size_request(60, -1); break;
			case 3:
			case 4:
				status[i].set_size_request(75, -1); break;
			case 5:
			case 6:
				status[i].set_size_request(90, -1); break;
		}
	}

	show_all ();
	
	lastUp = lastDown = lastUpdate = 0;

	proxy->addListener<MainWindow, TimerManagerListener>(this, TimerManager::getInstance());
	proxy->addListener<MainWindow, QueueManagerListener>(this, QueueManager::getInstance());

	showingHash = false;
	
	startSocket();
}

void MainWindow::startSocket() {
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();
	if(SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE)
	{
		return;
	}	

	short lastPort = (short)SETTING(IN_PORT);
	short firstPort = lastPort;

	while(true) {
		try {
			ConnectionManager::getInstance()->setPort(lastPort);
			Selecter::WSAASyncSelect(
				ConnectionManager::getInstance()->getServerSocket());

			SearchManager::getInstance()->setPort(lastPort);
			break;
		} catch(const Exception &e) {
			cout << "startSocket caught " << e.getError() << endl;
			short newPort = (short)((lastPort == 32000) ? 1025 : lastPort + 1);
			SettingsManager::getInstance()->
				setDefault(SettingsManager::IN_PORT, newPort);
				
			if(SETTING(IN_PORT) == lastPort || (firstPort == newPort)) {
				// Changing default didn't change port, a fixed port must be in
				// use...(or we tried all ports)
				//cout << STRING(PORT_IS_BUSY) << SETTING(IN_PORT) << endl;
				cout << "Port is busy " << SETTING(IN_PORT) << endl;
				break;
			}
			lastPort = newPort;
		}
	}
}

/*
	Adds a page to the notebook, IF an equivalent can not be found.
	If an equivalent is found, this becomes the active page.
	Returns true if the page was really inserted, otherwise false.
*/
bool MainWindow::addPage(BookEntry *page) {
	int i;
	BookEntry *e;

	for (i=0; i < book.get_n_pages(); i++) {
		e = dynamic_cast<BookEntry *>(book.get_nth_page(i));
		
		assert(e != NULL);
	
		if (*page == *e && !BookEntry::useMultipleTabs[page->getID()]) {
			book.set_current_page(i);
			return false;
		}
	}
	
	book.append_page(*page, page->getBox(), page->getLabel());
	page->setParent (&book);
	show_all();
	page->show_all();
	book.set_current_page(-1);

	return true;
}

/*
	Retieves an equivalent of page from the notebook if one exists.
	Otherwise NULL is returned
*/
BookEntry *MainWindow::getPage(BookEntry *page) {
	int i;
	BookEntry *e;

	for (i=0; i < book.get_n_pages(); i++) {
		e = dynamic_cast<BookEntry *>(book.get_nth_page(i));
		
		assert(e != NULL);
	
		if (*page == *e) {
			return e;
		}
	}

	return NULL;
}

BookEntry *MainWindow::findPage (ustring text)
{
	int i;
	BookEntry *e;
	
	for (i=0;i<book.get_n_pages();i++)
	{
		e = dynamic_cast<BookEntry*>(book.get_nth_page (i));
		
		assert (e != NULL);
		
		if (e->getLabel ().get_text () == text)
			return e;
	}
	
	return NULL;
}

void MainWindow::setStatus(string text, int num)
{
	if (num<0 || num>NUM_STATUS-1) return;

	status[num].pop(1);
	if (num == 0)
		status[num].push ("[" + Util::getShortTimeString() + "] " + WUtil::ConvertToUTF8 (text), 1);
	else
		status[num].push (WUtil::ConvertToUTF8 (text), 1);
}

void MainWindow::pubHubsClicked()  {
	PublicHubs *p = new PublicHubs(this);
	if (addPage(p)) manage(p);
	else delete p;
}

void MainWindow::searchClicked()  {
	Search *s = new Search(this);
	if (addPage(s)) manage(s);
	else delete s;
}
void MainWindow::queueClicked ()
{
	if (findPage ("Download queue"))
		return;
	DownloadQueue *q = new DownloadQueue (this);
	if (addPage (q)) manage (q);
	else delete q;
}

void MainWindow::exitClicked()  {
	Dialog d("Exit Wulfor?", *this, true);
	Label text("Do you really wan't to quit?");
	const int YES = 1, NO = 0;
	
	d.get_vbox()->pack_start(text, PACK_EXPAND_WIDGET, 2);
	text.show();
	d.add_button("Yes", YES);
	d.add_button("No", NO);
	
	if (d.run() == YES)
		quit();
}

void MainWindow::settingsClicked() {
	short lastPort = (short)SETTING(IN_PORT);
	int lastConn = SETTING(CONNECTION_TYPE);

	SettingsDialog d(this);
	if (d.run() == SettingsDialog::OK)
	{
		d.saveSettings();
		SettingsManager::getInstance()->save();
	}
	
	//maybe we need to restart the socket
	if (SETTING(CONNECTION_TYPE) != lastConn || SETTING(IN_PORT) != lastPort) {
		Selecter::quit();
		startSocket();
	}
}

void MainWindow::hashClicked()
{
	hashProgress->show_all ();
	showingHash = true;
	hashProgress->run ();
	showingHash = false;
	hashProgress->hide_all ();
}

bool MainWindow::on_delete_event(GdkEventAny *e) {
	exitClicked();
	return true;
}

//From TimerManagerListener
void MainWindow::on(TimerManagerListener::Second, u_int32_t ticks) throw()
{
	int64_t diff = (int64_t)((lastUpdate == 0) ? ticks - 1000 : 
		ticks - lastUpdate);
	int64_t updiff = Socket::getTotalUp() - lastUp;
	int64_t downdiff = Socket::getTotalDown() - lastDown;

	setStatus("H: " + Client::getCounts(), STATUS_HUBS);
	setStatus("S: " + Util::toString(SETTING(SLOTS) -  
		UploadManager::getInstance()->getRunning()) + '/' +
		Util::toString(SETTING(SLOTS)), STATUS_SLOTS);
	setStatus("D: " + Util::formatBytes(Socket::getTotalDown()), STATUS_DL);
	setStatus("U: " + Util::formatBytes(Socket::getTotalUp()), STATUS_UL);
	setStatus("D: " + Util::formatBytes((int64_t)(downdiff*1000)/diff) + "/s (" +
		Util::toString(DownloadManager::getInstance()->getDownloads()) + ")", 
		STATUS_DL_SPEED);
	setStatus("U: " + Util::formatBytes((int64_t)(updiff*1000)/diff) + "/s (" +
		Util::toString(UploadManager::getInstance()->getUploads()) + ")",
		STATUS_UL_SPEED);

	lastUpdate = ticks;
	lastUp = Socket::getTotalUp();
	lastDown = Socket::getTotalDown();

	if (!showingHash)
		hashProgress->updateStats ();
}

//From QueueManagerListener
void MainWindow::on(QueueManagerListener::Finished, QueueItem *item) throw()
{
	ShareBrowser *b;
	
	if ( item->isSet(QueueItem::FLAG_CLIENT_VIEW) &&
		item->isSet(QueueItem::FLAG_USER_LIST))
	{
		b = new ShareBrowser(item, this);
		addPage(b);
	}
}

void MainWindow::quit ()
{
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();
	Selecter::quit();
	Main::quit();
}
