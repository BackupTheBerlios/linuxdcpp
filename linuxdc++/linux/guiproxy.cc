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

#include "guiproxy.hh"

using namespace std;
using namespace SigC;
using namespace SigCX;
using namespace SigCX::Threads;

GuiProxy *GuiProxy::instance;

GuiProxy *GuiProxy::getInstance() {
	return instance;
}

void GuiProxy::startup() {
	instance = new GuiProxy;
}

void GuiProxy::shutdown() {
	map<SigC::Object *, std::vector<GuiListenerBase *> > ::iterator it;

	for (it = listeners.begin(); it != listeners.end(); it++)
		removeListener(it->first);

	delete this;
}

GuiProxy::GuiProxy(): tunnel(disp, ThreadTunnel::CurrentThread) {
}

GuiProxy::~GuiProxy() {
	tunnel.drain();
}

ThreadTunnel *GuiProxy::getTunnel() {
	return &tunnel;
}
