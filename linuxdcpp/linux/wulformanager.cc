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

		//this must be taken before the queuelock to avoid deadlock
		gdk_threads_enter();

		pthread_mutex_lock(&guiQueueLock);
		if (guiFuncs.size() == 0) {
			pthread_mutex_unlock(&guiQueueLock);
			gdk_threads_leave();
			continue;
		}
		func = guiFuncs.front();
		pthread_mutex_unlock(&guiQueueLock);
		
		func->call();

		pthread_mutex_lock(&guiQueueLock);
		delete guiFuncs.front();
		guiFuncs.erase(guiFuncs.begin());
		pthread_mutex_unlock(&guiQueueLock);

		gdk_threads_leave();
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
		if (!found)
		{
			if (dialogEntry)
			{
				if (func->comparePointer((void *)dialogEntry))
					found = true;
			}

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
		if (!found)
		{
			if (func->comparePointer((void *)dialogEntry))
				found = true;
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

	if (manager->mainWin)
		delete manager->mainWin;
}

WulforManager *WulforManager::get() {
	return manager;
}

MainWindow *WulforManager::createMainWindow() {
	mainWin = new MainWindow;
	//Autoconnect and autoopen calls stuff in wulformanager that needs to know
	//what mainWin is, so these cannot be called by the mainwindow constructor.
	mainWin->autoConnect_client();
	mainWin->autoOpen_gui();
	return mainWin;
}

void WulforManager::closeEntry_callback(GtkWidget *widget, gpointer data) {
	get()->deleteBookEntry_gui((BookEntry *)data);
}

void WulforManager::dialogCloseEntry_callback(GtkWidget *widget, gpointer data) {
	get()->deleteDialogEntry_gui();
}

WulforManager::WulforManager() {
	pthread_mutex_init(&guiQueueLock, NULL);
	pthread_mutex_init(&clientQueueLock, NULL);
	pthread_mutex_init(&clientCallLock, NULL);
	pthread_mutex_init(&bookEntryLock, NULL);

	sem_init(&guiSem, false, 0);
	sem_init(&clientSem, false, 0);
	pthread_create(&clientThread, NULL, &threadFunc_client, (void *)this);
	pthread_create(&guiThread, NULL, &threadFunc_gui, (void *)this);
	
	mainWin = NULL;
	dialogEntry = NULL;
}

WulforManager::~WulforManager() {
	pthread_mutex_destroy(&guiQueueLock);
	pthread_mutex_destroy(&clientQueueLock);
	pthread_mutex_destroy(&clientCallLock);
	pthread_mutex_destroy(&bookEntryLock);
	
	sem_destroy(&guiSem);
	sem_destroy(&clientSem);
}

string WulforManager::getPath() {
#ifdef _LIBDIR
	string ret = _LIBDIR;
	ret += "/ldcpp";
#else
	string ret = br_extract_dir(SELFPATH);
#endif
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

BookEntry *WulforManager::getBookEntry_gui(int nr) {
	BookEntry *ret = NULL;
	
	if (nr < 0 || nr >= bookEntrys.size ())
		return ret;
	
	pthread_mutex_lock(&bookEntryLock);
	ret = bookEntrys[nr];
	pthread_mutex_unlock(&bookEntryLock);
	
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
	vector<BookEntry *>::iterator bIt;
	//Save a pointer to the page before the entry is deleted
	GtkWidget *notebookPage = entry->getWidget();
	
	//pthread enter is called by gtk for callbacks
	//so no need to call it here
	pthread_mutex_lock(&clientCallLock);

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

	//remove the bookentry from the list
	pthread_mutex_lock(&bookEntryLock);
	for (bIt = bookEntrys.begin(); bIt != bookEntrys.end(); bIt++)
		if ((*bIt)->isEqual(entry)) {
			delete entry;
			bookEntrys.erase(bIt);
			break;
		}
	pthread_mutex_unlock(&bookEntryLock);

	pthread_mutex_unlock(&clientCallLock);

	//remove the flap from the notebook
	mainWin->removePage_gui(notebookPage);
}

void WulforManager::deleteAllBookEntries()
{
	for (int i = 0; i < bookEntrys.size(); i++)
		deleteBookEntry_gui(bookEntrys[i]);
}

void WulforManager::deleteDialogEntry_gui()
{
	vector<FuncBase *>::iterator it;
	
	//this is a callback, so gtk_thread_enter/leave is called automaticly
	pthread_mutex_lock(&clientCallLock);

	//erase any pending calls to this bookentry
	it = clientFuncs.begin();
	while (it != clientFuncs.end())
		for (it = clientFuncs.begin(); it != clientFuncs.end(); it++)
			if ((*it)->comparePointer((void *)dialogEntry)) {
				delete *it;
				clientFuncs.erase(it);
				break;
			}

	it = guiFuncs.begin();
	while (it != guiFuncs.end())
		for (it = guiFuncs.begin(); it != guiFuncs.end(); it++)
			if ((*it)->comparePointer((void *)dialogEntry)) {
				delete *it;
				guiFuncs.erase(it);
				break;
			}

	// Hide the dialog
	if (dialogEntry->getDialog())
	{
		gtk_widget_hide(dialogEntry->getDialog());
		dialogEntry->setDialog(NULL);
	}

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

DownloadQueue *WulforManager::addDownloadQueue_gui() {
	BookEntry *entry = getBookEntry_gui (DOWNLOAD_QUEUE, "", true);
	if (entry) return dynamic_cast<DownloadQueue*>(entry);

	DownloadQueue *dlQueue = new DownloadQueue(G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(dlQueue->getWidget(), dlQueue->getTitle(), true);
	gtk_widget_unref(dlQueue->getWidget());
	bookEntrys.push_back(dlQueue);

	return dlQueue;
}

FavoriteHubs *WulforManager::addFavoriteHubs_gui() {
	BookEntry *entry = getBookEntry_gui (FAVORITE_HUBS, "", true);
	if (entry) return dynamic_cast<FavoriteHubs*>(entry);

	FavoriteHubs *favHubs = new FavoriteHubs (G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(favHubs->getWidget (), favHubs->getTitle (), true);
	gtk_widget_unref (favHubs->getWidget ());
	bookEntrys.push_back(favHubs);
	
	return favHubs;
}

Settings *WulforManager::openSettingsDialog_gui() {
	Settings *s = new Settings();

	return s;
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

Search *WulforManager::addSearch_gui() {
	BookEntry *entry = getBookEntry_gui (SEARCH, "", true);
	if (entry) return dynamic_cast<Search*>(entry);

	Search *s = new Search (G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(s->getWidget(), s->getTitle(), true);
	gtk_widget_unref (s->getWidget());
	bookEntrys.push_back(s);

	return s;
}

Hash *WulforManager::openHashDialog_gui() {
	Hash *h = new Hash();
	h->applyCallback(G_CALLBACK(dialogCloseEntry_callback));
	dialogEntry = h;
	Func0<Hash> *func = new Func0<Hash> (h, &Hash::updateStats_gui);
	dispatchGuiFunc (func);	
	
	return h;
}

FinishedTransfers *WulforManager::addFinishedTransfers_gui(int type, string title)
{
	BookEntry *entry = getBookEntry_gui (type, title, true);
	if (entry) return dynamic_cast<FinishedTransfers*>(entry);
		
	FinishedTransfers *ft = new FinishedTransfers (type, title, G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(ft->getWidget (), ft->getTitle (), true);
	gtk_widget_unref (ft->getWidget ());
	bookEntrys.push_back(ft);
	
	return ft;
}
