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

#include <iostream>
#include "downloadqueue.hh"
#include "favoritehubs.hh"
#include "finishedtransfers.hh"
#include "hashdialog.hh"
#include "hub.hh"
#include "privatemessage.hh"
#include "publichubs.hh"
#include "search.hh"
#include "settingsdialog.hh"
#include "sharebrowser.hh"
#include "WulforUtil.hh"

using namespace std;

WulforManager *WulforManager::manager = NULL;

void WulforManager::start()
{
	// Create WulforManager
	dcassert(!manager);
	manager = new WulforManager();

	manager->createMainWindow();
}

void WulforManager::stop()
{
	dcassert(manager);
	pthread_detach(manager->guiThread);
	pthread_detach(manager->clientThread);
	delete manager;
	manager = NULL;
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

	// Determine path to data files
	path = string(_DATADIR) + PATH_SEPARATOR_STR + "linuxdcpp";

	struct stat s;
	if (stat(path.c_str(), &s) == -1)
	{
		cerr << path << " is inaccessible, falling back to current directory instead.\n";
		path = ".";
	}
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

void WulforManager::createMainWindow()
{
	dcassert(!mainWin);
	mainWin = new MainWindow();

	mainWin->applyCallback(GCallback(onCloseWindow_gui));

	// Autoconnect and autoopen calls stuff in wulformanager that needs to know
	// what mainWin is, so these cannot be called by the MainWindow constructor.
	typedef Func0<MainWindow> F0;
	F0 *f0 = new F0(mainWin, &MainWindow::autoConnect_client);
	WulforManager::get()->dispatchClientFunc(f0);

	f0 = new F0(mainWin, &MainWindow::autoOpen_gui);
	WulforManager::get()->dispatchGuiFunc(f0);
}

void WulforManager::deleteMainWindow()
{
	dcassert(mainWin);

	string id = mainWin->getID();
	vector<FuncBase *>::iterator fIt;

	pthread_mutex_lock(&clientCallLock);

	// Erase any pending calls to this entry.
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
			++fIt;
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
			++fIt;
	}
	pthread_mutex_unlock(&guiQueueLock);

	delete mainWin;
	mainWin = NULL;

	pthread_mutex_unlock(&clientCallLock);
}

void *WulforManager::threadFunc_gui(void *data)
{
	WulforManager *man = (WulforManager *)data;
	man->processGuiQueue();
	pthread_exit(NULL);
}

