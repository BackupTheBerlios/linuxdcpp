#include "wulformanager.hh"
#include "prefix.hh"
#include "publichubs.hh"
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
		pthread_mutex_lock(&clientLock);
		assert(clientFuncs.size() > 0);
		clientFuncs.front()->call();
		delete clientFuncs.front();
		clientFuncs.pop();
		pthread_mutex_unlock(&clientLock);
	}
}

void WulforManager::callGuiFunc() {
	pthread_mutex_lock(&guiLock);
	assert(guiFuncs.size() > 0);
	guiFuncs.front()->call();
	delete guiFuncs.front();
	guiFuncs.pop();
	pthread_mutex_unlock(&guiLock);
}

void WulforManager::dispatchGuiFunc(FuncBase *func) {
	pthread_mutex_lock(&guiLock);
	guiFuncs.push(func);
	pthread_mutex_unlock(&guiLock);
	sem_post(&guiSem);
}

void WulforManager::dispatchClientFunc(FuncBase *func) {
	pthread_mutex_lock(&clientLock);
	clientFuncs.push(func);
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
	mainWin->removePage_gui(entry->getWidget());
	
	vector<BookEntry *>::iterator it;
	for (it = bookEntrys.begin(); it != bookEntrys.end(); it++)
		if ((*it)->isEqual(entry)) {
			bookEntrys.erase(it);
			return;
		}
		
	assert(it != bookEntrys.end());
}

void WulforManager::addPublicHubs_gui() {
	if (getBookEntry(PUBLIC_HUBS, "", true)) return;

	PublicHubs *pubHubs = new PublicHubs(G_CALLBACK(closeEntry_callback));
	mainWin->addPage_gui(pubHubs->getWidget(), pubHubs->getTitle(), true);
	gtk_widget_unref(pubHubs->getWidget());
	bookEntrys.push_back(pubHubs);
}

