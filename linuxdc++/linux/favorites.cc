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

#include "guiproxy.hh"
#include "favorites.hh"
#include "hub.hh"

int FavoriteHubs::columnSize[] = { 80, 100, 290, 125, 100, 100, 125 };
using namespace std;
using namespace Gtk;
using namespace SigC;
using namespace SigCX;
using namespace Glib;
FavoriteHubs::~FavoriteHubs ()
{
	GuiProxy::getInstance ()->removeListener<FavoriteHubs>(this);
}	
FavoriteHubs::FavoriteHubs (MainWindow *mw) :
buttonBox (BUTTONBOX_START, 2),
bNew ("New..."),
bProperties ("Properties"),
bRemove ("Remove"),
bMoveUp ("Move Up"),
bMoveDown ("Move Down"),
bConnect ("Connect")
{
	Slot0<void> callback0;
	Slot1<void, GdkEventButton *> callback1;
	GuiProxy *proxy = GuiProxy::getInstance();
	ThreadTunnel *tunnel = proxy->getTunnel();
		
	window = mw;
	ID = BOOK_FAVORITE_HUBS;

	label.set_text ("Favorite Hubs");
	label.show_all ();

	hubStore = ListStore::create (columns);
	hubView.set_model (hubStore);
	CellRendererToggle *const renderer = new CellRendererToggle;
    TreeViewColumn  *const column = new TreeViewColumn("Auto connect", *manage(renderer));
	hubView.append_column(*manage(column));
	
    column->add_attribute(renderer->property_active(), columns.autoconnect);
    renderer->signal_toggled().connect(open_tunnel (tunnel, slot(*this, &FavoriteHubs::toggleClicked), true));
	hubView.append_column ("Name", columns.name);
	hubView.append_column ("Description", columns.description);
	hubView.append_column ("Nick", columns.nick);
	hubView.append_column ("Password", columns.password);
	hubView.append_column ("Server", columns.server);
	hubView.append_column ("User description", columns.userdescription);

	for (int i=0;i<columns.size ()-1;i++)
	{
		hubView.get_column (i)->set_sizing (TREE_VIEW_COLUMN_FIXED);
		hubView.get_column (i)->set_resizable (true);
		hubView.get_column (i)->set_fixed_width (columnSize[i]);
	}

	hubScroll.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);
	hubScroll.add(hubView);
	buttonBox.add (bNew);
	buttonBox.add (bProperties);
	buttonBox.add (bRemove);
	buttonBox.add (bMoveUp);
	buttonBox.add (bMoveDown);
	buttonBox.add (bConnect);
	box.pack_start (hubScroll, PACK_EXPAND_WIDGET);
	box.pack_start (buttonBox, PACK_SHRINK);
	add (box);
	//mainBox.pack_start (hubView, PACK_EXPAND_WIDGET, 0);

	using namespace Gtk::Menu_Helpers;
	MenuList items = menu.items ();
	items.push_back (MenuElem ("Connect", open_tunnel (tunnel, slot(*this, &FavoriteHubs::connect), true)));
	items.push_back (MenuElem ("New...", open_tunnel (tunnel, slot(*this, &FavoriteHubs::preNew), true)));
	items.push_back (MenuElem ("Properties", open_tunnel (tunnel, slot(*this, &FavoriteHubs::preEdit), true)));
	items.push_back (MenuElem ("Move Up", open_tunnel (tunnel, slot(*this, &FavoriteHubs::moveUp), true)));
	items.push_back (MenuElem ("Move Down", open_tunnel (tunnel, slot(*this, &FavoriteHubs::moveDown), true)));
	items.push_back (SeparatorElem ());
	items.push_back (MenuElem ("Remove", open_tunnel (tunnel, slot(*this, &FavoriteHubs::remove), true)));

	bNew.signal_clicked ().connect (open_tunnel (tunnel, slot(*this, &FavoriteHubs::preNew), true));
	bProperties.signal_clicked ().connect (open_tunnel (tunnel, slot(*this, &FavoriteHubs::preEdit), true));
	bRemove.signal_clicked ().connect (open_tunnel (tunnel, slot(*this, &FavoriteHubs::remove), true));
	bMoveUp.signal_clicked ().connect (open_tunnel (tunnel, slot(*this, &FavoriteHubs::moveUp), true));
	bMoveDown.signal_clicked ().connect (open_tunnel (tunnel, slot(*this, &FavoriteHubs::moveDown), true));
	bConnect.signal_clicked ().connect (open_tunnel (tunnel, slot(*this, &FavoriteHubs::connect), true));

	callback1 = open_tunnel(tunnel, slot(*this, &FavoriteHubs::buttonPressed), true);
	hubView.signal_button_press_event().connect_notify(callback1);
	callback1 = open_tunnel(tunnel, slot(*this, &FavoriteHubs::buttonReleased), true);
	hubView.signal_button_release_event().connect_notify(callback1);

	proxy->addListener<FavoriteHubs, HubManagerListener>(this, HubManager::getInstance ());
	updateList (HubManager::getInstance ()->getFavoriteHubs());

	show_all ();
}
void FavoriteHubs::toggleClicked (const ustring &path_str)
{
    TreeRow row = *hubStore->get_iter(TreePath (path_str));

	FavoriteHubEntry *e = row[columns.entry];
	row[columns.autoconnect] = !row[columns.autoconnect];
	e->setConnect (!e->getConnect ());
	HubManager::getInstance ()->save ();
}
TreeIter FavoriteHubs::addEntry(const FavoriteHubEntry* entry, int pos)
{
	char buffer[4];
	sprintf (buffer, "%d", pos);
	TreeIter iter = hubStore->get_iter (buffer);
	TreeIter row;
	if (iter)
		row =  hubStore->insert (iter);
	else
		row = hubStore->append ();

	(*row)[columns.name] = entry->getName ();
	(*row)[columns.description] = entry->getDescription ();
	(*row)[columns.nick] = entry->getNick ();
	(*row)[columns.password] = string (entry->getPassword ().size (), '*');
	(*row)[columns.server] = entry->getServer ();
	(*row)[columns.userdescription] = entry->getUserDescription ();
	(*row)[columns.entry] = (FavoriteHubEntry*)entry;
	(*row)[columns.autoconnect] = entry->getConnect ();
	return row;
}
void FavoriteHubs::remove ()
{
	TreeIter row = hubView.get_selection ()->get_selected ();

	if (!row)
		return;

	HubManager::getInstance ()->removeFavorite ((FavoriteHubEntry*)((*row)[columns.entry]));
}
void FavoriteHubs::moveUp ()
{
	TreeIter row = hubView.get_selection ()->get_selected ();

	if (!row)
		return;

	FavoriteHubEntry::List &fh = HubManager::getInstance ()->getFavoriteHubs ();
	for (int i=1; i < hubStore->children ().size (); i++)
	{
		if (fh[i] == (*row)[columns.entry])
		{
			FavoriteHubEntry *e = fh[i];
			swap (fh[i], fh[i-1]);
			hubStore->erase (row);
			hubView.get_selection ()->select (addEntry (e, i-1));
		}
	}
	HubManager::getInstance ()->save ();
}
void FavoriteHubs::moveDown ()
{
	TreeIter row = hubView.get_selection ()->get_selected ();

	if (!row)
		return;

	FavoriteHubEntry::List &fh = HubManager::getInstance ()->getFavoriteHubs ();
	for (int i=hubStore->children ().size ()-2; i >= 0; i--)
	{
		if (fh[i] == (*row)[columns.entry])
		{
			FavoriteHubEntry *e = fh[i];
			swap (fh[i], fh[i+1]);
			hubStore->erase (row);
			hubView.get_selection ()->select (addEntry (e, i+1));
		}
	}
	HubManager::getInstance ()->save ();	
}
bool FavoriteHubs::operator== (BookEntry &b) {
	FavoriteHubs *FH;
	
	FH = dynamic_cast<FavoriteHubs *>(&b);
	return FH != NULL;
}
void FavoriteHubs::buttonPressed (GdkEventButton *event)
{
	previous = event->type;
}
void FavoriteHubs::buttonReleased (GdkEventButton *event)
{
	if (previous == GDK_BUTTON_PRESS)
	{
		if (event->button == 3)
		{
			menu.popup(event->button, event->time);
			menu.show_all();
		}
	}
}
void FavoriteHubs::preNew ()
{
	addDialog (false);
}
void FavoriteHubs::preEdit ()
{
	TreeRow row = *hubView.get_selection ()->get_selected ();

	if (!row)
		return;	
	addDialog (true, row[columns.name], row[columns.server], row[columns.description], row[columns.nick], row[columns.password], row[columns.userdescription]);
}
void FavoriteHubs::addDialog (bool edit, ustring uname, ustring uaddress, ustring udesc, ustring unick, ustring upassword, ustring uuserdesc)
{
	Gtk::Dialog window ("Favorite Hub Properties", *window, true) ;
	Gtk::Entry nameEntry, addressEntry, descEntry, nickEntry, passwordEntry, userdescEntry;
	nameEntry.set_text (uname);
	addressEntry.set_text (uaddress);
	descEntry.set_text (udesc);
	nickEntry.set_text (unick);
	passwordEntry.set_text (upassword);
	userdescEntry.set_text (uuserdesc);
	Gtk::Label name("Name", 1.0, 0.5), address("Address", 1.0, 0.5), desc("Description", 1.0, 0.5), nick("Nick", 1.0, 0.5), password("Password", 1.0, 0.5), userdesc("Description", 1.0, 0.5);
	Gtk::Frame hubFrame("Hub"), identFrame("Identification (leave blank for defaults)");
	Gtk::Table hubTable (3, 2), identTable (3, 2);
	Gtk::VBox box;

	window.set_default_size (200, 50);

	window.get_vbox ()->pack_start (hubFrame, PACK_EXPAND_WIDGET);
	window.set_border_width (8);
	hubFrame.add (hubTable);
	hubTable.set_spacings (2);
	hubTable.attach(name, 0, 1, 0, 1);
	hubTable.attach(nameEntry, 1, 2, 0, 1);
	hubTable.attach(address, 0, 1, 1, 2);
	hubTable.attach(addressEntry, 1, 2, 1, 2);
	hubTable.attach(desc, 0, 1, 2, 3);
	hubTable.attach(descEntry, 1, 2, 2, 3);
	window.get_vbox()->pack_start (identFrame, PACK_EXPAND_WIDGET);
	identFrame.add (identTable);
	identTable.set_spacings (2);
	identTable.attach(nick, 0, 1, 0, 1);
	identTable.attach(nickEntry, 1, 2, 0, 1);
	passwordEntry.set_invisible_char ('*');
	passwordEntry.set_visibility (false);
	identTable.attach(password, 0, 1, 1, 2);
	identTable.attach(passwordEntry, 1, 2, 1, 2);
	identTable.attach(userdesc, 0, 1, 2, 3);
	identTable.attach(userdescEntry, 1, 2, 2, 3);
	window.add_button (GTK_STOCK_CANCEL, 0);
	window.add_button (GTK_STOCK_OK, 1);
		
	window.show_all ();
		
	if (window.run () == 1)
	{
		if (!edit)
		{
			FavoriteHubEntry e;
			e.setName (nameEntry.get_text ());
			e.setServer (addressEntry.get_text ());
			e.setDescription (descEntry.get_text ());
			e.setNick (nickEntry.get_text ());
			e.setPassword (passwordEntry.get_text ());
			e.setUserDescription (userdescEntry.get_text ());
			HubManager::getInstance()->addFavorite (e);
		}
		else
		{
			TreeRow row = *hubView.get_selection ()->get_selected ();
			FavoriteHubEntry::List &fh = HubManager::getInstance ()->getFavoriteHubs ();
			for (int i=0; i < hubStore->children ().size (); i++)
			{
				if (fh[i] == row[columns.entry])
				{
					fh[i]->setName (nameEntry.get_text ());
					fh[i]->setServer (addressEntry.get_text ());
					fh[i]->setDescription (descEntry.get_text ());
					fh[i]->setNick (nickEntry.get_text ());
					fh[i]->setPassword (passwordEntry.get_text ());
					fh[i]->setUserDescription (userdescEntry.get_text ());
					row[columns.name] = fh[i]->getName ();
					row[columns.description] = fh[i]->getDescription ();
					row[columns.nick] = fh[i]->getNick ();
					row[columns.password] = string (fh[i]->getPassword ().size (), '*');
					row[columns.server] = fh[i]->getServer ();
					row[columns.userdescription] = fh[i]->getUserDescription ();
     				break;
					
				}
			}
			HubManager::getInstance ()->save ();
		}
	}
}
void FavoriteHubs::connect ()
{
	TreeRow row = *hubView.get_selection ()->get_selected ();

	if (!row)
		return;
			
	Hub *h = new Hub (((ustring)row[columns.server]).raw (), window, ((ustring)row[columns.nick]).raw (), ((ustring)row[columns.password]).raw (), ((ustring)row[columns.userdescription]).raw ());
	if (window->addPage (h))
		manage (h);
	else
		delete h;
}
void FavoriteHubs::on(HubManagerListener::FavoriteAdded, const FavoriteHubEntry *entry) throw()
{
	addEntry(entry, hubStore->children ().size ());
}
void FavoriteHubs::on(HubManagerListener::FavoriteRemoved, const FavoriteHubEntry *entry) throw()
{
	for (TreeIter it= hubStore->children ().begin (); it != hubStore->children().end();)
	{
		if ((*it)[columns.entry] == entry)
			hubStore->erase (it);
		else
			it++;
	}	
}

void FavoriteHubs::close()
{
	getParent ()->remove_page (*this);
	delete this;
}
