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
#include <iostream>
#include <sstream>

using namespace std;
Settings::~Settings ()
{
	pthread_mutex_destroy(&settingsLock);
}
Settings::Settings ()
{
	string file = WulforManager::get()->getPath() + "/glade/settingsdialog.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	dialog = glade_xml_get_widget(xml, "settingsDialog");
	favoriteName = glade_xml_get_widget (xml, "favoriteName");
	publicHubs = glade_xml_get_widget (xml, "publicDialog");
	editPublic = glade_xml_get_widget (xml, "editPublic");
	virtualName = glade_xml_get_widget (xml, "virtualName");

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
	
	pthread_mutex_init(&settingsLock, NULL);
	
	initGeneral_gui ();
	initDownloads_gui ();
	initSharing_gui ();
}
void Settings::saveSettings_client ()
{
	SettingsManager *sm = SettingsManager::getInstance ();
	
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

	GtkTreeView *view = GTK_TREE_VIEW (downloadItems["View"]);
   	 g_signal_connect(G_OBJECT (view), "button_press_event", G_CALLBACK(onFavoriteButtonPressed_gui), (gpointer)this);

	downloadToStore = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model (view, GTK_TREE_MODEL (downloadToStore));
	downloadTo = new TreeViewFactory (view);

	downloadTo->addColumn_gui (DOWNLOADTO_NAME, "Favorite name", TreeViewFactory::STRING, 100);
	downloadTo->addColumn_gui (DOWNLOADTO_DIR, "Directory", TreeViewFactory::STRING, 200);
	gtk_widget_set_sensitive (downloadItems["Remove"], FALSE);
	StringPairList directories = HubManager::getInstance()->getFavoriteDirs();
	for(StringPairIter j = directories.begin(); j != directories.end(); j++)
	{
		GtkTreeIter iter;
		gtk_list_store_append (downloadToStore, &iter);
		gtk_list_store_set (	downloadToStore, &iter,
										DOWNLOADTO_NAME, j->second.c_str (),
										DOWNLOADTO_DIR, j->first.c_str (),
										-1);
	}

	GtkTreeView *view2 = GTK_TREE_VIEW (downloadItems["Public view"]);
	publicListStore = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_tree_view_set_model (view2, GTK_TREE_MODEL (publicListStore));

	publicList = new TreeViewFactory (view2);
	publicList->addColumn_gui (0, "List", TreeViewFactory::STRING, 200);
}
void Settings::onBrowseF_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	s->dirChooser = gtk_file_chooser_dialog_new (	"Choose directory",
											WulforManager::get ()->getMainWindow ()->getWindow (),
											GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
											GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											GTK_STOCK_OPEN, GTK_RESPONSE_OK,
											NULL);
	if (s->lastdir != "")
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (s->dirChooser), s->lastdir.c_str ());
 	gint response = gtk_dialog_run (GTK_DIALOG (s->dirChooser));
	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (s->dirChooser));
		s->lastdir = path;
		gtk_entry_set_text (GTK_ENTRY (s->downloadItems["Finished"]), path.c_str ());
	}
	gtk_widget_hide (s->dirChooser);	
}
void Settings::onBrowseUF_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	s->dirChooser = gtk_file_chooser_dialog_new (	"Choose directory",
											WulforManager::get ()->getMainWindow ()->getWindow (),
											GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
											GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											GTK_STOCK_OPEN, GTK_RESPONSE_OK,
											NULL);
	if (s->lastdir != "")
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (s->dirChooser), s->lastdir.c_str ());
 	gint response = gtk_dialog_run (GTK_DIALOG (s->dirChooser));
	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (s->dirChooser));
		s->lastdir = path;
		gtk_entry_set_text (GTK_ENTRY (s->downloadItems["Unfinished"]), path.c_str ());
	}
	gtk_widget_hide (s->dirChooser);	
}
void Settings::setPublicHubs_client (string list)
{
	pthread_mutex_lock (&settingsLock);
	SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, list);
	pthread_mutex_unlock (&settingsLock);
}
void Settings::onPublicHubs_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;

	s->publicInit_gui ();
	gint response = gtk_dialog_run (GTK_DIALOG (s->publicHubs));
	if (response == GTK_RESPONSE_OK)
	{
		string tmp;
		vector<string> items;
		TreeViewFactory::getColumn (GTK_TREE_MODEL (s->publicListStore), 0, &items);
		for(int i = 0; i < items.size (); i++)
			tmp += items[i] + ';';
		if(items.size () > 0)
			tmp.erase(tmp.size()-1);
		s->setPublicHubs_client (tmp);
	}
	gtk_widget_hide (s->publicHubs);
}
void Settings::onPublicAdd_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;

	string list = gtk_entry_get_text (GTK_ENTRY (s->downloadItems["Public list"]));
	if (list == "")
		return;

	GtkTreeIter iter;
	gtk_list_store_append (s->publicListStore, &iter);
	gtk_list_store_set (s->publicListStore, &iter, 0, list.c_str (), -1);
	gtk_entry_set_text (GTK_ENTRY (s->downloadItems["Public list"]), "");
}
void Settings::onPublicMU_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (s->publicListStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->publicList->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
		
	string tmp = TreeViewFactory::getValue<gchar*,string>(m, &iter, 0);
	vector<string> items;
	TreeViewFactory::getColumn (m, 0, &items);
	for(int i = 1; i < TreeViewFactory::getCount (m); i++)
	{
		if (items[i] == tmp)
		{
			gtk_list_store_remove (s->publicListStore, &iter);
			gtk_list_store_insert (s->publicListStore, &iter, i-1);
			gtk_list_store_set (s->publicListStore, &iter, 0, tmp.c_str (), -1);
			gtk_tree_selection_select_iter (selection, &iter);
		}
	}
}
void Settings::onPublicMD_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (s->publicListStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->publicList->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;
		
	string tmp = TreeViewFactory::getValue<gchar*,string>(m, &iter, 0);
	vector<string> items;
	TreeViewFactory::getColumn (m, 0, &items);
	for(int i = TreeViewFactory::getCount (m)-2; i >=0; i--)
	{
		if (items[i] == tmp)
		{
			gtk_list_store_remove (s->publicListStore, &iter);
			gtk_list_store_insert (s->publicListStore, &iter, i+1);
			gtk_list_store_set (s->publicListStore, &iter, 0, tmp.c_str (), -1);
			gtk_tree_selection_select_iter (selection, &iter);
		}
	}
}
void Settings::onPublicEdit_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (s->publicListStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->publicList->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	gtk_entry_set_text (GTK_ENTRY (s->downloadItems["Edit public"]), TreeViewFactory::getValue<gchar*> (m, &iter, 0));
	gint response = gtk_dialog_run (GTK_DIALOG (s->editPublic));
	if (response == GTK_RESPONSE_OK)
		gtk_list_store_set (s->publicListStore, &iter, 0, gtk_entry_get_text (GTK_ENTRY (s->downloadItems["Edit public"])), -1);

	gtk_widget_hide (s->editPublic);
}
void Settings::onPublicRemove_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (s->publicListStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->publicList->get ());

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
		gtk_list_store_set (publicListStore, &iter, 0, (*idx).c_str (), -1);
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
	s->dirChooser = gtk_file_chooser_dialog_new (	"Choose directory",
											WulforManager::get ()->getMainWindow ()->getWindow (),
											GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
											GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											GTK_STOCK_OPEN, GTK_RESPONSE_OK,
											NULL);
	if (s->lastdir != "")
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (s->dirChooser), s->lastdir.c_str ());
 	gint response = gtk_dialog_run (GTK_DIALOG (s->dirChooser));
	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (s->dirChooser));
		s->lastdir = path;
		gtk_widget_hide (s->dirChooser);

		gtk_entry_set_text (GTK_ENTRY (s->downloadItems["Name"]), "");
		response = gtk_dialog_run (GTK_DIALOG (s->favoriteName));
		if (response == GTK_RESPONSE_OK)
		{
			string name = gtk_entry_get_text (GTK_ENTRY (s->downloadItems["Name"]));
			if (path[path.length ()-1] != PATH_SEPARATOR)
				path += PATH_SEPARATOR;
			if (s->addFavoriteDir_client (path, name))
			{
				GtkTreeIter iter;
				gtk_list_store_append (s->downloadToStore, &iter);
				gtk_list_store_set (	s->downloadToStore, &iter,
												DOWNLOADTO_NAME, name.c_str (),
												DOWNLOADTO_DIR, path.c_str (),
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
		gtk_widget_hide (s->favoriteName);
	}
	else
		gtk_widget_hide (s->dirChooser);	
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
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->downloadTo->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	if (s->removeFavoriteDir_client (TreeViewFactory::getValue<gchar*,string>(m, &iter, DOWNLOADTO_DIR)))
	{
		gtk_list_store_remove (s->downloadToStore, &iter);
		gtk_widget_set_sensitive (s->downloadItems["Remove"], FALSE);
	}
}
gboolean Settings::onFavoriteButtonPressed_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (s->downloadToStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->downloadTo->get ());

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
	GtkTreeView *view = GTK_TREE_VIEW (shareItems["Shares"]);
	shareStore = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model (view, GTK_TREE_MODEL (shareStore));
	g_signal_connect(G_OBJECT (view), "button_press_event", G_CALLBACK(onShareButtonPressed_gui), (gpointer)this);
	shares = new TreeViewFactory (view);

	shares->addColumn_gui (SHARE_NAME, "Virtual name", TreeViewFactory::STRING, 100);
	shares->addColumn_gui (SHARE_DIR, "Directory", TreeViewFactory::STRING, 200);
	shares->addColumn_gui (SHARE_SIZE, "Size", TreeViewFactory::STRING, 100);
	gtk_widget_set_sensitive (shareItems["Remove"], FALSE);

	StringPairList directories = ShareManager::getInstance()->getDirectories();
	for(StringPairList::iterator it = directories.begin(); it != directories.end(); it++)
	{
		GtkTreeIter iter;
		gtk_list_store_append (shareStore, &iter);
		gtk_list_store_set (shareStore, &iter, 	SHARE_NAME, it->first.c_str (),
										SHARE_DIR, it->second.c_str (),
										SHARE_SIZE, Util::formatBytes(ShareManager::getInstance()->getShareSize(it->second)).c_str (),
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
}
void Settings::onAddShare_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	s->dirChooser = gtk_file_chooser_dialog_new (	"Choose directory",
											WulforManager::get ()->getMainWindow ()->getWindow (),
											GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
											GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											GTK_STOCK_OPEN, GTK_RESPONSE_OK,
											NULL);
	if (s->lastdir != "")
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (s->dirChooser), s->lastdir.c_str ());
 	gint response = gtk_dialog_run (GTK_DIALOG (s->dirChooser));
	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (s->dirChooser));
		s->lastdir = path;
		gtk_widget_hide (s->dirChooser);

		gtk_entry_set_text (GTK_ENTRY (s->shareItems["Virtual"]), "");
		response = gtk_dialog_run (GTK_DIALOG (s->virtualName));
		if (response == GTK_RESPONSE_OK)
		{
			try
			{
				string name = ShareManager::getInstance()->validateVirtual(gtk_entry_get_text (GTK_ENTRY (s->shareItems["Virtual"])));
				if (path[path.length ()-1] != PATH_SEPARATOR)
					path += PATH_SEPARATOR;
			
				s->modifyShare_client (true, path, name);
				GtkTreeIter iter;
				gtk_list_store_append (s->shareStore, &iter);
				gtk_list_store_set (s->shareStore, &iter, 	SHARE_NAME, name.c_str (),
													SHARE_DIR, path.c_str (),
													SHARE_SIZE, Util::formatBytes(ShareManager::getInstance()->getShareSize(path)).c_str (),
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
			gtk_label_set_text (GTK_LABEL (s->shareItems["Size"]), string ("Total size: " +Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str ());
		}
		gtk_widget_hide (s->virtualName);
	}
	else
		gtk_widget_hide (s->dirChooser);	
}
void Settings::onRemoveShare_gui (GtkWidget *widget, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL (s->shareStore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(s->shares->get ());

	if (!gtk_tree_selection_get_selected (selection, &m, &iter))
		return;

	s->modifyShare_client (false, TreeViewFactory::getValue<gchar*,string>(m, &iter, SHARE_DIR), "");
	gtk_list_store_remove (s->shareStore, &iter);
	gtk_widget_set_sensitive (s->shareItems["Remove"], FALSE);
}
gboolean Settings::onShareButtonPressed_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	Settings *s = (Settings*)user_data;
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
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
		s->shareHidden_client (true);
	else
		s->shareHidden_client (false);
		
	gtk_list_store_clear (s->shareStore);
	StringPairList directories = ShareManager::getInstance()->getDirectories();
	for(StringPairList::iterator it = directories.begin(); it != directories.end(); it++)
	{
		GtkTreeIter iter;
		gtk_list_store_append (s->shareStore, &iter);
		gtk_list_store_set (s->shareStore, &iter, 	SHARE_NAME, it->first.c_str (),
										SHARE_DIR, it->second.c_str (),
										SHARE_SIZE, Util::formatBytes(ShareManager::getInstance()->getShareSize(it->second)).c_str (),
										-1);
	}
	gtk_label_set_text (GTK_LABEL (s->shareItems["Size"]), string ("Total size: " +Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str ());
}
