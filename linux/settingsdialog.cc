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
	sockTable(4, 4),
	personalFrame("Personal information"),
	connectionFrame("Connection settings"),
	nick("Nick"),
	email("E-mail"),
	desc("Description"),
	connection("Connection"),
	ip("IP"),
	port("Port (empty=random)"),
	sockIp("Socks IP"),
	sockPort("Port"),
	user("Username"),
	pass("Password"),
	activeRB("Active"),
	passiveRB("Passive"),
	sock5RB("SOCKS5"),
	sock5CB("Use SOCK5 for hostnames"),
	
	
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
	
	add_button("Ok", SETTINGS_DIALOG_OK);
	add_button("Cancel", SETTINGS_DIALOG_CANCEL);

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
	char *items[] = {"28.8k modem", "33.6k modem", "57.6k modem", "ISDN",
		"Satellite", "Cable", "T1", "T3"};
	ThreadTunnel *tunnel = GuiProxy::getInstance()->getTunnel();
	Group g = activeRB.get_group();
	passiveRB.set_group(g);
	sock5RB.set_group(g);
	
	generalBox.pack_start(personalFrame, PACK_EXPAND_WIDGET, 10);
	generalBox.pack_start(connectionFrame, PACK_EXPAND_WIDGET, 10);

	personalFrame.add(infoTable);
	connectionFrame.add(connectionBox);

	for(i=0; i<8; i++) {
		connectionItems[i] = new MenuItem(items[i]);
		connectionMenu.append(*connectionItems[i]);
		manage(connectionItems[i]);
	}
		
	connectionOption.set_menu(connectionMenu);
	
	infoTable.attach(nick, 0, 1, 0, 1);
	infoTable.attach(nickEntry, 1, 2, 0, 1);
	infoTable.attach(email, 0, 1, 1, 2);
	infoTable.attach(emailEntry, 1, 2, 1, 2);
	infoTable.attach(desc, 0, 1, 2, 3);
	infoTable.attach(descEntry, 1, 2, 2, 3);
	infoTable.attach(connection, 0, 1, 3, 4);
	infoTable.attach(connectionOption, 1, 2, 3, 4);
	
	ipBox.pack_start(activeRB, PACK_EXPAND_WIDGET);
	ipBox.pack_start(ip, PACK_EXPAND_WIDGET);
	ipBox.pack_start(ipEntry, PACK_EXPAND_WIDGET);
	ipBox.pack_start(port, PACK_EXPAND_WIDGET);
	ipBox.pack_start(portEntry, PACK_EXPAND_WIDGET);

	sockTable.attach(sockIp, 0, 1, 0, 1);
	sockTable.attach(sockPort, 1, 2, 0, 1);
	sockTable.attach(sockIpEntry, 0, 1, 1, 2);
	sockTable.attach(sockPortEntry, 1, 2, 1, 2);
	sockTable.attach(user, 0, 1, 2, 3);
	sockTable.attach(pass, 1, 2, 2, 3);
	sockTable.attach(userEntry, 0, 1, 3, 4);
	sockTable.attach(passEntry, 1, 2, 3, 4);
	
	connectionBox.pack_start(ipBox, PACK_EXPAND_WIDGET);
	connectionBox.pack_start(passiveRB, PACK_EXPAND_WIDGET);
	connectionBox.pack_start(sock5RB, PACK_EXPAND_WIDGET);
	connectionBox.pack_start(sockTable, PACK_EXPAND_WIDGET);
	connectionBox.pack_start(sock5CB, PACK_EXPAND_WIDGET);

	//callback = slot(*this, &SettingsDialog::activeClicked);
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
	dirView.append_column("Directory", columns.dir);
	dirView.append_column("Size", columns.size);

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
	StringList directories = share->getDirectories();
	StringIter it;
	TreeModel::iterator jt;

	//General tab
	nickEntry.set_text(locale_to_utf8(SETTING(NICK)));
	emailEntry.set_text(locale_to_utf8(SETTING(EMAIL)));
	descEntry.set_text(locale_to_utf8(SETTING(DESCRIPTION)));
	connectionOption.set_history(SETTING(CONNECTION_TYPE));
	
	ipEntry.set_text(locale_to_utf8(SETTING(SERVER)));
	sprintf(temp, "%d", SETTING(IN_PORT));
	portEntry.set_text(locale_to_utf8(temp));
	sockIpEntry.set_text(locale_to_utf8(SETTING(SOCKS_SERVER)));
	sprintf(temp, "%d", SETTING(SOCKS_PORT));
	sockPortEntry.set_text(locale_to_utf8(temp));
	userEntry.set_text(locale_to_utf8(SETTING(SOCKS_USER)));
	passEntry.set_text(locale_to_utf8(SETTING(SOCKS_PASSWORD)));
	sock5CB.set_active(BOOLSETTING(SOCKS_RESOLVE));

	if (SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		activeClicked();
		activeRB.set_active(true);
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
	defDirEntry.set_text(locale_to_utf8(SETTING(DOWNLOAD_DIRECTORY)));
	unfDirEntry.set_text(locale_to_utf8(SETTING(TEMP_DOWNLOAD_DIRECTORY)));
	slotsScale.set_value(SETTING(DOWNLOAD_SLOTS));
	speedScale.set_value(SETTING(MAX_DOWNLOAD_SPEED));
	pubHubsEntry.set_text(locale_to_utf8(SETTING(HUBLIST_SERVERS)));
	httpProxyEntry.set_text(locale_to_utf8(SETTING(HTTP_PROXY)));

	//Sharing tab
	for(it = directories.begin(); it != directories.end(); it++) {
		jt = dirModel->append();
		(*jt)[columns.dir] = locale_to_utf8(*it);
		(*jt)[columns.size] = 
			locale_to_utf8(Util::formatBytes(share->getShareSize(*it)));
	}

	hiddenCB.set_active(BOOLSETTING(SHARE_HIDDEN));
	autoOpenScale.set_value(SETTING(MIN_UPLOAD_SPEED));
	uploadScale.set_value(SETTING(SLOTS));
	totalSize.set_text(locale_to_utf8("Total size: " + 		
		Util::formatBytes(share->getShareSize())));
}

