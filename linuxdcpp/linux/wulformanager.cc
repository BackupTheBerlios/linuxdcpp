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

#include "wulformanager.hh"
#include "prefix.hh"
#include "publichubs.hh"
#include "downloadqueue.hh"
#include "hub.hh"
#include <iostream>

using namespace std;

WulforManager *WulforManager::manager = NULL;

void WulforManager::start() {
	assert(!manager);
	manager = new WulforManager();
}

void WulforManager::stop() {
	assert(manager);

	if (manager->mainWin) delete manager->mainWin;
	for (int i=0; i<manager->bookEntrys.size(); i++)
		delete manager->bookEntrys[i];
}

WulforManager *WulforManager::get() {
	return manager;
}

MainWindow *WulforManager::createMainWindow() {
	mainWin = new MainWindow;
	/*
	autoConnect can't be called from the constructor because it calls 
	WulforManager::addHub, which requires WulforManager::mainWin to be defined. 
	It isn't defined until after the mainWin = new MainWindow statement however.
	*/
	mainWin->autoConnect();
	return mainWin;
}

void WulforManager::closeEntry_callback(GtkWidget *widget, gpointer data) {
	get()->deleteBookEntry((BookEntry *)data);
}

WulforManager::WulforManager() {
	mainWin = NULL;
}

WulforManager::~WulforManager() {
}

string WulforManager::getPath() {
	string ret = br_extract_dir(SELFPATH);
	return ret;
}

MainWindow *WulforManager::getMainWindow() {
	return mainWin;
}

PrivateMessage *WulforManager::getPrivMsg(User::Ptr user)
{
	BookEntry *entry = getBookEntry(PRIVATE_MSG, user->getFullNick(), false);
	if (!entry) return NULL;
	return dynamic_cast<PrivateMessage *>(entry);
}

BookEntry *WulforManager::getBookEntry(int type, string id, bool raise)
{
	BookEntry *ret = NULL;
	vector<BookEntry *>::iterator it;
	
	for (it = bookEntrys.begin(); it != bookEntrys.end(); it++)
		if ((*it)->isEqual(type, id)) ret = *it;
		
	if (ret && raise) {
		mainWin->raisePage(ret->getWidget());
	}

	return ret;
}

/*
BookEntry *WulforManager::getBookEntry(int nr)
{
	BookEntry *ret = NULL;
	
	if (nr < 0 || nr >= bookEntrys.size ())
		return ret;
	
	pthread_mutex_lock(&bookEntryLock);
	ret = bookEntrys[nr];
	pthread_mutex_unlock(&bookEntryLock);
	
	return ret;
}
*/

void WulforManager::deleteBookEntry(BookEntry *entry)
{
	vector<BookEntry *>::iterator it;

	//remove the flap from the notebook
	mainWin->removePage(entry->getWidget());
	
	//remove the bookentry from the list
	for (it = bookEntrys.begin(); it != bookEntrys.end(); it++)
		if ((*it)->isEqual(entry)) {
			delete *it;
			bookEntrys.erase(it);
			break;
		}
}

PublicHubs *WulforManager::addPublicHubs()
{
	BookEntry *entry = getBookEntry(PUBLIC_HUBS, "", true);
	if (entry) return dynamic_cast<PublicHubs *>(entry);

	PublicHubs *pubHubs = new PublicHubs(G_CALLBACK(closeEntry_callback));
	mainWin->addPage(pubHubs->getWidget(), pubHubs->getTitle(), true);
	gtk_widget_unref(pubHubs->getWidget());
	bookEntrys.push_back(pubHubs);
	
	pubHubs->downloadList();
	
	return pubHubs;
}

DownloadQueue *WulforManager::addDownloadQueue()
{
	BookEntry *entry = getBookEntry(DOWNLOAD_QUEUE, "", true);
	if (entry) return dynamic_cast<DownloadQueue*>(entry);

	DownloadQueue *dlQueue = new DownloadQueue(G_CALLBACK(closeEntry_callback));
	mainWin->addPage(dlQueue->getWidget(), dlQueue->getTitle(), true);
	gtk_widget_unref(dlQueue->getWidget());
	bookEntrys.push_back(dlQueue);

	return dlQueue;
}

