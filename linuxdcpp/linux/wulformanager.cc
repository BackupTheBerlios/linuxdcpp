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

#include "wulformanager.hh"
#include "prefix.hh"
#include "publichubs.hh"
#include "hub.hh"
#include <iostream>

using namespace std;

WulforManager *WulforManager::manager = NULL;

void *WulforManager::threadFunc_gui(void *data) {
	WulforManager *man = (WulforManager *)data;
	man->processGuiQueue();
}

void *WulforManager::threadFunc_client(void *data) {
	WulforManager *man = (WulforManager *)data;
	man->processClientQueue();
}

void WulforManager::processGuiQueue() {
	FuncBase *func;

	while (true) {
		sem_wait(&guiSem);

		pthread_mutex_lock(&guiCallLock);
		pthread_mutex_lock(&guiQueueLock);
		if (guiFuncs.size() == 0) {
			pthread_mutex_unlock(&guiQueueLock);
			pthread_mutex_unlock(&guiCallLock);
			continue;
		}
		func = guiFuncs.front();
		pthread_mutex_unlock(&guiQueueLock);
		
		gdk_threads_enter();
		func->call();
		gdk_threads_leave();

		pthread_mutex_lock(&guiQueueLock);
		delete guiFuncs.front();
		guiFuncs.erase(guiFuncs.begin());
		pthread_mutex_unlock(&guiQueueLock);
		pthread_mutex_unlock(&guiCallLock);
	}
}

void WulforManager::processClientQueue() {
	FuncBase *func;

	while (true) {
		sem_wait(&clientSem);

		pthread_mutex_lock(&clientCallLock);
		pthread_mutex_lock(&clientQueueLock);
		if (clientFuncs.size() == 0) {
			pthread_mutex_unlock(&clientCallLock);
			pthread_mutex_unlock(&clientQueueLock);
			continue;
		}
		func = clientFuncs.front();
		pthread_mutex_unlock(&clientQueueLock);

		func->call();
		
		pthread_mutex_lock(&clientQueueLock);
		delete clientFuncs.front();
		clientFuncs.erase(clientFuncs.begin());
		pthread_mutex_unlock(&clientQueueLock);
		pthread_mutex_unlock(&clientCallLock);
	}
}

void WulforManager::dispatchGuiFunc(FuncBase *func) {
	bool found = false;
	vector<BookEntry *>::iterator it;

	pthread_mutex_lock(&guiQueueLock);

	//make sure we're not adding functions to deleted objects
	if (func->comparePointer((void *)mainWin)) {
		found = true;
	} else {
		pthread_mutex_lock(&bookEntryLock);
		for (it = bookEntrys.begin(); it != bookEntrys.end(); it++)
		if (func->comparePointer((void *)*it)) {
			found = true;
			break;
		}
		pthread_mutex_unlock(&bookEntryLock);
	}
	if (found) guiFuncs.push_back(func);

	pthread_mutex_unlock(&guiQueueLock);
	sem_post(&guiSem);
}

void WulforManager::dispatchClientFunc(FuncBase *func) {
	bool found = false;
	vector<BookEntry *>::iterator it;

	pthread_mutex_lock(&clientQueueLock);

	//make sure we're not adding functions to deleted objects
	if (func->comparePointer((void *)mainWin)) {
		found = true;
	} else {
		pthread_mutex_lock(&bookEntryLock);
		for (it = bookEntrys.begin(); it != bookEntrys.end(); it++)
			if (func->comparePointer((void *)*it)) {
				found = true;
				break;
			}
		pthread_mutex_unlock(&bookEntryLock);
	}
	if (found) clientFuncs.push_back(func);

	pthread_mutex_unlock(&clientQueueLock);
	sem_post(&clientSem);
}

void WulforManager::start() {
	assert(!manager);
	manager = new WulforManager();
}

void WulforManager::stop() {
	assert(manager);
	pthread_detach(manager->guiThread);
	pthread_detach(manager->clientThread);

	if (manager->mainWin) delete manager->mainWin;
	for (int i=0; i<manager->bookEntrys.size(); i++)
		delete manager->bookEntrys[i];
}

WulforManager *WulforManager::get() {
	return manager;
}

void WulforManager::createMainWindow() {
	mainWin = new MainWindow;
	Func0<MainWindow> *func = new Func0<MainWindow>(mainWin, &MainWindow::createWindow_gui);
	dispatchGuiFunc(func);
}

void WulforManager::closeEntry_callback(GtkWidget *widget, gpointer data) {
	get()->deleteBookEntry_gui((BookEntry *)data);
}

WulforManager::WulforManager() {
	pthread_mutex_init(&guiQueueLock, NULL);
	pthread_mutex_init(&guiCallLock, NULL);
	pthread_mutex_init(&clientQueueLock, NULL);
	pthread_mutex_init(&clientCallLock, NULL);
	pthread_mutex_init(&bookEntryLock, NULL);

	sem_init(&guiSem, false, 0);
	sem_init(&clientSem, false, 0);
	pthread_create(&clientThread, NULL, &threadFunc_client, (void *)this);
	pthread_create(&guiThread, NULL, &threadFunc_gui, (void *)this);
	
	mainWin = NULL;
}

WulforManager::~WulforManager() {
	pthread_mutex_destroy(&guiQueueLock);
	pthread_mutex_destroy(&guiCallLock);
	pthread_mutex_destroy(&clientQueueLock);
	pthread_mutex_destroy(&clientCallLock);
	pthread_mutex_destroy(&bookEntryLock);
	
	sem_destroy(&guiSem);
	sem_destroy(&clientSem);
}

string WulforManager::getPath() {
	string ret = br_extract_dir(SELFPATH);
	return ret;
}

MainWindow *WulforManager::getMainWindow() {
	return mainWin;
}

