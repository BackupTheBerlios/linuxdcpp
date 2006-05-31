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

		//this must be taken before the queuelock to avoid deadlock
		gdk_threads_enter();

		pthread_mutex_lock(&guiQueueLock);
		if (guiFuncs.size() == 0)
		{
			pthread_mutex_unlock(&guiQueueLock);
			gdk_threads_leave();
			continue;
		}
		func = guiFuncs.front();
		pthread_mutex_unlock(&guiQueueLock);

		func->call();

		pthread_mutex_lock(&guiQueueLock);
		delete func;
		guiFuncs.erase(guiFuncs.begin());
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
		if (clientFuncs.size() == 0)
		{
			pthread_mutex_unlock(&clientCallLock);
			pthread_mutex_unlock(&clientQueueLock);
			continue;
		}
		func = clientFuncs.front();
		pthread_mutex_unlock(&clientQueueLock);

		func->call();

		pthread_mutex_lock(&clientQueueLock);
		delete func;
		clientFuncs.erase(clientFuncs.begin());
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
	mainWin = new MainWindow;
	//Autoconnect and autoopen calls stuff in wulformanager that needs to know
	//what mainWin is, so these cannot be called by the mainwindow constructor.
	mainWin->autoConnect_client();
	mainWin->autoOpen_gui();
	return mainWin;
}

MainWindow *WulforManager::getMainWindow()
{
	return mainWin;
}

void WulforManager::closeEntry_callback(GtkWidget *widget, gpointer data)
{
	get()->deleteBookEntry_gui((BookEntry *)data);
}

void WulforManager::dialogCloseEntry_callback(GtkDialog *dialog, gint response, gpointer data)
{
	get()->hideDialogEntry_gui((DialogEntry *)data);
}

