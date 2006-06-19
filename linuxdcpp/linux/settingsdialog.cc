/* 
 * Copyright © 2004-2006 Jens Oknelid, paskharen@gmail.com
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

#include "settingsdialog.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include <iostream>
#include <sstream>

using namespace std;

Settings::~Settings()
{
	pthread_mutex_destroy(&settingsLock);
	gtk_widget_destroy(favoriteName);
	gtk_widget_destroy(publicHubs);
	gtk_widget_destroy(editPublic);
	gtk_widget_destroy(virtualName);
	gtk_widget_destroy(dirChooser);
}

Settings::Settings()
{
	string file = WulforManager::get()->getPath() + "/glade/settingsdialog.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);
	if (xml == NULL)
	{
		cout << "Error: Missing required glade file: " << file << endl;
		exit(1);
	}

	dialog = glade_xml_get_widget(xml, "settingsDialog");
	favoriteName = glade_xml_get_widget(xml, "favoriteName");
	publicHubs = glade_xml_get_widget(xml, "publicDialog");
	editPublic = glade_xml_get_widget(xml, "editPublic");
	virtualName = glade_xml_get_widget(xml, "virtualName");
	dirChooser = glade_xml_get_widget(xml, "dirChooserDialog");

	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(favoriteName), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(publicHubs), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(editPublic), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(virtualName), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dirChooser), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	{ // General
		// Personal
		generalItems["Nick"] = glade_xml_get_widget(xml, "entryNick");
		generalItems["EMail"] = glade_xml_get_widget(xml, "entryEmail");
		generalItems["Description"] = glade_xml_get_widget(xml, "entryDesc");
		generalItems["Connection"] = glade_xml_get_widget(xml, "connectionBox");
		{ // Connection
			// Incomming
			connectionItems["Incomming_Direct"] = glade_xml_get_widget(xml, "radiobuttonIN-Direct");
			g_signal_connect(G_OBJECT(connectionItems["Incomming_Direct"]), "toggled", G_CALLBACK (onInDirect_gui), (gpointer)this);
			///@todo Uncomment when implemented
			//connectionItems["Incomming_FW_UPnP"] = glade_xml_get_widget(xml, "radiobuttonFW_UPnP");
			//g_signal_connect(G_OBJECT(connectionItems["Incomming_FW_UPnP"]), "toggled", G_CALLBACK(onInFW_UPnP_gui), (gpointer)this);
			connectionItems["Incomming_FW_NAT"] = glade_xml_get_widget(xml, "radiobuttonFW_NAT");
			g_signal_connect(G_OBJECT(connectionItems["Incomming_FW_NAT"]), "toggled", G_CALLBACK(onInFW_NAT_gui), (gpointer)this);
			connectionItems["ForceIP"] = glade_xml_get_widget(xml, "checkbuttonForce");

			connectionItems["Passive"] = glade_xml_get_widget(xml, "radiobuttonPassive");
			g_signal_connect(G_OBJECT(connectionItems["Passive"]), "toggled", G_CALLBACK(onInPassive_gui), (gpointer)this);
			// Outgoing
			connectionItems["Outgoing_Direct"] = glade_xml_get_widget(xml, "radiobuttonOutDirect");
			g_signal_connect(G_OBJECT(connectionItems["Outgoing_Direct"]), "toggled", G_CALLBACK(onOutDirect_gui), (gpointer)this);
			connectionItems["SOCKS5"] = glade_xml_get_widget(xml, "radiobuttonSocks");
			g_signal_connect(G_OBJECT (connectionItems["SOCKS5"]), "toggled", G_CALLBACK(onSocks5_gui), (gpointer)this);

			connectionItems["IP"] = glade_xml_get_widget(xml, "entryIP");
			connectionItems["IPLabel"] = glade_xml_get_widget(xml, "labelIP");
			connectionItems["TCP"] = glade_xml_get_widget(xml, "entryTCP");
			connectionItems["TCPLabel"] = glade_xml_get_widget(xml, "labelTCP");
			connectionItems["UDP"] = glade_xml_get_widget(xml, "entryUDP");
			connectionItems["UDPLabel"] = glade_xml_get_widget(xml, "labelUDP");
			connectionItems["Socks"] = glade_xml_get_widget(xml, "entrySocksIP");
			connectionItems["SocksLabel"] = glade_xml_get_widget(xml, "labelSocksIP");
			connectionItems["Username"] = glade_xml_get_widget(xml, "entryUser");
			connectionItems["UserLabel"] = glade_xml_get_widget(xml, "labelUser");
			connectionItems["Port"] = glade_xml_get_widget(xml, "entryPort");
			connectionItems["PortLabel"] = glade_xml_get_widget(xml, "labelPort");
			connectionItems["Password"] = glade_xml_get_widget(xml, "entryPass");
			connectionItems["PassLabel"] = glade_xml_get_widget(xml, "labelPass");
			connectionItems["Resolve"] = glade_xml_get_widget(xml, "checkbuttonSocks");
		}
	}
	{// Downloads
		downloadItems["Finished"] = glade_xml_get_widget(xml, "entryFinished");
		downloadItems["Unfinished"] = glade_xml_get_widget(xml, "entryUnFinished");
		downloadItems["ButtonF"] = glade_xml_get_widget(xml, "buttonFinished");
		g_signal_connect (G_OBJECT(downloadItems["ButtonF"]), "clicked", G_CALLBACK(onBrowseF_gui), (gpointer)this);
		downloadItems["ButtonUF"] = glade_xml_get_widget(xml, "buttonUnfinished");
		g_signal_connect (G_OBJECT(downloadItems["ButtonUF"]), "clicked", G_CALLBACK(onBrowseUF_gui), (gpointer)this);
		downloadItems["Downloads"] = glade_xml_get_widget(xml, "spinbuttonDownloads");
		downloadItems["New"] = glade_xml_get_widget(xml, "spinbuttonNew");
		downloadItems["Public hubs"] = glade_xml_get_widget(xml, "buttonPublic");
		g_signal_connect (G_OBJECT(downloadItems["Public hubs"]), "clicked", G_CALLBACK(onPublicHubs_gui), (gpointer)this);
		downloadItems["Proxy"] = glade_xml_get_widget(xml, "entryProxy");

		{ // Download to
			downloadItems["View"] = glade_xml_get_widget(xml, "treeviewFavorite");
			downloadItems["Remove"] = glade_xml_get_widget(xml, "buttonRemoveTo");
			g_signal_connect (G_OBJECT (downloadItems["Remove"]), "clicked", G_CALLBACK(onRemoveFavorite_gui), (gpointer)this);
			downloadItems["Add"] = glade_xml_get_widget(xml, "buttonAddTo");
			g_signal_connect (G_OBJECT (downloadItems["Add"]), "clicked", G_CALLBACK(onAddFavorite_gui), (gpointer)this);
			downloadItems["Name"] = glade_xml_get_widget(xml, "entryFavorite");
		}

		{ // Public Hubs
			downloadItems["Public list"] = glade_xml_get_widget(xml, "entryPublicHubs");
			downloadItems["Public add"] = glade_xml_get_widget(xml, "buttonPublicAdd");
			g_signal_connect (G_OBJECT (downloadItems["Public add"]), "clicked", G_CALLBACK(onPublicAdd_gui), (gpointer)this);
			downloadItems["Public mu"] = glade_xml_get_widget(xml, "buttonPublicMU");
			g_signal_connect (G_OBJECT (downloadItems["Public mu"]), "clicked", G_CALLBACK(onPublicMU_gui), (gpointer)this);
			downloadItems["Public md"] = glade_xml_get_widget(xml, "buttonPublicMD");
			g_signal_connect (G_OBJECT (downloadItems["Public md"]), "clicked", G_CALLBACK(onPublicMD_gui), (gpointer)this);
			downloadItems["Public edit"] = glade_xml_get_widget(xml, "buttonPublicEdit");
			g_signal_connect (G_OBJECT (downloadItems["Public edit"]), "clicked", G_CALLBACK(onPublicEdit_gui), (gpointer)this);
			downloadItems["Public remove"] = glade_xml_get_widget(xml, "buttonPublicRemove");
			g_signal_connect (G_OBJECT (downloadItems["Public remove"]), "clicked", G_CALLBACK(onPublicRemove_gui), (gpointer)this);
			downloadItems["Public view"] = glade_xml_get_widget(xml, "treeviewPublic");
			downloadItems["Edit public"] = glade_xml_get_widget(xml, "entryPublicEdit");
		}

		{ // Queue
			// Autoprio
			queueItems["prioHighest"] = glade_xml_get_widget(xml, "prioHighest");
			queueItems["prioNorm"] = glade_xml_get_widget(xml, "prioNorm");
			queueItems["prioHigh"] = glade_xml_get_widget(xml, "prioHigh");
			queueItems["prioLow"] = glade_xml_get_widget(xml, "prioLow");

			//Autodrop
			queueItems["dropSpeed"] = glade_xml_get_widget(xml, "dropSpeed");
			queueItems["dropElapsed"] = glade_xml_get_widget(xml, "dropElapsed");
			queueItems["dropMinSources"] = glade_xml_get_widget(xml, "dropMinSources");
			queueItems["dropCheck"] = glade_xml_get_widget(xml, "dropCheck");
			queueItems["dropInactiv"] = glade_xml_get_widget(xml, "dropInactiv");
			queueItems["dropSize"] = glade_xml_get_widget(xml, "dropSize");

			//Other settings
			queueItems["Options"] = glade_xml_get_widget(xml, "queueOther");
		}
	}
	{// Sharing
		shareItems["Shares"] = glade_xml_get_widget(xml, "treeviewShared");
		shareItems["Size"] = glade_xml_get_widget(xml, "labelSize");
		shareItems["Hidden"] = glade_xml_get_widget(xml, "checkbuttonHidden");
		g_signal_connect(G_OBJECT (shareItems["Hidden"]), "toggled", G_CALLBACK(onShareHiddenPressed_gui), (gpointer)this);
		shareItems["Remove"] = glade_xml_get_widget(xml, "buttonRemoveShared");
		g_signal_connect(G_OBJECT (shareItems["Remove"]), "clicked", G_CALLBACK(onRemoveShare_gui), (gpointer)this);
		shareItems["Add"] = glade_xml_get_widget(xml, "buttonAddShared");
		g_signal_connect(G_OBJECT (shareItems["Add"]), "clicked", G_CALLBACK(onAddShare_gui), (gpointer)this);
		shareItems["Extra"] = glade_xml_get_widget(xml, "spinbuttonExtraSlot");
		shareItems["Upload"] = glade_xml_get_widget(xml, "spinbuttonUploadSlot");
		shareItems["Virtual"] = glade_xml_get_widget(xml, "entryVirtual");
	}
	{// Appearance
		appearanceItems["Options"] = glade_xml_get_widget(xml, "Options");
		appearanceItems["Away"] = glade_xml_get_widget(xml, "entryAway");
		appearanceItems["Timestamp"] = glade_xml_get_widget(xml, "entryTimestamp");

		{ //Color and sound
			///@todo uncomment when implemented
			//appearanceItems["winColor"] = glade_xml_get_widget(xml, "winColor");
			//appearanceItems["upColor"] = glade_xml_get_widget(xml, "upColor");
			//appearanceItems["downColor"] = glade_xml_get_widget(xml, "downColor");
			//appearanceItems["textStyle"] = glade_xml_get_widget(xml, "textStyle");
			//g_signal_connect(G_OBJECT(appearanceItems["winColor"]), "clicked", G_CALLBACK(onWinColorClicked_gui), (gpointer)this);
			//g_signal_connect(G_OBJECT(appearanceItems["upColor"]), "clicked", G_CALLBACK(onUpColorClicked_gui), (gpointer)this);
			//g_signal_connect(G_OBJECT(appearanceItems["downColor"]), "clicked", G_CALLBACK(onDownColorClicked_gui), (gpointer)this);
			//g_signal_connect(G_OBJECT(appearanceItems["textStyle"]), "clicked", G_CALLBACK(onTextStyleClicked_gui), (gpointer)this);

			appearanceItems["Bolding"] = glade_xml_get_widget(xml, "ColorTreeView");
		}
		{ // Window
			appearanceItems["windowsTreeView1"] = glade_xml_get_widget(xml, "windowsTreeView1");
			appearanceItems["windowsTreeView2"] = glade_xml_get_widget(xml, "windowsTreeView2");
			appearanceItems["windowsTreeView3"] = glade_xml_get_widget(xml, "windowsTreeView3");
		}
	}
	{// Logs and sound
		logItems["Directory"] = glade_xml_get_widget(xml, "entryDir");
		logItems["Browse"] = glade_xml_get_widget(xml, "buttonLog");
		g_signal_connect(G_OBJECT(logItems["Browse"]), "clicked", G_CALLBACK(onLogBrowseClicked_gui), (gpointer)this);
		logItems["Main"] = glade_xml_get_widget(xml, "checkbuttonMain");
		g_signal_connect(G_OBJECT(logItems["Main"]), "toggled", G_CALLBACK(onLogMainClicked_gui), (gpointer)this);
		logItems["Format1"] = glade_xml_get_widget(xml, "labelFormat1");
		logItems["EMain"] = glade_xml_get_widget(xml, "entryMain");
		logItems["Private"] = glade_xml_get_widget(xml, "checkbuttonPrivate");
		g_signal_connect(G_OBJECT(logItems["Private"]), "toggled", G_CALLBACK(onLogPrivateClicked_gui), (gpointer)this);
		logItems["Format2"] = glade_xml_get_widget(xml, "labelFormat2");
		logItems["EPrivate"] = glade_xml_get_widget(xml, "entryPrivate");
		logItems["Download"] = glade_xml_get_widget(xml, "checkbuttonDownloads");
		g_signal_connect(G_OBJECT(logItems["Download"]), "toggled", G_CALLBACK(onLogDownloadClicked_gui), (gpointer)this);
		logItems["Format3"] = glade_xml_get_widget(xml, "labelFormat3");
		logItems["EDownload"] = glade_xml_get_widget(xml, "entryDownloads");
		logItems["Upload"] = glade_xml_get_widget(xml, "checkbuttonUploads");
		g_signal_connect(G_OBJECT(logItems["Upload"]), "toggled", G_CALLBACK(onLogUploadClicked_gui), (gpointer)this);
		logItems["Format4"] = glade_xml_get_widget(xml, "labelFormat4");
		logItems["EUpload"] = glade_xml_get_widget(xml, "entryUploads");

		logItems["System"] = glade_xml_get_widget(xml, "checkbuttonSystem");
		logItems["Status"] = glade_xml_get_widget(xml, "checkbuttonStatus");

		logItems["SPrivate"] = glade_xml_get_widget(xml, "checkbuttonPM");
		logItems["SPrivateWindow"] = glade_xml_get_widget(xml, "checkbuttonPMW");
	}
	{// Advanced
		advancedItems["rollBack"] = glade_xml_get_widget(xml, "rollBack");
		advancedItems["hash"] = glade_xml_get_widget(xml, "hash");
		advancedItems["write"] = glade_xml_get_widget(xml, "write");
		advancedItems["Advanced"] = glade_xml_get_widget(xml, "treeviewAdvanced");
		advancedItems["PM"] = glade_xml_get_widget(xml, "PM");
		advancedItems["slotSize"] = glade_xml_get_widget(xml, "slotSize");
		advancedItems["fileListSize"] = glade_xml_get_widget(xml, "fileListSize");
		advancedItems["CID"] = glade_xml_get_widget(xml, "CID");
		advancedItems["refresh"] = glade_xml_get_widget(xml, "refresh");
		advancedItems["tabs"] = glade_xml_get_widget(xml, "tabs");
		advancedItems["searchHistory"] = glade_xml_get_widget(xml, "searchHistory");
		advancedItems["bind"] = glade_xml_get_widget(xml, "bind");
		advancedItems["socketRead"] = glade_xml_get_widget(xml, "socketRead");
		advancedItems["socketWrite"] = glade_xml_get_widget(xml, "socketWrite");
		advancedItems["Encoding"] = glade_xml_get_widget(xml, "entryEncoding");
	}

	pthread_mutex_init(&settingsLock, NULL);

	initGeneral_gui();
	initConnection_gui();
	initDownloads_gui();
	initSharing_gui();
	initAppearance_gui();
	initLog_gui();
	initAdvanced_gui();
}

void Settings::saveSettings_client()
{
	SettingsManager *sm = SettingsManager::getInstance();
	GtkTreeIter iter;
	GtkTreeModel *m;
	gboolean valid, toggled;
	SettingsManager::IntSetting setting;

	{ // General
		pthread_mutex_lock(&settingsLock);
		sm->set(SettingsManager::NICK, gtk_entry_get_text(GTK_ENTRY(generalItems["Nick"])));
		sm->set(SettingsManager::EMAIL, gtk_entry_get_text(GTK_ENTRY(generalItems["EMail"])));
		sm->set(SettingsManager::DESCRIPTION, gtk_entry_get_text(GTK_ENTRY(generalItems["Description"])));
		sm->set(SettingsManager::UPLOAD_SPEED, SettingsManager::connectionSpeeds[gtk_combo_box_get_active(GTK_COMBO_BOX(generalItems["Connection_Speed"]))]);

		{ // Network
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(connectionItems["Incomming_Direct"])))
				sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_DIRECT);
			else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(connectionItems["Incomming_FW_NAT"])))
				sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_NAT);
			else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(connectionItems["Passive"])))
				sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);
			else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(connectionItems["SOCKS5"])))
			{
				sm->set (SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);
				sm->set (SettingsManager::OUTGOING_CONNECTIONS, SettingsManager::OUTGOING_SOCKS5);
			}
			sm->set(SettingsManager::EXTERNAL_IP, gtk_entry_get_text(GTK_ENTRY(connectionItems["IP"])));
			sm->set(SettingsManager::NO_IP_OVERRIDE, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(connectionItems["ForceIP"])));
			sm->set(SettingsManager::SOCKS_SERVER, gtk_entry_get_text(GTK_ENTRY(connectionItems["Socks"])));
			sm->set(SettingsManager::SOCKS_USER, gtk_entry_get_text(GTK_ENTRY(connectionItems["Username"])));
			sm->set(SettingsManager::SOCKS_PASSWORD, gtk_entry_get_text(GTK_ENTRY(connectionItems["Password"])));

			int port = Util::toInt(gtk_entry_get_text(GTK_ENTRY(connectionItems["TCP"])));
			if (port > 0 && port <= 65535) // atoi returns 0 on error
				sm->set(SettingsManager::TCP_PORT, port);
			port = Util::toInt(gtk_entry_get_text(GTK_ENTRY(connectionItems["UDP"])));
			if (port > 0 && port <= 65535)
				sm->set(SettingsManager::UDP_PORT, port);
			port = Util::toInt(gtk_entry_get_text(GTK_ENTRY(connectionItems["Port"])));
			if (port > 0 && port <= 65535)
				sm->set(SettingsManager::SOCKS_PORT, port);
		}
	}

	{// Downloads
		string tmp = gtk_entry_get_text(GTK_ENTRY(downloadItems["Finished"]));
		if (tmp[tmp.length() - 1] != PATH_SEPARATOR)
			tmp += PATH_SEPARATOR;
		sm->set(SettingsManager::DOWNLOAD_DIRECTORY, tmp.c_str());

		tmp = gtk_entry_get_text(GTK_ENTRY(downloadItems["Unfinished"]));
		if (!tmp.empty() && tmp[tmp.length() - 1] != PATH_SEPARATOR)
			tmp += PATH_SEPARATOR;
		sm->set(SettingsManager::TEMP_DOWNLOAD_DIRECTORY, tmp.c_str());
		sm->set(SettingsManager::DOWNLOAD_SLOTS, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(downloadItems["Downloads"])));
		sm->set(SettingsManager::MAX_DOWNLOAD_SPEED, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(downloadItems["New"])));
		sm->set(SettingsManager::HTTP_PROXY, gtk_entry_get_text(GTK_ENTRY(downloadItems["Proxy"])));

		{ //Queue
			// Autoprio
			sm->set(SettingsManager::PRIO_HIGHEST_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(queueItems["prioHighest"]))));
			sm->set(SettingsManager::PRIO_NORMAL_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(queueItems["prioNorm"]))));
			sm->set(SettingsManager::PRIO_HIGH_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(queueItems["prioHigh"]))));
			sm->set(SettingsManager::PRIO_LOW_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(queueItems["prioLow"]))));

			//Autodrop
			sm->set(SettingsManager::AUTODROP_SPEED, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(queueItems["dropSpeed"]))));
			sm->set(SettingsManager::AUTODROP_ELAPSED, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(queueItems["dropElapsed"]))));
			sm->set(SettingsManager::AUTODROP_MINSOURCES, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(queueItems["dropMinSources"]))));
			sm->set(SettingsManager::AUTODROP_INTERVAL, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(queueItems["dropCheck"]))));
			sm->set(SettingsManager::AUTODROP_INACTIVITY, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(queueItems["dropInactiv"]))));
			sm->set(SettingsManager::AUTODROP_FILESIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(queueItems["dropSize"]))));

			//queueSettings
			m = GTK_TREE_MODEL(queueStore);
			valid = gtk_tree_model_get_iter_first(m, &iter);

			while (valid)
			{
				setting = (SettingsManager::IntSetting)queueView.getValue<gint>(&iter, "Setting");
				toggled = queueView.getValue<gboolean>(&iter, "Use");
				sm->set(setting, toggled);
				valid = gtk_tree_model_iter_next(m, &iter);
			}
		}
	}

	{// Sharing
		sm->set(SettingsManager::MIN_UPLOAD_SPEED, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(shareItems["Extra"])));
		sm->set(SettingsManager::SLOTS, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(shareItems["Upload"])));
	}

	{// Appearance
		m = GTK_TREE_MODEL(appearanceStore);
		valid = gtk_tree_model_get_iter_first(m, &iter);

		while (valid)
		{
			setting = (SettingsManager::IntSetting)appearanceView.getValue<gint>(&iter, "Setting");
			toggled = appearanceView.getValue<gboolean>(&iter, "Use");
			sm->set(setting, toggled);
			valid = gtk_tree_model_iter_next(m, &iter);
		}

		///@todo: figre out how to use this...
		//WSET("show-tray-icon", appearanceView.getValue<gboolean>(&iter, "Use"););
		sm->set(SettingsManager::DEFAULT_AWAY_MESSAGE, string(gtk_entry_get_text (GTK_ENTRY (appearanceItems["Away"]))));
		sm->set(SettingsManager::TIME_STAMPS_FORMAT, string(gtk_entry_get_text (GTK_ENTRY (appearanceItems["Timestamp"]))));

		{ //Color and sound
			{ // Color
				m = GTK_TREE_MODEL(colorStore);
				valid = gtk_tree_model_get_iter_first(m, &iter);

				while (valid)
				{
					setting = (SettingsManager::IntSetting)colorView.getValue<gint>(&iter, "Setting");
					toggled = colorView.getValue<gboolean>(&iter, "Use");
					sm->set(setting, toggled);
					valid = gtk_tree_model_iter_next(m, &iter);
				}
			}
			{ // Sound
				sm->set(SettingsManager::PRIVATE_MESSAGE_BEEP, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logItems["SPrivate"])));
				sm->set(SettingsManager::PRIVATE_MESSAGE_BEEP_OPEN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logItems["SPrivateWindow"])));
			}
		}
		{ // Window
			m = GTK_TREE_MODEL(windowStore1);
			valid = gtk_tree_model_get_iter_first(m, &iter);

			while (valid)
			{
				setting = (SettingsManager::IntSetting)windowView1.getValue<gint>(&iter, "Setting");
				toggled = windowView1.getValue<gboolean>(&iter, "Use");
				sm->set(setting, toggled);
				valid = gtk_tree_model_iter_next(m, &iter);
			}

			m = GTK_TREE_MODEL(windowStore2);
			valid = gtk_tree_model_get_iter_first(m, &iter);

			while (valid)
			{
				setting = (SettingsManager::IntSetting)windowView2.getValue<gint>(&iter, "Setting");
				toggled = windowView2.getValue<gboolean>(&iter, "Use");
				sm->set(setting, toggled);
				valid = gtk_tree_model_iter_next(m, &iter);
			}

			m = GTK_TREE_MODEL(windowStore3);
			valid = gtk_tree_model_get_iter_first(m, &iter);

			while (valid)
			{
				setting = (SettingsManager::IntSetting)windowView3.getValue<gint>(&iter, "Setting");
				toggled = windowView3.getValue<gboolean>(&iter, "Use");
				sm->set(setting, toggled);
				valid = gtk_tree_model_iter_next(m, &iter);
			}
		}
	}

	{// Logs and sound
		sm->set(SettingsManager::LOG_DIRECTORY, string(gtk_entry_get_text(GTK_ENTRY(logItems["Directory"]))));
		sm->set(SettingsManager::LOG_MAIN_CHAT, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logItems["Main"])));
		sm->set(SettingsManager::LOG_FORMAT_MAIN_CHAT, string(gtk_entry_get_text(GTK_ENTRY(logItems["EMain"]))));
		sm->set(SettingsManager::LOG_PRIVATE_CHAT, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logItems["Private"])));
		sm->set(SettingsManager::LOG_FORMAT_PRIVATE_CHAT, string(gtk_entry_get_text(GTK_ENTRY(logItems["EPrivate"]))));
		sm->set(SettingsManager::LOG_DOWNLOADS, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logItems["Download"])));
		sm->set(SettingsManager::LOG_FORMAT_POST_DOWNLOAD, string(gtk_entry_get_text(GTK_ENTRY(logItems["EDownload"]))));
		sm->set(SettingsManager::LOG_UPLOADS, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logItems["Upload"])));
		sm->set(SettingsManager::LOG_FORMAT_POST_UPLOAD, string(gtk_entry_get_text(GTK_ENTRY(logItems["EUpload"]))));

		sm->set(SettingsManager::LOG_SYSTEM, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logItems["System"])));
		sm->set(SettingsManager::LOG_STATUS_MESSAGES, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logItems["Status"])));
	}

	{// Advanced
		m = GTK_TREE_MODEL(advancedStore);
		valid = gtk_tree_model_get_iter_first(m, &iter);

		while (valid)
		{
			setting = (SettingsManager::IntSetting)advancedView.getValue<gint>(&iter, "Setting");
			toggled = advancedView.getValue<gboolean>(&iter, "Use");
			sm->set(setting, toggled);
			valid = gtk_tree_model_iter_next(m, &iter);
		}

		{ // Expert
			sm->set(SettingsManager::ROLLBACK, Util::toString(gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(advancedItems["rollBack"]))));
			sm->set(SettingsManager::MAX_HASH_SPEED, Util::toString(gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(advancedItems["hash"]))));
			sm->set(SettingsManager::BUFFER_SIZE, Util::toString(gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(advancedItems["write"]))));
			sm->set(SettingsManager::SHOW_LAST_LINES_LOG, Util::toString(gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(advancedItems["PM"]))));
			sm->set(SettingsManager::SET_MINISLOT_SIZE, Util::toString(gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(advancedItems["slotSize"]))));
			sm->set(SettingsManager::MAX_FILELIST_SIZE, Util::toString(gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(advancedItems["fileListSize"]))));
			sm->set(SettingsManager::PRIVATE_ID, string(gtk_entry_get_text(GTK_ENTRY(advancedItems["CID"]))));
			sm->set(SettingsManager::AUTO_REFRESH_TIME, Util::toString (gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(advancedItems["refresh"]))));
			sm->set(SettingsManager::MAX_TAB_ROWS, Util::toString (gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(advancedItems["tabs"]))));
			sm->set(SettingsManager::SEARCH_HISTORY, Util::toString (gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(advancedItems["searchHistory"]))));
			sm->set(SettingsManager::BIND_ADDRESS, string(gtk_entry_get_text(GTK_ENTRY(advancedItems["bind"]))));
			sm->set(SettingsManager::SOCKET_IN_BUFFER, Util::toString(gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(advancedItems["socketRead"]))));
			sm->set(SettingsManager::SOCKET_OUT_BUFFER, Util::toString(gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(advancedItems["socketWrite"]))));
			WSET("fallback-encoding", Text::acpToUtf8(string(gtk_entry_get_text(GTK_ENTRY(advancedItems["Encoding"])))));
		}
	}
	pthread_mutex_unlock(&settingsLock);
}

void Settings::initGeneral_gui()
{
	gtk_entry_set_text(GTK_ENTRY(generalItems["Nick"]), SETTING(NICK).c_str());
	gtk_entry_set_text(GTK_ENTRY(generalItems["EMail"]), SETTING(EMAIL).c_str());
	gtk_entry_set_text(GTK_ENTRY(generalItems["Description"]), SETTING(DESCRIPTION).c_str());
	generalItems["Connection_Speed"] = gtk_combo_box_new_text();
	gtk_box_pack_start(GTK_BOX (generalItems["Connection"]), generalItems["Connection_Speed"], FALSE, TRUE, 0);
	gtk_widget_show(generalItems["Connection_Speed"]);

	for (StringIter i = SettingsManager::connectionSpeeds.begin(); i != SettingsManager::connectionSpeeds.end(); ++i)
		gtk_combo_box_append_text(GTK_COMBO_BOX(generalItems["Connection_Speed"]), (*i).c_str());

	for (StringIter i = SettingsManager::connectionSpeeds.begin(); i != SettingsManager::connectionSpeeds.end(); ++i)
	{
		if (SETTING(UPLOAD_SPEED) == (*i))
		{
			gtk_combo_box_set_active(GTK_COMBO_BOX(generalItems["Connection_Speed"]), i - SettingsManager::connectionSpeeds.begin());
			break;
		}
	}
}

void Settings::initConnection_gui()
{
	switch (SETTING(INCOMING_CONNECTIONS))
	{
		case SettingsManager::INCOMING_DIRECT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(connectionItems["Incomming_Direct"]), true);
			inDirectClicked_gui();
			break;
		case SettingsManager::INCOMING_FIREWALL_NAT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(connectionItems["Incomming_FW_NAT"]), true);
			inFW_NATClicked_gui();
			break;
		///@todo Add a case for UPnP
		case SettingsManager::INCOMING_FIREWALL_PASSIVE:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (connectionItems["Passive"]), true);
			inPassiveClicked_gui();
			break;
	}

	switch (SETTING(OUTGOING_CONNECTIONS))
	{
		case SettingsManager::OUTGOING_SOCKS5:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(connectionItems["SOCKS5"]), true);
			socks5Clicked_gui();
			break;
		case SettingsManager::OUTGOING_DIRECT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(connectionItems["Outgoing_Direct"]), true);
			outDirectClicked_gui();
			break;
	}

	char buf[8];
	gtk_entry_set_text(GTK_ENTRY(connectionItems["IP"]), SETTING(EXTERNAL_IP).c_str());
	sprintf(buf, "%d", SETTING (TCP_PORT));
	gtk_entry_set_text (GTK_ENTRY (connectionItems["TCP"]), buf);
	sprintf(buf, "%d", SETTING (UDP_PORT));
	gtk_entry_set_text(GTK_ENTRY (connectionItems["UDP"]), buf);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (connectionItems["ForceIP"]), SETTING(NO_IP_OVERRIDE));
	gtk_entry_set_text(GTK_ENTRY(connectionItems["Socks"]), SETTING (SOCKS_SERVER).c_str());
	gtk_entry_set_text(GTK_ENTRY(connectionItems["Username"]), SETTING (SOCKS_USER).c_str());
	sprintf(buf, "%d", SETTING(SOCKS_PORT));
	gtk_entry_set_text(GTK_ENTRY(connectionItems["Port"]), buf);
	gtk_entry_set_text(GTK_ENTRY(connectionItems["Password"]), SETTING (SOCKS_PASSWORD).c_str());

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (connectionItems["Resolve"]), SETTING(SOCKS_RESOLVE));
}

void Settings::onInDirect_gui(GtkToggleButton *button, gpointer data)
{
	((Settings*)data)->inDirectClicked_gui();
}

/**@todo Uncomment when implemented
void Settings::onInFW_UPnP_gui(GtkToggleButton *button, gpointer data)
{
	((Settings*)data)->inFW_UPnPClicked_gui();
}
*/

