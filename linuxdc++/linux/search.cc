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

#include <cmath>
#include <iostream>

#include "search.hh"
#include "guiproxy.hh"

using namespace std;
using namespace Gtk;
using namespace SigC;
using namespace SigCX;
using namespace Glib;

Search::Search(MainWindow *mw):
	barTable(4, 14),

	searchLabel("Search"),
	sizeLabel("Size"),
	typeLabel("File type"),
	optionLabel("Search options"),

	slotCB("Only users with free slots"),
	searchButton("Search")
{
	ID = BOOK_SEARCH;
	char *sizeItems[] = {"Normal", "At least", "At most"};
	char *unitItems[] = {"B", "kB", "MB", "GB"};
	char *typeItems[] = {"Any", "Audio", "Compressed", "Document",
		"Executable", "Picture", "Video", "Directory"};
	int i;
	MenuItem *item;
	Slot0<void> callback;
	GuiProxy *proxy = GuiProxy::getInstance();
	ThreadTunnel *tunnel = proxy->getTunnel();

	resultsStore = ListStore::create(columns);
	resultsView.set_model(resultsStore);
	resultsView.append_column("File", columns.file);
	resultsView.append_column("Size", columns.size);
	resultsView.append_column("Slots", columns.slots);
	resultsView.append_column("User", columns.user);
	resultsView.append_column("Path", columns.path);

	scroll.add(resultsView);

	for (i=0; i<3; i++) {
		item = new MenuItem(sizeItems[i]);
		sizeMenu.append(*item);
		manage(item);
	}
	sizeOM.set_menu(sizeMenu);

	for (i=0; i<4; i++) {
		item = new MenuItem(unitItems[i]);
		unitMenu.append(*item);
		manage(item);
	}
	unitOM.set_menu(unitMenu);

	for (i=0; i<8; i++) {
		item = new MenuItem(typeItems[i]);
		typeMenu.append(*item);
		manage(item);
	}
	typeOM.set_menu(typeMenu);

	barTable.attach(searchLabel, 0, 3, 0, 1);
	barTable.attach(search, 0, 3, 1, 2);
	barTable.attach(sizeLabel, 0, 3, 2, 3);
	barTable.attach(sizeOM, 0, 1, 3, 4);
	barTable.attach(sizeEntry, 1, 2, 3, 4);
	barTable.attach(unitOM, 2, 3, 3, 4);
	barTable.attach(typeLabel, 0, 3, 5, 6);
	barTable.attach(typeOM, 0, 3, 7, 8);
	barTable.attach(optionLabel, 0, 3, 8, 9);
	barTable.attach(slotCB, 0, 3, 9, 10);
	barTable.attach(searchButton, 2, 3, 10, 11);

	barBox.pack_start(barTable, PACK_SHRINK);

	/*	
	pane.add1(barBox);
	pane.add2(scroll);
	pane.set_position(300);

	pack_start(pane);
	*/

	mainBox.pack_start(barBox, PACK_SHRINK);
	mainBox.pack_start(scroll, PACK_EXPAND_WIDGET);

	pack_start(mainBox);
	show_all();

	label.set_text(WUtil::ConvertToUTF8("Search"));
	label.show();

	//callback = slot(*this, &Search::searchPressed);
	callback = open_tunnel(tunnel, slot(*this, &Search::searchPressed), false);
	searchButton.signal_clicked().connect(callback);
	search.get_entry()->signal_activate().connect(callback);

	proxy->addListener<Search, SearchManagerListener>(
		this, SearchManager::getInstance());

	this->mw = mw;
}

bool Search::operator== (BookEntry &b) {
	Search *s = dynamic_cast<Search *>(&b);
	
	return !(s == NULL);
}

void Search::searchPressed() {
	string name;
	int64_t size;
	SearchManager::TypeModes fileType;
	SearchManager::SizeModes sizeType;
	Label *l;
	ComboDropDownItem *i;

	resultsStore->clear();

	name = WUtil::ConvertFromUTF8(search.get_entry()->get_text());
	name = SearchManager::clean(name);

	size = pow((double)1024, unitOM.get_history());
	size *= Util::toInt64(WUtil::ConvertFromUTF8(sizeEntry.get_text()));
	fileType = (SearchManager::TypeModes)typeOM.get_history();
	sizeType = (SearchManager::SizeModes)sizeOM.get_history();

	l = new Label(search.get_entry()->get_text());
	i = new ComboDropDownItem();
	manage(l);
	manage(i);
	i->add(*l);
	search.get_list()->children().push_front(*i);
	l->show();

	SearchManager::getInstance()->search(name, size, fileType, sizeType);
}

void Search::on(SearchManagerListener::SR, SearchResult *result) throw() {
	TreeModel::iterator it;

	//in case we don't want users with no free slots
	if (slotCB.get_active() && result->getFreeSlots() < 1)	return;

	it = resultsStore->append();
	(*it)[columns.file] = WUtil::ConvertToUTF8(result->getFile());
	(*it)[columns.size] = WUtil::ConvertToUTF8(Util::formatBytes(result->getSize()));
	(*it)[columns.slots] = WUtil::ConvertToUTF8(result->getSlotString());
	(*it)[columns.user] = WUtil::ConvertToUTF8(result->getUser()->getNick());
	(*it)[columns.path] = WUtil::ConvertToUTF8(result->getFileName());
}



