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

gboolean WulforManager::guiCallback(gpointer data) {
	WulforManager *man = (WulforManager *)data;
	man->callGuiFunc();
	return FALSE;
}

void WulforManager::processGuiQueue() {
	while (true) {
		sem_wait(&guiSem);
		g_timeout_add(0, &guiCallback, (gpointer)this);		
	}
}

void WulforManager::processClientQueue() {
	while (true) {
		sem_wait(&clientSem);

		if (clientFuncs.size() == 0) continue;

		pthread_mutex_lock(&clientLock);
		clientFuncs.front()->call();
		delete clientFuncs.front();
		clientFuncs.erase(clientFuncs.begin());
		pthread_mutex_unlock(&clientLock);
	}
}

void WulforManager::callGuiFunc() {
	if (guiFuncs.size() == 0) return;

	pthread_mutex_lock(&guiLock);
	guiFuncs.front()->call();
	delete guiFuncs.front();
	guiFuncs.erase(guiFuncs.begin());
	pthread_mutex_unlock(&guiLock);
}

void WulforManager::dispatchGuiFunc(FuncBase *func) {
	bool found = false;
	vector<BookEntry *>::iterator it;

	pthread_mutex_lock(&guiLock);

	//make sure we're not adding functions to deleted objects
	if (func->comparePointer((void *)mainWin)) {
		found = true;
	} else {
	for (it = bookEntrys.begin(); it != bookEntrys.end(); it++)
		if (func->comparePointer((void *)*it)) {
			found = true;
			break;
		}
	}
	if (found) guiFuncs.push_back(func);

	pthread_mutex_unlock(&guiLock);
	sem_post(&guiSem);
}

void WulforManager::dispatchClientFunc(FuncBase *func) {
	bool found = false;
	vector<BookEntry *>::iterator it;

	pthread_mutex_lock(&clientLock);

	//make sure we're not adding functions to deleted objects
	if (func->comparePointer((void *)mainWin)) {
		found = true;
	} else {
		for (it = bookEntrys.begin(); it != bookEntrys.end(); it++)
			if (func->comparePointer((void *)*it)) {
				found = true;
				break;
			}
	}
	if (found) clientFuncs.push_back(func);

	pthread_mutex_unlock(&clientLock);
	sem_post(&clientSem);
}

void WulforManager::start() {
	assert(!manager);
	manager = new WulforManager();
}

void WulforManager::stop() {
	assert(manager);
	delete manager;
	manager = NULL;
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
	pthread_mutex_init(&guiLock, NULL);
	pthread_mutex_init(&clientLock, NULL);
	sem_init(&guiSem, false, 0);
	sem_init(&clientSem, false, 0);
	pthread_create(&clientThread, NULL, &threadFunc_client, (void *)this);
	pthread_create(&guiThread, NULL, &threadFunc_gui, (void *)this);
	
	mainWin = NULL;
}

WulforManager::~WulforManager() {
	pthread_mutex_destroy(&guiLock);
	pthread_mutex_destroy(&clientLock);
	sem_destroy(&guiSem);
	sem_destroy(&clientSem);
	pthread_detach(guiThread);
	pthread_detach(clientThread);
	
	if (mainWin) delete mainWin;
}

string WulforManager::getPath() {
	string ret = br_extract_dir(SELFPATH);
	return ret;
}

MainWindow *WulforManager::getMainWindow() {
	return mainWin;
}

BookEntry *WulforManager::getBookEntry(int type, string id, bool raise) {
	BookEntry *ret = NULL;
	vector<BookEntry *>::iterator it;
	
	for (it = bookEntrys.begin(); it != bookEntrys.end(); it++)
		if ((*it)->isEqual(type, id)) ret = *it;
		
	if (ret && raise) {
		Func1<MainWindow, GtkWidget *> *func = new Func1<MainWindow, GtkWidget*>
			(mainWin, &MainWindow::raisePage_gui, ret->getWidget());
		dispatchGuiFunc(func);
	}

	return ret;
}

void WulforManager::deleteBookEntry_gui(BookEntry *entry) {
	vector<FuncBase *>::iterator fIt;
	
	pthread_mutex_lock(&clientLock);
	pthread_mutex_lock(&guiLock);

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
	vector<BookEntry *>::iterator bIt;
	for (bIt = bookEntrys.begin(); bIt != bookEntrys.end(); bIt++)
		if ((*bIt)->isEqual(entry)) {
			delete *bIt;
			bookEntrys.erase(bIt);
			break;
		}

	pthread_mutex_unlock(&guiLock);
	pthread_mutex_unlock(&clientLock);
}

void WulforManager::addPublicHubs_gui() {
	if (getBookEntry(PUBLIC_HUBS, "", true)) return;

	PublicHubs *pubHubs = new PublicHubs(G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(pubHubs->getWidget(), pubHubs->getTitle(), true);
	gtk_widget_unref(pubHubs->getWidget());
	bookEntrys.push_back(pubHubs);
	
	Func0<PublicHubs> *func = new Func0<PublicHubs>(pubHubs, &PublicHubs::downloadList_client);
	dispatchClientFunc(func);
}

void WulforManager::addHub_gui(string address, string nick, string desc, string password) {
	if (getBookEntry(HUB, address, true)) return;

	Hub *hub = new Hub(address, G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(hub->getWidget(), hub->getTitle(), true);
	gtk_widget_unref(hub->getWidget());
	bookEntrys.push_back(hub);

	typedef Func4<Hub, string, string, string, string> F4;
	F4 *func = new F4(hub, &Hub::connectClient_client, address, nick, desc, password);
	WulforManager::get()->dispatchClientFunc(func);
}