void Settings::onInFW_NAT_gui(GtkToggleButton *button, gpointer data)
{
	((Settings*)data)->inFW_NATClicked_gui();
}

void Settings::onOutDirect_gui(GtkToggleButton *button, gpointer data)
{
	((Settings*)data)->outDirectClicked_gui();
}

void Settings::onInPassive_gui(GtkToggleButton *button, gpointer data)
{
	((Settings*)data)->inPassiveClicked_gui();
}

void Settings::onSocks5_gui(GtkToggleButton *button, gpointer data)
{
	((Settings*)data)->socks5Clicked_gui();
}

void Settings::inDirectClicked_gui()
{
	gtk_widget_set_sensitive(connectionItems["IP"], TRUE);
	gtk_widget_set_sensitive(connectionItems["IPLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["TCP"], TRUE);
	gtk_widget_set_sensitive(connectionItems["TCPLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["UDP"], TRUE);
	gtk_widget_set_sensitive(connectionItems["UDPLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["ForceIP"], TRUE);
}

///@todo Uncomment when implemented
/*void Settings::inFW_UPnPClicked_gui()
{
	gtk_widget_set_sensitive(connectionItems["IP"], TRUE);
	gtk_widget_set_sensitive(connectionItems["IPLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["TCP"], TRUE);
	gtk_widget_set_sensitive(connectionItems["TCPLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["UDP"], TRUE);
	gtk_widget_set_sensitive(connectionItems["UDPLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["ForceIP"], TRUE);
}*/

void Settings::inFW_NATClicked_gui()
{
	gtk_widget_set_sensitive(connectionItems["IP"], TRUE);
	gtk_widget_set_sensitive(connectionItems["IPLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["TCP"], TRUE);
	gtk_widget_set_sensitive(connectionItems["TCPLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["UDP"], TRUE);
	gtk_widget_set_sensitive(connectionItems["UDPLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["ForceIP"], TRUE);
}

void Settings::inPassiveClicked_gui()
{
	gtk_widget_set_sensitive(connectionItems["IP"], FALSE);
	gtk_widget_set_sensitive(connectionItems["IPLabel"], FALSE);
	gtk_widget_set_sensitive(connectionItems["TCP"], FALSE);
	gtk_widget_set_sensitive(connectionItems["TCPLabel"], FALSE);
	gtk_widget_set_sensitive(connectionItems["UDP"], FALSE);
	gtk_widget_set_sensitive(connectionItems["UDPLabel"], FALSE);
	gtk_widget_set_sensitive(connectionItems["ForceIP"], FALSE);
}

void Settings::outDirectClicked_gui()
{
	gtk_widget_set_sensitive(connectionItems["Socks"], FALSE);
	gtk_widget_set_sensitive(connectionItems["SocksLabel"], FALSE);
	gtk_widget_set_sensitive(connectionItems["Username"], FALSE);
	gtk_widget_set_sensitive(connectionItems["UserLabel"], FALSE);
	gtk_widget_set_sensitive(connectionItems["Port"], FALSE);
	gtk_widget_set_sensitive(connectionItems["PortLabel"], FALSE);
	gtk_widget_set_sensitive(connectionItems["Password"], FALSE);
	gtk_widget_set_sensitive(connectionItems["PassLabel"], FALSE);
	gtk_widget_set_sensitive(connectionItems["Resolve"], FALSE);
}

void Settings::socks5Clicked_gui()
{
	gtk_widget_set_sensitive(connectionItems["Socks"], TRUE);
	gtk_widget_set_sensitive(connectionItems["SocksLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["Username"], TRUE);
	gtk_widget_set_sensitive(connectionItems["UserLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["Port"], TRUE);
	gtk_widget_set_sensitive(connectionItems["PortLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["Password"], TRUE);
	gtk_widget_set_sensitive(connectionItems["PassLabel"], TRUE);
	gtk_widget_set_sensitive(connectionItems["Resolve"], TRUE);
}

void Settings::initDownloads_gui()
{
	{ //Downloads
		gtk_entry_set_text(GTK_ENTRY(downloadItems["Finished"]), SETTING(DOWNLOAD_DIRECTORY).c_str());
		if (SETTING(TEMP_DOWNLOAD_DIRECTORY) == SETTING(DOWNLOAD_DIRECTORY))
			gtk_entry_set_text(GTK_ENTRY(downloadItems["Unfinished"]), "");
		else
			gtk_entry_set_text(GTK_ENTRY(downloadItems["Unfinished"]), SETTING(TEMP_DOWNLOAD_DIRECTORY).c_str());

		gtk_spin_button_set_value(GTK_SPIN_BUTTON (downloadItems["Downloads"]), (double)SETTING(DOWNLOAD_SLOTS));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON (downloadItems["New"]), (double)SETTING(MAX_DOWNLOAD_SPEED));
		gtk_entry_set_text(GTK_ENTRY(downloadItems["Proxy"]), SETTING(HTTP_PROXY).c_str());
	}

	{ //downloadTo
		downloadToView.setView(GTK_TREE_VIEW(downloadItems["View"]));
		downloadToView.insertColumn("Favorite Name", G_TYPE_STRING, TreeView::STRING, -1);
		downloadToView.insertColumn("Directory", G_TYPE_STRING, TreeView::STRING, -1);
		downloadToView.finalize();
		downloadToStore = gtk_list_store_newv(downloadToView.getColCount(), downloadToView.getGTypes());
		gtk_tree_view_set_model(downloadToView.get(), GTK_TREE_MODEL(downloadToStore));
		g_object_unref(downloadToStore);

		gtk_widget_set_sensitive(downloadItems["Remove"], FALSE);
		g_signal_connect(G_OBJECT(downloadToView.get()), "button_release_event", G_CALLBACK(onFavoriteButtonReleased_gui), (gpointer)this);
		StringPairList directories = FavoriteManager::getInstance()->getFavoriteDirs();
		for (StringPairIter j = directories.begin(); j != directories.end(); j++)
		{
			GtkTreeIter iter;
			gtk_list_store_append(downloadToStore, &iter);
			gtk_list_store_set(downloadToStore, &iter,
				downloadToView.col("Favorite Name"), j->second.c_str(),
				downloadToView.col("Directory"), j->first.c_str(),
				-1);
		}

		publicListView.setView(GTK_TREE_VIEW(downloadItems["Public view"]));
		publicListView.insertColumn("List", G_TYPE_STRING, TreeView::STRING, -1);
		publicListView.finalize();
		publicListStore = gtk_list_store_newv(publicListView.getColCount(), publicListView.getGTypes());
		gtk_tree_view_set_model(publicListView.get(), GTK_TREE_MODEL(publicListStore));
		g_object_unref(publicListStore);
	}

	{ // Queue
		//Autoprio
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(queueItems["prioHighest"]), (double)SETTING(PRIO_HIGHEST_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(queueItems["prioNorm"]), (double)SETTING(PRIO_NORMAL_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(queueItems["prioHigh"]), (double)SETTING(PRIO_HIGH_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(queueItems["prioLow"]), (double)SETTING(PRIO_LOW_SIZE));

		//Autodrop
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(queueItems["dropSpeed"]), (double)SETTING(AUTODROP_SPEED));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(queueItems["dropElapsed"]), (double)SETTING (AUTODROP_ELAPSED));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(queueItems["dropMinSources"]), (double)SETTING(AUTODROP_MINSOURCES));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(queueItems["dropCheck"]), (double)SETTING(AUTODROP_INTERVAL));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(queueItems["dropInactiv"]), (double)SETTING(AUTODROP_INACTIVITY));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(queueItems["dropSize"]), (double)SETTING(AUTODROP_FILESIZE));

		// Others
		queueView.setView(GTK_TREE_VIEW(queueItems["Options"]));
		queueView.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		queueView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
		queueView.insertHiddenColumn("Setting", G_TYPE_INT);
		queueView.finalize();
		queueStore = gtk_list_store_newv(queueView.getColCount(), queueView.getGTypes());
		gtk_tree_view_set_model(queueView.get(), GTK_TREE_MODEL(queueStore));
		g_object_unref(queueStore);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(queueStore), queueView.col("Name"), GTK_SORT_ASCENDING);

		GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(queueView.get(), queueView.col("Use")));
		GtkCellRenderer *renderer = (GtkCellRenderer*)g_list_nth_data(list, 0);
		g_signal_connect (renderer, "toggled", G_CALLBACK (onQueueToggledClicked_gui), (gpointer)this);
		g_list_free(list);

		addOption(queueStore, queueView, "Set lowest priority for newly added files larger than Low priority size", SettingsManager::PRIO_LOWEST);
		addOption(queueStore, queueView, "Autodrop slow sources for all queue items (except filelists)", SettingsManager::AUTODROP_ALL);
		addOption(queueStore, queueView, "Remove slow filelists", SettingsManager::AUTODROP_FILELISTS);
		addOption(queueStore, queueView, "Don't remove the source when autodropping, only disconnect", SettingsManager::AUTODROP_DISCONNECT);
		addOption(queueStore, queueView, "Automatically search for alternative download locations", SettingsManager::AUTO_SEARCH);
		addOption(queueStore, queueView, "Automatically match queue for auto search hits", SettingsManager::AUTO_SEARCH_AUTO_MATCH);
		addOption(queueStore, queueView, "Skip zero-byte files", SettingsManager::SKIP_ZERO_BYTE);
		addOption(queueStore, queueView, "Don't download files already in share", SettingsManager::DONT_DL_ALREADY_SHARED);
		addOption(queueStore, queueView, "Use antifragmentation method for downloads", SettingsManager::ANTI_FRAG);
		addOption(queueStore, queueView, "Advanced resume using TTH", SettingsManager::ADVANCED_RESUME);
		addOption(queueStore, queueView, "Only download files that have a TTH", SettingsManager::ONLY_DL_TTH_FILES);
	}
}

void Settings::onBrowseF_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;

	if (!s->lastdir.empty())
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(s->dirChooser), s->lastdir.c_str());
 	gint response = gtk_dialog_run(GTK_DIALOG(s->dirChooser));
	gtk_widget_hide(s->dirChooser);

	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER (s->dirChooser));
		s->lastdir = path;
		gtk_entry_set_text(GTK_ENTRY (s->downloadItems["Finished"]), path.c_str());
	}
}

void Settings::onBrowseUF_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;

	if (!s->lastdir.empty())
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(s->dirChooser), s->lastdir.c_str());
 	gint response = gtk_dialog_run(GTK_DIALOG (s->dirChooser));
	gtk_widget_hide(s->dirChooser);

	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->dirChooser));
		s->lastdir = path;
		gtk_entry_set_text(GTK_ENTRY(s->downloadItems["Unfinished"]), path.c_str());
	}
}

void Settings::onPublicHubs_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;
	GtkTreeModel *m = GTK_TREE_MODEL(s->publicListStore);
	GtkTreeIter iter;
	string lists;

	s->publicInit_gui();
	gint response = gtk_dialog_run(GTK_DIALOG(s->publicHubs));
	gtk_widget_hide(s->publicHubs);

	if (response == GTK_RESPONSE_OK)
	{
		gtk_tree_model_get_iter_first(m, &iter);
		while (gtk_list_store_iter_is_valid(s->publicListStore, &iter))
		{
			lists += s->publicListView.getString(&iter, "List") + ";";
			gtk_tree_model_iter_next(m, &iter);
		}
		if (!lists.empty())
			lists.erase(lists.size() - 1);
		pthread_mutex_lock(&s->settingsLock);
		SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, lists);
		pthread_mutex_unlock(&s->settingsLock);
	}
}

