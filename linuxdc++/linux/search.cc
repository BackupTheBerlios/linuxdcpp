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

int Search::columnSize[] = { 200, 100, 50, 80, 100, 40, 70, 150, 80, 100, 125 };

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
	Slot1<void, GdkEventButton*> callback1;
	GuiProxy *proxy = GuiProxy::getInstance();
	ThreadTunnel *tunnel = proxy->getTunnel();

	resultsStore = ListStore::create(columns);
	resultsView.set_model(resultsStore);
	resultsView.append_column("File", columns.file);
	resultsView.append_column("User", columns.user);
	resultsView.append_column("Type", columns.type);
	resultsView.append_column("Size", columns.filesize);
	resultsView.append_column("Path", columns.path);
	resultsView.append_column("Slots", columns.slots);
	resultsView.append_column("Connection", columns.connection);
	resultsView.append_column("Hub", columns.hub);
	resultsView.append_column("Exact size", columns.exactsize);
	resultsView.append_column("IP", columns.ip);
	resultsView.append_column("TTH", columns.tth);

	scroll.add(resultsView);
	scroll.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);

	for (int i=0;i<columns.size ()-1;i++)
	{
		resultsView.get_column (i)->set_sizing (TREE_VIEW_COLUMN_FIXED);
		resultsView.get_column (i)->set_resizable (true);
		resultsView.get_column (i)->set_fixed_width (columnSize[i]);
	}
	
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

	using namespace Gtk::Menu_Helpers;
	MenuList items = popupMenu.items();
	items.push_back (MenuElem ("Download", open_tunnel (tunnel, slot (*this, &Search::download), true)));
	items.push_back (MenuElem ("Download to..."));
	items.back ().set_sensitive (false);
	items.push_back (MenuElem ("Download whole directory", open_tunnel (tunnel, slot (*this, &Search::downloadDir), true)));
	items.push_back (MenuElem ("Download whole directory to..."));
	items.back ().set_sensitive (false);
	items.push_back (SeparatorElem ());
	items.push_back (MenuElem ("Search by TTH"));
	items.back ().set_sensitive (false);
	items.push_back (SeparatorElem ());
	items.push_back (MenuElem ("Get file list", open_tunnel (tunnel, slot (*this, &Search::getFileList), true)));
	
	mainBox.pack_start(barBox, PACK_SHRINK);
	mainBox.pack_start(scroll, PACK_EXPAND_WIDGET);

	pack_start(mainBox);
	show_all();

	label.set_text(WUtil::ConvertToUTF8("Search"));
	label.show();

	//callback = slot(*this, &Search::searchPressed);
	callback = open_tunnel(tunnel, slot(*this, &Search::searchPressed), true);
	searchButton.signal_clicked().connect(callback);
	search.get_entry()->signal_activate().connect(callback);

	callback1 = open_tunnel(tunnel, slot(*this, &Search::buttonPressedResult), true);
	resultsView.signal_button_press_event().connect_notify(callback1);
	callback1 = open_tunnel(tunnel, slot(*this, &Search::buttonReleasedResult), true);
	resultsView.signal_button_release_event().connect_notify(callback1);

	proxy->addListener<Search, SearchManagerListener>(
		this, SearchManager::getInstance());

	this->mw = mw;
}

bool Search::operator== (BookEntry &b) {
	Search *s = dynamic_cast<Search *>(&b);
	
	return !(s == NULL);
}

void Search::buttonPressedResult (GdkEventButton *event)
{
	resultPrevious = event->type;
}
void Search::buttonReleasedResult (GdkEventButton *event)
{
	//single click
	if (resultPrevious == GDK_BUTTON_PRESS) {
		//left button
		if (event->button == 1)
		{
		}

		//right button
		if (event->button == 3)
		{
			popupMenu.popup(event->button, event->time);
			popupMenu.show_all();
		}
	}
}

void Search::searchFor (ustring searchString)
{
	search.get_entry ()->set_text (searchString);
	searchPressed ();
}

