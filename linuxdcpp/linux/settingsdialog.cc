/*
 * Copyright © 2004-2006 Jens Oknelid, paskharen@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 *(at your option)any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "settingsdialog.hh"
#include "wulformanager.hh"
#include <iostream>

using namespace std;

Settings::Settings() : DialogEntry("Settings")
{
	string file = WulforManager::get()->getPath() + "/glade/settingsdialog.glade";
	xml = glade_xml_new(file.c_str(), NULL, NULL);
	if (xml == NULL)
		gtk_main_quit();

	setDialog(widget("settingsDialog"));
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(widget("settingsDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(widget("favoriteNameDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(widget("publicHubsDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(widget("virtualNameDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(widget("dirChooserDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	initPersonal_gui();
	initConnection_gui();
	initDownloads_gui();
	initSharing_gui();
	initAppearance_gui();
	initLog_gui();
	initAdvanced_gui();
}

Settings::~Settings()
{
	gtk_widget_destroy(widget("favoriteNameDialog"));
	gtk_widget_destroy(widget("publicHubsDialog"));
	gtk_widget_destroy(widget("virtualNameDialog"));
	gtk_widget_destroy(widget("dirChooserDialog"));
	g_object_unref(xml);
}

void Settings::saveSettings()
{
	SettingsManager *sm = SettingsManager::getInstance();
	GtkTreeIter iter;
	GtkTreeModel *m;
	string path;
	gboolean valid, toggled;
	SettingsManager::IntSetting setting;

	{ // Personal
		sm->set(SettingsManager::NICK, gtk_entry_get_text(GTK_ENTRY(widget("nickEntry"))));
		sm->set(SettingsManager::EMAIL, gtk_entry_get_text(GTK_ENTRY(widget("emailEntry"))));
		sm->set(SettingsManager::DESCRIPTION, gtk_entry_get_text(GTK_ENTRY(widget("descriptionEntry"))));
		sm->set(SettingsManager::UPLOAD_SPEED, SettingsManager::connectionSpeeds[gtk_combo_box_get_active(connectionSpeedComboBox)]);
	}

	{ // Connection
		// Incoming connection
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("activeRadioButton"))))
			sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_DIRECT);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("portForwardRadioButton"))))
			sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_NAT);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("passiveRadioButton"))))
			sm->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);

		sm->set(SettingsManager::EXTERNAL_IP, gtk_entry_get_text(GTK_ENTRY(widget("ipEntry"))));
		sm->set(SettingsManager::NO_IP_OVERRIDE, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("forceIPCheckButton"))));

		int port = Util::toInt(gtk_entry_get_text(GTK_ENTRY(widget("tcpEntry"))));
		if (port > 0 && port <= 65535)
			sm->set(SettingsManager::TCP_PORT, port);
		port = Util::toInt(gtk_entry_get_text(GTK_ENTRY(widget("udpEntry"))));
		if (port > 0 && port <= 65535)
			sm->set(SettingsManager::UDP_PORT, port);

		// Outgoing connection
		int type = SETTING(OUTGOING_CONNECTIONS);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("outDirectRadioButton"))))
			sm->set(SettingsManager::OUTGOING_CONNECTIONS, SettingsManager::OUTGOING_DIRECT);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("socksRadioButton"))))
			sm->set(SettingsManager::OUTGOING_CONNECTIONS, SettingsManager::OUTGOING_SOCKS5);

		if (SETTING(OUTGOING_CONNECTIONS) != type)
			Socket::socksUpdated();

		sm->set(SettingsManager::SOCKS_SERVER, gtk_entry_get_text(GTK_ENTRY(widget("socksIPEntry"))));
		sm->set(SettingsManager::SOCKS_USER, gtk_entry_get_text(GTK_ENTRY(widget("socksUserEntry"))));
		sm->set(SettingsManager::SOCKS_PASSWORD, gtk_entry_get_text(GTK_ENTRY(widget("socksPassEntry"))));

		port = Util::toInt(gtk_entry_get_text(GTK_ENTRY(widget("socksPortEntry"))));
		if (port > 0 && port <= 65535)
			sm->set(SettingsManager::SOCKS_PORT, port);
	}

	{ // Downloads
		path = gtk_entry_get_text(GTK_ENTRY(widget("finishedDownloadsEntry")));
		if (path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sm->set(SettingsManager::DOWNLOAD_DIRECTORY, path);

		path = gtk_entry_get_text(GTK_ENTRY(widget("unfinishedDownloadsEntry")));
		if (!path.empty() && path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sm->set(SettingsManager::TEMP_DOWNLOAD_DIRECTORY, path);
		sm->set(SettingsManager::DOWNLOAD_SLOTS, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("maxDownloadsSpinButton"))));
		sm->set(SettingsManager::MAX_DOWNLOAD_SPEED, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("newDownloadsSpinButton"))));
		sm->set(SettingsManager::HTTP_PROXY, gtk_entry_get_text(GTK_ENTRY(widget("proxyEntry"))));

		{ // Queue
			// Auto-priority
			sm->set(SettingsManager::PRIO_HIGHEST_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("priorityHighestSpinButton")))));
			sm->set(SettingsManager::PRIO_HIGH_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("priorityHighSpinButton")))));
			sm->set(SettingsManager::PRIO_NORMAL_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("priorityNormalSpinButton")))));
			sm->set(SettingsManager::PRIO_LOW_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("priorityLowSpinButton")))));

			// Auto-drop
			sm->set(SettingsManager::AUTODROP_SPEED, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("dropSpeedSpinButton")))));
			sm->set(SettingsManager::AUTODROP_ELAPSED, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("dropElapsedSpinButton")))));
			sm->set(SettingsManager::AUTODROP_MINSOURCES, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("dropMinSourcesSpinButton")))));
			sm->set(SettingsManager::AUTODROP_INTERVAL, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("dropCheckSpinButton")))));
			sm->set(SettingsManager::AUTODROP_INACTIVITY, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("dropInactiveSpinButton")))));
			sm->set(SettingsManager::AUTODROP_FILESIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("dropSizeSpinButton")))));

			// Other queue options
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

	{ // Sharing
		sm->set(SettingsManager::MIN_UPLOAD_SPEED, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("sharedExtraSlotSpinButton"))));
		sm->set(SettingsManager::SLOTS, (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("sharedUploadSlotsSpinButton"))));
	}

	{ // Appearance
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
		sm->set(SettingsManager::DEFAULT_AWAY_MESSAGE, string(gtk_entry_get_text(GTK_ENTRY(widget("awayMessageEntry")))));
		sm->set(SettingsManager::TIME_STAMPS_FORMAT, string(gtk_entry_get_text(GTK_ENTRY(widget("timestampEntry")))));

		{ // Colors and sounds
			// Colors - not implemented

			// Sounds - not implemented
			//sm->set(SettingsManager::PRIVATE_MESSAGE_BEEP, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("soundPMReceivedCheckButton"))));
			//sm->set(SettingsManager::PRIVATE_MESSAGE_BEEP_OPEN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("soundPMWindowCheckButton"))));

			// Tab bolding
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

		{ // Window
			// Auto-open on startup
			m = GTK_TREE_MODEL(windowStore1);
			valid = gtk_tree_model_get_iter_first(m, &iter);

			while (valid)
			{
				setting = (SettingsManager::IntSetting)windowView1.getValue<gint>(&iter, "Setting");
				toggled = windowView1.getValue<gboolean>(&iter, "Use");
				sm->set(setting, toggled);
				valid = gtk_tree_model_iter_next(m, &iter);
			}

			// Window options
			m = GTK_TREE_MODEL(windowStore2);
			valid = gtk_tree_model_get_iter_first(m, &iter);

			while (valid)
			{
				setting = (SettingsManager::IntSetting)windowView2.getValue<gint>(&iter, "Setting");
				toggled = windowView2.getValue<gboolean>(&iter, "Use");
				sm->set(setting, toggled);
				valid = gtk_tree_model_iter_next(m, &iter);
			}

			// Confirm dialog options
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

	{ // Logs
		path = gtk_entry_get_text(GTK_ENTRY(widget("logDirectoryEntry")));
		if (!path.empty() && path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sm->set(SettingsManager::LOG_DIRECTORY, path);
		sm->set(SettingsManager::LOG_MAIN_CHAT, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("logMainCheckButton"))));
		sm->set(SettingsManager::LOG_FORMAT_MAIN_CHAT, string(gtk_entry_get_text(GTK_ENTRY(widget("logMainEntry")))));
		sm->set(SettingsManager::LOG_PRIVATE_CHAT, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("logPrivateCheckButton"))));
		sm->set(SettingsManager::LOG_FORMAT_PRIVATE_CHAT, string(gtk_entry_get_text(GTK_ENTRY(widget("logPrivateEntry")))));
		sm->set(SettingsManager::LOG_DOWNLOADS, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("logDownloadsCheckButton"))));
		sm->set(SettingsManager::LOG_FORMAT_POST_DOWNLOAD, string(gtk_entry_get_text(GTK_ENTRY(widget("logDownloadsEntry")))));
		sm->set(SettingsManager::LOG_UPLOADS, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("logUploadsCheckButton"))));
		sm->set(SettingsManager::LOG_FORMAT_POST_UPLOAD, string(gtk_entry_get_text(GTK_ENTRY(widget("logUploadsEntry")))));
		sm->set(SettingsManager::LOG_SYSTEM, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("logSystemCheckButton"))));
		sm->set(SettingsManager::LOG_STATUS_MESSAGES, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget("logStatusCheckButton"))));
	}

	{ // Advanced
		m = GTK_TREE_MODEL(advancedStore);
		valid = gtk_tree_model_get_iter_first(m, &iter);

		while (valid)
		{
			setting = (SettingsManager::IntSetting)advancedView.getValue<gint>(&iter, "Setting");
			toggled = advancedView.getValue<gboolean>(&iter, "Use");
			sm->set(setting, toggled);
			valid = gtk_tree_model_iter_next(m, &iter);
		}

		// Expert
		sm->set(SettingsManager::ROLLBACK, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("rollbackSpinButton")))));
		sm->set(SettingsManager::MAX_HASH_SPEED, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("hashSpeedSpinButton")))));
		sm->set(SettingsManager::BUFFER_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("writeBufferSpinButton")))));
		sm->set(SettingsManager::SHOW_LAST_LINES_LOG, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("pmHistorySpinButton")))));
		sm->set(SettingsManager::SET_MINISLOT_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("slotSizeSpinButton")))));
		sm->set(SettingsManager::MAX_FILELIST_SIZE, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("maxListSizeSpinButton")))));
		sm->set(SettingsManager::PRIVATE_ID, string(gtk_entry_get_text(GTK_ENTRY(widget("CIDEntry")))));
		sm->set(SettingsManager::AUTO_REFRESH_TIME, Util::toString (gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("autoRefreshSpinButton")))));
		sm->set(SettingsManager::MAX_TAB_ROWS, Util::toString (gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("maxTabRowsSpinButton")))));
		sm->set(SettingsManager::SEARCH_HISTORY, Util::toString (gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("searchHistorySpinButton")))));
		sm->set(SettingsManager::BIND_ADDRESS, string(gtk_entry_get_text(GTK_ENTRY(widget("bindAddressEntry")))));
		sm->set(SettingsManager::SOCKET_IN_BUFFER, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("socketReadSpinButton")))));
		sm->set(SettingsManager::SOCKET_OUT_BUFFER, Util::toString(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget("socketWriteSpinButton")))));
		WSET("fallback-encoding", Text::acpToUtf8(string(gtk_entry_get_text(GTK_ENTRY(widget("encodingEntry"))))));
	}

	sm->save();
}

GtkWidget* Settings::widget(string name)
{
	return glade_xml_get_widget(xml, name.c_str());
}

void Settings::addOption_gui(GtkListStore *store, string name, SettingsManager::IntSetting setting)
{
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, SettingsManager::getInstance()->get(setting), 1, name.c_str(), 2, setting,  -1);
}

void Settings::initPersonal_gui()
{
	gtk_entry_set_text(GTK_ENTRY(widget("nickEntry")), SETTING(NICK).c_str());
	gtk_entry_set_text(GTK_ENTRY(widget("emailEntry")), SETTING(EMAIL).c_str());
	gtk_entry_set_text(GTK_ENTRY(widget("descriptionEntry")), SETTING(DESCRIPTION).c_str());
	connectionSpeedComboBox = GTK_COMBO_BOX(gtk_combo_box_new_text());
	gtk_box_pack_start(GTK_BOX(widget("connectionBox")), GTK_WIDGET(connectionSpeedComboBox), FALSE, TRUE, 0);
	gtk_widget_show_all(GTK_WIDGET(connectionSpeedComboBox));

	for (StringIter i = SettingsManager::connectionSpeeds.begin(); i != SettingsManager::connectionSpeeds.end(); ++i)
	{
		gtk_combo_box_append_text(connectionSpeedComboBox, (*i).c_str());
		if (SETTING(UPLOAD_SPEED) == *i)
			gtk_combo_box_set_active(connectionSpeedComboBox, i - SettingsManager::connectionSpeeds.begin());
	}
}

void Settings::initConnection_gui()
{
	// Incoming
	g_signal_connect(G_OBJECT(widget("activeRadioButton")), "toggled", G_CALLBACK(onInDirect_gui), (gpointer)this);
	///@todo Uncomment when implemented
	//g_signal_connect(G_OBJECT(widget("upnpRadioButton"), "toggled", G_CALLBACK(onInFW_UPnP_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(widget("portForwardRadioButton")), "toggled", G_CALLBACK(onInFW_NAT_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(widget("passiveRadioButton")), "toggled", G_CALLBACK(onInPassive_gui), (gpointer)this);
	gtk_entry_set_text(GTK_ENTRY(widget("ipEntry")), SETTING(EXTERNAL_IP).c_str());
	gtk_entry_set_text(GTK_ENTRY(widget("tcpEntry")), Util::toString(SETTING(TCP_PORT)).c_str());
	gtk_entry_set_text(GTK_ENTRY(widget("udpEntry")), Util::toString(SETTING(UDP_PORT)).c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("forceIPCheckButton")), SETTING(NO_IP_OVERRIDE));

	switch (SETTING(INCOMING_CONNECTIONS))
	{
		case SettingsManager::INCOMING_DIRECT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("activeRadioButton")), TRUE);
			break;
		case SettingsManager::INCOMING_FIREWALL_NAT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("portForwardRadioButton")), TRUE);
			break;
		case SettingsManager::INCOMING_FIREWALL_UPNP:
			///@todo: implement
			break;
		case SettingsManager::INCOMING_FIREWALL_PASSIVE:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("passiveRadioButton")), TRUE);
			break;
	}

	// Outgoing
	g_signal_connect(G_OBJECT(widget("outDirectRadioButton")), "toggled", G_CALLBACK(onOutDirect_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(widget("socksRadioButton")), "toggled", G_CALLBACK(onSocks5_gui), (gpointer)this);
	gtk_entry_set_text(GTK_ENTRY(widget("socksIPEntry")), SETTING(SOCKS_SERVER).c_str());
	gtk_entry_set_text(GTK_ENTRY(widget("socksUserEntry")), SETTING(SOCKS_USER).c_str());
	gtk_entry_set_text(GTK_ENTRY(widget("socksPortEntry")), Util::toString(SETTING(SOCKS_PORT)).c_str());
	gtk_entry_set_text(GTK_ENTRY(widget("socksPassEntry")), SETTING(SOCKS_PASSWORD).c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("socksCheckButton")), SETTING(SOCKS_RESOLVE));

	switch (SETTING(OUTGOING_CONNECTIONS))
	{
		case SettingsManager::OUTGOING_DIRECT:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("outDirectRadioButton")), TRUE);
			onOutDirect_gui(NULL, (gpointer)this);
			break;
		case SettingsManager::OUTGOING_SOCKS5:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("socksRadioButton")), TRUE);
			break;
	}
}

void Settings::initDownloads_gui()
{
	{ // Downloads
		g_signal_connect(G_OBJECT(widget("finishedDownloadsButton")), "clicked", G_CALLBACK(onBrowseFinished_gui), (gpointer)this);
		g_signal_connect(G_OBJECT(widget("unfinishedDownloadsButton")), "clicked", G_CALLBACK(onBrowseUnfinished_gui), (gpointer)this);
		g_signal_connect(G_OBJECT(widget("publicHubsButton")), "clicked", G_CALLBACK(onPublicHubs_gui), (gpointer)this);
		g_signal_connect(G_OBJECT(widget("publicHubsDialogAddButton")), "clicked", G_CALLBACK(onPublicAdd_gui), (gpointer)this);
		g_signal_connect(G_OBJECT(widget("publicHubsDialogUpButton")), "clicked", G_CALLBACK(onPublicMoveUp_gui), (gpointer)this);
		g_signal_connect(G_OBJECT(widget("publicHubsDialogDownButton")), "clicked", G_CALLBACK(onPublicMoveDown_gui), (gpointer)this);
		g_signal_connect(G_OBJECT(widget("publicHubsDialogRemoveButton")), "clicked", G_CALLBACK(onPublicRemove_gui), (gpointer)this);

		gtk_entry_set_text(GTK_ENTRY(widget("finishedDownloadsEntry")), SETTING(DOWNLOAD_DIRECTORY).c_str());
		gtk_entry_set_text(GTK_ENTRY(widget("unfinishedDownloadsEntry")), SETTING(TEMP_DOWNLOAD_DIRECTORY).c_str());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("maxDownloadsSpinButton")), (double)SETTING(DOWNLOAD_SLOTS));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("newDownloadsSpinButton")), (double)SETTING(MAX_DOWNLOAD_SPEED));
		gtk_entry_set_text(GTK_ENTRY(widget("proxyEntry")), SETTING(HTTP_PROXY).c_str());

		publicListView.setView(GTK_TREE_VIEW(widget("publicHubsDialogTreeView")));
		publicListView.insertColumn("List", G_TYPE_STRING, TreeView::EDIT_STRING, -1);
		publicListView.finalize();
		publicListStore = gtk_list_store_newv(publicListView.getColCount(), publicListView.getGTypes());
		gtk_tree_view_set_model(publicListView.get(), GTK_TREE_MODEL(publicListStore));
		g_object_unref(publicListStore);
		gtk_tree_view_set_headers_visible(publicListView.get(), FALSE);
		GtkTreeViewColumn *col = gtk_tree_view_get_column(publicListView.get(), 0);
		GList *list = gtk_tree_view_column_get_cell_renderers(col);
		GObject *editRenderer = G_OBJECT(g_list_nth_data(list, 0));
		g_list_free(list);
		g_signal_connect(editRenderer, "edited", G_CALLBACK(onPublicEdit_gui), (gpointer)this);
	}

	{ // Download to
		g_signal_connect(G_OBJECT(widget("favoriteAddButton")), "clicked", G_CALLBACK(onAddFavorite_gui), (gpointer)this);
		g_signal_connect(G_OBJECT(widget("favoriteRemoveButton")), "clicked", G_CALLBACK(onRemoveFavorite_gui), (gpointer)this);
		downloadToView.setView(GTK_TREE_VIEW(widget("favoriteTreeView")));
		downloadToView.insertColumn("Favorite Name", G_TYPE_STRING, TreeView::STRING, -1);
		downloadToView.insertColumn("Directory", G_TYPE_STRING, TreeView::STRING, -1);
		downloadToView.finalize();
		downloadToStore = gtk_list_store_newv(downloadToView.getColCount(), downloadToView.getGTypes());
		gtk_tree_view_set_model(downloadToView.get(), GTK_TREE_MODEL(downloadToStore));
		g_object_unref(downloadToStore);
		g_signal_connect(G_OBJECT(downloadToView.get()), "button-release-event", G_CALLBACK(onFavoriteButtonReleased_gui), (gpointer)this);
		gtk_widget_set_sensitive(widget("favoriteRemoveButton"), FALSE);

		GtkTreeIter iter;
		StringPairList directories = FavoriteManager::getInstance()->getFavoriteDirs();
		for (StringPairIter j = directories.begin(); j != directories.end(); ++j)
		{
			gtk_list_store_append(downloadToStore, &iter);
			gtk_list_store_set(downloadToStore, &iter,
				downloadToView.col("Favorite Name"), j->second.c_str(),
				downloadToView.col("Directory"), j->first.c_str(),
				-1);
		}
	}

	{ // Queue
		// Auto-priority
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("priorityHighestSpinButton")), (double)SETTING(PRIO_HIGHEST_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("priorityHighSpinButton")), (double)SETTING(PRIO_HIGH_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("priorityNormalSpinButton")), (double)SETTING(PRIO_NORMAL_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("priorityLowSpinButton")), (double)SETTING(PRIO_LOW_SIZE));

		// Auto-drop
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("dropSpeedSpinButton")), (double)SETTING(AUTODROP_SPEED));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("dropElapsedSpinButton")), (double)SETTING(AUTODROP_ELAPSED));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("dropMinSourcesSpinButton")), (double)SETTING(AUTODROP_MINSOURCES));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("dropCheckSpinButton")), (double)SETTING(AUTODROP_INTERVAL));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("dropInactiveSpinButton")), (double)SETTING(AUTODROP_INACTIVITY));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("dropSizeSpinButton")), (double)SETTING(AUTODROP_FILESIZE));

		// Other queue options
		queueView.setView(GTK_TREE_VIEW(widget("queueOtherTreeView")));
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
		g_signal_connect(renderer, "toggled", G_CALLBACK(onQueueToggledClicked_gui), (gpointer)this);
		g_list_free(list);

		addOption_gui(queueStore, "Set lowest priority for newly added files larger than low priority size", SettingsManager::PRIO_LOWEST);
		addOption_gui(queueStore, "Auto-drop slow sources for all queue items (except filelists)", SettingsManager::AUTODROP_ALL);
		addOption_gui(queueStore, "Remove slow filelists", SettingsManager::AUTODROP_FILELISTS);
		addOption_gui(queueStore, "Don't remove the source when auto-dropping, only disconnect", SettingsManager::AUTODROP_DISCONNECT);
		addOption_gui(queueStore, "Automatically search for alternative download locations", SettingsManager::AUTO_SEARCH);
		addOption_gui(queueStore, "Automatically match queue for auto search hits", SettingsManager::AUTO_SEARCH_AUTO_MATCH);
		addOption_gui(queueStore, "Skip zero-byte files", SettingsManager::SKIP_ZERO_BYTE);
		addOption_gui(queueStore, "Don't download files already in share", SettingsManager::DONT_DL_ALREADY_SHARED);
		addOption_gui(queueStore, "Use antifragmentation method for downloads", SettingsManager::ANTI_FRAG);
		addOption_gui(queueStore, "Advanced resume using TTH", SettingsManager::ADVANCED_RESUME);
		addOption_gui(queueStore, "Only download files that have a TTH", SettingsManager::ONLY_DL_TTH_FILES);
	}
}

void Settings::initSharing_gui()
{
	g_signal_connect(G_OBJECT(widget("shareHiddenCheckButton")), "toggled", G_CALLBACK(onShareHiddenPressed_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(widget("sharedAddButton")), "clicked", G_CALLBACK(onAddShare_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(widget("sharedRemoveButton")), "clicked", G_CALLBACK(onRemoveShare_gui), (gpointer)this);

	shareView.setView(GTK_TREE_VIEW(widget("sharedTreeView")));
	shareView.insertColumn("Virtual Name", G_TYPE_STRING, TreeView::STRING, -1);
	shareView.insertColumn("Directory", G_TYPE_STRING, TreeView::STRING, -1);
	shareView.insertColumn("Size", G_TYPE_STRING, TreeView::STRING, -1);
	shareView.insertHiddenColumn("Real Size", G_TYPE_INT64);
	shareView.finalize();
	shareStore = gtk_list_store_newv(shareView.getColCount(), shareView.getGTypes());
	gtk_tree_view_set_model(shareView.get(), GTK_TREE_MODEL(shareStore));
	g_object_unref(shareStore);
	shareView.setSortColumn_gui("Size", "Real Size");
	g_signal_connect(G_OBJECT(shareView.get()), "button-release-event", G_CALLBACK(onShareButtonReleased_gui), (gpointer)this);
	gtk_widget_set_sensitive(widget("sharedRemoveButton"), FALSE);

	GtkTreeIter iter;
	StringPairList directories = ShareManager::getInstance()->getDirectories();
	for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it)
	{
		gtk_list_store_append(shareStore, &iter);
		gtk_list_store_set(shareStore, &iter,
			shareView.col("Virtual Name"), it->first.c_str(),
			shareView.col("Directory"), it->second.c_str(),
			shareView.col("Size"), Util::formatBytes(ShareManager::getInstance()->getShareSize(it->second)).c_str(),
			shareView.col("Real Size"), ShareManager::getInstance()->getShareSize(it->second),
			-1);
	}

	gtk_label_set_text(GTK_LABEL(widget("sharedSizeLabel")), string("Total size: " + Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("shareHiddenCheckButton")), BOOLSETTING(SHARE_HIDDEN));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("sharedExtraSlotSpinButton")), (double)SETTING(MIN_UPLOAD_SPEED));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("sharedUploadSlotsSpinButton")), (double)SETTING(SLOTS));
}

void Settings::initAppearance_gui()
{
	GList *list;
	GObject *renderer;
	{ // Appearance
		appearanceView.setView(GTK_TREE_VIEW(widget("appearanceOptionsTreeView")));
		appearanceView.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		appearanceView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
		appearanceView.insertHiddenColumn("Setting", G_TYPE_INT);
		appearanceView.finalize();
		appearanceStore = gtk_list_store_newv(appearanceView.getColCount(), appearanceView.getGTypes());
		gtk_tree_view_set_model(appearanceView.get(), GTK_TREE_MODEL(appearanceStore));
		g_object_unref(appearanceStore);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(appearanceStore), appearanceView.col("Name"), GTK_SORT_ASCENDING);

		list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(appearanceView.get(), appearanceView.col("Use")));
		renderer = (GObject *)g_list_nth_data(list, 0);
		g_signal_connect(renderer, "toggled", G_CALLBACK(onAppearanceToggledClicked_gui), (gpointer)this);
		g_list_free(list);

		addOption_gui(appearanceStore, "Filter kick and NMDC debug messages", SettingsManager::FILTER_MESSAGES);
		addOption_gui(appearanceStore, "Show tray icon", SettingsManager::MINIMIZE_TRAY);
		addOption_gui(appearanceStore, "Show timestamps in chat by default", SettingsManager::TIME_STAMPS);
		addOption_gui(appearanceStore, "View status messages in main chat", SettingsManager::STATUS_IN_CHAT);
		addOption_gui(appearanceStore, "Show joins / parts in chat by default", SettingsManager::SHOW_JOINS);
		addOption_gui(appearanceStore, "Only show joins / parts for favorite users", SettingsManager::FAV_SHOW_JOINS);
		addOption_gui(appearanceStore, "Use OEM monospaced font for viewing text files", SettingsManager::USE_OEM_MONOFONT);
		/// @todo: Uncomment when implemented
		//addOption_gui(appearanceStore, "User alternative sorting order for transfers", SettingsManager::ALT_SORT_ORDER);
		//addOption_gui(appearanceStore, "Minimize to tray", SettingsManager::MINIMIZE_TRAY);
		//addOption_gui(appearanceStore, "Use system icons when browsing files (slows browsing a bit)", SettingsManager::USE_SYSTEM_ICONS);
		//addOption_gui(appearanceStore, "Guess user country from IP", SettingsManager::GET_USER_COUNTRY);
		///@todo: uncomment when the save problem is solved. Using MINIMIZE_TRAY until then.
		//addOption_gui(appearanceStore, "Show tray icon", WGETI("show-tray-icon"));

		gtk_entry_set_text(GTK_ENTRY(widget("awayMessageEntry")), SETTING(DEFAULT_AWAY_MESSAGE).c_str());
		gtk_entry_set_text(GTK_ENTRY(widget("timestampEntry")), SETTING(TIME_STAMPS_FORMAT).c_str());
	}

	{ // Colors and sounds
		///@todo uncomment when implemented
		//g_signal_connect(G_OBJECT(widget("appearanceColor"), "clicked", G_CALLBACK(onWinColorClicked_gui), (gpointer)this);
		//g_signal_connect(G_OBJECT(widget("upColor"), "clicked", G_CALLBACK(onUpColorClicked_gui), (gpointer)this);
		//g_signal_connect(G_OBJECT(widget("downColor"), "clicked", G_CALLBACK(onDownColorClicked_gui), (gpointer)this);
		//g_signal_connect(G_OBJECT(widget("textStyle"), "clicked", G_CALLBACK(onTextStyleClicked_gui), (gpointer)this);
		//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("soundPMReceivedCheckButton")), BOOLSETTING(PRIVATE_MESSAGE_BEEP));
		//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("soundPMWindowCheckButton")), BOOLSETTING(PRIVATE_MESSAGE_BEEP_OPEN));
		gtk_widget_set_sensitive(widget("appearanceColor"), FALSE);
		gtk_widget_set_sensitive(widget("upColor"), FALSE);
		gtk_widget_set_sensitive(widget("downColor"), FALSE);
		gtk_widget_set_sensitive(widget("textStyle"), FALSE);
		gtk_widget_set_sensitive(widget("soundPMReceivedCheckButton"), FALSE);
		gtk_widget_set_sensitive(widget("soundPMWindowCheckButton"), FALSE);

		colorView.setView(GTK_TREE_VIEW(widget("tabBoldingTreeView")));
		colorView.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		colorView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
		colorView.insertHiddenColumn("Setting", G_TYPE_INT);
		colorView.finalize();
		colorStore = gtk_list_store_newv(colorView.getColCount(), colorView.getGTypes());
		gtk_tree_view_set_model(colorView.get(), GTK_TREE_MODEL(colorStore));
		g_object_unref(colorStore);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(colorStore), colorView.col("Name"), GTK_SORT_ASCENDING);

		list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(colorView.get(), colorView.col("Use")));
		renderer = (GObject *)g_list_nth_data(list, 0);
		g_signal_connect(renderer, "toggled", G_CALLBACK(onColorToggledClicked_gui), (gpointer)this);
		g_list_free(list);

		addOption_gui(colorStore, "Finished downloads", SettingsManager::BOLD_FINISHED_DOWNLOADS);
		addOption_gui(colorStore, "Finished Uploads", SettingsManager::BOLD_FINISHED_UPLOADS);
		addOption_gui(colorStore, "Download Queue", SettingsManager::BOLD_QUEUE);
		addOption_gui(colorStore, "Hub", SettingsManager::BOLD_HUB);
		addOption_gui(colorStore, "Private message", SettingsManager::BOLD_PM);
		addOption_gui(colorStore, "Search", SettingsManager::BOLD_SEARCH);
		/// @todo: Uncomment when implemented
		//addOption_gui(colorStore, "Waiting Users", SettingsManager::BOLD_WAITING_USERS);
		//addOption_gui(colorStore, "System Log", SettingsManager::BOLD_SYSTEM_LOG);
	}

	{ // Window
		// Auto-open
		windowView1.setView(GTK_TREE_VIEW(widget("windowsAutoOpenTreeView")));
		windowView1.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		windowView1.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
		windowView1.insertHiddenColumn("Setting", G_TYPE_INT);
		windowView1.finalize();
		windowStore1 = gtk_list_store_newv(windowView1.getColCount(), windowView1.getGTypes());
		gtk_tree_view_set_model(windowView1.get(), GTK_TREE_MODEL(windowStore1));
		g_object_unref(windowStore1);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(windowStore1), windowView1.col("Name"), GTK_SORT_ASCENDING);

		list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(windowView1.get(), windowView1.col("Use")));
		renderer = (GObject *)g_list_nth_data(list, 0);
		g_signal_connect(renderer, "toggled", G_CALLBACK(onWindowView1ToggledClicked_gui), (gpointer)this);
		g_list_free(list);

		addOption_gui(windowStore1, "Public Hubs", SettingsManager::OPEN_PUBLIC);
		addOption_gui(windowStore1, "Favorite Hubs", SettingsManager::OPEN_FAVORITE_HUBS);
		addOption_gui(windowStore1, "Download Queue", SettingsManager::OPEN_QUEUE);
		addOption_gui(windowStore1, "Finished Downloads", SettingsManager::OPEN_FINISHED_DOWNLOADS);
		addOption_gui(windowStore1, "Finished Uploads", SettingsManager::OPEN_FINISHED_UPLOADS);
		/// @todo: Uncomment when implemented
		//addOption_gui(windowStore1, "Favorite Users", SettingsManager::OPEN_FAVORITE_USERS);
		//addOption_gui(windowStore1, "Waiting Users", SettingsManager::OPEN_WAITING_USERS);
		//addOption_gui(windowStore1, "Search Spy", SettingsManager::OPEN_SEARCH_SPY);
		//addOption_gui(windowStore1, "Network Statistics", SettingsManager::OPEN_NETWORK_STATISTICS);
		//addOption_gui(windowStore1, "Notepad", SettingsManager::OPEN_NOTEPAD);
		//addOption_gui(windowStore1, "System Log", SettingsManager::OPEN_SYSTEM_LOG);

		// Window options
		windowView2.setView(GTK_TREE_VIEW(widget("windowsOptionsTreeView")));
		windowView2.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		windowView2.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
		windowView2.insertHiddenColumn("Setting", G_TYPE_INT);
		windowView2.finalize();
		windowStore2 = gtk_list_store_newv(windowView2.getColCount(), windowView2.getGTypes());
		gtk_tree_view_set_model(windowView2.get(), GTK_TREE_MODEL(windowStore2));
		g_object_unref(windowStore2);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(windowStore2), windowView2.col("Name"), GTK_SORT_ASCENDING);

		list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(windowView2.get(), windowView2.col("Use")));
		renderer = (GObject *)g_list_nth_data(list, 0);
		g_signal_connect(renderer, "toggled", G_CALLBACK(onWindowView2ToggledClicked_gui), (gpointer)this);
		g_list_free(list);

		addOption_gui(windowStore2, "Open file list window in the background", SettingsManager::POPUNDER_FILELIST);
		addOption_gui(windowStore2, "Open new private message windows in the background", SettingsManager::POPUNDER_PM);
		addOption_gui(windowStore2, "Open new window when using /join", SettingsManager::JOIN_OPEN_NEW_WINDOW);
		addOption_gui(windowStore2, "Ignore private messages from offline users", SettingsManager::IGNORE_OFFLINE);
		/// @todo: Uncomment when implemented
		//addOption_gui(windowStore2, "Open private messages in their own window", SettingsManager::POPUP_PMS);
		//addOption_gui(windowStore2, "Open private messages from offline users in their own window", SettingsManager::POPUP_OFFLINE);
		//addOption_gui(windowStore2, "Toggle window when selecting an active tab", SettingsManager::TOGGLE_ACTIVE_WINDOW);

		// Confirmation dialog
		windowView3.setView(GTK_TREE_VIEW(widget("windowsConfirmTreeView")));
		windowView3.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		windowView3.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
		windowView3.insertHiddenColumn("Setting", G_TYPE_INT);
		windowView3.finalize();
		windowStore3 = gtk_list_store_newv(windowView3.getColCount(), windowView3.getGTypes());
		gtk_tree_view_set_model(windowView3.get(), GTK_TREE_MODEL(windowStore3));
		g_object_unref(windowStore3);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(windowStore3), windowView3.col("Name"), GTK_SORT_ASCENDING);

		list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(windowView3.get(), windowView3.col("Use")));
		renderer = (GObject *)g_list_nth_data(list, 0);
		g_signal_connect(renderer, "toggled", G_CALLBACK(onWindowView3ToggledClicked_gui), (gpointer)this);
		g_list_free(list);

		addOption_gui(windowStore3, "Confirm application exit", SettingsManager::CONFIRM_EXIT);
		addOption_gui(windowStore3, "Confirm favorite hub removal", SettingsManager::CONFIRM_HUB_REMOVAL);
		/// @todo: Uncomment when implemented
		//addOption_gui(windowStore3, "Confirm item removal in download queue", SettingsManager::CONFIRM_ITEM_REMOVAL);
	}
}

void Settings::initLog_gui()
{
	g_signal_connect(G_OBJECT(widget("logBrowseButton")), "clicked", G_CALLBACK(onLogBrowseClicked_gui), (gpointer)this);
	gtk_entry_set_text(GTK_ENTRY(widget("logDirectoryEntry")), SETTING(LOG_DIRECTORY).c_str());

	g_signal_connect(G_OBJECT(widget("logMainCheckButton")), "toggled", G_CALLBACK(onLogMainClicked_gui), (gpointer)this);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("logMainCheckButton")), BOOLSETTING(LOG_MAIN_CHAT));
	gtk_entry_set_text(GTK_ENTRY(widget("logMainEntry")), SETTING(LOG_FORMAT_MAIN_CHAT).c_str());
	gtk_widget_set_sensitive(widget("logMainLabel"), BOOLSETTING(LOG_MAIN_CHAT));
	gtk_widget_set_sensitive(widget("logMainEntry"), BOOLSETTING(LOG_MAIN_CHAT));

	g_signal_connect(G_OBJECT(widget("logPrivateCheckButton")), "toggled", G_CALLBACK(onLogPrivateClicked_gui), (gpointer)this);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("logPrivateCheckButton")), BOOLSETTING(LOG_PRIVATE_CHAT));
	gtk_entry_set_text(GTK_ENTRY(widget("logPrivateEntry")), SETTING(LOG_FORMAT_PRIVATE_CHAT).c_str());
	gtk_widget_set_sensitive(widget("logPrivateLabel"), BOOLSETTING(LOG_PRIVATE_CHAT));
	gtk_widget_set_sensitive(widget("logPrivateEntry"), BOOLSETTING(LOG_PRIVATE_CHAT));

	g_signal_connect(G_OBJECT(widget("logDownloadsCheckButton")), "toggled", G_CALLBACK(onLogDownloadClicked_gui), (gpointer)this);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("logDownloadsCheckButton")), BOOLSETTING(LOG_DOWNLOADS));
	gtk_entry_set_text(GTK_ENTRY(widget("logDownloadsEntry")), SETTING(LOG_FORMAT_POST_DOWNLOAD).c_str());
	gtk_widget_set_sensitive(widget("logDownloadsLabel"), BOOLSETTING(LOG_DOWNLOADS));
	gtk_widget_set_sensitive(widget("logDownloadsEntry"), BOOLSETTING(LOG_DOWNLOADS));

	g_signal_connect(G_OBJECT(widget("logUploadsCheckButton")), "toggled", G_CALLBACK(onLogUploadClicked_gui), (gpointer)this);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("logUploadsCheckButton")), BOOLSETTING(LOG_UPLOADS));
	gtk_entry_set_text(GTK_ENTRY(widget("logUploadsEntry")), SETTING(LOG_FORMAT_POST_UPLOAD).c_str());
	gtk_widget_set_sensitive(widget("logUploadsLabel"), BOOLSETTING(LOG_UPLOADS));
	gtk_widget_set_sensitive(widget("logUploadsEntry"), BOOLSETTING(LOG_UPLOADS));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("logSystemCheckButton")), BOOLSETTING(LOG_SYSTEM));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget("logStatusCheckButton")), BOOLSETTING(LOG_STATUS_MESSAGES));
}

void Settings::initAdvanced_gui()
{
	{ // Advanced
		advancedView.setView(GTK_TREE_VIEW(widget("advancedTreeView")));
		advancedView.insertColumn("Use", G_TYPE_BOOLEAN, TreeView::BOOL, -1);
		advancedView.insertColumn("Name", G_TYPE_STRING, TreeView::STRING, -1);
		advancedView.insertHiddenColumn("Setting", G_TYPE_INT);
		advancedView.finalize();
		advancedStore = gtk_list_store_newv(advancedView.getColCount(), advancedView.getGTypes());
		gtk_tree_view_set_model(advancedView.get(), GTK_TREE_MODEL(advancedStore));
		g_object_unref(advancedStore);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(advancedStore), advancedView.col("Name"), GTK_SORT_ASCENDING);

		GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(advancedView.get(), advancedView.col("Use")));
		GtkCellRenderer *renderer = (GtkCellRenderer*)g_list_nth_data(list, 0);
		g_signal_connect(renderer, "toggled", G_CALLBACK(onAdvancedToggledClicked_gui), (gpointer)this);
		g_list_free(list);

		addOption_gui(advancedStore, "Auto-away on minimize (and back on restore)", SettingsManager::AUTO_AWAY);
		addOption_gui(advancedStore, "Automatically follow redirects", SettingsManager::AUTO_FOLLOW);
		addOption_gui(advancedStore, "Clear search box after each search", SettingsManager::CLEAR_SEARCH);
		addOption_gui(advancedStore, "Keep duplicate files in your file list (duplicates never count towards your share size)", SettingsManager::LIST_DUPES);
		addOption_gui(advancedStore, "Don't delete file lists when exiting", SettingsManager::KEEP_LISTS);
		addOption_gui(advancedStore, "Automatically disconnect users who leave the hub",
			SettingsManager::AUTO_KICK);
		addOption_gui(advancedStore, "Show progress bars for transfers", SettingsManager::SHOW_PROGRESS_BARS);
		addOption_gui(advancedStore, "Enable automatic SFV checking", SettingsManager::SFV_CHECK);
		addOption_gui(advancedStore, "Enable safe and compressed transfers", SettingsManager::COMPRESS_TRANSFERS);
		addOption_gui(advancedStore, "Accept custom user commands from hub", SettingsManager::HUB_USER_COMMANDS);
		addOption_gui(advancedStore, "Send unknown /commands to the hub", SettingsManager::SEND_UNKNOWN_COMMANDS);
		addOption_gui(advancedStore, "Add finished files to share instantly (if shared)", SettingsManager::ADD_FINISHED_INSTANTLY);
		/// @todo: Uncomment when implemented
		//addOption_gui(advancedStore, "Register with the OS to handle dchub:// and adc:// URL links", SettingsManager::URL_HANDLER);
		//addOption_gui(advancedStore, "Register with the OS to handle magnet: URL links", SettingsManager::MAGNET_REGISTER);
		//addOption_gui(advancedStore, "Don't send the away message to bots", SettingsManager::NO_AWAYMSG_TO_BOTS);
		//addOption_gui(advancedStore, "Break on first ADLSearch match", SettingsManager::ADLS_BREAK_ON_FIRST);
		//addOption_gui(advancedStore, "Use CTRL for line history", SettingsManager::USE_CTRL_FOR_LINE_HISTORY);
		//addOption_gui(advancedStore, "Use SSL when remote client supports it", SettingsManager::USE_SSL);
	}

	{ // User Commands
		gtk_widget_set_sensitive(widget("buttonAddUC"), FALSE);
		gtk_widget_set_sensitive(widget("buttonChangeUC"), FALSE);
		gtk_widget_set_sensitive(widget("buttonMUUC"), FALSE);
		gtk_widget_set_sensitive(widget("buttonMDUC"), FALSE);
		gtk_widget_set_sensitive(widget("buttonRemoveUC"), FALSE);
	}

	{ // Experts
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("rollbackSpinButton")), (double)SETTING(ROLLBACK));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("hashSpeedSpinButton")), (double)SETTING(MAX_HASH_SPEED));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("writeBufferSpinButton")), (double)SETTING(BUFFER_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("pmHistorySpinButton")), (double)SETTING(SHOW_LAST_LINES_LOG));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("slotSizeSpinButton")), (double)SETTING(SET_MINISLOT_SIZE));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("maxListSizeSpinButton")), (double)SETTING(MAX_FILELIST_SIZE));
		gtk_entry_set_text(GTK_ENTRY(widget("CIDEntry")), SETTING(PRIVATE_ID).c_str());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("autoRefreshSpinButton")), (double)SETTING(AUTO_REFRESH_TIME));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("maxTabRowsSpinButton")), (double)SETTING(MAX_TAB_ROWS));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("searchHistorySpinButton")), (double)SETTING(SEARCH_HISTORY));
		gtk_entry_set_text(GTK_ENTRY(widget("bindAddressEntry")), SETTING(BIND_ADDRESS).c_str());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("socketReadSpinButton")), (double)SETTING(SOCKET_IN_BUFFER));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget("socketWriteSpinButton")), (double)SETTING(SOCKET_OUT_BUFFER));
		gtk_entry_set_text(GTK_ENTRY(widget("encodingEntry")), WGETS("fallback-encoding").c_str());
	}
}

void Settings::onAddShare_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;

 	gint response = gtk_dialog_run(GTK_DIALOG(s->widget("dirChooserDialog")));
	gtk_widget_hide(s->widget("dirChooserDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->widget("dirChooserDialog")));
		string path = temp;
		g_free(temp);

		gtk_entry_set_text(GTK_ENTRY(s->widget("virtualNameDialogEntry")), "");
		response = gtk_dialog_run(GTK_DIALOG(s->widget("virtualNameDialog")));
		gtk_widget_hide(s->widget("virtualNameDialog"));

		if (response == GTK_RESPONSE_OK)
		{
			string name = gtk_entry_get_text(GTK_ENTRY(s->widget("virtualNameDialogEntry")));
			typedef Func2<Settings, string, string> F2;
			F2 *func = new F2(s, &Settings::addShare_client, path, name);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void Settings::onInDirect_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->widget("ipEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("ipLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("tcpEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("tcpLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("udpEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("udpLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("forceIPCheckButton"), TRUE);
}

/**@todo Uncomment when implemented
void Settings::onInFW_UPnP_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->widget("ipEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("ipLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("tcpEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("tcpLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("udpEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("udpLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("forceIPCheckButton"), TRUE);
}
*/