void Settings::onPublicAdd_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;

	string list = gtk_entry_get_text (GTK_ENTRY (s->downloadItems["Public list"]));
	if (list.empty())
		return;

	GtkTreeIter iter;
	gtk_list_store_append (s->publicListStore, &iter);
	gtk_list_store_set (s->publicListStore, &iter, s->publicListView.col("List"), list.c_str (), -1);
	gtk_entry_set_text (GTK_ENTRY (s->downloadItems["Public list"]), "");
}

void Settings::onPublicMU_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;
	GtkTreeModel *m = GTK_TREE_MODEL(s->publicListStore);
	GtkTreeSelection *sel = gtk_tree_view_get_selection(s->publicListView.get());
	GtkTreeIter cur, prev, next;

	if (!gtk_tree_selection_get_selected(sel, NULL, &cur))
		return;

	gtk_tree_model_get_iter_first(m, &next);
	while (gtk_list_store_iter_is_valid(s->publicListStore, &next))
	{
		prev = next;
		gtk_tree_model_iter_next(m, &next);

		if (next.stamp == cur.stamp && next.user_data == cur.user_data &&
			next.user_data2 == cur.user_data2 && next.user_data3 == cur.user_data3)
		{
			gtk_list_store_swap(s->publicListStore, &cur, &prev);
			return;
		}
	}
}