//saves the values currently set in different widgets
void SettingsDialog::saveSettings() {
	SettingsManager *mgr = SettingsManager::getInstance();
	int temp;

	//General tab
	mgr->set(SettingsManager::NICK, locale_from_utf8(nickEntry.get_text()));
	mgr->set(SettingsManager::EMAIL, locale_from_utf8(emailEntry.get_text()));
	mgr->set(SettingsManager::DESCRIPTION,
		locale_from_utf8(descEntry.get_text()));
	mgr->set(SettingsManager::CONNECTION_TYPE, connectionOption.get_history());

	mgr->set(SettingsManager::SERVER, locale_from_utf8(ipEntry.get_text()));
	mgr->set(SettingsManager::IN_PORT, atoi(portEntry.get_text().c_str()));
	mgr->set(SettingsManager::SOCKS_SERVER,
		locale_from_utf8(sockIpEntry.get_text()));
	mgr->set(SettingsManager::SOCKS_PORT,
		atoi(sockPortEntry.get_text().c_str()));
	mgr->set(SettingsManager::SOCKS_USER,
		locale_from_utf8(userEntry.get_text()));
	mgr->set(SettingsManager::SOCKS_PASSWORD,
		locale_from_utf8(passEntry.get_text()));

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
		locale_from_utf8(defDirEntry.get_text()));
	mgr->set(SettingsManager::TEMP_DOWNLOAD_DIRECTORY,
		locale_from_utf8(unfDirEntry.get_text()));
	mgr->set(SettingsManager::DOWNLOAD_SLOTS, (int)slotsScale.get_value());
	mgr->set(SettingsManager::MAX_DOWNLOAD_SPEED, (int)speedScale.get_value());
	mgr->set(SettingsManager::HUBLIST_SERVERS,
		locale_from_utf8(pubHubsEntry.get_text()));
	mgr->set(SettingsManager::HTTP_PROXY,
		locale_from_utf8(httpProxyEntry.get_text()));

	//Sharing tab
	mgr->set(SettingsManager::SHARE_HIDDEN, hiddenCB.get_active());
	mgr->set(SettingsManager::MIN_UPLOAD_SPEED, (int)autoOpenScale.get_value());
	mgr->set(SettingsManager::SLOTS, (int)uploadScale.get_value());
}

