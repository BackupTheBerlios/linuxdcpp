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

#include "privatemsg.hh"

using namespace std;
using namespace Gtk;
using namespace SigC;
using namespace Glib;

PrivateMsg::PrivateMsg(User::Ptr userName, MainWindow *mw) {
	user = userName;

	text.set_editable(false);
	scroll.add(text);
	
	box.pack_start(scroll, PACK_EXPAND_WIDGET, 3);
	box.pack_start(entry, PACK_SHRINK, 3);

	pack_start(box, PACK_EXPAND_WIDGET);

	label.set_text(locale_to_utf8(user->getNick()));
	label.show();
	
	this->mw = mw;
}

bool PrivateMsg::operator== (BookEntry &b) {
	PrivateMsg *msg = dynamic_cast<PrivateMsg *>(&b);
	
	if (msg == NULL) return false;
	return msg->user->getNick() == user->getNick();
}

bool PrivateMsg::operator== (User::Ptr &otherUser) {
	return user->getNick() == otherUser->getNick();
}

void PrivateMsg::update() {
	label.set_text(locale_to_utf8(user->getNick()));
	label.show();
}

void PrivateMsg::addMsg(std::string msg) {
	RefPtr<TextBuffer> buffer = text.get_buffer();
	buffer->insert(buffer->end(), locale_to_utf8(msg));
}