void Settings::onPublicMD_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;
	GtkTreeSelection *sel = gtk_tree_view_get_selection(s->publicListView.get());
	GtkTreeIter cur, next;

	if (!gtk_tree_selection_get_selected(sel, NULL, &cur))
		return;

	next = cur;
	if (gtk_tree_model_iter_next(GTK_TREE_MODEL(s->publicListStore), &next))
		gtk_list_store_swap(s->publicListStore, &cur, &next);
}

void Settings::onPublicEdit_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(s->publicListStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->publicListView.get());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	gtk_entry_set_text(GTK_ENTRY(s->downloadItems["Edit public"]), s->publicListView.getString(&iter, "List").c_str());
	gint response = gtk_dialog_run(GTK_DIALOG (s->editPublic));
	gtk_widget_hide(s->editPublic);

	if (response == GTK_RESPONSE_OK)
		gtk_list_store_set(s->publicListStore, &iter, 0, gtk_entry_get_text(GTK_ENTRY(s->downloadItems["Edit public"])), -1);
}

void Settings::onPublicRemove_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(s->publicListStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->publicListView.get());

	if (!gtk_tree_selection_get_selected(selection, &m, &iter))
		return;

	gtk_list_store_remove(s->publicListStore, &iter);
}

void Settings::publicInit_gui()
{
	GtkTreeIter iter;
	gtk_list_store_clear(publicListStore);
	StringList lists(FavoriteManager::getInstance()->getHubLists());
	for (StringList::iterator idx = lists.begin(); idx != lists.end(); ++idx)
	{
		gtk_list_store_append(publicListStore, &iter);
		gtk_list_store_set (publicListStore, &iter, publicListView.col("List"), (*idx).c_str(), -1);
	}
}

