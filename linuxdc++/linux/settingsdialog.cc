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

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "../client/ShareManager.h"
#include "../client/Util.h"

#include "settingsdialog.hh"
#include "mainwindow.hh"
#include "guiproxy.hh"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace Gtk;
using namespace Gdk;
using namespace SigC;
using namespace SigCX;
using namespace Glib;
using namespace std;
using namespace RadioButton_Helpers;

SettingsDialog::SettingsDialog(MainWindow *mw):
	Dialog("Settings", *mw, true),
	
	//general
	infoTable(4, 2),
	conTable (3, 5),
	sockTable(4, 4),
	personalFrame("Personal Information"),
	connectionFrame("Connection Settings (see the help file if unsure)"),
	nick("Nick", 1.0, 0.5),
	email("E-Mail", 1.0, 0.5),
	desc("Description", 1.0, 0.5),
	connection("Connection Type", 1.0, 0.5),
	ip("IP", 1.0, 0.5),
	tcpport("TCP Port", 0.5, 0.5),
	udpport("UDP Port", 0.5, 0.5),
	sockIp("Socks IP", 0.0, 0.5),
	sockPort("Port", 0.0, 0.5),
	user("Username", 0.0, 0.5),
	pass("Password", 0.0, 0.5),
	activeRB("Active"),
	passiveRB("Passive"),
	sock5RB("SOCKS5"),
	sock5CB("Use SOCKS5 server to resolve hostnames"),
	
	
	//download
	defDir("Download files to"),
	unfDir("Store unfinished downloads in (empty = download dir)"),
	slots("Download slots (0 = infinite)"),
	speed("Limit download speed to (kB/s, 0 = disable limit)"),
	note("Note: This is somewhat inaccurate..."),
	pubHubs("Public hubs URL"),
	httpProxy("HTTP proxy for hublist"),
	dirFrame("Directories"),
	limitFrame("Limits"),
	hubListFrame("Hub list"),
	dirTable(6, 2),
	slotTable(3, 2),
	defBrowse("Browse..."),
	unfBrowse("Browse..."),
	
	//sharing
	totalSize("Total size: 0"),
	autoOpen("Open extra slot if speed below (0 = disable)"),
	upload("Upload slots"),
	kb("kB/s"),
	hiddenCB("Share hidden files"),
	add("Add..."),
	remove("Remove"),
	shareFrame("Shared directories"),
	shareTable(2, 3)
{
	createGeneral();
	createDownload();
	createSharing();

	book.append_page(generalBox, "General", "General");
	book.append_page(downloadBox, "Download", "Download");
	book.append_page(sharingBox, "Sharing", "Sharing");

	get_vbox()->pack_start(book, PACK_EXPAND_WIDGET);
	
	add_button("Ok", OK);
	add_button("Cancel", CANCEL);

	setValues();

	show_all();
}

SettingsDialog::~SettingsDialog() {
	int i;
	
	//for (i=0; i<8; i++)
	//	delete connectionItems[i];
}

