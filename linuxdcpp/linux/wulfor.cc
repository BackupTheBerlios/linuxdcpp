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

#include <gtk/gtk.h>
#include <glade/glade.h>

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/SettingsManager.h>
#include <client/ShareManager.h>

#include "wulformanager.hh"
#include <iostream>

using namespace std;

void callBack(void* x, const string& a) {
	cout << "Loading: " << a << endl;
}

int main(int argc, char *argv[]) {
	//starts the dc++ client part
	startup(callBack, NULL);
	
	SettingsManager *smgr = SettingsManager::getInstance();
	smgr->load();
	if (SETTING (DOWNLOAD_DIRECTORY) == SETTING (TEMP_DOWNLOAD_DIRECTORY))
		smgr->set(SettingsManager::TEMP_DOWNLOAD_DIRECTORY, "");
	TimerManager::getInstance()->start();

	g_thread_init(NULL);
	gtk_init(&argc, &argv);
	glade_init();

	WulforManager::start();
	WulforManager::get()->createMainWindow();
	gtk_main();
	WulforManager::stop();

	cout << "Shutting down..." << endl;
	if (SETTING (TEMP_DOWNLOAD_DIRECTORY) == "")
		smgr->set(SettingsManager::TEMP_DOWNLOAD_DIRECTORY, SETTING (DOWNLOAD_DIRECTORY));
	shutdown();
	
	return 0;
}