FavoriteHubs *WulforManager::addFavoriteHubs()
{
	BookEntry *entry = getBookEntry(FAVORITE_HUBS, "", true);
	if (entry) return dynamic_cast<FavoriteHubs*>(entry);

	FavoriteHubs *favHubs = new FavoriteHubs(G_CALLBACK(closeEntry_callback));
	mainWin->addPage(favHubs->getWidget(), favHubs->getTitle(), true);
	gtk_widget_unref(favHubs->getWidget());
	bookEntrys.push_back(favHubs);
	
	return favHubs;
}

Hub *WulforManager::addHub(string address, string nick, string desc, string password)
{
	BookEntry *entry = getBookEntry(HUB, address, true);
	if (entry) return dynamic_cast<Hub *>(entry);

	Hub *hub = new Hub(address, G_CALLBACK(closeEntry_callback));
	mainWin->addPage(hub->getWidget(), hub->getTitle(), true);
	gtk_widget_unref(hub->getWidget());
	bookEntrys.push_back(hub);

	hub->connectClient(address, nick, desc, password);
	
	return hub;
}

PrivateMessage *WulforManager::addPrivMsg(User::Ptr user)
{
	BookEntry *entry = getBookEntry(PRIVATE_MSG, user->getFullNick(), true);
	if (entry) return dynamic_cast<PrivateMessage *>(entry);

	PrivateMessage *privMsg = new PrivateMessage(user, G_CALLBACK(closeEntry_callback));
	mainWin->addPage(privMsg->getWidget(), privMsg->getTitle(), true);
	gtk_widget_unref(privMsg->getWidget());
	bookEntrys.push_back(privMsg);
	
	return privMsg;
}

ShareBrowser *WulforManager::addShareBrowser(User::Ptr user, string file)
{
	BookEntry *entry = getBookEntry(SHARE_BROWSER, user->getFullNick(), true);
	if (entry) return dynamic_cast<ShareBrowser *>(entry);

	ShareBrowser *browser = new ShareBrowser(user, file, G_CALLBACK(closeEntry_callback));
	mainWin->addPage(browser->getWidget(), browser->getTitle(), true);
	gtk_widget_unref(browser->getWidget());
	bookEntrys.push_back(browser);
	
	return browser;
}

Search *WulforManager::addSearch()
{
	BookEntry *entry = getBookEntry(SEARCH, "", true);
	if (entry) return dynamic_cast<Search*>(entry);

	Search *s = new Search(G_CALLBACK(closeEntry_callback));
	mainWin->addPage(s->getWidget(), s->getTitle(), true);
	gtk_widget_unref (s->getWidget());
	bookEntrys.push_back(s);

	return s;
}

FinishedTransfers *WulforManager::addFinishedDownloads()
{
	BookEntry *entry = getBookEntry(FINISHED_DOWNLOADS, "Finished downloads", true);
	if (entry) return dynamic_cast<FinishedTransfers*>(entry);
		
	FinishedTransfers *ft = new FinishedTransfers(
		FINISHED_DOWNLOADS, "Finished downloads", G_CALLBACK(closeEntry_callback));
	mainWin->addPage(ft->getWidget(), ft->getTitle(), true);
	gtk_widget_unref(ft->getWidget());
	bookEntrys.push_back(ft);
	
	return ft;
}

FinishedTransfers *WulforManager::addFinishedUploads()
{
	BookEntry *entry = getBookEntry(FINISHED_UPLOADS, "Finished uploads", true);
	if (entry) return dynamic_cast<FinishedTransfers*>(entry);
		
	FinishedTransfers *ft = new FinishedTransfers(
		FINISHED_UPLOADS, "Finished uploads", G_CALLBACK(closeEntry_callback));
	mainWin->addPage(ft->getWidget(), ft->getTitle(), true);
	gtk_widget_unref (ft->getWidget());
	bookEntrys.push_back(ft);
	
	return ft;
}

void WulforManager::openFileList(string user, string path) const
{
	User::Ptr tmp = new User(user);
	ShareBrowser *browser = new ShareBrowser(tmp, path, G_CALLBACK(closeEntry_callback));
	mainWin->addPage(browser->getWidget(), browser->getTitle(), true);
	gtk_widget_unref(browser->getWidget());
	WulforManager::get()->bookEntrys.push_back(browser);
	//browser->setLabel_gui(user);
}