void SettingsDialog::createGeneral() {
	Slot0<void> callback;
	int i;
	ThreadTunnel *tunnel = GuiProxy::getInstance()->getTunnel();
	Group g = activeRB.get_group();
	passiveRB.set_group(g);
	sock5RB.set_group(g);
	
	generalBox.pack_start(personalFrame, PACK_EXPAND_WIDGET, 10);
	generalBox.pack_start(connectionFrame, PACK_EXPAND_WIDGET, 10);

	personalFrame.add(infoTable);
	connectionFrame.add(connectionBox);

	for(i=0; i<SettingsManager::SPEED_LAST; i++) {
		connectionItems[i] = new MenuItem(SettingsManager::connectionSpeeds[i]);
		connectionMenu.append(*connectionItems[i]);
		manage(connectionItems[i]);
	}
		
	connectionOption.set_menu(connectionMenu);

	infoTable.set_border_width (8);
	infoTable.set_spacings (1);
	infoTable.attach(nick, 0, 1, 0, 1);
	infoTable.attach(nickEntry, 1, 2, 0, 1);
	infoTable.attach(email, 0, 1, 1, 2);
	infoTable.attach(emailEntry, 1, 2, 1, 2);
	infoTable.attach(desc, 0, 1, 2, 3);
	infoTable.attach(descEntry, 1, 2, 2, 3);
	infoTable.attach(connection, 0, 1, 3, 4);
	infoTable.attach(connectionOption, 1, 2, 3, 4);

	conTable.set_border_width (8);
	conTable.set_spacings (1);
	conTable.attach (activeRB, 0, 1, 0, 1);
	conTable.attach (ip, 1, 2, 0, 1);
	conTable.attach (ipEntry, 2, 3, 0, 1);
	conTable.attach (tcpport, 3, 4, 0, 1);
	conTable.attach (tcpportEntry, 4, 5, 0, 1);
	conTable.attach (passiveRB, 0, 1, 1, 2);
	conTable.attach (udpport, 3, 4, 1, 2);
	conTable.attach (udpportEntry, 4, 5, 1, 2);
	conTable.attach (sock5RB, 0, 1, 2, 3);

	sockTable.set_border_width (8);
	sockTable.set_spacings (1);
	sockTable.attach(sockIp, 0, 1, 0, 1);
	sockTable.attach(sockPort, 1, 2, 0, 1);
	sockTable.attach(sockIpEntry, 0, 1, 1, 2);
	sockTable.attach(sockPortEntry, 1, 2, 1, 2);
	sockTable.attach(user, 0, 1, 2, 3);
	sockTable.attach(pass, 1, 2, 2, 3);
	sockTable.attach(userEntry, 0, 1, 3, 4);
	sockTable.attach(passEntry, 1, 2, 3, 4);
	
	connectionBox.pack_start(conTable, PACK_EXPAND_WIDGET);
	connectionBox.pack_start(sockTable, PACK_EXPAND_WIDGET);
	connectionBox.pack_start(sock5CB, PACK_EXPAND_WIDGET);

	callback = open_tunnel(tunnel, slot(*this, &SettingsDialog::activeClicked), false);
	activeRB.signal_clicked().connect(callback);
	callback = open_tunnel(tunnel, slot(*this, &SettingsDialog::passiveClicked), false);
	passiveRB.signal_clicked().connect(callback);
	callback = open_tunnel(tunnel, slot(*this, &SettingsDialog::sockClicked), false);
	sock5RB.signal_clicked().connect(callback);
}

void SettingsDialog::createDownload() {
	Slot0<void> callback;
	ThreadTunnel *tunnel = GuiProxy::getInstance()->getTunnel();

	slotsScale.set_range(0, 10);
	speedScale.set_range(0, 1000);
	slotsScale.set_increments(1, 1);
	speedScale.set_increments(1, 10);
	slotsScale.set_digits(0);
	speedScale.set_digits(0);

	dirTable.attach(defDir, 0, 2, 0, 1);
	dirTable.attach(defDirEntry, 0, 1, 1, 2);
	dirTable.attach(defBrowse, 1, 2, 1, 2);
	dirTable.attach(unfDir, 0, 2, 2, 3);
	dirTable.attach(unfDirEntry, 0, 1, 3, 4);
	dirTable.attach(unfBrowse, 1, 2, 3, 4);
	
	slotTable.attach(slotsScale, 0, 1, 0, 1);
	slotTable.attach(slots, 1, 2, 0, 1);
	slotTable.attach(speedScale, 0, 1, 1, 2);
	slotTable.attach(speed, 1, 2, 1, 2);
	slotTable.attach(note, 0, 3, 2, 3);
	
	hubListBox.pack_start(pubHubs, PACK_EXPAND_WIDGET);
	hubListBox.pack_start(pubHubsEntry, PACK_EXPAND_WIDGET);
	hubListBox.pack_start(httpProxy, PACK_EXPAND_WIDGET);
	hubListBox.pack_start(httpProxyEntry, PACK_EXPAND_WIDGET);

	dirFrame.add(dirTable);
	limitFrame.add(slotTable);
	hubListFrame.add(hubListBox);
	
	downloadBox.pack_start(dirFrame, PACK_EXPAND_WIDGET);
	downloadBox.pack_start(limitFrame, PACK_EXPAND_WIDGET);
	downloadBox.pack_start(hubListFrame, PACK_EXPAND_WIDGET);

	callback = open_tunnel(tunnel, slot(*this, &SettingsDialog::defDirClicked), false);
	defBrowse.signal_clicked().connect(callback);
	callback = open_tunnel(tunnel, slot(*this, &SettingsDialog::tempDirClicked), false);
	unfBrowse.signal_clicked().connect(callback);
}