void SettingsDialog::activeClicked() {
	sockTable.set_sensitive(false);
	sock5CB.set_sensitive(false);

	ip.set_sensitive(true);
	ipEntry.set_sensitive(true);
	port.set_sensitive(true);
	portEntry.set_sensitive(true);
}

void SettingsDialog::passiveClicked() {
	sockTable.set_sensitive(false);
	sock5CB.set_sensitive(false);

	ip.set_sensitive(false);
	ipEntry.set_sensitive(false);
	port.set_sensitive(false);
	portEntry.set_sensitive(false);
}

void SettingsDialog::sockClicked() {
	sockTable.set_sensitive(true);
	sock5CB.set_sensitive(true);

	ip.set_sensitive(false);
	ipEntry.set_sensitive(false);
	port.set_sensitive(false);
	portEntry.set_sensitive(false);
}

void SettingsDialog::addDirectory() {
	string file;
	FileSelection sel(locale_to_utf8("Add directory"));
	ShareManager *mgr = ShareManager::getInstance();
	StringList directories;
	StringIter it;
	TreeModel::iterator jt;

	sel.set_select_multiple(false);
	sel.hide_fileop_buttons();
	sel.get_file_list()->get_parent()->hide();
	sel.get_selection_entry()->hide();
	
	if (sel.run() == -5) {
		file = sel.get_filename();
		
		try {
			mgr->addDirectory(file);
		} catch (ShareException se) {
			cout << "Could not add " << file << " to share" << endl;
			return;		
		}

		//ugly but works
		mgr->refresh(true, true, true);
		dirModel->clear();
		directories = mgr->getDirectories();
		for(it = directories.begin(); it != directories.end(); it++) {
			jt = dirModel->append();
			(*jt)[columns.dir] = locale_to_utf8(*it);
			(*jt)[columns.size] = 
				locale_to_utf8(Util::formatBytes(mgr->getShareSize(*it)));
		}

		totalSize.set_text(locale_to_utf8("Total size: " + 		
			Util::formatBytes(mgr->getShareSize())));
	}
}

void SettingsDialog::removeDirectory() {
	RefPtr<TreeSelection> sel = dirView.get_selection(); 
	TreeModel::iterator it = sel->get_selected();
	ShareManager *mgr = ShareManager::getInstance();
	
	if (it == NULL) return;
	mgr->removeDirectory(locale_from_utf8((*it)[columns.dir]));
	dirModel->erase(it);
	totalSize.set_text(locale_to_utf8("Total size: " + 		
		Util::formatBytes(mgr->getShareSize())));
}

void SettingsDialog::defDirClicked() {
	string dir;
	FileSelection sel(locale_to_utf8("Default download directory"));

	sel.set_select_multiple(false);
	sel.hide_fileop_buttons();
	sel.get_file_list()->get_parent()->hide();
	sel.get_selection_entry()->hide();

	if (sel.run() == -5) {
		dir = sel.get_filename();
		defDirEntry.set_text(locale_to_utf8(dir));
	}
}

void SettingsDialog::tempDirClicked() {
	string dir;
	FileSelection sel(locale_to_utf8("Temp download directory"));

	sel.set_select_multiple(false);
	sel.hide_fileop_buttons();
	sel.get_file_list()->get_parent()->hide();
	sel.get_selection_entry()->hide();

	if (sel.run() == -5) {
		dir = sel.get_filename();
		unfDirEntry.set_text(locale_to_utf8(dir));
	}
}