void Settings::onInFW_NAT_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->widget("ipEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("ipLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("tcpEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("tcpLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("udpEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("udpLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("forceIPCheckButton"), TRUE);
}

void Settings::onInPassive_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->widget("ipEntry"), FALSE);
	gtk_widget_set_sensitive(s->widget("ipLabel"), FALSE);
	gtk_widget_set_sensitive(s->widget("tcpEntry"), FALSE);
	gtk_widget_set_sensitive(s->widget("tcpLabel"), FALSE);
	gtk_widget_set_sensitive(s->widget("udpEntry"), FALSE);
	gtk_widget_set_sensitive(s->widget("udpLabel"), FALSE);
	gtk_widget_set_sensitive(s->widget("forceIPCheckButton"), FALSE);
}

void Settings::onOutDirect_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->widget("socksIPEntry"), FALSE);
	gtk_widget_set_sensitive(s->widget("socksIPLabel"), FALSE);
	gtk_widget_set_sensitive(s->widget("socksUserEntry"), FALSE);
	gtk_widget_set_sensitive(s->widget("socksUserLabel"), FALSE);
	gtk_widget_set_sensitive(s->widget("socksPortEntry"), FALSE);
	gtk_widget_set_sensitive(s->widget("socksPortLabel"), FALSE);
	gtk_widget_set_sensitive(s->widget("socksPassEntry"), FALSE);
	gtk_widget_set_sensitive(s->widget("socksPassLabel"), FALSE);
	gtk_widget_set_sensitive(s->widget("socksCheckButton"), FALSE);
}