void SettingsDialog::createSharing() {
	Slot0<void> callback;
	ThreadTunnel *tunnel = GuiProxy::getInstance()->getTunnel();

	autoOpenScale.set_range(0, 1000);
	uploadScale.set_range(0, 10);
	autoOpenScale.set_increments(1, 10);
	uploadScale.set_increments(1, 1);
	autoOpenScale.set_digits(0);
	uploadScale.set_digits(0);

	shareOptions.pack_start(totalSize, PACK_EXPAND_WIDGET);
	shareOptions.pack_start(hiddenCB, PACK_EXPAND_WIDGET);
	shareOptions.pack_start(add, PACK_SHRINK);
	shareOptions.pack_start(remove, PACK_SHRINK);

	dirModel = ListStore::create(columns);
	dirView.set_model(dirModel);
	//TODO: make this editable
	dirView.append_column("Name", columns.virt);
	dirView.append_column("Size", columns.size);
	dirView.append_column("Directory", columns.dir);

	shareBox.pack_start(dirView, PACK_EXPAND_WIDGET);
	shareBox.pack_start(shareOptions, PACK_SHRINK);

	shareTable.attach(autoOpen, 0, 1, 0, 1);
	shareTable.attach(autoOpenScale, 1, 2, 0, 1);
	shareTable.attach(kb, 2, 3, 0, 1);
	shareTable.attach(upload, 0, 1, 1, 2);
	shareTable.attach(uploadScale, 1, 2, 1, 2);

	shareFrame.add(shareBox);
	
	sharingBox.pack_start(shareFrame, PACK_EXPAND_WIDGET);
	sharingBox.pack_start(shareTable, PACK_SHRINK);

	callback = open_tunnel(tunnel, slot(*this, &SettingsDialog::addDirectory), false);
	add.signal_clicked().connect(callback);
	callback = open_tunnel(tunnel, slot(*this, &SettingsDialog::removeDirectory), false);
	remove.signal_clicked().connect(callback);
}

//retrieves values for all widgets from the client
void SettingsDialog::setValues() {
	char temp[10];
	ShareManager *share = ShareManager::getInstance();
	StringPairList directories = share->getDirectories();
	StringPairList::iterator it;
	TreeModel::iterator jt;
	int i;

	//General tab
	nickEntry.set_text(SETTING(NICK));
	emailEntry.set_text(SETTING(EMAIL));
	descEntry.set_text(SETTING(DESCRIPTION));
	for (i=0;i<SettingsManager::SPEED_LAST;i++)
		if (SETTING (CONNECTION) == SettingsManager::connectionSpeeds[i])
		{
			connectionOption.set_history(i);
			break;
		}
	
	ipEntry.set_text(SETTING(SERVER));
	sprintf(temp, "%d", SETTING(IN_PORT));
	tcpportEntry.set_text(temp);
	sockIpEntry.set_text(SETTING(SOCKS_SERVER));
	sprintf(temp, "%d", SETTING(SOCKS_PORT));
	sockPortEntry.set_text(temp);
	userEntry.set_text(SETTING(SOCKS_USER));
	passEntry.set_text(SETTING(SOCKS_PASSWORD));
	sock5CB.set_active(BOOLSETTING(SOCKS_RESOLVE));

	if (SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		activeRB.set_active(true);
		activeClicked();	
	}
	if (SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_PASSIVE) {
		passiveRB.set_active(true);
		passiveClicked();
	}
	if (SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_SOCKS5) {
		sock5RB.set_active(true);
		sockClicked();
	}		

	//Download tab
	defDirEntry.set_text(SETTING(DOWNLOAD_DIRECTORY));
	if (SETTING(DOWNLOAD_DIRECTORY) == SETTING(TEMP_DOWNLOAD_DIRECTORY))
		unfDirEntry.set_text ("");
	else
		unfDirEntry.set_text(SETTING(TEMP_DOWNLOAD_DIRECTORY));
	slotsScale.set_value(SETTING(DOWNLOAD_SLOTS));
	speedScale.set_value(SETTING(MAX_DOWNLOAD_SPEED));
	pubHubsEntry.set_text(SETTING(HUBLIST_SERVERS));
	httpProxyEntry.set_text(SETTING(HTTP_PROXY));

	//Sharing tab
	for(it = directories.begin(); it != directories.end(); it++) {
		jt = dirModel->append();
		(*jt)[columns.virt] = it->first;
		(*jt)[columns.size] = Util::formatBytes(share->getShareSize(it->second));
		(*jt)[columns.dir] = it->second;
	}

	hiddenCB.set_active(BOOLSETTING(SHARE_HIDDEN));
	autoOpenScale.set_value(SETTING(MIN_UPLOAD_SPEED));
	uploadScale.set_value(SETTING(SLOTS));
	totalSize.set_text(WUtil::ConvertToUTF8("Total size: " +
		Util::formatBytes(share->getShareSize())));
}

