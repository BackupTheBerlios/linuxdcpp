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

#include "bookentry.hh"
#include <iostream>

using namespace Gtk;
using namespace SigC;
using namespace Glib;
using namespace std;
using namespace Toolbar_Helpers;

const bool BookEntry::useMultipleTabs[] = {false, false, true, false, true, false, false};

BookEntry::BookEntry()
{
	button.add(*manage(new Gtk::Image(Stock::CLOSE, ICON_SIZE_MENU)));
	button.set_size_request (16, 16);
	button.signal_clicked().connect(slot(*this, &BookEntry::close));
	box.pack_start(label, Gtk::PACK_EXPAND_WIDGET);
	box.pack_start(button, Gtk::PACK_SHRINK);
	box.show_all();
}

BookEntry::~BookEntry()
{
}

Label &BookEntry::getLabel() {
	return label;
}

HBox &BookEntry::getBox() {
	return box;
}

int BookEntry::getID() {
	return ID;
}

void BookEntry::setParent (Notebook *p) {
	parent = p;
}