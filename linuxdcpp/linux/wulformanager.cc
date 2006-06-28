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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "wulformanager.hh"
#include "prefix.hh"
#include "publichubs.hh"
#include "downloadqueue.hh"
#include "hub.hh"
#include <iostream>

using namespace std;

WulforManager *WulforManager::manager = NULL;

void WulforManager::start()
{
	dcassert(!manager);
	manager = new WulforManager();
	manager->createMainWindow();
}

void WulforManager::stop()
{
	dcassert(manager);
	pthread_detach(manager->guiThread);
	pthread_detach(manager->clientThread);

	if (manager->mainWin)
		delete manager->mainWin;
}

WulforManager *WulforManager::get()
{
	return manager;
}

WulforManager::WulforManager()
{
	pthread_mutex_init(&guiQueueLock, NULL);
	pthread_mutex_init(&clientQueueLock, NULL);
	pthread_mutex_init(&clientCallLock, NULL);
	pthread_mutex_init(&bookEntryLock, NULL);
	pthread_mutex_init(&dialogEntryLock, NULL);

	sem_init(&guiSem, false, 0);
	sem_init(&clientSem, false, 0);
	pthread_create(&clientThread, NULL, &threadFunc_client, (void *)this);
	pthread_create(&guiThread, NULL, &threadFunc_gui, (void *)this);
	
	mainWin = NULL;
}

WulforManager::~WulforManager()
{
	pthread_mutex_destroy(&guiQueueLock);
	pthread_mutex_destroy(&clientQueueLock);
	pthread_mutex_destroy(&clientCallLock);
	pthread_mutex_destroy(&bookEntryLock);
	pthread_mutex_destroy(&dialogEntryLock);
	
	sem_destroy(&guiSem);
	sem_destroy(&clientSem);
}

void *WulforManager::threadFunc_gui(void *data)
{
	WulforManager *man = (WulforManager *)data;
	man->processGuiQueue();
}

void *WulforManager::threadFunc_client(void *data)
{
	WulforManager *man = (WulforManager *)data;
	man->processClientQueue();
}

void WulforManager::processGuiQueue()
{
	FuncBase *func;

	while (true)
	{
		sem_wait(&guiSem);

		// This must be taken before the queuelock to avoid deadlock.
		gdk_threads_enter();

		pthread_mutex_lock(&guiQueueLock);
		if (guiFuncs.size() > 0)
		{
			func = guiFuncs.front();
			pthread_mutex_unlock(&guiQueueLock);

			func->call();

			pthread_mutex_lock(&guiQueueLock);
			delete func;
			guiFuncs.erase(guiFuncs.begin());
		}
		pthread_mutex_unlock(&guiQueueLock);

		gdk_threads_leave();
	}
}

void WulforManager::processClientQueue()
{
	FuncBase *func;

	while (true)
	{
		sem_wait(&clientSem);

		pthread_mutex_lock(&clientCallLock);
		pthread_mutex_lock(&clientQueueLock);
		if (clientFuncs.size() > 0)
		{
			func = clientFuncs.front();
			pthread_mutex_unlock(&clientQueueLock);

			func->call();

			pthread_mutex_lock(&clientQueueLock);
			delete func;
			clientFuncs.erase(clientFuncs.begin());
		}
		pthread_mutex_unlock(&clientQueueLock);
		pthread_mutex_unlock(&clientCallLock);
	}
}

void WulforManager::dispatchGuiFunc(FuncBase *func)
{
	string id = func->getID();

	pthread_mutex_lock(&guiQueueLock);
	pthread_mutex_lock(&bookEntryLock);
	pthread_mutex_lock(&dialogEntryLock);

	// Make sure we're not adding functions to deleted objects.
	if (id == "Main Window" || id == "Settings" ||
		bookEntries.find(id) != bookEntries.end() ||
		dialogEntries.find(id) != dialogEntries.end())
	{
		guiFuncs.push_back(func);
	}
	else
	{
		delete func;
	}

	pthread_mutex_unlock(&dialogEntryLock);
	pthread_mutex_unlock(&bookEntryLock);
	pthread_mutex_unlock(&guiQueueLock);
	sem_post(&guiSem);
}

void WulforManager::dispatchClientFunc(FuncBase *func)
{
	string id = func->getID();

	pthread_mutex_lock(&clientQueueLock);
	pthread_mutex_lock(&bookEntryLock);
	pthread_mutex_lock(&dialogEntryLock);

	// Make sure we're not adding functions to deleted objects.
	if (id == "Main Window" || id == "Settings" ||
		bookEntries.find(id) != bookEntries.end() ||
		dialogEntries.find(id) != dialogEntries.end())
	{
		clientFuncs.push_back(func);
	}
	else
	{
		delete func;
	}

	pthread_mutex_unlock(&dialogEntryLock);
	pthread_mutex_unlock(&bookEntryLock);
	pthread_mutex_unlock(&clientQueueLock);
	sem_post(&clientSem);
}