void Settings::onSocks5_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	gtk_widget_set_sensitive(s->widget("socksIPEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("socksIPLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("socksUserEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("socksUserLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("socksPortEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("socksPortLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("socksPassEntry"), TRUE);
	gtk_widget_set_sensitive(s->widget("socksPassLabel"), TRUE);
	gtk_widget_set_sensitive(s->widget("socksCheckButton"), TRUE);
}

void Settings::onBrowseFinished_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;

 	gint response = gtk_dialog_run(GTK_DIALOG(s->widget("dirChooserDialog")));
	gtk_widget_hide(s->widget("dirChooserDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->widget("dirChooserDialog")));
		string path = temp;
		g_free(temp);
		if (!path.empty() && path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		gtk_entry_set_text(GTK_ENTRY(s->widget("finishedDownloadsEntry")), path.c_str());
	}
}

void Settings::onBrowseUnfinished_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;

 	gint response = gtk_dialog_run(GTK_DIALOG(s->widget("dirChooserDialog")));
	gtk_widget_hide(s->widget("dirChooserDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->widget("dirChooserDialog")));
		string path = temp;
		g_free(temp);
		if (!path.empty() && path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		gtk_entry_set_text(GTK_ENTRY(s->widget("unfinishedDownloadsEntry")), path.c_str());
	}
}

void Settings::onPublicHubs_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;

	gtk_list_store_clear(s->publicListStore);
	StringList lists(FavoriteManager::getInstance()->getHubLists());
	for (StringList::iterator idx = lists.begin(); idx != lists.end(); ++idx)
	{
		gtk_list_store_append(s->publicListStore, &iter);
		gtk_list_store_set(s->publicListStore, &iter, s->publicListView.col("List"), (*idx).c_str(), -1);
	}

	gint response = gtk_dialog_run(GTK_DIALOG(s->widget("publicHubsDialog")));
	gtk_widget_hide(s->widget("publicHubsDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		string lists = "";
		GtkTreeModel *m = GTK_TREE_MODEL(s->publicListStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, &iter);
		while (valid)
		{
			lists += s->publicListView.getString(&iter, "List") + ";";
			valid = gtk_tree_model_iter_next(m, &iter);
		}
		if (!lists.empty())
			lists.erase(lists.size() - 1);
		SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, lists);
	}
}

void Settings::onPublicAdd_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeViewColumn *col;

	gtk_list_store_append(s->publicListStore, &iter);
	gtk_list_store_set(s->publicListStore, &iter, s->publicListView.col("List"), "New list", -1);
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(s->publicListStore), &iter);
	col = gtk_tree_view_get_column(s->publicListView.get(), 0);
	gtk_tree_view_set_cursor(s->publicListView.get(), path, col, TRUE);
	gtk_tree_path_free(path);
}

void Settings::onPublicMoveUp_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter prev, current;
	GtkTreeModel *m = GTK_TREE_MODEL(s->publicListStore);
	GtkTreeSelection *sel = gtk_tree_view_get_selection(s->publicListView.get());

	if (gtk_tree_selection_get_selected(sel, NULL, &current))
	{
		GtkTreePath *path = gtk_tree_model_get_path(m, &current);
		if (gtk_tree_path_prev(path) && gtk_tree_model_get_iter(m, &prev, path))
			gtk_list_store_swap(s->publicListStore, &current, &prev);
		gtk_tree_path_free(path);
	}
}

