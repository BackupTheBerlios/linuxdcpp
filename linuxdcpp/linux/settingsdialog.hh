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

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/SettingsManager.h>

#include "dialogentry.hh"
#include "treeview.hh"

class Settings:
	public DialogEntry
{
	public:
		Settings();
		~Settings();

		void saveSettings();

	private:
		// GUI functions
		void addOption_gui(GtkListStore *store, const std::string &name, SettingsManager::IntSetting setting);
		void initPersonal_gui();
		void initConnection_gui();
		void initDownloads_gui();
		void initSharing_gui();
		void initAppearance_gui();
		void initLog_gui();
		void initAdvanced_gui();
		void addShare_gui(std::string path, std::string name, std::string error);

		// GUI callbacks
		static void onInDirect_gui(GtkToggleButton *button, gpointer data);
		///@todo Uncomment when implemented
		//static void onInFW_UPnP_gui(GtkToggleButton *button, gpointer data);
		static void onInPassive_gui(GtkToggleButton *button, gpointer data);
		static void onInFW_NAT_gui(GtkToggleButton *button, gpointer data);
		static void onOutDirect_gui(GtkToggleButton *button, gpointer data);
		static void onSocks5_gui(GtkToggleButton *button, gpointer data);
		static void onBrowseFinished_gui(GtkWidget *widget, gpointer data);
		static void onBrowseUnfinished_gui(GtkWidget *widget, gpointer data);
		static void onPublicHubs_gui(GtkWidget *widget, gpointer data);
		static void onPublicAdd_gui(GtkWidget *widget, gpointer data);
		static void onPublicMoveUp_gui(GtkWidget *widget, gpointer data);
		static void onPublicMoveDown_gui(GtkWidget *widget, gpointer data);
		static void onPublicEdit_gui(GtkCellRendererText *cell, char *path, char *text, gpointer data);
		static void onPublicRemove_gui(GtkWidget *widget, gpointer data);
		static void onAddFavorite_gui(GtkWidget *widget, gpointer data);
		static void onRemoveFavorite_gui(GtkWidget *widget, gpointer data);
		static gboolean onFavoriteButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onQueueToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onAddShare_gui(GtkWidget *widget, gpointer data);
		static void onRemoveShare_gui(GtkWidget *widget, gpointer data);
		static gboolean onShareButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onShareHiddenPressed_gui(GtkToggleButton *button, gpointer data);
		static void onAppearanceToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onColorToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		///@todo Uncomment when implemented
		//static void onWinColorClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		//static void onDownColorClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		//static void onUpColorClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		//static void onTextStyleClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onWindowView1ToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onWindowView2ToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onWindowView3ToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onLogBrowseClicked_gui(GtkWidget *widget, gpointer data);
		static void onLogMainClicked_gui(GtkToggleButton *button, gpointer data);
		static void onLogPrivateClicked_gui(GtkToggleButton *button, gpointer data);
		static void onLogDownloadClicked_gui(GtkToggleButton *button, gpointer data);
		static void onLogUploadClicked_gui(GtkToggleButton *button, gpointer data);
		static void onAdvancedToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onCertificatesPrivateBrowseClicked_gui(GtkWidget *widget, gpointer data);
		static void onCertificatesFileBrowseClicked_gui(GtkWidget *widget, gpointer data);
		static void onCertificatesPathBrowseClicked_gui(GtkWidget *widget, gpointer data);
		static void onCertificatesToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onGenerateCertificatesClicked_gui(GtkWidget *widget, gpointer data);

		// Client functions
		void shareHidden_client(bool show);
		void addShare_client(std::string path, std::string name);
		void generateCertificates_client();

		GtkComboBox *connectionSpeedComboBox;
		GtkListStore *charsetStore, *downloadToStore, *publicListStore, *queueStore,
			*shareStore, *appearanceStore, *colorStore, *windowStore1,
			*windowStore2, *windowStore3, *advancedStore, *certificatesStore;
		TreeView downloadToView, publicListView, queueView, shareView,
			appearanceView, colorView, windowView1, windowView2,
			windowView3, advancedView, certificatesView;
};

#else
class Settings;
#endif
