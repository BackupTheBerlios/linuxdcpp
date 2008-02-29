/*
* Copyright © 2004-2008 Jens Oknelid, paskharen@gmail.com
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
	abort = FALSE;

	// Initialize sempahore variables
	guiCondValue = 0;
	clientCondValue = 0;
	guiCond = g_cond_new();
	clientCond = g_cond_new();
	guiCondMutex = g_mutex_new();
	clientCondMutex = g_mutex_new();

	clientCallMutex = g_mutex_new();
	guiQueueMutex = g_mutex_new();
	clientQueueMutex = g_mutex_new();
	g_static_rw_lock_init(&entryMutex);

	GError *error = NULL;
	guiThread = g_thread_create(threadFunc_gui, (gpointer)this, TRUE, &error);
	if (error != NULL)
	{
		cerr << "Unable to create gui thread: " << error->message << endl;
		g_error_free(error);
		exit(EXIT_FAILURE);
	}

	g_clear_error(&error);

	clientThread = g_thread_create(threadFunc_client, (gpointer)this, TRUE, &error);
	if (error != NULL)
	{
		cerr << "Unable to create client thread: " << error->message << endl;
		g_error_free(error);
		exit(EXIT_FAILURE);
	}

	mainWin = NULL;

	// Determine path to data files
	path = string(_DATADIR) + PATH_SEPARATOR_STR + "linuxdcpp";
	if (!g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
	{
		cerr << path << " is inaccessible, falling back to current directory instead.\n";
		path = ".";
	}
}

WulforManager::~WulforManager()
{
	abort = TRUE;

	g_mutex_lock(guiCondMutex);
	guiCondValue++;
	g_cond_signal(guiCond);
	g_mutex_unlock(guiCondMutex);

	g_mutex_lock(clientCondMutex);
	clientCondValue++;
	g_cond_signal(clientCond);
	g_mutex_unlock(clientCondMutex);

	g_thread_join(guiThread);
	g_thread_join(clientThread);

	g_cond_free(guiCond);
	g_cond_free(clientCond);
	g_mutex_free(clientCondMutex);
	g_mutex_free(guiCondMutex);
	g_mutex_free(clientCallMutex);
	g_mutex_free(guiQueueMutex);
	g_mutex_free(clientQueueMutex);
	g_static_rw_lock_free(&entryMutex);
}

void WulforManager::createMainWindow()
{
	dcassert(!mainWin);
	mainWin = new MainWindow();

	// Autoconnect and autoopen calls stuff in wulformanager that needs to know
	// what mainWin is, so these cannot be called by the MainWindow constructor.
	typedef Func0<MainWindow> F0;
	F0 *f0 = new F0(mainWin, &MainWindow::autoConnect_client);
	WulforManager::get()->dispatchClientFunc(f0);

	f0 = new F0(mainWin, &MainWindow::autoOpen_gui);
	WulforManager::get()->dispatchGuiFunc(f0);
}

gpointer WulforManager::threadFunc_gui(gpointer data)
{
	WulforManager *man = (WulforManager *)data;
	man->processGuiQueue();
	return NULL;
}

gpointer WulforManager::threadFunc_client(gpointer data)
{
	WulforManager *man = (WulforManager *)data;
	man->processClientQueue();
	return NULL;
}

void WulforManager::processGuiQueue()
{
	FuncBase *func;

	while (!abort)
	{
		g_mutex_lock(guiCondMutex);
		while (guiCondValue < 1)
			g_cond_wait(guiCond, guiCondMutex);
		guiCondValue--;
		g_mutex_unlock(guiCondMutex);

		// This must be taken before the queuelock to avoid deadlock.
		gdk_threads_enter();

		g_mutex_lock(guiQueueMutex);
		while (guiFuncs.size() > 0)
		{
			func = guiFuncs.front();
			guiFuncs.erase(guiFuncs.begin());
			g_mutex_unlock(guiQueueMutex);

			func->call();
			delete func;

			g_mutex_lock(guiQueueMutex);
		}
		g_mutex_unlock(guiQueueMutex);

		gdk_flush();
		gdk_threads_leave();
	}

	g_thread_exit(NULL);
}

void WulforManager::processClientQueue()
{
	FuncBase *func;

	while (!abort)
	{
		g_mutex_lock(clientCondMutex);
		while (clientCondValue < 1)
			g_cond_wait(clientCond, clientCondMutex);
		clientCondValue--;
		g_mutex_unlock(clientCondMutex);

		g_mutex_lock(clientCallMutex);
		g_mutex_lock(clientQueueMutex);
		while (clientFuncs.size() > 0)
		{
			func = clientFuncs.front();
			clientFuncs.erase(clientFuncs.begin());
			g_mutex_unlock(clientQueueMutex);

			func->call();
			delete func;

			g_mutex_lock(clientQueueMutex);
		}
		g_mutex_unlock(clientQueueMutex);
		g_mutex_unlock(clientCallMutex);
	}

	g_thread_exit(NULL);
}

void WulforManager::dispatchGuiFunc(FuncBase *func)
{
	g_static_rw_lock_reader_lock(&entryMutex);

	// Make sure we're not adding functions to deleted objects.
	if (func->getID() == "Main Window" || entries.find(func->getID()) != entries.end())
	{
		g_mutex_lock(guiQueueMutex);
		guiFuncs.push_back(func);
		g_mutex_unlock(guiQueueMutex);

		g_mutex_lock(guiCondMutex);
		guiCondValue++;
		g_cond_signal(guiCond);
		g_mutex_unlock(guiCondMutex);
	}
	else
		delete func;

	g_static_rw_lock_reader_unlock(&entryMutex);
}

void WulforManager::dispatchClientFunc(FuncBase *func)
{
	g_static_rw_lock_reader_lock(&entryMutex);

	// Make sure we're not adding functions to deleted objects.
	if (func->getID() == "Main Window" || entries.find(func->getID()) != entries.end())
	{
		g_mutex_lock(clientQueueMutex);
		clientFuncs.push_back(func);
		g_mutex_unlock(clientQueueMutex);

		g_mutex_lock(clientCondMutex);
		clientCondValue++;
		g_cond_signal(clientCond);
		g_mutex_unlock(clientCondMutex);
	}
	else
		delete func;

	g_static_rw_lock_reader_unlock(&entryMutex);
}

MainWindow *WulforManager::getMainWindow()
{
	dcassert(mainWin);
	return mainWin;
}

string WulforManager::getPath()
{
	return path;
}

BookEntry *WulforManager::getBookEntry_gui(const string &id, bool raise)
{
	BookEntry *ret = NULL;

	g_static_rw_lock_reader_lock(&entryMutex);
	if (entries.find(id) != entries.end())
		ret = dynamic_cast<BookEntry *>(entries[id]);
	g_static_rw_lock_reader_unlock(&entryMutex);

	if (ret && raise)
		mainWin->raisePage_gui(ret->getContainer());

	return ret;
}

void WulforManager::insertEntry_gui(Entry *entry)
{
	g_static_rw_lock_writer_lock(&entryMutex);
	entries[entry->getID()] = entry;
	g_static_rw_lock_writer_unlock(&entryMutex);
}

// Should be called from a callback, so gdk_threads_enter/leave is called automatically.
void WulforManager::deleteEntry_gui(Entry *entry)
{
	const string &id = entry->getID();
	vector<FuncBase *>::iterator fIt;

	g_mutex_lock(clientCallMutex);

	// Erase any pending calls to this bookentry.
	g_mutex_lock(clientQueueMutex);
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
	g_mutex_unlock(clientQueueMutex);

	g_mutex_lock(guiQueueMutex);
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
	g_mutex_unlock(guiQueueMutex);

	// Remove the bookentry from the list.
	g_static_rw_lock_writer_lock(&entryMutex);
	if (entries.find(id) != entries.end())
		entries.erase(id);
	g_static_rw_lock_writer_unlock(&entryMutex);

	g_mutex_unlock(clientCallMutex);

	delete entry;
	entry = NULL;
}

// Should be called from a callback, so gdk_threads_enter/leave is called automatically.
void WulforManager::deleteAllEntries()
{
	while (entries.size() > 0)
		deleteEntry_gui(entries.begin()->second);

	WulforManager::get()->deleteEntry_gui(mainWin);
}

DialogEntry* WulforManager::getDialogEntry_gui(const string &id)
{
	DialogEntry *ret = NULL;

	g_static_rw_lock_reader_lock(&entryMutex);
	if (entries.find(id) != entries.end())
		ret = dynamic_cast<DialogEntry *>(entries[id]);
	g_static_rw_lock_reader_unlock(&entryMutex);

	return ret;
}

BookEntry *WulforManager::addPublicHubs_gui()
{
	BookEntry *entry = getBookEntry_gui(_("Public Hubs"));

	if (entry == NULL)
		entry = new PublicHubs();

	return entry;
}

BookEntry *WulforManager::addDownloadQueue_gui()
{
	BookEntry *entry = getBookEntry_gui(_("Download Queue"));

	if (entry == NULL)
		entry = new DownloadQueue();

	return entry;
}

BookEntry *WulforManager::addFavoriteHubs_gui()
{
	BookEntry *entry = getBookEntry_gui(_("Favorite Hubs"));

	if (entry == NULL)
		entry = new FavoriteHubs();

	return entry;
}

BookEntry *WulforManager::addHub_gui(const string &address, const string &encoding)
{
	BookEntry *entry = getBookEntry_gui(_("Hub: ") + address);

	if (entry == NULL)
		entry = new Hub(address, encoding);

	return entry;
}

BookEntry *WulforManager::addPrivMsg_gui(const std::string &cid, bool raise)
{
	BookEntry *entry = getBookEntry_gui(_("PM: ") + WulforUtil::getNicks(cid), FALSE);

	if (entry == NULL)
		entry = new PrivateMessage(cid, raise);

	return entry;
}

BookEntry *WulforManager::addShareBrowser_gui(User::Ptr user, const string &file, bool raise)
{
	BookEntry *entry = getBookEntry_gui(_("List: ") + WulforUtil::getNicks(user), raise);

	if (entry == NULL)
		entry = new ShareBrowser(user, file, raise);

	return entry;
}

BookEntry *WulforManager::addSearch_gui()
{
	BookEntry *entry = new Search();

	return entry;
}

BookEntry *WulforManager::addFinishedTransfers_gui(const string &title)
{
	BookEntry *entry = getBookEntry_gui(title);

	if (entry == NULL)
		entry = new FinishedTransfers(title);

	return entry;
}

gint WulforManager::openHashDialog_gui()
{
	Hash *h = new Hash();
	gint response = h->run();

	return response;
}

gint WulforManager::openSettingsDialog_gui()
{
	Settings *s = new Settings();
	gint response = s->run();

	return response;
}