void Settings::onPublicMoveDown_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter current, next;
	GtkTreeSelection *sel = gtk_tree_view_get_selection(s->publicListView.get());

	if (gtk_tree_selection_get_selected(sel, NULL, &current))
	{
		next = current;
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(s->publicListStore), &next))
			gtk_list_store_swap(s->publicListStore, &current, &next);
	}
}

void Settings::onPublicEdit_gui(GtkCellRendererText *cell, char *path, char *text, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->publicListStore), &iter, path))
		gtk_list_store_set(s->publicListStore, &iter, 0, text, -1);
}

void Settings::onPublicRemove_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->publicListView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		gtk_list_store_remove(s->publicListStore, &iter);
}

void Settings::onAddFavorite_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;

 	gint response = gtk_dialog_run(GTK_DIALOG(s->widget("dirChooserDialog")));
	gtk_widget_hide(s->widget("dirChooserDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->widget("dirChooserDialog")));
		string path = temp;
		g_free(temp);

		gtk_entry_set_text(GTK_ENTRY(s->widget("favoriteNameDialogEntry")), "");
		response = gtk_dialog_run(GTK_DIALOG(s->widget("favoriteNameDialog")));
		gtk_widget_hide(s->widget("favoriteNameDialog"));

		if (response == GTK_RESPONSE_OK)
		{
			string name = gtk_entry_get_text(GTK_ENTRY(s->widget("favoriteNameDialogEntry")));
			if (path[path.length() - 1] != PATH_SEPARATOR)
				path += PATH_SEPARATOR;

			if (!name.empty() && FavoriteManager::getInstance()->addFavoriteDir(path, name))
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
				GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(s->widget("settingsDialog")), GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Directory or favorite name already exists");
				gtk_dialog_run(GTK_DIALOG(d));
				gtk_widget_destroy(d);
			}
		}
	}
}

