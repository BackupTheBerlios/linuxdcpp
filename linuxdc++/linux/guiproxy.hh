/* 
* Copyright (C) 2001-2003 Jens Oknelid, paskharen@spray.se
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

#ifndef WULFOR_GUI_PROXY
#define WULFOR_GUI_PROXY

#include <map>
#include <vector>

#include <sigcx/thread_tunnel.h>
#include <sigcx/gtk_dispatch.h>

#include "guilistener.hh"

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"
#include "../client/CriticalSection.h"
#include "../client/Speaker.h"

/*
	This class is a proxy that uses thread-safe tunnels to call
	::on-methods from the backend. (client/) The gui objects
	register with the proxy and the proxy delivers calls from the
	backend to the gui objects. This ensures that there is only one 
	thread making calls to gtk, which is needed since gtk is not
	thread-safe.
*/
class GuiProxy {
	public:
		//this needs to be called from the main thread
		static void startup();
		void shutdown();
		static GuiProxy *getInstance();

		template <class Listener, class Type>
		void addListener(Listener *listener, Speaker<Type> *speaker) {
			Lock l(cs);
			GuiListener<Listener, Type> *tmp;

			if (listeners.find(listener) == listeners.end()) {
				std::vector<GuiListenerBase *> v;
				listeners[listener] = v;
			}

			std::vector<GuiListenerBase *> &v = listeners[listener];
			tmp = new GuiListener<Listener, Type>(listener, speaker, &tunnel);
			v.push_back(tmp);
		}

		template <class Listener>
		void removeListener(Listener *listener) {
			Lock l(cs);
			std::vector<GuiListenerBase *> &v = listeners[listener];
			std::vector<GuiListenerBase *>::iterator it;
			
			for (it = v.begin(); it != v.end(); it++)
				delete *it;

			listeners.erase(listener);
		}

		SigCX::ThreadTunnel *getTunnel();

	private:
		GuiProxy();
		~GuiProxy();

		SigCX::GtkDispatcher disp;
		SigCX::ThreadTunnel tunnel;

		static GuiProxy *instance;
		CriticalSection cs;
		std::map<SigC::Object *, std::vector<GuiListenerBase *> > listeners;
};

#else
class GuiProxy;
#endif