MainWindow *WulforManager::createMainWindow()
{
	dcassert(!mainWin);
	mainWin = new MainWindow();

	// Autoconnect and autoopen calls stuff in wulformanager that needs to know
	// what mainWin is, so these cannot be called by the mainwindow constructor.
	typedef Func0<MainWindow> F0;
	F0 *f0 = new F0(mainWin, &MainWindow::autoConnect_client);
	WulforManager::get()->dispatchClientFunc(f0);

	f0 = new F0(mainWin, &MainWindow::autoOpen_gui);
	WulforManager::get()->dispatchGuiFunc(f0);

	return mainWin;
}

MainWindow *WulforManager::getMainWindow()
{
	return mainWin;
}

void WulforManager::closeBookEntry_callback(GtkWidget *widget, gpointer data)
{
	get()->deleteBookEntry_gui((BookEntry *)data);
}

void WulforManager::closeDialogEntry_callback(GtkDialog *dialog, gint response, gpointer data)
{
	get()->hideDialogEntry_gui((DialogEntry *)data);
}

string WulforManager::getPath()
{
#ifdef _DATADIR
	string ret = string(_DATADIR) + PATH_SEPARATOR_STR + "ldcpp";
#else
	char *temp = br_extract_dir(SELFPATH);
	string ret = string(temp);
	free(temp);
#endif
	return ret;
}

BookEntry *WulforManager::getBookEntry_gui(string id, bool raise)
{
	BookEntry *ret = NULL;

	pthread_mutex_lock(&bookEntryLock);
	ret = bookEntries[id];
	pthread_mutex_unlock(&bookEntryLock);
		
	if (ret && raise)
		mainWin->raisePage_gui(ret->getWidget());

	return ret;
}

void WulforManager::insertBookEntry_gui(BookEntry *entry)
{
	// Associates id string to the widget for later retrieval in MainWindow::switchPage_gui()
	g_object_set_data_full(G_OBJECT(entry->getWidget()), "id", g_strdup(entry->getID().c_str()), g_free);

	entry->applyCallback(G_CALLBACK(closeBookEntry_callback));
	mainWin->addPage_gui(entry->getWidget(), entry->getTitle());
	gtk_widget_unref(entry->getWidget());

	pthread_mutex_lock(&bookEntryLock);
	bookEntries[entry->getID()] = entry;
	pthread_mutex_unlock(&bookEntryLock);
}

// This is a callback, so gtk_thread_enter/leave is called automatically.
void WulforManager::deleteBookEntry_gui(BookEntry *entry)
{
	vector<FuncBase *>::iterator fIt;
	//Save a pointer to the page before the entry is deleted
	GtkWidget *notebookPage = entry->getWidget();
	string id = entry->getID();

	pthread_mutex_lock(&clientCallLock);

	// Erase any pending calls to this bookentry.
	pthread_mutex_lock(&clientQueueLock);
	fIt = clientFuncs.begin();
	while (fIt != clientFuncs.end())
	{
		if ((*fIt)->getID() == id)
		{
			delete *fIt;
			clientFuncs.erase(fIt);
		}
		else
			fIt++;
	}
	pthread_mutex_unlock(&clientQueueLock);

	pthread_mutex_lock(&guiQueueLock);
	fIt = guiFuncs.begin();
	while (fIt != guiFuncs.end())
	{
		if ((*fIt)->getID() == id)
		{
			delete *fIt;
			guiFuncs.erase(fIt);
		}
		else
			fIt++;
	}
	pthread_mutex_unlock(&guiQueueLock);

	// Remove the bookentry from the list.
	pthread_mutex_lock(&bookEntryLock);
	if (bookEntries.find(id) != bookEntries.end())
	{
		if (id.substr(0, 5) == "Hub: " || id.substr(0, 4) == "PM: ")
			mainWin->removeWindowItem(notebookPage);
		bookEntries.erase(id);
		delete entry;
	}
	pthread_mutex_unlock(&bookEntryLock);

	pthread_mutex_unlock(&clientCallLock);

	//remove the flap from the notebook
	mainWin->removePage_gui(notebookPage);
}

void WulforManager::deleteAllBookEntries()
{
	while (bookEntries.size() > 0)
		deleteBookEntry_gui(bookEntries.begin()->second);
}

DialogEntry* WulforManager::getDialogEntry_gui(string id)
{
	DialogEntry *ret = NULL;

	pthread_mutex_lock(&dialogEntryLock);
	ret = dialogEntries[id];
	pthread_mutex_unlock(&dialogEntryLock);

	return ret;
}

void WulforManager::insertDialogEntry_gui(DialogEntry *entry)
{
	entry->applyCallback(G_CALLBACK(closeDialogEntry_callback));

	pthread_mutex_lock(&dialogEntryLock);
	dialogEntries[entry->getID()] = entry;
	pthread_mutex_unlock(&dialogEntryLock);
}

void WulforManager::hideDialogEntry_gui(DialogEntry *entry)
{
	if (entry->getDialog())
		gtk_widget_hide(entry->getDialog());
}

