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

#ifndef WULFOR_DISPATCHER
#define WULFOR_DISPATCHER

#include "slot7.hh"

#include <gtkmm.h>
#include <pthread.h>

class Dispatcher {
	public:
		static Dispatcher *getInstance();
		static void startup();
				
		template<class R, class T1>
		void addSlot(Slot1<R, T1> &s) {
			pthread_lock(&addLock);
			
			//marshall
						
			pthread_semaphore_signal(sem);
			pthread_lock(&addLock);
		}

	private:
		static Dispatcher *instance;
		
		Dispatcher();
	
		pthread_mutex_t
		
};
#else
class Dispatcher;
#endif