void Settings::onRemoveFavorite_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->downloadToView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string path = s->downloadToView.getString(&iter, "Directory");
		if (FavoriteManager::getInstance()->removeFavoriteDir(path))
		{
			gtk_list_store_remove(s->downloadToStore, &iter);
			gtk_widget_set_sensitive(s->widget("favoriteRemoveButton"), FALSE);
		}
	}
}

gboolean Settings::onFavoriteButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->downloadToView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		gtk_widget_set_sensitive(s->widget("favoriteRemoveButton"), TRUE);
	else
		gtk_widget_set_sensitive(s->widget("favoriteRemoveButton"), FALSE);

	return FALSE;
}

void Settings::onQueueToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	Settings *s = (Settings *)data;
  	GtkTreeIter iter;

  	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->queueStore), &iter, path))
  	{
		gboolean fixed = s->queueView.getValue<gboolean>(&iter,"Use");
  		gtk_list_store_set(s->queueStore, &iter, s->queueView.col("Use"), !fixed, -1);
	}
}

void Settings::addShare_gui(string path, string name, string error)
{
	if (error.empty())
	{
		GtkTreeIter iter;
		int64_t size = ShareManager::getInstance()->getShareSize(path);

		gtk_list_store_append(shareStore, &iter);
		gtk_list_store_set(shareStore, &iter,
			shareView.col("Virtual Name"), name.c_str(),
			shareView.col("Directory"), path.c_str(),
			shareView.col("Size"), Util::formatBytes(size).c_str(),
			shareView.col("Real Size"), size,
			-1);
	}
	else
	{
		GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(widget("settingsDialog")),
			GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, error.c_str());
		gtk_dialog_run(GTK_DIALOG(d));
		gtk_widget_destroy(d);
	}
}

