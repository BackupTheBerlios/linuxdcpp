/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "ConnectionManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"
#include "CryptoManager.h"
#include "ShareManager.h"
#include "SearchManager.h"
#include "QueueManager.h"
#include "ClientManager.h"
#include "HashManager.h"
#include "LogManager.h"
#include "HubManager.h"
#include "SettingsManager.h"
#include "FinishedManager.h"
#include "ADLSearch.h"

#include "StringTokenizer.h"

#include <iostream>

void startup(void (*f)(void*, const string&), void* p) {
	// "Dedicated to the near-memory of Nev. Let's start remembering people while they're still alive."
	// Nev's great contribution to dc++
	while(1) break;

	Util::initialize();

	ResourceManager::newInstance();
	SettingsManager::newInstance();

	LogManager::newInstance();
	TimerManager::newInstance();
	HashManager::newInstance();
	CryptoManager::newInstance();
	SearchManager::newInstance();
	ClientManager::newInstance();
	ConnectionManager::newInstance();
	DownloadManager::newInstance();
	UploadManager::newInstance();
	ShareManager::newInstance();
	HubManager::newInstance();
	QueueManager::newInstance();
	FinishedManager::newInstance();
	ADLSearchManager::newInstance();

	SettingsManager::getInstance()->load();

	if(!SETTING(LANGUAGE_FILE).empty()) {
		ResourceManager::getInstance()->loadLanguage(SETTING(LANGUAGE_FILE));
	}

	HubManager::getInstance()->load();
	int i;
	for(i = 0; i < SettingsManager::SPEED_LAST; i++) {
		if(SETTING(CONNECTION) == SettingsManager::connectionSpeeds[i])
			break;
	}
	if(i == SettingsManager::SPEED_LAST) {
		SettingsManager::getInstance()->set(SettingsManager::CONNECTION, SettingsManager::connectionSpeeds[0]);
	}

	double v = Util::toDouble(SETTING(CONFIG_VERSION));
	if(v <= 0.22) {
		// Disable automatic public hublist opening
		SettingsManager::getInstance()->set(SettingsManager::OPEN_PUBLIC, false);
	}
	if(v <= 0.251) {
		StringTokenizer st(SETTING(HUBLIST_SERVERS), ';');
		StringList& sl = st.getTokens();
		StringList sl2;
		bool defFound = false;
		StringIter si;
		for(si = sl.begin(); si != sl.end(); ++si) {
			if((si->find("http://dcplusplus.sourceforge.net") != string::npos) ||
				(si->find("http://dcpp.lichlord.org") != string::npos))
			{
				if(!defFound) {
					sl2.push_back("http://www.hublist.org/PublicHubList.config.bz2");
					defFound = true;
				}
			} else {
				sl2.push_back(*si);
			}
		}
		string tmp;
		for(si = sl2.begin(); si != sl2.end(); ++si) {
			tmp += *si + ';';
		}

		if(!tmp.empty()) {
			tmp.erase(tmp.length()-1);
			SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, tmp);
		}
	}

	if(f != NULL)
		(*f)(p, STRING(HASH_DATABASE));
	HashManager::getInstance()->startup();
	if(f != NULL)
		(*f)(p, STRING(SHARED_FILES));
	ShareManager::getInstance()->refresh(false, false, true);
	if(f != NULL)
		(*f)(p, STRING(DOWNLOAD_QUEUE));
	QueueManager::getInstance()->loadQueue();

}

void shutdown() {
	std::cout << "a"<< endl;
	ConnectionManager::getInstance()->shutdown();
	std::cout << "b"<< endl;
	HashManager::getInstance()->shutdown();
	std::cout << "c"<< endl;

	TimerManager::getInstance()->removeListeners();
	std::cout << "d"<< endl;
	SettingsManager::getInstance()->save();
	std::cout << "e"<< endl;
	
	ADLSearchManager::deleteInstance();
	std::cout << "f"<< endl;
	FinishedManager::deleteInstance();
	std::cout << "g"<< endl;
	ShareManager::deleteInstance();
	std::cout << "h"<< endl;
	CryptoManager::deleteInstance();
	std::cout << "i"<< endl;
	DownloadManager::deleteInstance();
	std::cout << "j"<< endl;
	UploadManager::deleteInstance();
	std::cout << "k"<< endl;
	QueueManager::deleteInstance();
	std::cout << "l"<< endl;
	ConnectionManager::deleteInstance();
	std::cout << "m"<< endl;
	SearchManager::deleteInstance();
	std::cout << "n"<< endl;
	ClientManager::deleteInstance();
	std::cout << "o"<< endl;
	HubManager::deleteInstance();
	std::cout << "p"<< endl;
	HashManager::deleteInstance();
	std::cout << "q"<< endl;
	LogManager::deleteInstance();
	std::cout << "r"<< endl;
	SettingsManager::deleteInstance();
	std::cout << "s"<< endl;
	TimerManager::deleteInstance();
	std::cout << "t"<< endl;
	ResourceManager::deleteInstance();
	std::cout << "assa"<< endl;

}

/**
 * @file
 * $Id: DCPlusPlus.cpp,v 1.1 2004/10/04 19:43:51 paskharen Exp $
 */