bool Settings::addFavoriteDir_client(string path, string name)
{
	pthread_mutex_lock(&settingsLock);
	bool value = FavoriteManager::getInstance()->addFavoriteDir(path, name);
	pthread_mutex_unlock(&settingsLock);
	return value;
}

void Settings::onAddFavorite_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;

	if (!s->lastdir.empty())
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(s->dirChooser), s->lastdir.c_str());
 	gint response = gtk_dialog_run(GTK_DIALOG(s->dirChooser));
	gtk_widget_hide(s->dirChooser);

	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER (s->dirChooser));
		s->lastdir = path;

		gtk_entry_set_text(GTK_ENTRY(s->downloadItems["Name"]), "");
		response = gtk_dialog_run(GTK_DIALOG(s->favoriteName));
		gtk_widget_hide(s->favoriteName);
		if (response == GTK_RESPONSE_OK)
		{
			string name = gtk_entry_get_text(GTK_ENTRY(s->downloadItems["Name"]));
			if (path[path.length ()-1] != PATH_SEPARATOR)
				path += PATH_SEPARATOR;
			if (s->addFavoriteDir_client(path, name))
			{
				GtkTreeIter iter;
				gtk_list_store_append(s->downloadToStore, &iter);
				gtk_list_store_set(s->downloadToStore, &iter,
					s->downloadToView.col("Favorite Name"), name.c_str(),
					s->downloadToView.col("Directory"), path.c_str(),
					-1);
			}
			else
			{
				GtkWidget *d = gtk_message_dialog_new(WulforManager::get()->getMainWindow()->getWindow(),
					GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Directory or directory name already exists", NULL);
				gtk_dialog_run(GTK_DIALOG(d));
				gtk_widget_hide(d);
			}
		}
	}		
}

bool Settings::removeFavoriteDir_client(string path)
{
	pthread_mutex_lock(&settingsLock);
	bool value = FavoriteManager::getInstance()->removeFavoriteDir(path);
	pthread_mutex_unlock(&settingsLock);
	return value;
}

void Settings::onRemoveFavorite_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(s->downloadToStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->downloadToView.get());

	if (!gtk_tree_selection_get_selected(selection, &m, &iter))
		return;

	if (s->removeFavoriteDir_client(s->downloadToView.getString(&iter, "Directory")))
	{
		gtk_list_store_remove(s->downloadToStore, &iter);
		gtk_widget_set_sensitive (s->downloadItems["Remove"], FALSE);
	}
}

gboolean Settings::onFavoriteButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Settings *s = (Settings*)data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(s->downloadToStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->downloadToView.get());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
	{
		gtk_widget_set_sensitive(s->downloadItems["Remove"], FALSE);
		return FALSE;
	}
	gtk_widget_set_sensitive(s->downloadItems["Remove"], TRUE);
	return FALSE;
}

