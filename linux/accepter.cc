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

#include "accepter.hh"
#include <iostream>
#include <signal.h>

using namespace std;

pthread_t Accepter::thread;
vector<UserConnection *> Accepter::connections;

void *Accepter::threadFunc(void *arg) {
	ConnectionManager *conMgr = ConnectionManager::getInstance();
	ServerSocket *socket = (ServerSocket *)arg;
	UserConnection *uc = NULL;

	while (true) {
		//This could probably be made to work really easy, just by
		//adding something like a UserConnection::setSocket method in
		//the client part...
		/*
		if(now > floodCounter) {
			floodCounter = now + FLOOD_ADD;
		} else {
			if(now + FLOOD_TRIGGER < floodCounter) {
				Socket s;
				try {
					s.accept(socket);
				} catch(const SocketException&) {
					// ...
				}
				dcdebug("Connection flood detected!\n");
				return;
			} else {
				floodCounter += 2000;
			}
		}
		*/

		try { 
			uc = conMgr->getConnection();
			connections.push_back(uc);
			uc->setFlag(UserConnection::FLAG_INCOMING);
			uc->setState(UserConnection::STATE_NICK);
			uc->setLastActivity(GET_TICK());
			uc->accept(*socket);
		} catch(const SocketException& e) {
			cout << "Error in accepter:" << e.getError() << endl;
			if (uc) {
				conMgr->putConnection(uc);
				connections.pop_back();
			}
		}
	}

	pthread_exit(0);
}

void Accepter::registerSocket(ServerSocket *socket) {
	pthread_create(&thread, NULL, &threadFunc, (void *)socket);
}

void Accepter::quit() {
	vector<UserConnection *>::iterator it;
	
	for (it = connections.begin(); it != connections.end(); it++)
		ConnectionManager::getInstance()->putConnection(*it);

	pthread_cancel(thread);
}

