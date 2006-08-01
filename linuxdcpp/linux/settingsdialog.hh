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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef WULFOR_SETTINGS_DIALOG_HH
#define WULFOR_SETTINGS_DIALOG_HH

#include <gtk/gtk.h>
#include <glade/glade.h>
#include "treeview.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/SettingsManager.h>
#include <client/FavoriteManager.h>
#include <client/ShareManager.h>

class Settings
{
public:
	Settings();
	~Settings();

	GtkWidget *getDialog() { return dialog; }
	std::string getID() { return "Settings"; }
	void saveSettings_client();
	
private:
	GtkWidget *dialog;
	GtkWidget *dirChooser;
	GtkWidget *favoriteName;
	GtkWidget *publicHubs;
	GtkWidget *editPublic;
	GtkWidget *virtualName;

	void addOption(GtkListStore *store, TreeView view, string name, SettingsManager::IntSetting setting);
	void onToggledClicked_gui(TreeView view, GtkCellRendererToggle *cell, gchar *path, gpointer data);

	// General
	void initGeneral_gui();

	// Connection
	void initConnection_gui();
	static void onInDirect_gui(GtkToggleButton *button, gpointer data);
	static void onOutDirect_gui(GtkToggleButton *button, gpointer data);
	static void onInPassive_gui(GtkToggleButton *button, gpointer data);
	static void onInFW_NAT_gui(GtkToggleButton *button, gpointer data);
	///@todo Uncomment when implemented
	//static void onInFW_UPnP_gui(GtkToggleButton *button, gpointer data);
	static void onSocks5_gui(GtkToggleButton *button, gpointer data);
	void inDirectClicked_gui();
	void inFW_NATClicked_gui();
	///@todo Uncomment when implemented
	//void inFW_UPnPClicked_gui();
	void outDirectClicked_gui();
	void inPassiveClicked_gui();
	void socks5Clicked_gui();

	// Downloads
	void initDownloads_gui();
	static void onBrowseF_gui(GtkWidget *widget, gpointer data);
	static void onBrowseUF_gui(GtkWidget *widget, gpointer data);

	// Public hubs dialog
	static void onPublicHubs_gui(GtkWidget *widget, gpointer data);
	static void onPublicAdd_gui(GtkWidget *widget, gpointer data);
	static void onPublicMU_gui(GtkWidget *widget, gpointer data);
	static void onPublicMD_gui(GtkWidget *widget, gpointer data);
	static void onPublicEdit_gui(GtkWidget *widget, gpointer data);
	static void onPublicRemove_gui(GtkWidget *widget, gpointer data);

	// downloadsTo
	TreeView downloadToView, publicListView;
	GtkListStore *downloadToStore, *publicListStore;
	void publicInit_gui();
	bool addFavoriteDir_client(string path, string name);
	bool removeFavoriteDir_client(string path);
	static void onAddFavorite_gui(GtkWidget *widget, gpointer data);
	static void onRemoveFavorite_gui(GtkWidget *widget, gpointer data);
	static gboolean onFavoriteButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);

	// Queue
	TreeView queueView;
	GtkListStore *queueStore;
	static void onQueueToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);

	// Sharing
	TreeView shareView;
	GtkListStore *shareStore;
	GdkEventType sharePrevious;
	void initSharing_gui();
	static void onAddShare_gui(GtkWidget *widget, gpointer data);
	static void onRemoveShare_gui(GtkWidget *widget, gpointer data);
	static gboolean onShareButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
	static gboolean onShareHiddenPressed_gui(GtkToggleButton *togglebutton, gpointer data);
	void shareHidden_client(bool show);
	void modifyShare_client(bool add, string path, string name);
	void modifyShare_gui(std::string path, std::string name, std::string error);

	// Appearance
	TreeView appearanceView;
	GtkListStore *appearanceStore;
	void initAppearance_gui();
	static void onAppearanceToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);

	// Colors and Sound
	TreeView colorView;
	GtkListStore *colorStore;
	static void onColorToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
	///@todo Uncomment when implemented
	//static void onWinColorClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
	//static void onDownColorClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
	//static void onUpColorClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
	//static void onTextStyleClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);

	// Windows
	TreeView windowView1, windowView2, windowView3;
	GtkListStore *windowStore1, *windowStore2, *windowStore3;
	static void onWindowView1ToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
	static void onWindowView2ToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
	static void onWindowView3ToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);

	// Logs and sound
	void initLog_gui();
	static void onLogBrowseClicked_gui(GtkWidget *widget, gpointer data);
	static void onLogMainClicked_gui(GtkToggleButton *togglebutton, gpointer data);
	static void onLogPrivateClicked_gui(GtkToggleButton *togglebutton, gpointer data);
	static void onLogDownloadClicked_gui(GtkToggleButton *togglebutton, gpointer data);
	static void onLogUploadClicked_gui(GtkToggleButton *togglebutton, gpointer data);
	void checkClicked();

	// Advanced
	TreeView advancedView;
	GtkListStore *advancedStore;
	void initAdvanced_gui();
	static void onAdvancedToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
	void addAdvanced(string name, bool use);

	hash_map<string, GtkWidget *> generalItems;
	hash_map<string, GtkWidget *> connectionItems;
	hash_map<string, GtkWidget *> queueItems;
	hash_map<string, GtkWidget *> downloadItems;
	hash_map<string, GtkWidget *> shareItems;
	hash_map<string, GtkWidget *> appearanceItems;
	hash_map<string, GtkWidget *> logItems;
	hash_map<string, GtkWidget *> advancedItems;

	string lastdir;
	
	pthread_mutex_t settingsLock;
	
};

#else
class Settings;
#endif