void *WulforManager::threadFunc_client(void *data)
{
	WulforManager *man = (WulforManager *)data;
	man->processClientQueue();
	pthread_exit(NULL);
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
	pthread_mutex_lock(&guiQueueLock);
	pthread_mutex_lock(&bookEntryLock);
	pthread_mutex_lock(&dialogEntryLock);

	string id = func->getID();

	// Make sure we're not adding functions to deleted objects.
	if (id == "Main Window" || bookEntries.find(id) != bookEntries.end() ||
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
	pthread_mutex_lock(&clientQueueLock);
	pthread_mutex_lock(&bookEntryLock);
	pthread_mutex_lock(&dialogEntryLock);

	string id = func->getID();

	// Make sure we're not adding functions to deleted objects.
	if (id == "Main Window" || bookEntries.find(id) != bookEntries.end() ||
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

MainWindow *WulforManager::getMainWindow()
{
	return mainWin;
}

gboolean WulforManager::onCloseWindow_gui(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	get()->deleteAllDialogEntries();
	get()->deleteAllBookEntries();
	get()->deleteMainWindow();
	gtk_main_quit();
	return TRUE;
}

void WulforManager::onCloseBookEntry_gui(GtkWidget *widget, gpointer data)
{
	get()->deleteBookEntry_gui((BookEntry *)data);
}

void WulforManager::onCloseDialogEntry_gui(GtkDialog *dialog, gint response, gpointer data)
{
	DialogEntry *entry = (DialogEntry *)data;
	entry->setResponseID(response);

	// We must save the settings if OK was clicked. Can't do this anywhere else.
	if (response == GTK_RESPONSE_OK && entry->getID() == "Settings")
		((Settings *)entry)->saveSettings();

	get()->deleteDialogEntry_gui(entry);
}

string WulforManager::getPath()
{
	return path;
}

BookEntry *WulforManager::getBookEntry_gui(const string &id, bool raise)
{
	BookEntry *ret = NULL;

	pthread_mutex_lock(&bookEntryLock);
	ret = bookEntries[id];
	pthread_mutex_unlock(&bookEntryLock);

	if (ret && raise)
		mainWin->raisePage_gui(ret->getContainer());

	return ret;
}

void WulforManager::insertBookEntry_gui(BookEntry *entry, bool raise)
{
	// Associates entry to the widget for later retrieval in MainWindow::switchPage_gui()
	g_object_set_data(G_OBJECT(entry->getContainer()), "entry", (gpointer)entry);

	entry->applyCallback(G_CALLBACK(onCloseBookEntry_gui));
	mainWin->addPage_gui(entry->getContainer(), entry->getTitle(), raise);

	pthread_mutex_lock(&bookEntryLock);
	bookEntries[entry->getID()] = entry;
	pthread_mutex_unlock(&bookEntryLock);
}

// This is a callback, so gdk_threads_enter/leave is called automatically.
void WulforManager::deleteBookEntry_gui(BookEntry *entry)
{
	// Save a pointer to the page before the entry is deleted
	GtkWidget *notebookPage = entry->getContainer();
	string id = entry->getID();
	vector<FuncBase *>::iterator fIt;

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
			++fIt;
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
			++fIt;
	}
	pthread_mutex_unlock(&guiQueueLock);

	// Remove the bookentry from the list.
	pthread_mutex_lock(&bookEntryLock);
	if (bookEntries.find(id) != bookEntries.end())
		bookEntries.erase(id);
	pthread_mutex_unlock(&bookEntryLock);

	delete entry;

	pthread_mutex_unlock(&clientCallLock);

	if (id.substr(0, 5) == "Hub: " || id.substr(0, 4) == "PM: ")
		mainWin->removeWindowItem(notebookPage);

	// Remove the flap from the notebook
	mainWin->removePage_gui(notebookPage);
}

void WulforManager::deleteAllBookEntries()
{
	while (bookEntries.size() > 0)
		deleteBookEntry_gui(bookEntries.begin()->second);
}

DialogEntry* WulforManager::getDialogEntry_gui(const string &id)
{
	DialogEntry *ret = NULL;

	pthread_mutex_lock(&dialogEntryLock);
	ret = dialogEntries[id];
	pthread_mutex_unlock(&dialogEntryLock);

	return ret;
}

void WulforManager::insertDialogEntry_gui(DialogEntry *entry)
{
	entry->applyCallback(G_CALLBACK(onCloseDialogEntry_gui));

	pthread_mutex_lock(&dialogEntryLock);
	dialogEntries[entry->getID()] = entry;
	pthread_mutex_unlock(&dialogEntryLock);

	gtk_dialog_run(GTK_DIALOG(entry->getContainer()));
}

// This is a callback, so gdk_threads_enter/leave is called automatically.
void WulforManager::deleteDialogEntry_gui(DialogEntry *entry)
{
	vector<FuncBase *>::iterator it;
	string id = entry->getID();
	GtkWidget *dialog = entry->getContainer();

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
			++it;
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
			++it;
	}
	pthread_mutex_unlock(&guiQueueLock);

	pthread_mutex_lock(&dialogEntryLock);
	if (dialogEntries.find(id) != dialogEntries.end())
		dialogEntries.erase(id);
	pthread_mutex_unlock(&dialogEntryLock);

	delete entry;

	pthread_mutex_unlock(&clientCallLock);

	gtk_widget_destroy(dialog);
}

void WulforManager::deleteAllDialogEntries()
{
	while (dialogEntries.size() > 0)
		deleteDialogEntry_gui(dialogEntries.begin()->second);
}

BookEntry *WulforManager::addPublicHubs_gui()
{
	BookEntry *entry = getBookEntry_gui("Public Hubs");
	if (entry) return entry;

	PublicHubs *pubHubs = new PublicHubs();
	insertBookEntry_gui(pubHubs);

	dispatchClientFunc(new Func0<PublicHubs>(pubHubs, &PublicHubs::downloadList_client));

	return pubHubs;
}

BookEntry *WulforManager::addDownloadQueue_gui()
{
	BookEntry *entry = getBookEntry_gui("Download Queue");
	if (entry) return entry;

	DownloadQueue *dlQueue = new DownloadQueue();
	insertBookEntry_gui(dlQueue);

	return dlQueue;
}

BookEntry *WulforManager::addFavoriteHubs_gui()
{
	BookEntry *entry = getBookEntry_gui("Favorite Hubs");
	if (entry) return entry;

	FavoriteHubs *favHubs = new FavoriteHubs();
	insertBookEntry_gui(favHubs);

	return favHubs;
}

BookEntry *WulforManager::addHub_gui(const string &address, const string &nick, const string &desc, const string &password)
{
	BookEntry *entry = getBookEntry_gui("Hub: " + address);
	if (entry) return entry;

	Hub *hub = new Hub(address);
	mainWin->appendWindowItem(hub->getContainer(), hub->getID());
	insertBookEntry_gui(hub);

	typedef Func4<Hub, string, string, string, string> F4;
	F4 *func = new F4(hub, &Hub::connectClient_client, address, nick, desc, password);
	dispatchClientFunc(func);

	return hub;
}

BookEntry *WulforManager::addPrivMsg_gui(User::Ptr user, bool raise)
{
	BookEntry *entry = getBookEntry_gui("PM: " + WulforUtil::getNicks(user), FALSE);
	if (entry) return entry;

	PrivateMessage *privMsg = new PrivateMessage(user);
	mainWin->appendWindowItem(privMsg->getContainer(), privMsg->getID());
	insertBookEntry_gui(privMsg, raise);

	return privMsg;
}

BookEntry *WulforManager::addShareBrowser_gui(User::Ptr user, const string &file, bool raise)
{
	BookEntry *entry = getBookEntry_gui("List: " + WulforUtil::getNicks(user));
	if (entry) return entry;

	ShareBrowser *browser = new ShareBrowser(user, file);
	insertBookEntry_gui(browser, raise);

	return browser;
}

BookEntry *WulforManager::addSearch_gui()
{
	Search *s = new Search();
	insertBookEntry_gui(s);

	return s;
}

BookEntry *WulforManager::addFinishedTransfers_gui(const string &title)
{
	BookEntry *entry = getBookEntry_gui(title);
	if (entry) return entry;

	FinishedTransfers *ft = new FinishedTransfers(title);
	insertBookEntry_gui(ft);

	return ft;
}

int WulforManager::openHashDialog_gui()
{
	Hash *h = new Hash();
	insertDialogEntry_gui(h);

	return Hash::getResponseID();
}

int WulforManager::openSettingsDialog_gui()
{
	Settings *s = new Settings();
	insertDialogEntry_gui(s);

	return Settings::getResponseID();
}
