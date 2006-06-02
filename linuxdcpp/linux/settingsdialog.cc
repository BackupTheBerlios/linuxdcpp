/* 
* Copyright (C) 2004 Jens Oknelid, paskharen@gmail.com
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

SettingsManager::IntSetting Settings::optionSettings[] = { SettingsManager::FILTER_MESSAGES,
	SettingsManager::TIME_STAMPS,
	SettingsManager::CONFIRM_EXIT,
	SettingsManager::STATUS_IN_CHAT,
	SettingsManager::SHOW_JOINS,
	SettingsManager::FAV_SHOW_JOINS,
	SettingsManager::FINISHED_DIRTY,
	SettingsManager::QUEUE_DIRTY,
	SettingsManager::TAB_DIRTY,
	SettingsManager::CONFIRM_HUB_REMOVAL};

SettingsManager::IntSetting Settings::advancedSettings[] = { SettingsManager::AUTO_AWAY,
	SettingsManager::AUTO_FOLLOW,
	SettingsManager::CLEAR_SEARCH,
	SettingsManager::OPEN_PUBLIC,
	SettingsManager::OPEN_QUEUE,
	SettingsManager::OPEN_FAVORITE_HUBS,
	SettingsManager::OPEN_FINISHED_DOWNLOADS,
	SettingsManager::AUTO_SEARCH,
	SettingsManager::POPUP_PMS,
	SettingsManager::IGNORE_OFFLINE,
	SettingsManager::POPUP_OFFLINE,
	SettingsManager::POPUNDER_FILELIST,
	SettingsManager::POPUNDER_PM,
	SettingsManager::LIST_DUPES,
	SettingsManager::SMALL_SEND_BUFFER,
	SettingsManager::KEEP_LISTS,
	SettingsManager::AUTO_KICK,
	SettingsManager::SHOW_PROGRESS_BARS,
	SettingsManager::SFV_CHECK,
	SettingsManager::AUTO_UPDATE_LIST,
	SettingsManager::ANTI_FRAG,
	SettingsManager::NO_AWAYMSG_TO_BOTS,
	SettingsManager::SKIP_ZERO_BYTE,
	SettingsManager::ADLS_BREAK_ON_FIRST,
	SettingsManager::TAB_COMPLETION,
	SettingsManager::COMPRESS_TRANSFERS,
	SettingsManager::HUB_USER_COMMANDS,
	SettingsManager::AUTO_SEARCH_AUTO_MATCH,
	SettingsManager::LOG_FILELIST_TRANSFERS,
	SettingsManager::SEND_UNKNOWN_COMMANDS,
	SettingsManager::ADD_FINISHED_INSTANTLY,
	SettingsManager::SETTINGS_USE_UPNP,
	SettingsManager::DONT_DL_ALREADY_SHARED,
	SettingsManager::SETTINGS_USE_CTRL_FOR_LINE_HISTORY,
	SettingsManager::SETTINGS_OPEN_NEW_WINDOW
	};
	
Settings::~Settings ()
{
	pthread_mutex_destroy(&settingsLock);
	gtk_widget_destroy(favoriteName);
	gtk_widget_destroy(publicHubs);
	gtk_widget_destroy(editPublic);
	gtk_widget_destroy(virtualName);
	gtk_widget_destroy(dirChooser);
}
Settings::Settings () : DialogEntry("Settings")
{
	string file = WulforManager::get()->getPath() + "/glade/settingsdialog.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	dialog = glade_xml_get_widget(xml, "settingsDialog");
	favoriteName = glade_xml_get_widget(xml, "favoriteName");
	publicHubs = glade_xml_get_widget(xml, "publicDialog");
	editPublic = glade_xml_get_widget(xml, "editPublic");
	virtualName = glade_xml_get_widget(xml, "virtualName");
	dirChooser = glade_xml_get_widget(xml, "dirChooserDialog");

	setDialog(dialog);

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
		// Connection
		generalItems["Active"] = glade_xml_get_widget(xml, "radiobuttonActive");
		g_signal_connect(G_OBJECT (generalItems["Active"]), "toggled", G_CALLBACK (onActive_gui), (gpointer)this);
		generalItems["Passive"] = glade_xml_get_widget(xml, "radiobuttonPassive");
		g_signal_connect (G_OBJECT (generalItems["Passive"]), "toggled", G_CALLBACK (onPassive_gui), (gpointer)this);
		generalItems["SOCKS5"] = glade_xml_get_widget(xml, "radiobuttonSocks");
		g_signal_connect (G_OBJECT (generalItems["SOCKS5"]), "toggled", G_CALLBACK (onSocks5_gui), (gpointer)this);
		generalItems["IP"] = glade_xml_get_widget(xml, "entryIP");
		generalItems["IPLabel"] = glade_xml_get_widget(xml, "label13");
		generalItems["TCP"] = glade_xml_get_widget(xml, "entryTCP");
		generalItems["TCPLabel"] = glade_xml_get_widget(xml, "label14");
		generalItems["UDP"] = glade_xml_get_widget(xml, "entryUDP");
		generalItems["UDPLabel"] = glade_xml_get_widget(xml, "label19");
		generalItems["Socks"] = glade_xml_get_widget(xml, "entrySocks");
		generalItems["SocksLabel"] = glade_xml_get_widget(xml, "label15");
		generalItems["Username"] = glade_xml_get_widget(xml, "entryUser");
		generalItems["UserLabel"] = glade_xml_get_widget(xml, "label16");
		generalItems["Port"] = glade_xml_get_widget(xml, "entryPort");
		generalItems["PortLabel"] = glade_xml_get_widget(xml, "label17");
		generalItems["Password"] = glade_xml_get_widget(xml, "entryPass");
		generalItems["PassLabel"] = glade_xml_get_widget(xml, "label18");
		generalItems["Resolve"] = glade_xml_get_widget(xml, "checkbuttonSocks");
	}
	{// Downloads
		downloadItems["Finished"] = glade_xml_get_widget(xml, "entryFinished");
		downloadItems["Unfinished"] = glade_xml_get_widget(xml, "entryUnFinished");
		downloadItems["ButtonF"] = glade_xml_get_widget(xml, "buttonFinished");
		g_signal_connect (G_OBJECT(downloadItems["ButtonF"]), "clicked", G_CALLBACK (onBrowseF_gui), (gpointer)this);
		downloadItems["ButtonUF"] = glade_xml_get_widget(xml, "buttonUnfinished");
		g_signal_connect (G_OBJECT(downloadItems["ButtonUF"]), "clicked", G_CALLBACK (onBrowseUF_gui), (gpointer)this);
		downloadItems["Downloads"] = glade_xml_get_widget(xml, "spinbuttonDownloads");
		downloadItems["New"] = glade_xml_get_widget(xml, "spinbuttonNew");
		downloadItems["Public hubs"] = glade_xml_get_widget(xml, "buttonPublic");
		g_signal_connect (G_OBJECT(downloadItems["Public hubs"]), "clicked", G_CALLBACK (onPublicHubs_gui), (gpointer)this);
		downloadItems["Proxy"] = glade_xml_get_widget(xml, "entryProxy");

		// Download to
		downloadItems["View"] = glade_xml_get_widget(xml, "treeviewFavorite");
		downloadItems["Remove"] = glade_xml_get_widget(xml, "buttonRemoveTo");
		g_signal_connect (G_OBJECT (downloadItems["Remove"]), "clicked", G_CALLBACK (onRemoveFavorite_gui), (gpointer)this);
		downloadItems["Add"] = glade_xml_get_widget(xml, "buttonAddTo");
		g_signal_connect (G_OBJECT (downloadItems["Add"]), "clicked", G_CALLBACK (onAddFavorite_gui), (gpointer)this);
		downloadItems["Name"] = glade_xml_get_widget(xml, "entryFavorite");

		// Public Hubs
		downloadItems["Public list"] = glade_xml_get_widget(xml, "entryPublicHubs");
		downloadItems["Public add"] = glade_xml_get_widget(xml, "buttonPublicAdd");
		g_signal_connect (G_OBJECT (downloadItems["Public add"]), "clicked", G_CALLBACK (onPublicAdd_gui), (gpointer)this);
		downloadItems["Public mu"] = glade_xml_get_widget(xml, "buttonPublicMU");
		g_signal_connect (G_OBJECT (downloadItems["Public mu"]), "clicked", G_CALLBACK (onPublicMU_gui), (gpointer)this);
		downloadItems["Public md"] = glade_xml_get_widget(xml, "buttonPublicMD");
		g_signal_connect (G_OBJECT (downloadItems["Public md"]), "clicked", G_CALLBACK (onPublicMD_gui), (gpointer)this);
		downloadItems["Public edit"] = glade_xml_get_widget(xml, "buttonPublicEdit");
		g_signal_connect (G_OBJECT (downloadItems["Public edit"]), "clicked", G_CALLBACK (onPublicEdit_gui), (gpointer)this);
		downloadItems["Public remove"] = glade_xml_get_widget(xml, "buttonPublicRemove");
		g_signal_connect (G_OBJECT (downloadItems["Public remove"]), "clicked", G_CALLBACK (onPublicRemove_gui), (gpointer)this);
		downloadItems["Public view"] = glade_xml_get_widget(xml, "treeviewPublic");
		downloadItems["Edit public"] = glade_xml_get_widget(xml, "entryPublicEdit");
	}
	{// Sharing
		shareItems["Shares"] = glade_xml_get_widget(xml, "treeviewShared");
		shareItems["Size"] = glade_xml_get_widget(xml, "labelSize");
		shareItems["Hidden"] = glade_xml_get_widget(xml, "checkbuttonHidden");
		g_signal_connect (G_OBJECT (shareItems["Hidden"]), "toggled", G_CALLBACK(onShareHiddenPressed_gui), (gpointer)this);
		shareItems["Remove"] = glade_xml_get_widget(xml, "buttonRemoveShared");
		g_signal_connect (G_OBJECT (shareItems["Remove"]), "clicked", G_CALLBACK (onRemoveShare_gui), (gpointer)this);
		shareItems["Add"] = glade_xml_get_widget(xml, "buttonAddShared");
		g_signal_connect (G_OBJECT (shareItems["Add"]), "clicked", G_CALLBACK (onAddShare_gui), (gpointer)this);
		shareItems["Extra"] = glade_xml_get_widget(xml, "spinbuttonExtraSlot");
		shareItems["Upload"] = glade_xml_get_widget(xml, "spinbuttonUploadSlot");
		shareItems["Virtual"] = glade_xml_get_widget(xml, "entryVirtual");
	}
	{// Appearance
		appearanceItems["Options"] = glade_xml_get_widget(xml, "treeviewOptions");
		appearanceItems["Away"] = glade_xml_get_widget(xml, "entryAway");
		appearanceItems["Timestamp"] = glade_xml_get_widget(xml, "entryTimestamp");
	}
	{// Logs and sound
		logItems["Directory"] = glade_xml_get_widget(xml, "entryDir");
		logItems["Browse"] = glade_xml_get_widget(xml, "buttonLog");
		g_signal_connect (G_OBJECT (logItems["Browse"]), "clicked", G_CALLBACK (onLogBrowseClicked_gui), (gpointer)this);
		logItems["Main"] = glade_xml_get_widget(xml, "checkbuttonMain");
		g_signal_connect (G_OBJECT (logItems["Main"]), "toggled", G_CALLBACK (onLogMainClicked_gui), (gpointer)this);
		logItems["Format1"] = glade_xml_get_widget(xml, "labelFormat1");
		logItems["EMain"] = glade_xml_get_widget(xml, "entryMain");
		logItems["Private"] = glade_xml_get_widget(xml, "checkbuttonPrivate");
		g_signal_connect (G_OBJECT (logItems["Private"]), "toggled", G_CALLBACK (onLogPrivateClicked_gui), (gpointer)this);
		logItems["Format2"] = glade_xml_get_widget(xml, "labelFormat2");
		logItems["EPrivate"] = glade_xml_get_widget(xml, "entryPrivate");		
		logItems["Download"] = glade_xml_get_widget(xml, "checkbuttonDownloads");
		g_signal_connect (G_OBJECT (logItems["Download"]), "toggled", G_CALLBACK (onLogDownloadClicked_gui), (gpointer)this);
		logItems["Format3"] = glade_xml_get_widget(xml, "labelFormat3");
		logItems["EDownload"] = glade_xml_get_widget(xml, "entryDownloads");		
		logItems["Upload"] = glade_xml_get_widget(xml, "checkbuttonUploads");
		g_signal_connect (G_OBJECT (logItems["Upload"]), "toggled", G_CALLBACK (onLogUploadClicked_gui), (gpointer)this);
		logItems["Format4"] = glade_xml_get_widget(xml, "labelFormat4");
		logItems["EUpload"] = glade_xml_get_widget(xml, "entryUploads");
		
		logItems["System"] = glade_xml_get_widget(xml, "checkbuttonSystem");
		logItems["Status"] = glade_xml_get_widget(xml, "checkbuttonStatus");
		logItems["SPrivate"] = glade_xml_get_widget(xml, "checkbuttonPM");
		logItems["SPrivateWindow"] = glade_xml_get_widget(xml, "checkbuttonPMW");
	}
	{// Advanced
		advancedItems["Rollback"] = glade_xml_get_widget(xml, "spinbuttonRollback");
		advancedItems["Hash"] = glade_xml_get_widget(xml, "spinbuttonHash");
		advancedItems["Write"] = glade_xml_get_widget(xml, "spinbuttonWrite");
		advancedItems["Advanced"] = glade_xml_get_widget(xml, "treeviewAdvanced");
	}
	
	pthread_mutex_init(&settingsLock, NULL);
	
	initGeneral_gui ();
	initDownloads_gui ();
	initSharing_gui ();
	initAppearance_gui ();
	initLog_gui ();
	initAdvanced_gui ();
}
void Settings::saveSettings_client ()
{
	SettingsManager *sm = SettingsManager::getInstance ();
	GtkTreeIter iter;
	GtkTreeModel *m;
	vector<gboolean> o;
	
	// General
	pthread_mutex_lock(&settingsLock);
	sm->set (SettingsManager::NICK, gtk_entry_get_text (GTK_ENTRY (generalItems["Nick"])));
	sm->set (SettingsManager::EMAIL, gtk_entry_get_text (GTK_ENTRY (generalItems["EMail"])));
	sm->set (SettingsManager::DESCRIPTION, gtk_entry_get_text (GTK_ENTRY (generalItems["Description"])));
	sm->set (SettingsManager::CONNECTION, SettingsManager::connectionSpeeds[gtk_combo_box_get_active (GTK_COMBO_BOX (generalItems["Connection type"]))]);
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (generalItems["Active"])))
		sm->set (SettingsManager::CONNECTION_TYPE, SettingsManager::CONNECTION_ACTIVE);
	else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (generalItems["Passive"])))
		sm->set (SettingsManager::CONNECTION_TYPE, SettingsManager::CONNECTION_PASSIVE);
	else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (generalItems["SOCKS5"])))
		sm->set (SettingsManager::CONNECTION_TYPE, SettingsManager::CONNECTION_SOCKS5);
	sm->set (SettingsManager::SERVER, gtk_entry_get_text (GTK_ENTRY (generalItems["IP"])));
	sm->set (SettingsManager::IN_PORT, atoi (gtk_entry_get_text (GTK_ENTRY (generalItems["TCP"]))));	
	sm->set (SettingsManager::UDP_PORT, atoi (gtk_entry_get_text (GTK_ENTRY (generalItems["UDP"]))));
	sm->set (SettingsManager::SOCKS_SERVER, gtk_entry_get_text (GTK_ENTRY (generalItems["Socks"])));
	sm->set (SettingsManager::SOCKS_USER, gtk_entry_get_text (GTK_ENTRY (generalItems["Username"])));
	sm->set (SettingsManager::SOCKS_PORT, atoi (gtk_entry_get_text (GTK_ENTRY (generalItems["Port"]))));
	sm->set (SettingsManager::SOCKS_PASSWORD, gtk_entry_get_text (GTK_ENTRY (generalItems["Password"])));
	
	// Downloads
	string tmp = gtk_entry_get_text (GTK_ENTRY (downloadItems["Finished"]));
	if (tmp[tmp.length ()-1] != PATH_SEPARATOR)
		tmp += PATH_SEPARATOR;
	sm->set (SettingsManager::DOWNLOAD_DIRECTORY, tmp.c_str ());
	tmp = gtk_entry_get_text (GTK_ENTRY (downloadItems["Unfinished"]));
	if (tmp != "")
		if (tmp[tmp.length ()-1] != PATH_SEPARATOR)
			tmp += PATH_SEPARATOR;
	sm->set (SettingsManager::TEMP_DOWNLOAD_DIRECTORY, tmp.c_str ());
	sm->set (SettingsManager::DOWNLOAD_SLOTS, (int)gtk_spin_button_get_value (GTK_SPIN_BUTTON (downloadItems["Downloads"])));
	sm->set (SettingsManager::MAX_DOWNLOAD_SPEED, (int)gtk_spin_button_get_value (GTK_SPIN_BUTTON (downloadItems["New"])));
	sm->set (SettingsManager::HTTP_PROXY, gtk_entry_get_text (GTK_ENTRY (downloadItems["Proxy"])));
	
	// Sharing
	sm->set (SettingsManager::MIN_UPLOAD_SPEED, (int)gtk_spin_button_get_value (GTK_SPIN_BUTTON (shareItems["Extra"])));
	sm->set (SettingsManager::SLOTS, (int)gtk_spin_button_get_value (GTK_SPIN_BUTTON (shareItems["Upload"])));
	
	// Appearance
	m = GTK_TREE_MODEL (appearanceStore);
	if (gtk_tree_model_get_iter_first (m, &iter))
	{
		while (1)
		{
			o.push_back (appearanceView.getValue<gboolean>(&iter, "Use"));
			if (!gtk_tree_model_iter_next (m, &iter))
				break;
		}
	}
	
	for (int i = 0; i < o.size() - 1; i++)
		sm->set(optionSettings[i], o[i]);
	WSET("show-tray-icon", o[o.size() - 1]);
		
	sm->set (SettingsManager::DEFAULT_AWAY_MESSAGE, string (gtk_entry_get_text (GTK_ENTRY (appearanceItems["Away"]))));
	sm->set (SettingsManager::TIME_STAMPS_FORMAT, string (gtk_entry_get_text (GTK_ENTRY (appearanceItems["Timestamp"]))));

	// Logs and sound
		
	sm->set (SettingsManager::LOG_DIRECTORY, string (gtk_entry_get_text (GTK_ENTRY (logItems["Directory"]))));
	sm->set (SettingsManager::LOG_MAIN_CHAT, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (logItems["Main"])));
	sm->set (SettingsManager::LOG_FORMAT_MAIN_CHAT, string (gtk_entry_get_text (GTK_ENTRY (logItems["EMain"]))));
	sm->set (SettingsManager::LOG_PRIVATE_CHAT, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (logItems["Private"])));
	sm->set (SettingsManager::LOG_FORMAT_PRIVATE_CHAT, string (gtk_entry_get_text (GTK_ENTRY (logItems["EPrivate"]))));
	sm->set (SettingsManager::LOG_DOWNLOADS, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (logItems["Download"])));
	sm->set (SettingsManager::LOG_FORMAT_POST_DOWNLOAD, string (gtk_entry_get_text (GTK_ENTRY (logItems["EDownload"]))));
	sm->set (SettingsManager::LOG_UPLOADS, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (logItems["Upload"])));
	sm->set (SettingsManager::LOG_FORMAT_POST_UPLOAD, string (gtk_entry_get_text (GTK_ENTRY (logItems["EUpload"]))));	
	
	sm->set (SettingsManager::LOG_SYSTEM, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (logItems["System"])));
	sm->set (SettingsManager::LOG_STATUS_MESSAGES, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (logItems["Status"])));
	sm->set (SettingsManager::PRIVATE_MESSAGE_BEEP, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (logItems["SPrivate"])));
	sm->set (SettingsManager::PRIVATE_MESSAGE_BEEP_OPEN, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (logItems["SPrivateWindow"])));
	
	// Advanced
	o.clear ();
	m = GTK_TREE_MODEL (advancedStore);
	if (gtk_tree_model_get_iter_first (m, &iter))
	{
		while (1)
		{
			o.push_back(advancedView.getValue<gboolean>(&iter, "Use"));
			if (!gtk_tree_model_iter_next (m, &iter))
				break;
		}
	}
	
	for (int i=0;i<o.size (); i++)
		sm->set (advancedSettings[i], o[i]);
		
	sm->set (SettingsManager::ROLLBACK, Util::toString (gtk_spin_button_get_value (GTK_SPIN_BUTTON (advancedItems["Rollback"]))));
	sm->set (SettingsManager::MAX_HASH_SPEED, Util::toString (gtk_spin_button_get_value (GTK_SPIN_BUTTON (advancedItems["Hash"]))));
	sm->set (SettingsManager::BUFFER_SIZE, Util::toString (gtk_spin_button_get_value (GTK_SPIN_BUTTON (advancedItems["Write"]))));
	
	pthread_mutex_unlock(&settingsLock);
}
void Settings::initGeneral_gui ()
{
	gtk_entry_set_text (GTK_ENTRY (generalItems["Nick"]), SETTING(NICK).c_str());
	gtk_entry_set_text (GTK_ENTRY (generalItems["EMail"]), SETTING(EMAIL).c_str());
	gtk_entry_set_text (GTK_ENTRY (generalItems["Description"]), SETTING(DESCRIPTION).c_str());
	generalItems["Connection type"] = gtk_combo_box_new_text ();
	gtk_box_pack_start (GTK_BOX (generalItems["Connection"]), generalItems["Connection type"], FALSE, TRUE, 0);
	gtk_widget_show (generalItems["Connection type"]);
	for (int i=0;i<SettingsManager::SPEED_LAST;i++)
		gtk_combo_box_append_text (GTK_COMBO_BOX (generalItems["Connection type"]), SettingsManager::connectionSpeeds[i].c_str ());

	for (int i=0;i<SettingsManager::SPEED_LAST;i++)
		if (SETTING (CONNECTION) == SettingsManager::connectionSpeeds[i])
		{
			gtk_combo_box_set_active (GTK_COMBO_BOX (generalItems["Connection type"]), i);
			break;
		}

	switch (SETTING (CONNECTION_TYPE))
	{
		case SettingsManager::CONNECTION_ACTIVE:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (generalItems["Active"]), true);
			activeClicked_gui ();
			break;
		case SettingsManager::CONNECTION_PASSIVE:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (generalItems["Passive"]), true);
			passiveClicked_gui ();
			break;
		case SettingsManager::CONNECTION_SOCKS5:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (generalItems["SOCKS5"]), true);
			socks5Clicked_gui ();
			break;	
	}

	char buf[8];
	gtk_entry_set_text (GTK_ENTRY (generalItems["IP"]), SETTING(SERVER).c_str ());
	sprintf (buf, "%d", SETTING (IN_PORT));
	gtk_entry_set_text (GTK_ENTRY (generalItems["TCP"]), buf);
	sprintf (buf, "%d", SETTING (UDP_PORT));
	gtk_entry_set_text (GTK_ENTRY (generalItems["UDP"]), buf);
	gtk_entry_set_text (GTK_ENTRY (generalItems["Socks"]), SETTING (SOCKS_SERVER).c_str ());
	gtk_entry_set_text (GTK_ENTRY (generalItems["Username"]), SETTING (SOCKS_USER).c_str ());
	sprintf (buf, "%d", SETTING (SOCKS_PORT));
	gtk_entry_set_text (GTK_ENTRY (generalItems["Port"]), buf);
	gtk_entry_set_text (GTK_ENTRY (generalItems["Password"]), SETTING (SOCKS_PASSWORD).c_str ());

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (generalItems["Resolve"]), SETTING (SOCKS_RESOLVE));
}
void Settings::onActive_gui (GtkToggleButton *button, gpointer user_data)
{
	((Settings*)user_data)->activeClicked_gui ();
}
void Settings::onPassive_gui (GtkToggleButton *button, gpointer user_data)
{
	((Settings*)user_data)->passiveClicked_gui ();
}
void Settings::onSocks5_gui (GtkToggleButton *button, gpointer user_data)
{
	((Settings*)user_data)->socks5Clicked_gui ();
}
void Settings::activeClicked_gui ()
{
	gtk_widget_set_sensitive (generalItems["IP"], TRUE);
	gtk_widget_set_sensitive (generalItems["IPLabel"], TRUE);
	gtk_widget_set_sensitive (generalItems["TCP"], TRUE);
	gtk_widget_set_sensitive (generalItems["TCPLabel"], TRUE);
	gtk_widget_set_sensitive (generalItems["UDP"], TRUE);
	gtk_widget_set_sensitive (generalItems["UDPLabel"], TRUE);
	gtk_widget_set_sensitive (generalItems["Socks"], FALSE);
	gtk_widget_set_sensitive (generalItems["SocksLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["Username"], FALSE);
	gtk_widget_set_sensitive (generalItems["UserLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["Port"], FALSE);
	gtk_widget_set_sensitive (generalItems["PortLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["Password"], FALSE);
	gtk_widget_set_sensitive (generalItems["PassLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["Resolve"], FALSE);
}
void Settings::passiveClicked_gui ()
{
	gtk_widget_set_sensitive (generalItems["IP"], FALSE);
	gtk_widget_set_sensitive (generalItems["IPLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["TCP"], FALSE);
	gtk_widget_set_sensitive (generalItems["TCPLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["UDP"], FALSE);
	gtk_widget_set_sensitive (generalItems["UDPLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["Socks"], FALSE);
	gtk_widget_set_sensitive (generalItems["SocksLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["Username"], FALSE);
	gtk_widget_set_sensitive (generalItems["UserLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["Port"], FALSE);
	gtk_widget_set_sensitive (generalItems["PortLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["Password"], FALSE);
	gtk_widget_set_sensitive (generalItems["PassLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["Resolve"], FALSE);
}
void Settings::socks5Clicked_gui ()
{
	gtk_widget_set_sensitive (generalItems["IP"], FALSE);
	gtk_widget_set_sensitive (generalItems["IPLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["TCP"], FALSE);
	gtk_widget_set_sensitive (generalItems["TCPLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["UDP"], FALSE);
	gtk_widget_set_sensitive (generalItems["UDPLabel"], FALSE);
	gtk_widget_set_sensitive (generalItems["Socks"], TRUE);
	gtk_widget_set_sensitive (generalItems["SocksLabel"], TRUE);
	gtk_widget_set_sensitive (generalItems["Username"], TRUE);
	gtk_widget_set_sensitive (generalItems["UserLabel"], TRUE);
	gtk_widget_set_sensitive (generalItems["Port"], TRUE);
	gtk_widget_set_sensitive (generalItems["PortLabel"], TRUE);
	gtk_widget_set_sensitive (generalItems["Password"], TRUE);
	gtk_widget_set_sensitive (generalItems["PassLabel"], TRUE);
	gtk_widget_set_sensitive (generalItems["Resolve"], TRUE);
}

void Settings::initDownloads_gui ()
{
	gtk_entry_set_text (GTK_ENTRY (downloadItems["Finished"]), SETTING (DOWNLOAD_DIRECTORY).c_str());
	if (SETTING (TEMP_DOWNLOAD_DIRECTORY) == SETTING (DOWNLOAD_DIRECTORY))
		gtk_entry_set_text (GTK_ENTRY (downloadItems["Unfinished"]), "");
	else
		gtk_entry_set_text (GTK_ENTRY (downloadItems["Unfinished"]), SETTING (TEMP_DOWNLOAD_DIRECTORY).c_str());

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (downloadItems["Downloads"]), (double)SETTING (DOWNLOAD_SLOTS));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (downloadItems["New"]), (double)SETTING (MAX_DOWNLOAD_SPEED));
	gtk_entry_set_text (GTK_ENTRY (downloadItems["Proxy"]), SETTING (HTTP_PROXY).c_str ());

	downloadToView.setView(GTK_TREE_VIEW(downloadItems["View"]));
	downloadToView.insertColumn("Favorite Name", G_TYPE_STRING, TreeView::STRING, -1);
	downloadToView.insertColumn("Directory", G_TYPE_STRING, TreeView::STRING, -1);
	downloadToView.finalize();
	downloadToStore = gtk_list_store_newv(downloadToView.getColCount(), downloadToView.getGTypes());
	gtk_tree_view_set_model(downloadToView.get(), GTK_TREE_MODEL(downloadToStore));
	g_object_unref(downloadToStore);

	gtk_widget_set_sensitive (downloadItems["Remove"], FALSE);
	g_signal_connect(G_OBJECT(downloadToView.get()), "button_release_event", G_CALLBACK(onFavoriteButtonReleased_gui), (gpointer)this);
	StringPairList directories = HubManager::getInstance()->getFavoriteDirs();
	for(StringPairIter j = directories.begin(); j != directories.end(); j++)
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
void Settings::onBrowseF_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;

	if (s->lastdir != "")
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (s->dirChooser), s->lastdir.c_str ());
 	gint response = gtk_dialog_run (GTK_DIALOG (s->dirChooser));
	gtk_widget_hide(s->dirChooser);

	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (s->dirChooser));
		s->lastdir = path;
		gtk_entry_set_text (GTK_ENTRY (s->downloadItems["Finished"]), path.c_str ());
	}
}
void Settings::onBrowseUF_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;

	if (s->lastdir != "")
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (s->dirChooser), s->lastdir.c_str ());
 	gint response = gtk_dialog_run (GTK_DIALOG (s->dirChooser));
	gtk_widget_hide(s->dirChooser);

	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (s->dirChooser));
		s->lastdir = path;
		gtk_entry_set_text (GTK_ENTRY (s->downloadItems["Unfinished"]), path.c_str ());
	}
}

void Settings::onPublicHubs_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings*)data;
	GtkTreeModel *m = GTK_TREE_MODEL(s->publicListStore);
	GtkTreeIter iter;
	string lists;

	s->publicInit_gui ();
	gint response = gtk_dialog_run (GTK_DIALOG (s->publicHubs));
	gtk_widget_hide(s->publicHubs);

	if (response == GTK_RESPONSE_OK)
	{
		gtk_tree_model_get_iter_first(m, &iter);
		while (gtk_list_store_iter_is_valid(s->publicListStore, &iter))
		{
			lists += s->publicListView.getString(&iter, "List") + ';';
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
void Settings::onPublicEdit_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (s->publicListStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->publicListView.get());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	gtk_entry_set_text(GTK_ENTRY(s->downloadItems["Edit public"]), s->publicListView.getString(&iter, "List").c_str());
	gint response = gtk_dialog_run (GTK_DIALOG (s->editPublic));
	gtk_widget_hide(s->editPublic);

	if (response == GTK_RESPONSE_OK)
		gtk_list_store_set (s->publicListStore, &iter, 0, gtk_entry_get_text (GTK_ENTRY (s->downloadItems["Edit public"])), -1);
}
void Settings::onPublicRemove_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (s->publicListStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->publicListView.get());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	gtk_list_store_remove (s->publicListStore, &iter);
}
void Settings::publicInit_gui ()
{
	gtk_list_store_clear (publicListStore);
	StringList lists(HubManager::getInstance()->getHubLists());
	for(StringList::iterator idx = lists.begin(); idx != lists.end(); ++idx)
	{
		GtkTreeIter iter;
		gtk_list_store_append (publicListStore, &iter);
		gtk_list_store_set (publicListStore, &iter, publicListView.col("List"), (*idx).c_str (), -1);
	}
}
bool Settings::addFavoriteDir_client (string path, string name)
{
	pthread_mutex_lock (&settingsLock);
	bool value = HubManager::getInstance()->addFavoriteDir(path, name);
	pthread_mutex_unlock (&settingsLock);
	return value;
}
void Settings::onAddFavorite_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;

	if (s->lastdir != "")
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (s->dirChooser), s->lastdir.c_str ());
 	gint response = gtk_dialog_run (GTK_DIALOG (s->dirChooser));
	gtk_widget_hide(s->dirChooser);

	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (s->dirChooser));
		s->lastdir = path;

		gtk_entry_set_text (GTK_ENTRY (s->downloadItems["Name"]), "");
		response = gtk_dialog_run (GTK_DIALOG (s->favoriteName));
		gtk_widget_hide(s->favoriteName);
		if (response == GTK_RESPONSE_OK)
		{
			string name = gtk_entry_get_text (GTK_ENTRY (s->downloadItems["Name"]));
			if (path[path.length ()-1] != PATH_SEPARATOR)
				path += PATH_SEPARATOR;
			if (s->addFavoriteDir_client (path, name))
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
				GtkWidget *d = gtk_message_dialog_new (WulforManager::get ()->getMainWindow ()->getWindow (),
																					GTK_DIALOG_MODAL,
																					GTK_MESSAGE_ERROR,
																					GTK_BUTTONS_OK,
																					"Directory or directory name already exists",
																					NULL);
				gtk_dialog_run (GTK_DIALOG (d));
				gtk_widget_hide (d);
			}
		}
	}		
}
bool Settings::removeFavoriteDir_client (string path)
{
	pthread_mutex_lock (&settingsLock);
	bool value = HubManager::getInstance()->removeFavoriteDir(path);
	pthread_mutex_unlock (&settingsLock);
	return value;
}
void Settings::onRemoveFavorite_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (s->downloadToStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->downloadToView.get());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	if (s->removeFavoriteDir_client (s->downloadToView.getString(&iter, "Directory")))
	{
		gtk_list_store_remove (s->downloadToStore, &iter);
		gtk_widget_set_sensitive (s->downloadItems["Remove"], FALSE);
	}
}
gboolean Settings::onFavoriteButtonReleased_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (s->downloadToStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->downloadToView.get());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
	{
		gtk_widget_set_sensitive (s->downloadItems["Remove"], FALSE);
		return FALSE;
	}
	gtk_widget_set_sensitive (s->downloadItems["Remove"], TRUE);
	return FALSE;
}
void Settings::initSharing_gui ()
{
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
	for(StringPairList::iterator it = directories.begin(); it != directories.end(); it++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(shareStore, &iter);
		gtk_list_store_set(shareStore, &iter,
			shareView.col("Virtual Name"), it->first.c_str(),
			shareView.col("Directory"), it->second.c_str(),
			shareView.col("Size"), Util::formatBytes(ShareManager::getInstance()->getShareSize(it->second)).c_str(),
			shareView.col("Real Size"), ShareManager::getInstance()->getShareSize(it->second),
			-1);
	}
	gtk_label_set_text (GTK_LABEL (shareItems["Size"]), string ("Total size: " +Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str ());
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (shareItems["Hidden"]), BOOLSETTING(SHARE_HIDDEN));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (shareItems["Extra"]), (double)SETTING (MIN_UPLOAD_SPEED));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (shareItems["Upload"]), (double)SETTING (SLOTS));
}
void Settings::modifyShare_client (bool add, string path, string name)
{
	if (add)
		ShareManager::getInstance()->addDirectory (path, name);
	else
		ShareManager::getInstance ()->removeDirectory (path);
		
	gtk_label_set_text (GTK_LABEL (shareItems["Size"]), string ("Total size: " +Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str ());
}
void Settings::onAddShare_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;

	if (s->lastdir != "")
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (s->dirChooser), s->lastdir.c_str ());
 	gint response = gtk_dialog_run (GTK_DIALOG (s->dirChooser));
	gtk_widget_hide(s->dirChooser);

	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (s->dirChooser));
		s->lastdir = path;

		gtk_entry_set_text (GTK_ENTRY (s->shareItems["Virtual"]), "");
		response = gtk_dialog_run (GTK_DIALOG (s->virtualName));
		gtk_widget_hide(s->virtualName);
		if (response == GTK_RESPONSE_OK)
		{
			try
			{
				string name = ShareManager::getInstance()->validateVirtual(gtk_entry_get_text (GTK_ENTRY (s->shareItems["Virtual"])));
				if (path[path.length ()-1] != PATH_SEPARATOR)
					path += PATH_SEPARATOR;
			
				s->modifyShare_client(true, path, name);
				GtkTreeIter iter;
				gtk_list_store_append(s->shareStore, &iter);
				gtk_list_store_set(s->shareStore, &iter,
					s->shareView.col("Virtual Name"), name.c_str(),
					s->shareView.col("Directory"), path.c_str(),
					s->shareView.col("Size"), Util::formatBytes(ShareManager::getInstance()->getShareSize(path)).c_str(),
					s->shareView.col("Real Size"), ShareManager::getInstance()->getShareSize(path),
					-1);
			}
			catch (const ShareException& e)
			{
				GtkWidget *d = gtk_message_dialog_new (WulforManager::get ()->getMainWindow ()->getWindow (),
																					GTK_DIALOG_MODAL,
																					GTK_MESSAGE_ERROR,
																					GTK_BUTTONS_OK,
																					string (e.getError ()).c_str (),
																					NULL);
				gtk_dialog_run (GTK_DIALOG (d));
				gtk_widget_hide (d);
			}
		}
	}
}
void Settings::onRemoveShare_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (s->shareStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->shareView.get());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	s->modifyShare_client(false, s->shareView.getString(&iter, "Directory"), "");
	gtk_list_store_remove (s->shareStore, &iter);
	gtk_widget_set_sensitive (s->shareItems["Remove"], FALSE);
}
gboolean Settings::onShareButtonReleased_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection(s->shareView.get());
	if (gtk_tree_selection_count_selected_rows(selection)  == 0)
	{
		gtk_widget_set_sensitive (s->shareItems["Remove"], FALSE);
		return FALSE;
	}
	gtk_widget_set_sensitive (s->shareItems["Remove"], TRUE);
	return FALSE;
}
void Settings::shareHidden_client (bool show)
{
	pthread_mutex_lock (&settingsLock);
	SettingsManager::getInstance ()->set (SettingsManager::SHARE_HIDDEN, show);
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true, false, true);
	pthread_mutex_unlock (&settingsLock);
}
gboolean Settings::onShareHiddenPressed_gui (GtkToggleButton *togglebutton, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (s->shareItems["Hidden"])))
		WulforManager::get ()->dispatchClientFunc (new Func1<Settings, bool> (s, &Settings::shareHidden_client, true));
	else
		WulforManager::get ()->dispatchClientFunc (new Func1<Settings, bool> (s, &Settings::shareHidden_client, false));
		
	gtk_list_store_clear (s->shareStore);
	StringPairList directories = ShareManager::getInstance()->getDirectories();
	for(StringPairList::iterator it = directories.begin(); it != directories.end(); it++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(s->shareStore, &iter);
		gtk_list_store_set(s->shareStore, &iter,
			s->shareView.col("Virtual Name"), it->first.c_str(),
			s->shareView.col("Directory"), it->second.c_str(),
			s->shareView.col("Size"), Util::formatBytes(ShareManager::getInstance()->getShareSize(it->second)).c_str(),
			s->shareView.col("Real Size"), ShareManager::getInstance()->getShareSize(it->second),
			-1);
	}
	gtk_label_set_text (GTK_LABEL (s->shareItems["Size"]), string ("Total size: " +Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str ());
}

// Appearance
void Settings::initAppearance_gui ()
{
	appearanceView.setView(GTK_TREE_VIEW(appearanceItems["Options"]));
	appearanceView.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
	appearanceView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
	appearanceView.finalize();
	appearanceStore = gtk_list_store_newv(appearanceView.getColCount(), appearanceView.getGTypes());
	gtk_tree_view_set_model(appearanceView.get(), GTK_TREE_MODEL(appearanceStore));
	g_object_unref(appearanceStore);
	
	GList *list = gtk_tree_view_column_get_cell_renderers (gtk_tree_view_get_column(appearanceView.get(), appearanceView.col("Use")));
	GtkCellRenderer *renderer = (GtkCellRenderer*)list->data;
	g_signal_connect (renderer, "toggled", G_CALLBACK (onAppearanceToggledClicked_gui), (gpointer)this);	
	
	addOption ("Filter kick and NMDC debug messages", SETTING (FILTER_MESSAGES));
	addOption ("Show timestamps in chat by default", SETTING (TIME_STAMPS));
	addOption ("Confirm application exit", SETTING (CONFIRM_EXIT));
	addOption ("View status messages in main chat", SETTING (STATUS_IN_CHAT));
	addOption ("Show joins / parts in chat by default", SETTING (SHOW_JOINS));
	addOption ("Only show joins / parts for favorite users", SETTING (FAV_SHOW_JOINS));
	addOption ("Set Finished Manager(s) tab bold when an entry is added", SETTING (FINISHED_DIRTY));
	addOption ("Set Download Queue tab bold when contents change", SETTING (QUEUE_DIRTY));
	addOption ("Set hub/PM tab bold when contents change", SETTING (TAB_DIRTY));
	addOption ("Confirm favorite hub removal", SETTING (CONFIRM_HUB_REMOVAL));
	addOption("Show tray icon", WGETI("show-tray-icon"));
	
	gtk_entry_set_text (GTK_ENTRY (appearanceItems["Away"]), SETTING (DEFAULT_AWAY_MESSAGE).c_str ());
	gtk_entry_set_text (GTK_ENTRY (appearanceItems["Timestamp"]), SETTING (TIME_STAMPS_FORMAT).c_str ());
}

void Settings::addOption (string name, bool use)
{
	GtkTreeIter iter;
	gtk_list_store_append(appearanceStore, &iter);
	gtk_list_store_set(appearanceStore, &iter, appearanceView.col("Use"), use, appearanceView.col("Name"), name.c_str (), -1);
}

void Settings::onAppearanceToggledClicked_gui (GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
	Settings *s = (Settings *)data;
  	GtkTreeIter iter;
  	gboolean fixed;
	GtkTreeModel *m = gtk_tree_view_get_model(s->appearanceView.get());
  	gtk_tree_model_get_iter (m, &iter, gtk_tree_path_new_from_string (path_str));
  	gtk_tree_model_get(m, &iter, s->appearanceView.col("Use"), &fixed, -1);
  	fixed = !fixed;
  	gtk_list_store_set(GTK_LIST_STORE (m), &iter, s->appearanceView.col("Use"), fixed, -1);
}

void Settings::checkClicked ()
{	
	bool bmain = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (logItems["Main"]));
	bool bprivate = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (logItems["Private"]));
	bool bdownload = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (logItems["Download"]));
	bool bupload = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (logItems["Upload"]));
	{ // Main
		gtk_widget_set_sensitive (logItems["Format1"], bmain);
		gtk_widget_set_sensitive (logItems["EMain"], bmain);
	}
	{ // Private
		gtk_widget_set_sensitive (logItems["Format2"], bprivate);
		gtk_widget_set_sensitive (logItems["EPrivate"], bprivate);
	}
	{ // Downloads
		gtk_widget_set_sensitive (logItems["Format3"], bdownload);
		gtk_widget_set_sensitive (logItems["EDownload"], bdownload);
	}
	{ // Uploads
		gtk_widget_set_sensitive (logItems["Format4"], bupload);
		gtk_widget_set_sensitive (logItems["EUpload"], bupload);
	}
}

void Settings::initLog_gui ()
{
	gtk_entry_set_text (GTK_ENTRY (logItems["Directory"]), SETTING (LOG_DIRECTORY).c_str ());
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logItems["Main"]), BOOLSETTING (LOG_MAIN_CHAT));
	gtk_entry_set_text (GTK_ENTRY (logItems["EMain"]), SETTING (LOG_FORMAT_MAIN_CHAT).c_str ());
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logItems["Private"]), BOOLSETTING (LOG_PRIVATE_CHAT));
	gtk_entry_set_text (GTK_ENTRY (logItems["EPrivate"]), SETTING (LOG_FORMAT_PRIVATE_CHAT).c_str ());
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logItems["Download"]), BOOLSETTING (LOG_DOWNLOADS));
	gtk_entry_set_text (GTK_ENTRY (logItems["EDownload"]), SETTING (LOG_FORMAT_POST_DOWNLOAD).c_str ());
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logItems["Upload"]), BOOLSETTING (LOG_UPLOADS));
	gtk_entry_set_text (GTK_ENTRY (logItems["EUpload"]), SETTING (LOG_FORMAT_POST_UPLOAD).c_str ());
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logItems["System"]), BOOLSETTING (LOG_SYSTEM));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logItems["Status"]), BOOLSETTING (LOG_STATUS_MESSAGES));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logItems["SPrivate"]), BOOLSETTING (PRIVATE_MESSAGE_BEEP));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (logItems["SPrivateWindow"]), BOOLSETTING (PRIVATE_MESSAGE_BEEP_OPEN));
	
	checkClicked ();
}

void Settings::onLogBrowseClicked_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;

	if (s->lastdir != "")
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (s->dirChooser), s->lastdir.c_str ());
	else
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (s->dirChooser), gtk_entry_get_text (GTK_ENTRY (s->logItems["Directory"])));
 	gint response = gtk_dialog_run (GTK_DIALOG (s->dirChooser));
	gtk_widget_hide(s->dirChooser);

	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (s->dirChooser));
		s->lastdir = path;
		if (path[path.length ()-1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		gtk_entry_set_text (GTK_ENTRY (s->logItems["Directory"]), path.c_str ());
	}
}
void Settings::onLogMainClicked_gui (GtkToggleButton *togglebutton, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	s->checkClicked ();
}
void Settings::onLogPrivateClicked_gui (GtkToggleButton *togglebutton, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	s->checkClicked ();
}
void Settings::onLogDownloadClicked_gui (GtkToggleButton *togglebutton, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	s->checkClicked ();
}
void Settings::onLogUploadClicked_gui (GtkToggleButton *togglebutton, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	s->checkClicked ();
}

void Settings::initAdvanced_gui ()
{
	advancedView.setView(GTK_TREE_VIEW(advancedItems["Advanced"]));
	advancedView.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
	advancedView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
	advancedView.finalize();
	advancedStore = gtk_list_store_newv(advancedView.getColCount(), advancedView.getGTypes());
	gtk_tree_view_set_model(advancedView.get(), GTK_TREE_MODEL(advancedStore));
	g_object_unref(advancedStore);

	GList *list = gtk_tree_view_column_get_cell_renderers (gtk_tree_view_get_column(advancedView.get(), advancedView.col("Use")));
	GtkCellRenderer *renderer = (GtkCellRenderer*)list->data;
	g_signal_connect (renderer, "toggled", G_CALLBACK (onAdvancedToggledClicked_gui), (gpointer)this);	
	
	addAdvanced ("Auto-away on minimize (and back on restore)", SETTING (AUTO_AWAY));
	addAdvanced ("Automatically follow redirects", SETTING (AUTO_FOLLOW));
	addAdvanced ("Clear search box after each search", SETTING (CLEAR_SEARCH));
	addAdvanced ("Open the public hubs window at startup", SETTING (OPEN_PUBLIC));
	addAdvanced ("Open the download queue window at startup", SETTING (OPEN_QUEUE));
	addAdvanced ("Open the favorite hubs window at startup", SETTING (OPEN_FAVORITE_HUBS));
	addAdvanced ("Open the finished downloads window at startup", SETTING (OPEN_FINISHED_DOWNLOADS));
	addAdvanced ("Automatically search for alternative download locations", SETTING (AUTO_SEARCH));
	addAdvanced ("Open private messages in their own window", SETTING (POPUP_PMS));
	addAdvanced ("Ignore private messages from offline users", SETTING (IGNORE_OFFLINE));
	addAdvanced ("Open private messages from offline users in their own window", SETTING (POPUP_OFFLINE));
	addAdvanced ("Open new file list windows in the background", SETTING (POPUNDER_FILELIST));
	addAdvanced ("Open new private message windows in the background", SETTING (POPUNDER_PM));
	addAdvanced ("Keep duplicate files in your file list (duplicates never count towards your share size)", SETTING (LIST_DUPES));
	addAdvanced ("Use small send buffer (enable if uploads slow downloads a lot)", SETTING (SMALL_SEND_BUFFER));
	addAdvanced ("Don't delete file lists when exiting", SETTING (KEEP_LISTS));
	addAdvanced ("Automatically disconnect users who leave the hub (does not disconnect when hub goes down / you leave it)", SETTING (AUTO_KICK));
	addAdvanced ("Show progress bars for transfers", SETTING (SHOW_PROGRESS_BARS));
	addAdvanced ("Enable automatic SFV checking", SETTING (SFV_CHECK));
	addAdvanced ("Automatically refresh share list every hour", SETTING (AUTO_UPDATE_LIST));
	addAdvanced ("Use antifragmentation method for downloads", SETTING (ANTI_FRAG));
	addAdvanced ("Don't send the away message to bots", SETTING (NO_AWAYMSG_TO_BOTS));
	addAdvanced ("Skip zero-byte files", SETTING (SKIP_ZERO_BYTE));
	addAdvanced ("Break on first ADLSearch match", SETTING (ADLS_BREAK_ON_FIRST));
	addAdvanced ("Tab completion of nicks in chat", SETTING (TAB_COMPLETION));
	addAdvanced ("Enable safe and compressed transfers", SETTING (COMPRESS_TRANSFERS));
	addAdvanced ("Accept custom user commands from hub", SETTING (HUB_USER_COMMANDS));
	addAdvanced ("Automatically match queue for auto search hits", SETTING (AUTO_SEARCH_AUTO_MATCH));
	addAdvanced ("Log filelist transfers", SETTING (LOG_FILELIST_TRANSFERS));
	addAdvanced ("Send unknown /commands to the hub", SETTING (SEND_UNKNOWN_COMMANDS));
	addAdvanced ("Add finished files to share instantly (if shared)", SETTING (ADD_FINISHED_INSTANTLY));
	addAdvanced ("Use UPnP Control", SETTING (SETTINGS_USE_UPNP));
	addAdvanced ("Don't download files already in share", SETTING (DONT_DL_ALREADY_SHARED));
	addAdvanced ("Use CTRL for line history", SETTING (SETTINGS_USE_CTRL_FOR_LINE_HISTORY));
	addAdvanced ("Open new window when using /join", SETTING (SETTINGS_OPEN_NEW_WINDOW));
	
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["Rollback"]), (double)SETTING (ROLLBACK));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["Hash"]), (double)SETTING (MAX_HASH_SPEED));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (advancedItems["Write"]), (double)SETTING (BUFFER_SIZE));
}

void Settings::addAdvanced (string name, bool use)
{
	GtkTreeIter iter;
	gtk_list_store_append(advancedStore, &iter);
	gtk_list_store_set(advancedStore, &iter, advancedView.col("Use"), use, advancedView.col("Name"), name.c_str(), -1);
}

void Settings::onAdvancedToggledClicked_gui (GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
	Settings *s = (Settings *)data;
  	GtkTreeIter iter;
  	gboolean fixed;
	GtkTreeModel *m = gtk_tree_view_get_model(s->advancedView.get());
  	gtk_tree_model_get_iter(m, &iter, gtk_tree_path_new_from_string(path_str));
  	gtk_tree_model_get(m, &iter, s->advancedView.col("Use"), &fixed, -1);
  	fixed = !fixed;
  	gtk_list_store_set(GTK_LIST_STORE(m), &iter, s->advancedView.col("Use"), fixed, -1);
}