//saves the values currently set in different widgets
void SettingsDialog::saveSettings() {
	SettingsManager *mgr = SettingsManager::getInstance();
	int temp;

	//General tab
	mgr->set(SettingsManager::NICK, nickEntry.get_text());
	mgr->set(SettingsManager::EMAIL, emailEntry.get_text());
	mgr->set(SettingsManager::DESCRIPTION, descEntry.get_text());
	mgr->set(SettingsManager::CONNECTION, SettingsManager::connectionSpeeds[connectionOption.get_history()]);

	mgr->set(SettingsManager::SERVER, ipEntry.get_text());
	mgr->set(SettingsManager::IN_PORT, atoi(tcpportEntry.get_text().c_str()));
	mgr->set(SettingsManager::SOCKS_SERVER, sockIpEntry.get_text());
	mgr->set(SettingsManager::SOCKS_PORT,
		atoi(sockPortEntry.get_text().c_str()));
	mgr->set(SettingsManager::SOCKS_USER, userEntry.get_text());
	mgr->set(SettingsManager::SOCKS_PASSWORD, passEntry.get_text());

	mgr->set(SettingsManager::SOCKS_RESOLVE, sock5CB.get_active());

	if (activeRB.get_active())
		mgr->set(SettingsManager::CONNECTION_TYPE,
		SettingsManager::CONNECTION_ACTIVE);
	if (passiveRB.get_active())
		mgr->set(SettingsManager::CONNECTION_TYPE,
		SettingsManager::CONNECTION_PASSIVE);
	if (sock5RB.get_active())
		mgr->set(SettingsManager::CONNECTION_TYPE,
		SettingsManager::CONNECTION_SOCKS5);

	//Download tab
	mgr->set(SettingsManager::DOWNLOAD_DIRECTORY, 
		defDirEntry.get_text());
	if (unfDirEntry.get_text () == "")
		mgr->set(SettingsManager::TEMP_DOWNLOAD_DIRECTORY, SETTING (DOWNLOAD_DIRECTORY));
	else
		mgr->set(SettingsManager::TEMP_DOWNLOAD_DIRECTORY, unfDirEntry.get_text());
	mgr->set(SettingsManager::DOWNLOAD_SLOTS, (int)slotsScale.get_value());
	mgr->set(SettingsManager::MAX_DOWNLOAD_SPEED, (int)speedScale.get_value());
	mgr->set(SettingsManager::HUBLIST_SERVERS, pubHubsEntry.get_text());
	mgr->set(SettingsManager::HTTP_PROXY, httpProxyEntry.get_text());

	//Sharing tab
	mgr->set(SettingsManager::SHARE_HIDDEN, hiddenCB.get_active());
	mgr->set(SettingsManager::MIN_UPLOAD_SPEED, (int)autoOpenScale.get_value());
	mgr->set(SettingsManager::SLOTS, (int)uploadScale.get_value());
}

