/*
* Copyright © 2004-2007 Jens Oknelid, paskharen@gmail.com
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
#include <glib/gi18n.h>
#include "downloadqueue.hh"
#include "favoritehubs.hh"
#include "finishedtransfers.hh"
#include "hashdialog.hh"
#include "hub.hh"
#include "privatemessage.hh"
#include "publichubs.hh"
#include "search.hh"
#include "settingsdialog.hh"
#include "settingsmanager.hh"
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
	dcassert(manager);
	return manager;
}

WulforManager::WulforManager()
{
	if (pthread_mutex_init(&clientCallLock, NULL) != 0)
	{
		perror("Unable to initialize clientCallLock");
		exit(EXIT_FAILURE);
	}
	if (pthread_rwlock_init(&guiQueueLock, NULL) != 0)
	{
		perror("Unable to initialize guiQueueLock");
		exit(EXIT_FAILURE);
	}
	if (pthread_rwlock_init(&clientQueueLock, NULL) != 0)
	{
		perror("Unable to initialize clientQueueLock");
		exit(EXIT_FAILURE);
	}
	if (pthread_rwlock_init(&entryLock, NULL) != 0)
	{
		perror("Unable to initialize entryLock");
		exit(EXIT_FAILURE);
	}

	if (sem_init(&guiSem, false, 0) == -1)
	{
		perror("Unable to initialize guiSem");
		exit(EXIT_FAILURE);
	}
	if (sem_init(&clientSem, false, 0) == -1)
	{
		perror("Unable to initialize clientSem");
		exit(EXIT_FAILURE);
	}

	if (pthread_create(&clientThread, NULL, &threadFunc_client, (void *)this) != 0)
	{
		perror("Unable to create client thread");
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&guiThread, NULL, &threadFunc_gui, (void *)this) != 0)
	{
		perror("Unable to create gui thread");
		exit(EXIT_FAILURE);
	}

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
	sem_destroy(&guiSem);
	sem_destroy(&clientSem);

	pthread_mutex_destroy(&clientCallLock);
	pthread_rwlock_destroy(&guiQueueLock);
	pthread_rwlock_destroy(&clientQueueLock);
	pthread_rwlock_destroy(&entryLock);
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
		if (sem_wait(&guiSem) != 0)
		{
			if (errno == EINTR)
				continue;
			else
				break;
		}

		// This must be taken before the queuelock to avoid deadlock.
		gdk_threads_enter();

		pthread_rwlock_rdlock(&guiQueueLock);
		if (guiFuncs.size() > 0)
		{
			func = guiFuncs.front();
			pthread_rwlock_unlock(&guiQueueLock);

			func->call();

			pthread_rwlock_wrlock(&guiQueueLock);
			delete func;
			guiFuncs.erase(guiFuncs.begin());
		}
		pthread_rwlock_unlock(&guiQueueLock);

		gdk_flush();
		gdk_threads_leave();
	}
}

void WulforManager::processClientQueue()
{
	FuncBase *func;

	while (true)
	{
		if (sem_wait(&clientSem) != 0)
		{
			if (errno == EINTR)
				continue;
			else
				break;
		}

		pthread_mutex_lock(&clientCallLock);
		pthread_rwlock_rdlock(&clientQueueLock);
		if (clientFuncs.size() > 0)
		{
			func = clientFuncs.front();
			pthread_rwlock_unlock(&clientQueueLock);

			func->call();

			pthread_rwlock_wrlock(&clientQueueLock);
			delete func;
			clientFuncs.erase(clientFuncs.begin());
		}
		pthread_rwlock_unlock(&clientQueueLock);
		pthread_mutex_unlock(&clientCallLock);
	}
}

void WulforManager::dispatchGuiFunc(FuncBase *func)
{
	pthread_rwlock_wrlock(&guiQueueLock);
	pthread_rwlock_rdlock(&entryLock);

	// Make sure we're not adding functions to deleted objects.
	if (func->getID() == "Main Window" || entries.find(func->getID()) != entries.end())
		guiFuncs.push_back(func);
	else
		delete func;

	pthread_rwlock_unlock(&entryLock);
	pthread_rwlock_unlock(&guiQueueLock);
	sem_post(&guiSem);
}

void WulforManager::dispatchClientFunc(FuncBase *func)
{
	pthread_rwlock_wrlock(&clientQueueLock);
	pthread_rwlock_rdlock(&entryLock);

	// Make sure we're not adding functions to deleted objects.
	if (func->getID() == "Main Window" || entries.find(func->getID()) != entries.end())
		clientFuncs.push_back(func);
	else
		delete func;

	pthread_rwlock_unlock(&entryLock);
	pthread_rwlock_unlock(&clientQueueLock);
	sem_post(&clientSem);
}

MainWindow *WulforManager::getMainWindow()
{
	return mainWin;
}

gboolean WulforManager::onCloseWindow_gui(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	get()->deleteAllEntries();
	get()->deleteEntry_gui(get()->getMainWindow());
	gtk_main_quit();
	return TRUE;
}

void WulforManager::onCloseBookEntry_gui(GtkWidget *widget, gpointer data)
{
	get()->deleteEntry_gui((Entry *)data);
}

void WulforManager::onCloseDialogEntry_gui(GtkDialog *dialog, gint response, gpointer data)
{
	DialogEntry *entry = (DialogEntry *)data;
	entry->setResponseID(response);

	// We must save the settings if OK was clicked. Can't do this anywhere else. ugh.
	if (response == GTK_RESPONSE_OK && entry->getID() == "Settings")
		dynamic_cast<Settings *>(entry)->saveSettings();

	get()->deleteEntry_gui(entry);
}

string WulforManager::getPath()
{
	return path;
}

BookEntry *WulforManager::getBookEntry_gui(const string &id, bool raise)
{
	BookEntry *ret = NULL;

	pthread_rwlock_rdlock(&entryLock);
	if (entries.find(id) != entries.end())
		ret = dynamic_cast<BookEntry *>(entries[id]);
	pthread_rwlock_unlock(&entryLock);

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

	pthread_rwlock_wrlock(&entryLock);
	entries[entry->getID()] = entry;
	pthread_rwlock_unlock(&entryLock);
}

// This is a callback, so gdk_threads_enter/leave is called automatically.
void WulforManager::deleteEntry_gui(Entry *entry)
{
	string id = entry->getID();
	vector<FuncBase *>::iterator fIt;

	pthread_mutex_lock(&clientCallLock);

	// Erase any pending calls to this bookentry.
	pthread_rwlock_wrlock(&clientQueueLock);
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
	pthread_rwlock_unlock(&clientQueueLock);

	pthread_rwlock_wrlock(&guiQueueLock);
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
	pthread_rwlock_unlock(&guiQueueLock);

	// Remove the bookentry from the list.
	pthread_rwlock_wrlock(&entryLock);
	if (entries.find(id) != entries.end())
		entries.erase(id);
	pthread_rwlock_unlock(&entryLock);

	pthread_mutex_unlock(&clientCallLock);

	delete entry;
	entry = NULL;
}

void WulforManager::deleteAllEntries()
{
	while (entries.size() > 0)
		deleteEntry_gui(entries.begin()->second);
}

DialogEntry* WulforManager::getDialogEntry_gui(const string &id)
{
	DialogEntry *ret = NULL;

	pthread_rwlock_rdlock(&entryLock);
	if (entries.find(id) != entries.end())
		ret = dynamic_cast<DialogEntry *>(entries[id]);
	pthread_rwlock_unlock(&entryLock);

	return ret;
}

void WulforManager::insertDialogEntry_gui(DialogEntry *entry)
{
	entry->applyCallback(G_CALLBACK(onCloseDialogEntry_gui));

	pthread_rwlock_wrlock(&entryLock);
	entries[entry->getID()] = entry;
	pthread_rwlock_unlock(&entryLock);

	gtk_dialog_run(GTK_DIALOG(entry->getContainer()));
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

BookEntry *WulforManager::addHub_gui(const string &address, const string &encoding)
{
	BookEntry *entry = getBookEntry_gui("Hub: " + address);
	if (entry) return entry;

	Hub *hub = new Hub(address);
	mainWin->appendWindowItem(hub->getContainer(), hub->getID());
	insertBookEntry_gui(hub);

	string charset;
	if (address.substr(0, 6) == "adc://" || address.substr(0, 7) == "adcs://")
		charset = "UTF-8";
	else if (encoding.empty())
		charset = WGETS("default-charset");
	else if (encoding == _("System default"))
		charset = Text::getSystemCharset();
	else if (encoding.find(' ', 0) != string::npos)
		charset = encoding.substr(0, encoding.find(' ', 0));
	else
		charset = encoding;

	typedef Func2<Hub, string, string> F2;
	F2 *func = new F2(hub, &Hub::connectClient_client, address, charset);
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

BookEntry *WulforManager::addShareBrowser_gui(User::Ptr user, const string &file, const string &dir, bool raise)
{
	BookEntry *entry = getBookEntry_gui("List: " + WulforUtil::getNicks(user));
	if (entry) return entry;

	ShareBrowser *browser = new ShareBrowser(user, file, dir);
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
