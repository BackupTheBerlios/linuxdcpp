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

#ifndef WULFOR_SETTINGS_DIALOG
#define WULFOR_SETTINGS_DIALOG

#include <gtkmm.h>

class MainWindow;

class SettingsDialog: public Gtk::Dialog {
	public:
		SettingsDialog(MainWindow *mw);
		~SettingsDialog();
		void saveSettings();
		
		void activeClicked();
		void passiveClicked();
		void sockClicked();
		void addDirectory();
		void removeDirectory();
		void defDirClicked();
		void tempDirClicked();		

		static const int OK = 0, CANCEL = 1;

	private:
		class DirColumns: public Gtk::TreeModel::ColumnRecord {
			public:
				DirColumns() {
					add(dir);
					add(virt);
					add(size);
				}

				Gtk::TreeModelColumn<Glib::ustring> dir, size, virt;
		};

		void createGeneral();
		void createDownload();
		void createSharing();
		void setValues();
		
		Gtk::Notebook book;
		Gtk::Frame personalFrame, connectionFrame, dirFrame, limitFrame,
			 hubListFrame, shareFrame;
		Gtk::TreeView dirView;
		Glib::RefPtr<Gtk::ListStore> dirModel;
		DirColumns columns;
		Gtk::Label nick, email, desc, connection;
		Gtk::Label ip, tcpport, udpport, sockIp, sockPort, user, pass;
		
		Gtk::VBox connectionBox, generalBox, sharingBox, downloadBox;
		Gtk::HBox ipBox;
		Gtk::Table infoTable, conTable, sockTable, dirTable, slotTable;
		
		Gtk::Entry nickEntry, emailEntry, descEntry, ipEntry, tcpportEntry, udpportEntry;
		Gtk::Entry sockIpEntry, sockPortEntry, userEntry, passEntry;
		
		Gtk::OptionMenu connectionOption;
		Gtk::Menu connectionMenu;
		Gtk::MenuItem *connectionItems[SettingsManager::SPEED_LAST];
		Gtk::CheckButton sock5CB;
		Gtk::RadioButton activeRB, passiveRB, sock5RB;
		
		Gtk::VBox hubListBox;
		Gtk::Label defDir, unfDir, slots, speed, note, pubHubs, httpProxy;
		Gtk::Entry defDirEntry, unfDirEntry, pubHubsEntry, httpProxyEntry;
		Gtk::HScale slotsScale, speedScale;
		Gtk::Button defBrowse, unfBrowse;

		Gtk::VBox shareBox;
		Gtk::HBox shareOptions;
		Gtk::Table shareTable;
		Gtk::Label totalSize, autoOpen, upload, kb;
		Gtk::CheckButton hiddenCB;
		Gtk::Button add, remove;
		Gtk::HScale autoOpenScale, uploadScale;
};

#else
class SettingsDialog
#endif
