#include "wulformanager.hh"
#include "prefix.hh"
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

void WulforManager::addPublicHubs_gui() {


}