void Settings::initSharing_gui()
{
	GtkTreeIter iter;

	shareView.setView(GTK_TREE_VIEW(shareItems["Shares"]));
	shareView.insertColumn("Virtual Name", G_TYPE_STRING, TreeView::STRING, -1);
	shareView.insertColumn("Directory", G_TYPE_STRING, TreeView::STRING, -1);
	shareView.insertColumn("Size", G_TYPE_STRING, TreeView::STRING, -1);
	shareView.insertHiddenColumn("Real Size", G_TYPE_INT64);
	shareView.finalize();
	shareStore = gtk_list_store_newv(shareView.getColCount(), shareView.getGTypes());
	gtk_tree_view_set_model(shareView.get(), GTK_TREE_MODEL(shareStore));
	g_object_unref(shareStore);
	shareView.setSortColumn_gui("Size", "Real Size");
	g_signal_connect(G_OBJECT(shareView.get()), "button_release_event", G_CALLBACK(onShareButtonReleased_gui), (gpointer)this);
	gtk_widget_set_sensitive(shareItems["Remove"], FALSE);

	StringPairList directories = ShareManager::getInstance()->getDirectories();
	for (StringPairList::iterator it = directories.begin(); it != directories.end(); it++)
	{
		gtk_list_store_append(shareStore, &iter);
		gtk_list_store_set(shareStore, &iter,
			shareView.col("Virtual Name"), it->first.c_str(),
			shareView.col("Directory"), it->second.c_str(),
			shareView.col("Size"), Util::formatBytes(ShareManager::getInstance()->getShareSize(it->second)).c_str(),
			shareView.col("Real Size"), ShareManager::getInstance()->getShareSize(it->second),
			-1);
	}

	gtk_label_set_text(GTK_LABEL(shareItems["Size"]), string("Total size: " + Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (shareItems["Hidden"]), BOOLSETTING(SHARE_HIDDEN));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(shareItems["Extra"]), (double)SETTING(MIN_UPLOAD_SPEED));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(shareItems["Upload"]), (double)SETTING(SLOTS));
}

void Settings::modifyShare_client(bool add, string path, string name)
{
	if (add)
		ShareManager::getInstance()->addDirectory(path, name);
	else
		ShareManager::getInstance()->removeDirectory(path);

	gtk_label_set_text(GTK_LABEL(shareItems["Size"]), string("Total size: " + 
		Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());
}

void Settings::onAddShare_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;

	if (!s->lastdir.empty())
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(s->dirChooser), s->lastdir.c_str());
 	gint response = gtk_dialog_run(GTK_DIALOG (s->dirChooser));
	gtk_widget_hide(s->dirChooser);

	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->dirChooser));
		s->lastdir = path;

		gtk_entry_set_text(GTK_ENTRY(s->shareItems["Virtual"]), "");
		response = gtk_dialog_run (GTK_DIALOG (s->virtualName));
		gtk_widget_hide(s->virtualName);
		string name = string(gtk_entry_get_text(GTK_ENTRY(s->shareItems["Virtual"])));

		if (response == GTK_RESPONSE_OK && !name.empty())
		{
			try
			{
				name = ShareManager::getInstance()->validateVirtual(name);
				if (path[path.length ()-1] != PATH_SEPARATOR)
					path += PATH_SEPARATOR;

				typedef Func3<Settings, bool, string, string> F3;
				F3 *f3 = new F3(s, &Settings::modifyShare_client, TRUE, path, name);
				WulforManager::get()->dispatchClientFunc(f3);

				GtkTreeIter iter;
				gtk_list_store_append(s->shareStore, &iter);
				gtk_list_store_set(s->shareStore, &iter,
					s->shareView.col("Virtual Name"), name.c_str(),
					s->shareView.col("Directory"), path.c_str(),
					s->shareView.col("Size"), Util::formatBytes(0).c_str(),
					s->shareView.col("Real Size"), 0,
					-1);
			}
			catch (const ShareException& e)
			{
				GtkWidget *d = gtk_message_dialog_new(WulforManager::get()->getMainWindow()->getWindow(),
											GTK_DIALOG_MODAL,
											GTK_MESSAGE_ERROR,
											GTK_BUTTONS_OK,
											string(e.getError()).c_str(),
											NULL);
				gtk_dialog_run (GTK_DIALOG (d));
				gtk_widget_hide (d);
			}
		}
	}
}

void Settings::onRemoveShare_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(s->shareStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->shareView.get());

	if (!gtk_tree_selection_get_selected(selection, &m, &iter))
		return;

	s->modifyShare_client(false, s->shareView.getString(&iter, "Directory"), "");
	gtk_list_store_remove(s->shareStore, &iter);
	gtk_widget_set_sensitive(s->shareItems["Remove"], FALSE);
}

gboolean Settings::onShareButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Settings *s = (Settings*)data;
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection(s->shareView.get());
	if (gtk_tree_selection_count_selected_rows(selection) == 0)
	{
		gtk_widget_set_sensitive(s->shareItems["Remove"], FALSE);
		return FALSE;
	}
	gtk_widget_set_sensitive(s->shareItems["Remove"], TRUE);
	return FALSE;
}

void Settings::shareHidden_client(bool show)
{
	pthread_mutex_lock(&settingsLock);
	SettingsManager::getInstance()->set(SettingsManager::SHARE_HIDDEN, show);
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true, false, true);
	pthread_mutex_unlock(&settingsLock);
}

gboolean Settings::onShareHiddenPressed_gui(GtkToggleButton *togglebutton, gpointer data)
{
	Settings *s = (Settings*)data;
	GtkTreeIter iter;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->shareItems["Hidden"])))
		WulforManager::get()->dispatchClientFunc(new Func1<Settings, bool>(s, &Settings::shareHidden_client, true));
	else
		WulforManager::get()->dispatchClientFunc(new Func1<Settings, bool>(s, &Settings::shareHidden_client, false));

	gtk_list_store_clear(s->shareStore);
	StringPairList directories = ShareManager::getInstance()->getDirectories();
	for (StringPairList::iterator it = directories.begin(); it != directories.end(); it++)
	{
		gtk_list_store_append(s->shareStore, &iter);
		gtk_list_store_set(s->shareStore, &iter,
			s->shareView.col("Virtual Name"), it->first.c_str(),
			s->shareView.col("Directory"), it->second.c_str(),
			s->shareView.col("Size"), Util::formatBytes(ShareManager::getInstance()->getShareSize(it->second)).c_str(),
			s->shareView.col("Real Size"), ShareManager::getInstance()->getShareSize(it->second),
			-1);
	}
	gtk_label_set_text(GTK_LABEL(s->shareItems["Size"]), string("Total size: " +Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());
}