void Settings::onRemoveShare_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->shareView.get());

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		string path = s->shareView.getString(&iter, "Directory");
		gtk_list_store_remove(s->shareStore, &iter);
		gtk_widget_set_sensitive(s->widget("sharedRemoveButton"), FALSE);

		ShareManager::getInstance()->removeDirectory(path);
	}
}

gboolean Settings::onShareButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->shareView.get());

	if (gtk_tree_selection_count_selected_rows(selection) == 0)
		gtk_widget_set_sensitive(s->widget("sharedRemoveButton"), FALSE);
	else
		gtk_widget_set_sensitive(s->widget("sharedRemoveButton"), TRUE);

	return FALSE;
}

gboolean Settings::onShareHiddenPressed_gui(GtkToggleButton *togglebutton, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;
	bool show;
	int64_t size;

	show = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->widget("shareHiddenCheckButton")));

	Func1<Settings, bool> *func = new Func1<Settings, bool>(s, &Settings::shareHidden_client, show);
	WulforManager::get()->dispatchClientFunc(func);

	gtk_list_store_clear(s->shareStore);
	StringPairList directories = ShareManager::getInstance()->getDirectories();
	for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it)
	{
		size = ShareManager::getInstance()->getShareSize(it->second);
		gtk_list_store_append(s->shareStore, &iter);
		gtk_list_store_set(s->shareStore, &iter,
			s->shareView.col("Virtual Name"), it->first.c_str(),
			s->shareView.col("Directory"), it->second.c_str(),
			s->shareView.col("Size"), Util::formatBytes(size).c_str(),
			s->shareView.col("Real Size"), size,
			-1);
	}

	gtk_label_set_text(GTK_LABEL(s->widget("sharedSizeLabel")), string("Total size: " +
		Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());

	return FALSE;
}

