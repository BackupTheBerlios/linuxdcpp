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

#include "selecter.hh"
#include <iostream>
#include <signal.h>
#include <sys/poll.h>

using namespace std;

pthread_t Selecter::thread;
pthread_mutex_t Selecter::lock;
bool Selecter::exitPlease, Selecter::running;

void *Selecter::threadFunc(void *arg) {
	ServerSocket *socket = (ServerSocket *)arg;
	struct pollfd pollData;
	int retVal;
	const int TIMEOUT = 1000;

	pthread_mutex_lock(&lock);
	running = true;
	pthread_mutex_unlock(&lock);
	
	pollData.fd = socket->getSocket();
	pollData.events = (POLLIN | POLLPRI);
	pollData.revents = 0;
	while ((retVal = poll(&pollData, 1, TIMEOUT)) != -1) {
		//0 = timeout
		if (retVal == 0) {
			pthread_mutex_lock(&lock);
			if (exitPlease) {
				running = false;
				pthread_mutex_unlock(&lock);
				pthread_exit(0);
			}
			pthread_mutex_unlock(&lock);
		} else {
			socket->incoming();
		}
	}

	pthread_mutex_lock(&lock);
	running = false;
	pthread_mutex_unlock(&lock);
	pthread_exit(0);
}

void Selecter::WSAASyncSelect(ServerSocket &socket) {
	exitPlease = false;
	running = false;
	pthread_mutex_init(&lock, NULL);
	pthread_create(&thread, NULL, &threadFunc, (void *)&socket);
}

void Selecter::quit() {
	pthread_mutex_lock(&lock);
	if (!running) {
		pthread_mutex_unlock(&lock);
		return;
	}
	exitPlease = true;
	pthread_mutex_unlock(&lock);
	pthread_join(thread, NULL);
}