// Appearance tab
void Settings::initAppearance_gui()
{
	{//Appearance
		appearanceView.setView(GTK_TREE_VIEW(appearanceItems["Options"]));
		appearanceView.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		appearanceView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
		appearanceView.insertHiddenColumn("Setting", G_TYPE_INT);
		appearanceView.finalize();
		appearanceStore = gtk_list_store_newv(appearanceView.getColCount(), appearanceView.getGTypes());
		gtk_tree_view_set_model(appearanceView.get(), GTK_TREE_MODEL(appearanceStore));
		g_object_unref(appearanceStore);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(appearanceStore), appearanceView.col("Name"), GTK_SORT_ASCENDING);

		GList *list = gtk_tree_view_column_get_cell_renderers (gtk_tree_view_get_column(appearanceView.get(), appearanceView.col("Use")));
		GtkCellRenderer *renderer = (GtkCellRenderer*)g_list_nth_data(list, 0);
		g_signal_connect(renderer, "toggled", G_CALLBACK(onAppearanceToggledClicked_gui), (gpointer)this);
		g_list_free(list);

		/// @todo: Uncomment when implemented
		//addOption(appearanceStore, appearanceView, "User alternative sorting order for transfers", SettingsManager::ALT_SORT_ORDER);
		addOption(appearanceStore, appearanceView, "Filter kick and NMDC debug messages", SettingsManager::FILTER_MESSAGES);
		//addOption(appearanceStore, appearanceView, "Minimize to tray", SettingsManager::MINIMIZE_TRAY);
		addOption(appearanceStore, appearanceView, "Show timestamps in chat by default", SettingsManager::TIME_STAMPS);
		addOption(appearanceStore, appearanceView, "View status messages in main chat", SettingsManager::STATUS_IN_CHAT);
		addOption(appearanceStore, appearanceView, "Show joins / parts in chat by default", SettingsManager::SHOW_JOINS);
		addOption(appearanceStore, appearanceView, "Only show joins / parts for favorite users", SettingsManager::FAV_SHOW_JOINS);
		//addOption(appearanceStore, appearanceView, "Use system icons when browsing files (slows browsing a bit)", SettingsManager::USE_SYSTEM_ICONS);
		addOption(appearanceStore, appearanceView, "Use OEM monospaced font for viewing text files", SettingsManager::USE_OEM_MONOFONT);
		//addOption(appearanceStore, appearanceView, "Guess user country from IP", SettingsManager::GET_USER_COUNTRY);
		///@todo: uncomment when the save problem is solved.
		//addOption(appearanceStore, appearanceView, "Show tray icon", WGETI("show-tray-icon"));

		gtk_entry_set_text(GTK_ENTRY(appearanceItems["Away"]), SETTING(DEFAULT_AWAY_MESSAGE).c_str());
		gtk_entry_set_text(GTK_ENTRY(appearanceItems["Timestamp"]), SETTING(TIME_STAMPS_FORMAT).c_str());
	}

	{// Colors and sounds
		colorView.setView(GTK_TREE_VIEW(appearanceItems["Bolding"]));
		colorView.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		colorView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
		colorView.insertHiddenColumn("Setting", G_TYPE_INT);
		colorView.finalize();
		colorStore = gtk_list_store_newv(colorView.getColCount(), colorView.getGTypes());
		gtk_tree_view_set_model(colorView.get(), GTK_TREE_MODEL(colorStore));
		g_object_unref(colorStore);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(colorStore), colorView.col("Name"), GTK_SORT_ASCENDING);

		GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(colorView.get(), colorView.col("Use")));
		GtkCellRenderer *renderer = (GtkCellRenderer*)g_list_nth_data(list, 0);
		g_signal_connect(renderer, "toggled", G_CALLBACK(onColorToggledClicked_gui), (gpointer)this);
		g_list_free(list);

		addOption(colorStore, colorView, "Finished downloads", SettingsManager::BOLD_FINISHED_DOWNLOADS);
		addOption(colorStore, colorView, "Finished Uploads", SettingsManager::BOLD_FINISHED_UPLOADS);
		addOption(colorStore, colorView, "Download Queue", SettingsManager::BOLD_QUEUE);
		addOption(colorStore, colorView, "Hub", SettingsManager::BOLD_HUB);
		addOption(colorStore, colorView, "Private message", SettingsManager::BOLD_PM);
		addOption(colorStore, colorView, "Search", SettingsManager::BOLD_SEARCH);
		/// @todo: Uncomment when implemented
		//addOption(colorStore, colorView, "Waiting Users", SettingsManager::BOLD_WAITING_USERS);
		//addOption(colorStore, colorView, "System Log", SettingsManager::BOLD_SYSTEM_LOG);

		gtk_entry_set_text(GTK_ENTRY(appearanceItems["Away"]), SETTING(DEFAULT_AWAY_MESSAGE).c_str());
		gtk_entry_set_text(GTK_ENTRY(appearanceItems["Timestamp"]), SETTING(TIME_STAMPS_FORMAT).c_str());
	}

	{// Window
		{ // Auto-open
			windowView1.setView(GTK_TREE_VIEW(appearanceItems["windowsTreeView1"]));
			windowView1.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
			windowView1.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
			windowView1.insertHiddenColumn("Setting", G_TYPE_INT);
			windowView1.finalize();
			windowStore1 = gtk_list_store_newv(windowView1.getColCount(), windowView1.getGTypes());
			gtk_tree_view_set_model(windowView1.get(), GTK_TREE_MODEL(windowStore1));
			g_object_unref(windowStore1);
			gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(windowStore1), windowView1.col("Name"), GTK_SORT_ASCENDING);

			GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(windowView1.get(), windowView1.col("Use")));
			GtkCellRenderer *renderer = (GtkCellRenderer*)g_list_nth_data(list, 0);
			g_signal_connect(renderer, "toggled", G_CALLBACK(onWindowView1ToggledClicked_gui), (gpointer)this);
			g_list_free(list);

			/// @todo: Uncomment when implemented
			addOption(windowStore1, windowView1, "Public Hubs", SettingsManager::OPEN_PUBLIC);
			addOption(windowStore1, windowView1, "Favorite Hubs", SettingsManager::OPEN_FAVORITE_HUBS);
			addOption(windowStore1, windowView1, "Favorite Users", SettingsManager::OPEN_FAVORITE_USERS);
			addOption(windowStore1, windowView1, "Download Queue", SettingsManager::OPEN_QUEUE);
			addOption(windowStore1, windowView1, "Finished Downloads", SettingsManager::OPEN_FINISHED_DOWNLOADS);
			//addOption(windowStore1, windowView1, "Waiting Users", SettingsManager::OPEN_WAITING_USERS);
			addOption(windowStore1, windowView1, "Finished Uploads", SettingsManager::OPEN_FINISHED_UPLOADS);
			//addOption(windowStore1, windowView1, "Search Spy", SettingsManager::OPEN_SEARCH_SPY);
			//addOption(windowStore1, windowView1, "Network Statistics", SettingsManager::OPEN_NETWORK_STATISTICS);
			//addOption(windowStore1, windowView1, "Notepad", SettingsManager::OPEN_NOTEPAD);
			//addOption(windowStore1, windowView1, "System Log", SettingsManager::OPEN_SYSTEM_LOG);
			}
		{ // Window options
			windowView2.setView(GTK_TREE_VIEW(appearanceItems["windowsTreeView2"]));
			windowView2.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
			windowView2.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
			windowView2.insertHiddenColumn("Setting", G_TYPE_INT);
			windowView2.finalize();
			windowStore2 = gtk_list_store_newv(windowView2.getColCount(), windowView2.getGTypes());
			gtk_tree_view_set_model(windowView2.get(), GTK_TREE_MODEL(windowStore2));
			g_object_unref(windowStore2);
			gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(windowStore2), windowView2.col("Name"), GTK_SORT_ASCENDING);

			GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(windowView2.get(), windowView2.col("Use")));
			GtkCellRenderer *renderer = (GtkCellRenderer*)g_list_nth_data(list, 0);
			g_signal_connect(renderer, "toggled", G_CALLBACK(onWindowView2ToggledClicked_gui), (gpointer)this);
			g_list_free(list);

			//addOption(windowStore2, windowView2, "Open private messages in their own window", SettingsManager::POPUP_PMS);
			//addOption(windowStore2, windowView2, "Open private messages from offline users in their own window", SettingsManager::POPUP_OFFLINE);
			//addOption(windowStore2, windowView2, "Open file list window in the background", SettingsManager::POPUNDER_FILELIST);
			//addOption(windowStore2, windowView2, "Open new private message windows in the background", SettingsManager::POPUNDER_PM);
			//addOption(windowStore2, windowView2, "Open new window when using /join", SettingsManager::JOIN_OPEN_NEW_WINDOW);
			//addOption(windowStore2, windowView2, "Ignore private messages from offline users", SettingsManager::IGNORE_OFFLINE);
			//addOption(windowStore2, windowView2, "Toggle window when selecting an active tab", SettingsManager::TOGGLE_ACTIVE_WINDOW);
		}
		{ // Confirmation dialog
			windowView3.setView(GTK_TREE_VIEW(appearanceItems["windowsTreeView3"]));
			windowView3.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
			windowView3.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
			windowView3.insertHiddenColumn("Setting", G_TYPE_INT);
			windowView3.finalize();
			windowStore3 = gtk_list_store_newv(windowView3.getColCount(), windowView3.getGTypes());
			gtk_tree_view_set_model(windowView3.get(), GTK_TREE_MODEL(windowStore3));
			g_object_unref(windowStore3);
			gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(windowStore3), windowView3.col("Name"), GTK_SORT_ASCENDING);

			GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(windowView3.get(), windowView3.col("Use")));
			GtkCellRenderer *renderer = (GtkCellRenderer*)g_list_nth_data(list, 0);
			g_signal_connect(renderer, "toggled", G_CALLBACK(onWindowView3ToggledClicked_gui), (gpointer)this);
			g_list_free(list);

			addOption(windowStore3, windowView3, "Confirm application exit", SettingsManager::CONFIRM_EXIT);
			addOption(windowStore3, windowView3, "Confirm favorite hub removal", SettingsManager::CONFIRM_HUB_REMOVAL);
			/// @todo: Uncomment when implemented
			//addOption(windowStore3, windowView3, "Confirm item removal in download queue", SettingsManager::CONFIRM_ITEM_REMOVAL);
		}
	}
}
void Settings::addOption(GtkListStore *store, TreeView view, string name, SettingsManager::IntSetting setting)
{
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, view.col("Use"), SettingsManager::getInstance()->get(setting),
		view.col("Name"), name.c_str(), view.col("Setting"), setting,  -1);
}

void Settings::onAppearanceToggledClicked_gui(GtkCellRendererToggle *cell, gchar *pathString, gpointer data)
{
	Settings *s = (Settings *)data;
  	GtkTreeIter iter;
  	gboolean fixed;
  	GtkTreePath *path = gtk_tree_path_new_from_string(pathString);
	GtkTreeModel *m = GTK_TREE_MODEL(s->appearanceStore);
  	gtk_tree_model_get_iter(m, &iter, path);
  	fixed = s->appearanceView.getValue<gboolean>(&iter,"Use");
  	gtk_list_store_set(s->appearanceStore, &iter, s->appearanceView.col("Use"), !fixed, -1);
	gtk_tree_path_free(path);
}

void Settings::onQueueToggledClicked_gui(GtkCellRendererToggle *cell, gchar *pathString, gpointer data)
{
	Settings *s = (Settings *)data;
  	GtkTreeIter iter;
  	gboolean fixed;
  	GtkTreePath *path = gtk_tree_path_new_from_string(pathString);
	GtkTreeModel *m = GTK_TREE_MODEL(s->queueStore);
  	gtk_tree_model_get_iter(m, &iter, path);
  	fixed = s->queueView.getValue<gboolean>(&iter,"Use");
  	gtk_list_store_set(s->queueStore, &iter, s->queueView.col("Use"), !fixed, -1);
	gtk_tree_path_free(path);
}

void Settings::checkClicked()
{
	bool bmain = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logItems["Main"]));
	bool bprivate = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logItems["Private"]));
	bool bdownload = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logItems["Download"]));
	bool bupload = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(logItems["Upload"]));
	{ // Main
		gtk_widget_set_sensitive (logItems["Format1"], bmain);
		gtk_widget_set_sensitive (logItems["EMain"], bmain);
	}
	{ // Private
		gtk_widget_set_sensitive(logItems["Format2"], bprivate);
		gtk_widget_set_sensitive(logItems["EPrivate"], bprivate);
	}
	{ // Downloads
		gtk_widget_set_sensitive(logItems["Format3"], bdownload);
		gtk_widget_set_sensitive(logItems["EDownload"], bdownload);
	}
	{ // Uploads
		gtk_widget_set_sensitive(logItems["Format4"], bupload);
		gtk_widget_set_sensitive(logItems["EUpload"], bupload);
	}
}