void Settings::onAppearanceToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	Settings *s = (Settings *)data;
  	GtkTreeIter iter;

  	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->appearanceStore), &iter, path))
  	{
		gboolean fixed = s->appearanceView.getValue<gboolean>(&iter,"Use");
  		gtk_list_store_set(s->appearanceStore, &iter, s->appearanceView.col("Use"), !fixed, -1);
	}
}

void Settings::onColorToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	Settings *s = (Settings *)data;
  	GtkTreeIter iter;

  	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->colorStore), &iter, path))
  	{
		gboolean fixed = s->colorView.getValue<gboolean>(&iter, "Use");
  		gtk_list_store_set(s->colorStore, &iter, s->colorView.col("Use"), !fixed, -1);
	}
}

void Settings::onWindowView1ToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->windowStore1), &iter, path))
	{
		gboolean fixed = s->windowView1.getValue<gboolean>(&iter, "Use");
		gtk_list_store_set(s->windowStore1, &iter, s->windowView1.col("Use"), !fixed, -1);
	}
}

void Settings::onWindowView2ToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->windowStore2), &iter, path))
	{
		gboolean fixed = s->windowView2.getValue<gboolean>(&iter, "Use");
		gtk_list_store_set(s->windowStore2, &iter, s->windowView2.col("Use"), !fixed, -1);
	}
}