void SettingsDialog::activeClicked() {
	sockTable.set_sensitive(false);
	sock5CB.set_sensitive(false);

	ipEntry.set_sensitive(true);
	tcpportEntry.set_sensitive(true);
}

void SettingsDialog::passiveClicked() {
	sockTable.set_sensitive(false);
	sock5CB.set_sensitive(false);

	ipEntry.set_sensitive(false);
	tcpportEntry.set_sensitive(false);
}

void SettingsDialog::sockClicked() {
	sockTable.set_sensitive(true);
	sock5CB.set_sensitive(true);

	ipEntry.set_sensitive(false);
	tcpportEntry.set_sensitive(false);
}

void SettingsDialog::addDirectory() {
	string file, name;
	FileSelection sel(WUtil::ConvertToUTF8("Add directory"));
	ShareManager *mgr = ShareManager::getInstance();
	StringPairList directories;
	StringPairList::iterator it;
	TreeModel::iterator jt;
	int start, stop;

	sel.set_select_multiple(false);
	sel.hide_fileop_buttons();
	sel.get_file_list()->get_parent()->hide();
	sel.get_selection_entry()->hide();
	
	if (sel.run() == -5) {
		file = sel.get_filename();
		
		//find virtual name
		if (file[file.length() - 1] == PATH_SEPARATOR)
			stop = file.length() - 2;
		else
			stop = file.length() - 1;
		
		for (start = stop; 
			start>=0 && file[start-1] != PATH_SEPARATOR; start--);
		
		name = file.substr(start, stop - start + 1);
		
		try {
			mgr->addDirectory(file, name);
		} catch (ShareException se) {
			cout << "Could not add " << file << " to share" << endl;
			return;
		}

		mgr->refresh(true, true, true);
		dirModel->clear();
		directories = mgr->getDirectories();
		for(it = directories.begin(); it != directories.end(); it++) {
			jt = dirModel->append();
			(*jt)[columns.virt] = it->first;
			(*jt)[columns.size] = Util::formatBytes(
				mgr->getShareSize(it->second));
			(*jt)[columns.dir] = it->second;
		}

		totalSize.set_text(WUtil::ConvertToUTF8("Total size: ") +
			mgr->getShareSizeString());
	}
}

void SettingsDialog::removeDirectory() {
	RefPtr<TreeSelection> sel = dirView.get_selection(); 
	TreeModel::iterator it = sel->get_selected();
	ShareManager *mgr = ShareManager::getInstance();
	ustring tmpUstring;

	if (!it) return;
	
	tmpUstring = (*it)[columns.dir];
	mgr->removeDirectory(tmpUstring.raw());
	dirModel->erase(it);
	totalSize.set_text(WUtil::ConvertToUTF8("Total size: ") +
		Util::formatBytes(mgr->getShareSize()));
}

void SettingsDialog::defDirClicked() {
	string dir;
	FileSelection sel(WUtil::ConvertToUTF8("Default download directory"));

	sel.set_select_multiple(false);
	sel.hide_fileop_buttons();
	sel.get_file_list()->get_parent()->hide();
	sel.get_selection_entry()->hide();

	if (sel.run() == -5) {
		dir = sel.get_filename();
		if (dir[dir.size() - 1] != PATH_SEPARATOR)
			dir += PATH_SEPARATOR_STR;
		defDirEntry.set_text(dir);
	}
}

void SettingsDialog::tempDirClicked() {
	string dir;
	FileSelection sel(WUtil::ConvertToUTF8("Temp download directory"));

	sel.set_select_multiple(false);
	sel.hide_fileop_buttons();
	sel.get_file_list()->get_parent()->hide();
	sel.get_selection_entry()->hide();

	if (sel.run() == -5) {
		dir = sel.get_filename();
		if (dir[dir.size() - 1] != PATH_SEPARATOR)
			dir += PATH_SEPARATOR_STR;
		unfDirEntry.set_text(dir);
	}
}