PrivateMessage *WulforManager::getPrivMsg_gui(User::Ptr user) {
	BookEntry *entry = getBookEntry_gui(PRIVATE_MSG, user->getFullNick(), false);
	if (!entry) return NULL;
	return dynamic_cast<PrivateMessage *>(entry);
}

PrivateMessage *WulforManager::getPrivMsg_client(User::Ptr user) {
	BookEntry *entry = getBookEntry_client(PRIVATE_MSG, user->getFullNick(), false);
	if (!entry) return NULL;
	return dynamic_cast<PrivateMessage *>(entry);
}

BookEntry *WulforManager::getBookEntry_gui(int type, string id, bool raise) {
	BookEntry *ret = NULL;
	vector<BookEntry *>::iterator it;
	
	pthread_mutex_lock(&bookEntryLock);
	for (it = bookEntrys.begin(); it != bookEntrys.end(); it++)
		if ((*it)->isEqual(type, id)) ret = *it;
	pthread_mutex_unlock(&bookEntryLock);
		
	if (ret && raise) {
		mainWin->raisePage_gui(ret->getWidget());
	}

	return ret;
}

BookEntry *WulforManager::getBookEntry_client(int type, string id, bool raise) {
	BookEntry *ret = NULL;
	vector<BookEntry *>::iterator it;
	
	pthread_mutex_lock(&bookEntryLock);
	for (it = bookEntrys.begin(); it != bookEntrys.end(); it++)
		if ((*it)->isEqual(type, id)) ret = *it;
	pthread_mutex_unlock(&bookEntryLock);
		
	if (ret && raise) {
		Func1<MainWindow, GtkWidget *> *func = new Func1<MainWindow, GtkWidget*>
			(mainWin, &MainWindow::raisePage_gui, ret->getWidget());
		dispatchGuiFunc(func);
	}

	return ret;
}

void WulforManager::deleteBookEntry_gui(BookEntry *entry) {
	vector<FuncBase *>::iterator fIt;
	
	pthread_mutex_lock(&clientCallLock);
	pthread_mutex_lock(&guiCallLock);

	//erase any pending calls to this bookentry
	fIt = clientFuncs.begin();
	while (fIt != clientFuncs.end())
		for (fIt = clientFuncs.begin(); fIt != clientFuncs.end(); fIt++)
			if ((*fIt)->comparePointer((void *)entry)) {
				delete *fIt;
				clientFuncs.erase(fIt);
				break;
			}

	fIt = guiFuncs.begin();
	while (fIt != guiFuncs.end())
		for (fIt = guiFuncs.begin(); fIt != guiFuncs.end(); fIt++)
			if ((*fIt)->comparePointer((void *)entry)) {
				delete *fIt;
				guiFuncs.erase(fIt);
				break;
			}

	//remove the flap from the notebook
	mainWin->removePage_gui(entry->getWidget());
	
	//remove the bookentry from the list
	pthread_mutex_lock(&bookEntryLock);
	vector<BookEntry *>::iterator bIt;
	for (bIt = bookEntrys.begin(); bIt != bookEntrys.end(); bIt++)
		if ((*bIt)->isEqual(entry)) {
			delete *bIt;
			bookEntrys.erase(bIt);
			break;
		}
	pthread_mutex_unlock(&bookEntryLock);

	pthread_mutex_unlock(&guiCallLock);
	pthread_mutex_unlock(&clientCallLock);
}

PublicHubs *WulforManager::addPublicHubs_gui() {
	BookEntry *entry = getBookEntry_gui(PUBLIC_HUBS, "", true);
	if (entry) return dynamic_cast<PublicHubs *>(entry);

	PublicHubs *pubHubs = new PublicHubs(G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(pubHubs->getWidget(), pubHubs->getTitle(), true);
	gtk_widget_unref(pubHubs->getWidget());
	bookEntrys.push_back(pubHubs);
	
	Func0<PublicHubs> *func = new Func0<PublicHubs>(pubHubs, &PublicHubs::downloadList_client);
	dispatchClientFunc(func);
	
	return pubHubs;
}

Hub *WulforManager::addHub_gui(string address, string nick, string desc, string password) {
	BookEntry *entry = getBookEntry_gui(HUB, address, true);
	if (entry) return dynamic_cast<Hub *>(entry);

	Hub *hub = new Hub(address, G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(hub->getWidget(), hub->getTitle(), true);
	gtk_widget_unref(hub->getWidget());
	bookEntrys.push_back(hub);

	typedef Func4<Hub, string, string, string, string> F4;
	F4 *func = new F4(hub, &Hub::connectClient_client, address, nick, desc, password);
	WulforManager::get()->dispatchClientFunc(func);
	
	return hub;
}

PrivateMessage *WulforManager::addPrivMsg_gui(User::Ptr user) {
	BookEntry *entry = getBookEntry_gui(PRIVATE_MSG, user->getFullNick(), true);
	if (entry) return dynamic_cast<PrivateMessage *>(entry);

	PrivateMessage *privMsg = new PrivateMessage(user, G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(privMsg->getWidget(), privMsg->getTitle(), true);
	gtk_widget_unref(privMsg->getWidget());
	bookEntrys.push_back(privMsg);
	
	return privMsg;
}

ShareBrowser *WulforManager::addShareBrowser_gui(User::Ptr user, string file) {
	BookEntry *entry = getBookEntry_gui(SHARE_BROWSER, user->getFullNick(), true);
	if (entry) return dynamic_cast<ShareBrowser *>(entry);

	ShareBrowser *browser = new ShareBrowser(user, file, G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(browser->getWidget(), browser->getTitle(), true);
	gtk_widget_unref(browser->getWidget());
	bookEntrys.push_back(browser);
	
	return browser;
}

