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

#ifndef WULFOR_ACCEPTER
#define WULFOR_ACCEPTER

#include <vector>
#include <pthread.h>

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"
#include "../client/ServerSocket.h"
#include "../client/ConnectionManager.h"

class Accepter {
	public:
		static void registerSocket(ServerSocket *socket);
		static void quit();

	private:
		static void *threadFunc(void *arg);
		static std::vector<UserConnection *> connections;
		static pthread_t thread;
};
#else
class Accepter;
#endif