void Search::searchPressed() {
	string name;
	int64_t size;
	SearchManager::TypeModes fileType;
	SearchManager::SizeModes sizeType;
	Label *l;
	ComboDropDownItem *i;

	resultsStore->clear();

	name = search.get_entry()->get_text();
	name = SearchManager::clean(name);

	size = pow((double)1024, unitOM.get_history());
	size *= Util::toInt64(sizeEntry.get_text());
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

	if(result->getType() == SearchResult::TYPE_FILE)
	{
		string file = WUtil::linuxSeparator (result->getFile ());
		(*it)[columns.path] = Util::getFilePath(file);
		(*it)[columns.file] = Util::getFileName (file);

		ustring type = Util::getFileExt(file);
		if(!type.empty() && type[0] == '.')
			type.erase(0, 1);

		(*it)[columns.type] = type;			
		(*it)[columns.filesize] = Util::formatBytes(result->getSize());
		(*it)[columns.exactsize] = Util::formatExactSize(result->getSize());
	}
	else
	{
		(*it)[columns.file] = WUtil::linuxSeparator (result->getFileName ());
		(*it)[columns.path] = WUtil::linuxSeparator (result->getFile ());
		(*it)[columns.type] = "Directory";
	}
	(*it)[columns.user] = result->getUser()->getNick();
	(*it)[columns.connection] = result->getUser()->getConnection();
	(*it)[columns.hub] = result->getHubName();
	(*it)[columns.slots] = result->getSlotString();
	(*it)[columns.ip] = result->getIP();
	if(result->getTTH() != NULL)
		(*it)[columns.tth] = result->getTTH()->toBase32();
	
	(*it)[columns.info] = new SearchInfo (result);
}

void Search::SearchInfo::download ()
{
	try
	{
		if(sr->getType () == SearchResult::TYPE_FILE) {
			string target = WUtil::ConvertFromUTF8(SETTING(DOWNLOAD_DIRECTORY));
			
			if (sr->getUtf8()) {
				target += WUtil::ConvertFromUTF8(
					Util::getFileName (WUtil::linuxSeparator (sr->getFile ())));
			} else {
				target += 
					Util::getFileName (WUtil::linuxSeparator (sr->getFile ()));
			}
			
			QueueManager::getInstance()->add(
				sr->getFile (), 
				sr->getSize(), 
				sr->getUser(), 
				target,
				sr->getTTH(), 
				QueueItem::FLAG_RESUME | (sr->getUtf8() ? QueueItem::FLAG_SOURCE_UTF8 : 0), 
				QueueItem::DEFAULT);
		} else {
			QueueManager::getInstance()->addDirectory(
				sr->getFile(), 
				sr->getUser(), 
				WUtil::ConvertFromUTF8(SETTING(DOWNLOAD_DIRECTORY)), 
				QueueItem::DEFAULT);
		}
	}
	catch(...)
	{
		cout << "Couldn't add file for download." << endl;
	}		
}

void Search::SearchInfo::downloadDir ()
{
	try
	{
		if(sr->getType() == SearchResult::TYPE_FILE) {
			QueueManager::getInstance()->addDirectory(
				WUtil::windowsSeparator (Util::getFilePath (WUtil::linuxSeparator (sr->getFile ()))),
				sr->getUser(),
				WUtil::ConvertFromUTF8(SETTING (DOWNLOAD_DIRECTORY)), 
				QueueItem::DEFAULT);
		} else {
			QueueManager::getInstance()->addDirectory(
				sr->getFile (),
				sr->getUser(),
				WUtil::ConvertFromUTF8(SETTING (DOWNLOAD_DIRECTORY)), 
				QueueItem::DEFAULT);
		}
	}
	catch(...)
	{
		cout << "Couldn't add dir for download." << endl;
	}	
}

void Search::SearchInfo::getFilelist ()
{
	try
	{
		QueueManager::getInstance ()->addList(sr->getUser (), QueueItem::FLAG_CLIENT_VIEW);
	}
	catch (...)
	{
		cout << "Couldn't add filelist." << endl;
	}
}

void Search::getFileList ()
{
	TreeModel::Row r = *(resultsView.get_selection ()->get_selected ());

	if (!r)
		return;

	SearchInfo *s = r[columns.info];

	s->getFilelist ();
}

void Search::download ()
{
	TreeModel::Row r = *(resultsView.get_selection ()->get_selected ());

	if (!r)
		return;

	SearchInfo *s = r[columns.info];
		
	s->download ();
}

void Search::downloadDir ()
{
	TreeModel::Row r = *(resultsView.get_selection ()->get_selected ());

	if (!r)
		return;

	SearchInfo *s = r[columns.info];

	s->downloadDir ();
}

void Search::close()
{
	getParent ()->remove_page (*this);
	delete this;
}