void Settings::onWindowView3ToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	Settings *s = (Settings *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->windowStore3), &iter, path))
	{
		gboolean fixed = s->windowView3.getValue<gboolean>(&iter, "Use");
		gtk_list_store_set(s->windowStore3, &iter, s->windowView3.col("Use"), !fixed, -1);
	}
}

void Settings::onLogBrowseClicked_gui(GtkWidget *widget, gpointer data)
{
	Settings *s = (Settings *)data;

 	gint response = gtk_dialog_run(GTK_DIALOG(s->widget("dirChooserDialog")));
	gtk_widget_hide(s->widget("dirChooserDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(s->widget("dirChooserDialog")));
		string path = temp;
		g_free(temp);
		if (path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		gtk_entry_set_text(GTK_ENTRY(s->widget("logDirectoryEntry")), path.c_str());
	}
}

void Settings::onLogMainClicked_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	bool toggled = gtk_toggle_button_get_active(button);
	gtk_widget_set_sensitive(s->widget("logMainLabel"), toggled);
	gtk_widget_set_sensitive(s->widget("logMainEntry"), toggled);
}

void Settings::onLogPrivateClicked_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	bool toggled = gtk_toggle_button_get_active(button);
	gtk_widget_set_sensitive(s->widget("logPrivateLabel"), toggled);
	gtk_widget_set_sensitive(s->widget("logPrivateEntry"), toggled);
}

void Settings::onLogDownloadClicked_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	bool toggled = gtk_toggle_button_get_active(button);
	gtk_widget_set_sensitive(s->widget("logDownloadsLabel"), toggled);
	gtk_widget_set_sensitive(s->widget("logDownloadsEntry"), toggled);
}

void Settings::onLogUploadClicked_gui(GtkToggleButton *button, gpointer data)
{
	Settings *s = (Settings *)data;
	bool toggled = gtk_toggle_button_get_active(button);
	gtk_widget_set_sensitive(s->widget("logUploadsLabel"), toggled);
	gtk_widget_set_sensitive(s->widget("logUploadsEntry"), toggled);
}

void Settings::onAdvancedToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	Settings *s = (Settings *)data;
  	GtkTreeIter iter;

  	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->advancedStore), &iter, path))
  	{
		gboolean fixed = s->advancedView.getValue<gboolean>(&iter, "Use");
  		gtk_list_store_set(s->advancedStore, &iter, s->advancedView.col("Use"), !fixed, -1);
	}
}

void Settings::shareHidden_client(bool show)
{
	SettingsManager::getInstance()->set(SettingsManager::SHARE_HIDDEN, show);
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(TRUE, FALSE, TRUE);
}

void Settings::addShare_client(string path, string name)
{
	string error;

	try
	{
		ShareManager::getInstance()->addDirectory(path, name);
	}
	catch (const ShareException &e)
	{
		error = e.getError();
	}

	typedef Func3<Settings, string, string, string> F3;
	F3 *func = new F3(this, &Settings::addShare_gui, path, name, error);
	WulforManager::get()->dispatchGuiFunc(func);
}