string WulforManager::getPath()
{
#ifdef _DATADIR
	string ret = string(_DATADIR) + PATH_SEPARATOR + "ldcpp";
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

BookEntry *WulforManager::getBookEntry_client(string id, bool raise)
{
	BookEntry *ret = NULL;
	
	pthread_mutex_lock(&bookEntryLock);
	ret = bookEntries[id];
	pthread_mutex_unlock(&bookEntryLock);
		
	if (ret && raise)
	{
		Func1<MainWindow, GtkWidget *> *func = new Func1<MainWindow, GtkWidget*>
			(mainWin, &MainWindow::raisePage_gui, ret->getWidget());
		dispatchGuiFunc(func);
	}

	return ret;
}

void WulforManager::insertBookEntry(BookEntry *entry)
{
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

void WulforManager::insertDialogEntry(DialogEntry *entry)
{
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

PrivateMessage *WulforManager::getPrivMsg_gui(User::Ptr user)
{
	BookEntry *entry = getBookEntry_gui("PM: " + user->getNick());
	if (!entry) return NULL;
	return dynamic_cast<PrivateMessage *>(entry);
}

PrivateMessage *WulforManager::getPrivMsg_client(User::Ptr user)
{
	BookEntry *entry = getBookEntry_client("PM: " + user->getNick());
	if (!entry) return NULL;
	return dynamic_cast<PrivateMessage *>(entry);
}

PublicHubs *WulforManager::addPublicHubs_gui()
{
	BookEntry *entry = getBookEntry_gui("Public Hubs");
	if (entry) return dynamic_cast<PublicHubs *>(entry);

	PublicHubs *pubHubs = new PublicHubs(G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(pubHubs->getWidget(), pubHubs->getTitle(), true);
	gtk_widget_unref(pubHubs->getWidget());
	insertBookEntry(pubHubs);

	Func0<PublicHubs> *func = new Func0<PublicHubs>(pubHubs, &PublicHubs::downloadList_client);
	dispatchClientFunc(func);

	return pubHubs;
}

DownloadQueue *WulforManager::addDownloadQueue_gui()
{
	BookEntry *entry = getBookEntry_gui("Download Queue");
	if (entry) return dynamic_cast<DownloadQueue*>(entry);

	DownloadQueue *dlQueue = new DownloadQueue(G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(dlQueue->getWidget(), dlQueue->getTitle());
	gtk_widget_unref(dlQueue->getWidget());
	insertBookEntry(dlQueue);

	return dlQueue;
}

FavoriteHubs *WulforManager::addFavoriteHubs_gui()
{
	BookEntry *entry = getBookEntry_gui("Favorite Hubs");
	if (entry) return dynamic_cast<FavoriteHubs*>(entry);

	FavoriteHubs *favHubs = new FavoriteHubs(G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(favHubs->getWidget(), favHubs->getTitle());
	gtk_widget_unref(favHubs->getWidget());
	insertBookEntry(favHubs);

	return favHubs;
}

Settings *WulforManager::openSettingsDialog_gui()
{
	Settings *s = new Settings();

	return s;
}

Hub *WulforManager::addHub_gui(string address, string nick, string desc, string password)
{
	BookEntry *entry = getBookEntry_gui("Hub: " + address);
	if (entry) return dynamic_cast<Hub *>(entry);

	Hub *hub = new Hub(address, G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(hub->getWidget(), hub->getTitle());
	mainWin->appendWindowItem(hub->getWidget(), "Hub: " + address);
	gtk_widget_unref(hub->getWidget());
	insertBookEntry(hub);

	typedef Func4<Hub, string, string, string, string> F4;
	F4 *func = new F4(hub, &Hub::connectClient_client, address, nick, desc, password);
	WulforManager::get()->dispatchClientFunc(func);

	return hub;
}

PrivateMessage *WulforManager::addPrivMsg_gui(User::Ptr user)
{
	BookEntry *entry = getBookEntry_gui("PM: " + user->getNick());
	if (entry) return dynamic_cast<PrivateMessage *>(entry);

	PrivateMessage *privMsg = new PrivateMessage(user, G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(privMsg->getWidget(), privMsg->getTitle());
	mainWin->appendWindowItem(privMsg->getWidget(), "PM: " + user->getNick());
	gtk_widget_unref(privMsg->getWidget());
	insertBookEntry(privMsg);

	return privMsg;
}

ShareBrowser *WulforManager::addShareBrowser_gui(User::Ptr user, string file)
{
	BookEntry *entry = getBookEntry_gui("List: " + user->getNick());
	if (entry) return dynamic_cast<ShareBrowser *>(entry);

	ShareBrowser *browser = new ShareBrowser(user, file, G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(browser->getWidget(), browser->getTitle());
	gtk_widget_unref(browser->getWidget());
	insertBookEntry(browser);

	return browser;
}

Search *WulforManager::addSearch_gui()
{
	Search *s = new Search(G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(s->getWidget(), s->getTitle());
	gtk_widget_unref(s->getWidget());
	insertBookEntry(s);

	return s;
}

Hash *WulforManager::openHashDialog_gui()
{
	DialogEntry *entry = getDialogEntry_gui("Hash");
	if (entry) return dynamic_cast<Hash *>(entry);

	Hash *h = new Hash();
	h->applyCallback(G_CALLBACK(dialogCloseEntry_callback));
	insertDialogEntry(h);
	Func0<Hash> *func = new Func0<Hash>(h, &Hash::updateStats_gui);
	dispatchGuiFunc(func);	

	return h;
}

FinishedTransfers *WulforManager::addFinishedTransfers_gui(string title)
{
	BookEntry *entry = getBookEntry_gui(title);
	if (entry) return dynamic_cast<FinishedTransfers*>(entry);

	FinishedTransfers *ft = new FinishedTransfers(title, G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(ft->getWidget(), ft->getTitle());
	gtk_widget_unref(ft->getWidget());
	insertBookEntry(ft);

	return ft;
}