// This is a callback, so gtk_thread_enter/leave is called automatically.
void WulforManager::deleteDialogEntry_gui(DialogEntry *entry)
{
	vector<FuncBase *>::iterator it;
	string id = entry->getID();

	pthread_mutex_lock(&clientCallLock);

	// Erase any pending calls to this dialog.
	pthread_mutex_lock(&clientQueueLock);
	it = clientFuncs.begin();
	while (it != clientFuncs.end())
	{
		if ((*it)->getID() == id)
		{
			delete *it;
			clientFuncs.erase(it);
		}
		else
			it++;
	}
	pthread_mutex_unlock(&clientQueueLock);

	pthread_mutex_lock(&guiQueueLock);
	it = guiFuncs.begin();
	while (it != guiFuncs.end())
	{
		if ((*it)->getID() == id)
		{
			delete *it;
			guiFuncs.erase(it);
		}
		else
			it++;
	}
	pthread_mutex_unlock(&guiQueueLock);

	pthread_mutex_lock(&dialogEntryLock);
	if (dialogEntries.find(id) != dialogEntries.end())
	{
		dialogEntries.erase(id);
		delete entry;
	}
	pthread_mutex_unlock(&dialogEntryLock);

	pthread_mutex_unlock(&clientCallLock);	
}

void WulforManager::deleteAllDialogEntries()
{
	while (dialogEntries.size() > 0)
		deleteDialogEntry_gui(dialogEntries.begin()->second);
}

PublicHubs *WulforManager::addPublicHubs_gui()
{
	BookEntry *entry = getBookEntry_gui("Public Hubs");
	if (entry) return dynamic_cast<PublicHubs *>(entry);

	PublicHubs *pubHubs = new PublicHubs();
	insertBookEntry_gui(pubHubs);

	Func0<PublicHubs> *func = new Func0<PublicHubs>(pubHubs, &PublicHubs::downloadList_client);
	dispatchClientFunc(func);

	return pubHubs;
}

DownloadQueue *WulforManager::addDownloadQueue_gui()
{
	BookEntry *entry = getBookEntry_gui("Download Queue");
	if (entry) return dynamic_cast<DownloadQueue*>(entry);

	DownloadQueue *dlQueue = new DownloadQueue();
	insertBookEntry_gui(dlQueue);

	return dlQueue;
}

FavoriteHubs *WulforManager::addFavoriteHubs_gui()
{
	BookEntry *entry = getBookEntry_gui("Favorite Hubs");
	if (entry) return dynamic_cast<FavoriteHubs*>(entry);

	FavoriteHubs *favHubs = new FavoriteHubs();
	insertBookEntry_gui(favHubs);

	return favHubs;
}

Hub *WulforManager::addHub_gui(string address, string nick, string desc, string password)
{
	BookEntry *entry = getBookEntry_gui("Hub: " + address);
	if (entry) return dynamic_cast<Hub *>(entry);

	Hub *hub = new Hub(address);
	mainWin->appendWindowItem(hub->getWidget(), hub->getID());
	insertBookEntry_gui(hub);

	typedef Func4<Hub, string, string, string, string> F4;
	F4 *func = new F4(hub, &Hub::connectClient_client, address, nick, desc, password);
	WulforManager::get()->dispatchClientFunc(func);

	return hub;
}

PrivateMessage *WulforManager::addPrivMsg_gui(User::Ptr user)
{
	BookEntry *entry = getBookEntry_gui("PM: " + WulforUtil::getNicks(user), FALSE);
	if (entry) return dynamic_cast<PrivateMessage *>(entry);

	PrivateMessage *privMsg = new PrivateMessage(user);
	mainWin->appendWindowItem(privMsg->getWidget(), privMsg->getID());
	insertBookEntry_gui(privMsg);

	return privMsg;
}

ShareBrowser *WulforManager::addShareBrowser_gui(User::Ptr user, string file)
{
	BookEntry *entry = getBookEntry_gui("List: " + WulforUtil::getNicks(user));
	if (entry) return dynamic_cast<ShareBrowser *>(entry);

	ShareBrowser *browser = new ShareBrowser(user, file);
	insertBookEntry_gui(browser);

	return browser;
}

Search *WulforManager::addSearch_gui()
{
	Search *s = new Search();
	insertBookEntry_gui(s);

	return s;
}

FinishedTransfers *WulforManager::addFinishedTransfers_gui(string title)
{
	BookEntry *entry = getBookEntry_gui(title);
	if (entry) return dynamic_cast<FinishedTransfers*>(entry);

	FinishedTransfers *ft = new FinishedTransfers(title);
	insertBookEntry_gui(ft);

	return ft;
}

Hash *WulforManager::openHashDialog_gui()
{
	DialogEntry *entry = getDialogEntry_gui("Hash");
	if (entry) return dynamic_cast<Hash *>(entry);

	Hash *h = new Hash();
	insertDialogEntry_gui(h);

	Func0<Hash> *func = new Func0<Hash>(h, &Hash::updateStats_gui);
	dispatchGuiFunc(func);	

	return h;
}

Settings *WulforManager::openSettingsDialog_gui()
{
	Settings *s = new Settings();

	return s;
}