void Settings::initLog_gui()
{
	gtk_entry_set_text(GTK_ENTRY(logItems["Directory"]), SETTING(LOG_DIRECTORY).c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(logItems["Main"]), BOOLSETTING(LOG_MAIN_CHAT));
	gtk_entry_set_text(GTK_ENTRY(logItems["EMain"]), SETTING(LOG_FORMAT_MAIN_CHAT).c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(logItems["Private"]), BOOLSETTING(LOG_PRIVATE_CHAT));
	gtk_entry_set_text(GTK_ENTRY(logItems["EPrivate"]), SETTING(LOG_FORMAT_PRIVATE_CHAT).c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(logItems["Download"]), BOOLSETTING(LOG_DOWNLOADS));
	gtk_entry_set_text(GTK_ENTRY(logItems["EDownload"]), SETTING(LOG_FORMAT_POST_DOWNLOAD).c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(logItems["Upload"]), BOOLSETTING(LOG_UPLOADS));
	gtk_entry_set_text(GTK_ENTRY(logItems["EUpload"]), SETTING(LOG_FORMAT_POST_UPLOAD).c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(logItems["System"]), BOOLSETTING(LOG_SYSTEM));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(logItems["Status"]), BOOLSETTING(LOG_STATUS_MESSAGES));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(logItems["SPrivate"]), BOOLSETTING(PRIVATE_MESSAGE_BEEP));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(logItems["SPrivateWindow"]), BOOLSETTING(PRIVATE_MESSAGE_BEEP_OPEN));

	checkClicked();
}

void Settings::onLogBrowseClicked_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;

	if (!s->lastdir.empty())
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (s->dirChooser), s->lastdir.c_str());
	else
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (s->dirChooser), gtk_entry_get_text(GTK_ENTRY(s->logItems["Directory"])));
 	gint response = gtk_dialog_run(GTK_DIALOG(s->dirChooser));
	gtk_widget_hide(s->dirChooser);

	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER (s->dirChooser));
		s->lastdir = path;
		if (path[path.length()-1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		gtk_entry_set_text(GTK_ENTRY(s->logItems["Directory"]), path.c_str());
	}
}
void Settings::onLogMainClicked_gui(GtkToggleButton *togglebutton, gpointer data)
{
	Settings *s = (Settings*)data;
	s->checkClicked();
}
void Settings::onLogPrivateClicked_gui(GtkToggleButton *togglebutton, gpointer data)
{
	Settings *s = (Settings*)data;
	s->checkClicked();
}
void Settings::onLogDownloadClicked_gui(GtkToggleButton *togglebutton, gpointer data)
{
	Settings *s = (Settings*)data;
	s->checkClicked();
}
void Settings::onLogUploadClicked_gui(GtkToggleButton *togglebutton, gpointer data)
{
	Settings *s = (Settings*)data;
	s->checkClicked();
}

void Settings::initAdvanced_gui()
{
	advancedView.setView(GTK_TREE_VIEW(advancedItems["Advanced"]));
	advancedView.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
	advancedView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
	advancedView.insertHiddenColumn("Setting", G_TYPE_INT);
	advancedView.finalize();
	advancedStore = gtk_list_store_newv(advancedView.getColCount(), advancedView.getGTypes());
	gtk_tree_view_set_model(advancedView.get(), GTK_TREE_MODEL(advancedStore));
	g_object_unref(advancedStore);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(advancedStore), advancedView.col("Name"), GTK_SORT_ASCENDING);

	GList *list = gtk_tree_view_column_get_cell_renderers (gtk_tree_view_get_column(advancedView.get(), advancedView.col("Use")));
	GtkCellRenderer *renderer = (GtkCellRenderer*)g_list_nth_data(list, 0);
	g_signal_connect(renderer, "toggled", G_CALLBACK(onAdvancedToggledClicked_gui), (gpointer)this);
	g_list_free(list);

	/// @todo: Uncomment when implemented
	addOption(advancedStore, advancedView, "Auto-away on minimize (and back on restore)", SettingsManager::AUTO_AWAY);
	addOption(advancedStore, advancedView, "Automatically follow redirects", SettingsManager::AUTO_FOLLOW);
	addOption(advancedStore, advancedView, "Clear search box after each search", SettingsManager::CLEAR_SEARCH);
	addOption(advancedStore, advancedView, "Keep duplicate files in your file list (duplicates never count towards your share size)", SettingsManager::LIST_DUPES);
	//addOption (advancedStore, advancedView, "Register with the OS to handle dchub:// and adc:// URL links", SettingsManager::URL_HANDLER);
	//addOption (advancedStore, advancedView, "Register with the OS to handle magnet: URL links", SettingsManager::MAGNET_REGISTER);
	addOption(advancedStore, advancedView, "Don't delete file lists when exiting", SettingsManager::KEEP_LISTS);
	addOption(advancedStore, advancedView, "Automatically disconnect users who leave the hub (does not disconnect when hub goes down / you leave it)",
		SettingsManager::AUTO_KICK);
	addOption(advancedStore, advancedView, "Show progress bars for transfers", SettingsManager::SHOW_PROGRESS_BARS);
	addOption(advancedStore, advancedView, "Enable automatic SFV checking", SettingsManager::SFV_CHECK);
	//addOption(advancedStore, advancedView, "Don't send the away message to bots", SettingsManager::NO_AWAYMSG_TO_BOTS);
	//addOption(advancedStore, advancedView, "Break on first ADLSearch match", SettingsManager::ADLS_BREAK_ON_FIRST);
	addOption(advancedStore, advancedView, "Enable safe and compressed transfers", SettingsManager::COMPRESS_TRANSFERS);
	addOption(advancedStore, advancedView, "Accept custom user commands from hub", SettingsManager::HUB_USER_COMMANDS);
	//addOption(advancedStore, advancedView, "Send unknown /commands to the hub", SettingsManager::SEND_UNKNOWN_COMMANDS);
	addOption(advancedStore, advancedView, "Add finished files to share instantly (if shared)", SettingsManager::ADD_FINISHED_INSTANTLY);
	//addOption(advancedStore, advancedView, "Use CTRL for line history", SettingsManager::USE_CTRL_FOR_LINE_HISTORY);
	//addOption(advancedStore, advancedView, "Use SSL when remote client supports it", SettingsManager::USE_SSL);

	{ // Experts
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["rollBack"]), (double)SETTING(ROLLBACK));
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["hash"]), (double)SETTING(MAX_HASH_SPEED));
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["write"]), (double)SETTING(BUFFER_SIZE));
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["PM"]), (double)SETTING(SHOW_LAST_LINES_LOG));
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["slotSize"]), (double)SETTING(SET_MINISLOT_SIZE));
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["fileListSize"]), (double)SETTING(MAX_FILELIST_SIZE));
		gtk_entry_set_text (GTK_ENTRY (advancedItems["CID"]), SETTING(PRIVATE_ID).c_str());
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["refresh"]), (double)SETTING(AUTO_REFRESH_TIME));
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["tabs"]), (double)SETTING(MAX_TAB_ROWS));
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["searchHistory"]), (double)SETTING(SEARCH_HISTORY));
		gtk_entry_set_text (GTK_ENTRY (advancedItems["bind"]), SETTING(BIND_ADDRESS).c_str());
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["socketRead"]), (double)SETTING(SOCKET_IN_BUFFER));
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["socketWrite"]), (double)SETTING(SOCKET_OUT_BUFFER));
		gtk_entry_set_text(GTK_ENTRY(advancedItems["Encoding"]), WGETS("fallback-encoding").c_str());
	}

}

void Settings::onColorToggledClicked_gui(GtkCellRendererToggle *cell, gchar *pathString, gpointer data)
{
	Settings *s = (Settings *)data;
  	GtkTreeIter iter;
  	gboolean fixed;
  	GtkTreePath *path = gtk_tree_path_new_from_string(pathString);
	GtkTreeModel *m = GTK_TREE_MODEL(s->colorStore);
  	gtk_tree_model_get_iter(m, &iter, path);
  	gtk_tree_model_get(m, &iter, s->colorView.col("Use"), &fixed, -1);
	fixed = s->colorView.getValue<gboolean>(&iter, "Use");
  	gtk_list_store_set(s->colorStore, &iter, s->colorView.col("Use"), !fixed, -1);
	gtk_tree_path_free(path);
}

void Settings::onWindowView1ToggledClicked_gui(GtkCellRendererToggle *cell, gchar *pathString, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	gboolean fixed;
  	GtkTreePath *path = gtk_tree_path_new_from_string(pathString);
	GtkTreeModel *m = GTK_TREE_MODEL(s->windowStore1);
	gtk_tree_model_get_iter(m, &iter, path);
	fixed = s->windowView1.getValue<gboolean>(&iter, "Use");
	gtk_list_store_set(s->windowStore1, &iter, s->windowView1.col("Use"), !fixed, -1);
	gtk_tree_path_free(path);
}

void Settings::onWindowView2ToggledClicked_gui(GtkCellRendererToggle *cell, gchar *pathString, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	gboolean fixed;
  	GtkTreePath *path = gtk_tree_path_new_from_string(pathString);
	GtkTreeModel *m = GTK_TREE_MODEL(s->windowStore2);
	gtk_tree_model_get_iter(m, &iter, path);
	fixed = s->windowView2.getValue<gboolean>(&iter, "Use");
	gtk_list_store_set(s->windowStore2, &iter, s->windowView2.col("Use"), !fixed, -1);
	gtk_tree_path_free(path);
}

void Settings::onWindowView3ToggledClicked_gui(GtkCellRendererToggle *cell, gchar *pathString, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	gboolean fixed;
  	GtkTreePath *path = gtk_tree_path_new_from_string(pathString);
	GtkTreeModel *m = GTK_TREE_MODEL(s->windowStore3);
	gtk_tree_model_get_iter(m, &iter, path);
	fixed = s->windowView3.getValue<gboolean>(&iter, "Use");
	gtk_list_store_set(s->windowStore3, &iter, s->windowView3.col("Use"), !fixed, -1);
	gtk_tree_path_free(path);
}

void Settings::onAdvancedToggledClicked_gui(GtkCellRendererToggle *cell, gchar *pathString, gpointer data)
{
	Settings *s = (Settings *)data;
  	GtkTreeIter iter;
  	gboolean fixed;
  	GtkTreePath *path = gtk_tree_path_new_from_string(pathString);
	GtkTreeModel *m = GTK_TREE_MODEL(s->advancedStore);
  	gtk_tree_model_get_iter(m, &iter, path);
	fixed = s->advancedView.getValue<gboolean>(&iter, "Use");
  	gtk_list_store_set(s->advancedStore, &iter, s->advancedView.col("Use"), !fixed, -1);
	gtk_tree_path_free(path);
}
