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

#ifndef WULFOR_SETTINGS_DIALOG_HH
#define WULFOR_SETTINGS_DIALOG_HH

#include <gtk/gtk.h>
#include <glade/glade.h>
#include "treeviewfactory.hh"
//#include "favoritedirectory.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/SettingsManager.h>
#include <client/HubManager.h>
#include <client/ShareManager.h>

class Settings
{
public:
	Settings ();
	~Settings ();

	GtkWidget *getDialog () { return dialog; }
	void saveSettings_client ();
	void initGeneral_gui ();
	void initDownloads_gui ();
	void initSharing_gui ();
	
private:
	GtkWidget *dialog;
	GtkWidget *dirChooser;
	GtkWidget *favoriteName;
	GtkWidget *publicHubs;
	GtkWidget *editPublic;
	GtkWidget *virtualName;

	// General
	static void onActive_gui (GtkToggleButton *button, gpointer user_data);
	static void onPassive_gui (GtkToggleButton *button, gpointer user_data);
	static void onSocks5_gui (GtkToggleButton *button, gpointer user_data);
	void activeClicked_gui ();
	void passiveClicked_gui ();
	void socks5Clicked_gui ();
	// Downloads
	static void onBrowseF_gui (GtkWidget *widget, gpointer user_data);
	static void onBrowseUF_gui (GtkWidget *widget, gpointer user_data);
	void setPublicHubs_client (string list);
	static void onPublicHubs_gui (GtkWidget *widget, gpointer user_data);
	static void onPublicAdd_gui (GtkWidget *widget, gpointer user_data);
	static void onPublicMU_gui (GtkWidget *widget, gpointer user_data);
	static void onPublicMD_gui (GtkWidget *widget, gpointer user_data);
	static void onPublicEdit_gui (GtkWidget *widget, gpointer user_data);
	static void onPublicRemove_gui (GtkWidget *widget, gpointer user_data);
	void publicInit_gui ();
	bool addFavoriteDir_client (string path, string name);
	bool removeFavoriteDir_client (string path);
	static void onAddFavorite_gui (GtkWidget *widget, gpointer user_data);
	static void onRemoveFavorite_gui (GtkWidget *widget, gpointer user_data);	
	static gboolean onFavoriteButtonReleased_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
	enum {
		DOWNLOADTO_FIRST,
		DOWNLOADTO_NAME = DOWNLOADTO_FIRST,
		DOWNLOADTO_DIR,
		DOWNLOADTO_LAST
	};
	TreeViewFactory *downloadTo;
	TreeViewFactory *publicList;
	GtkListStore *downloadToStore;
	GtkListStore *publicListStore;
	// Sharing
	static void onAddShare_gui (GtkWidget *widget, gpointer user_data);
	static void onRemoveShare_gui (GtkWidget *widget, gpointer user_data);
	static gboolean onShareButtonReleased_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
	static gboolean onShareHiddenPressed_gui (GtkToggleButton *togglebutton, gpointer user_data);
	void shareHidden_client (bool show);
	void modifyShare_client (bool add, string path, string name);
	enum {
		SHARE_FIRST,
		SHARE_NAME = SHARE_FIRST,
		SHARE_DIR,
		SHARE_SIZE,
		SHARE_LAST
	};
	TreeViewFactory *shares;
	GtkListStore *shareStore;
	GdkEventType sharePrevious;

	std::map<string,GtkWidget*> generalItems;
	std::map<string,GtkWidget*> downloadItems;
	std::map<string,GtkWidget*> shareItems;

	string lastdir;
	
	pthread_mutex_t settingsLock;
	
};

#else
class Settings;
#endif
