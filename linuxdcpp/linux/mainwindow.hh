/*
 * Copyright © 2004-2008 Jens Oknelid, paskharen@gmail.com
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

#ifndef WULFOR_MAIN_WINDOW_HH
#define WULFOR_MAIN_WINDOW_HH

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/ConnectionManager.h>
#include <client/DownloadManager.h>
#include <client/LogManager.h>
#include <client/QueueManager.h>
#include <client/TimerManager.h>
#include <client/UploadManager.h>

#include "entry.hh"
#include "treeview.hh"

class MainWindow:
	public Entry,
	public ConnectionManagerListener,
	public DownloadManagerListener,
	public LogManagerListener,
	public QueueManagerListener,
	public TimerManagerListener,
	public UploadManagerListener
{
	public:
		MainWindow();
		~MainWindow();

		// Inherited from Entry
		GtkWidget *getContainer();
		void applyCallback(GCallback closeCallback);

		// GUI functions
		void autoOpen_gui();
		void addPage_gui(GtkWidget *page, GtkWidget *label, bool raise = TRUE);
		GtkWidget *currentPage_gui();
		void raisePage_gui(GtkWidget *page);
		void removePage_gui(GtkWidget *page);
		GtkWidget *appendWindowItem(GtkWidget *page, const std::string &title);
		void modifyWindowItem(GtkWidget *menuItem, std::string title);
		void removeWindowItem(GtkWidget *menuItem);
		bool getUserCommandLines_gui(const std::string &command, StringMap &ucParams);
		void openMagnetDialog_gui(const std::string &magnet);

		// Client functions
		void autoConnect_client();

	private:
		// GUI functions
		void createTrayIcon_gui();
		void updateTrayToolTip_gui(std::string download, std::string upload);
		void setStatus_gui(std::string statusBar, std::string text);
		void setStats_gui(std::string hub, std::string slot,
			std::string dTot, std::string uTot, std::string dl, std::string ul);
		void addShareBrowser_gui(User::Ptr user, std::string filename, std::string dir, bool useSetting);
		void openHub_gui(std::string server, std::string encoding);
		bool findTransfer_gui(const std::string &cid, bool download, GtkTreeIter *iter);
		void updateTransfer_gui(StringMap params, bool download);
		void removeTransfer_gui(std::string cid, bool download);
		void setTabPosition_gui(int position);
		void setToolbarStyle_gui(int style);

		// GUI Callbacks
		static gboolean onDeleteWindow_gui(GtkWidget *widget, GdkEvent *event, gpointer data);
		static gboolean onKeyPressed_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onButtonPressPage_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onTransferButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onTransferButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onRaisePage_gui(GtkMenuItem *item, gpointer data);
		static void onPageSwitched_gui(GtkNotebook *notebook, GtkNotebookPage *page, guint num, gpointer data);
		static void onPaneRealized_gui(GtkWidget *pane, gpointer data);
		static void onConnectClicked_gui(GtkWidget *widget, gpointer data);
		static void onFavoriteHubsClicked_gui(GtkWidget *widget, gpointer data);
		static void onPublicHubsClicked_gui(GtkWidget *widget, gpointer data);
		static void onPreferencesClicked_gui(GtkWidget *widget, gpointer data);
		static void onHashClicked_gui(GtkWidget *widget, gpointer data);
		static void onSearchClicked_gui(GtkWidget *widget, gpointer data);
		static void onDownloadQueueClicked_gui(GtkWidget *widget, gpointer data);
		static void onFinishedDownloadsClicked_gui(GtkWidget *widget, gpointer data);
		static void onFinishedUploadsClicked_gui(GtkWidget *widget, gpointer data);
		static void onQuitClicked_gui(GtkWidget *widget, gpointer data);
		static void onOpenFileListClicked_gui(GtkWidget *widget, gpointer data);
		static void onOpenOwnListClicked_gui(GtkWidget *widget, gpointer data);
		static void onRefreshFileListClicked_gui(GtkWidget *widget, gpointer data);
		static void onReconnectClicked_gui(GtkWidget *widget, gpointer data);
		static void onCloseClicked_gui(GtkWidget *widget, gpointer data);
		static void onAboutClicked_gui(GtkWidget *widget, gpointer data);
		static void onAboutDialogActivateLink_gui(GtkAboutDialog *dialog, const gchar *link, gpointer data);
		static void onGetFileListClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMatchQueueClicked_gui(GtkMenuItem *item, gpointer data);
		static void onPrivateMessageClicked_gui(GtkMenuItem *item, gpointer data);
		static void onAddFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data);
		static void onGrantExtraSlotClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveUserFromQueueClicked_gui(GtkMenuItem *item, gpointer data);
		static void onForceAttemptClicked_gui(GtkMenuItem *item, gpointer data);
		static void onCloseConnectionClicked_gui(GtkMenuItem *item, gpointer data);
		static void onTrayIconClicked_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onToggleWindowVisibility_gui(GtkMenuItem *item, gpointer data);

		// Client functions
		void startSocket_client();
		void openOwnList_client();
		void refreshFileList_client();
		void getFileList_client(std::string cid);
		void matchQueue_client(std::string cid);
		void addFavoriteUser_client(std::string cid);
		void grantExtraSlot_client(std::string cid);
		void removeUserFromQueue_client(std::string cid);
		void forceAttempt_client(std::string cid);
		void closeConnection_client(std::string cid, bool download);
		void transferComplete_client(Transfer *t);

		// Client callbacks
		virtual void on(ConnectionManagerListener::Added, ConnectionQueueItem *item) throw();
		virtual void on(ConnectionManagerListener::Removed, ConnectionQueueItem *item) throw();
		virtual void on(ConnectionManagerListener::Failed, ConnectionQueueItem *item, const string &reason) throw();
		virtual void on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem *item) throw();
		virtual void on(DownloadManagerListener::Starting, Download *dl) throw();
		virtual void on(DownloadManagerListener::Tick, const Download::List &list) throw();
		virtual void on(DownloadManagerListener::Complete, Download *dl) throw();
		virtual void on(DownloadManagerListener::Failed, Download *dl, const string &reason) throw();
		virtual void on(UploadManagerListener::Starting, Upload *ul) throw();
		virtual void on(UploadManagerListener::Tick, const Upload::List &list) throw();
		virtual void on(UploadManagerListener::Complete, Upload *ul) throw();
		virtual void on(LogManagerListener::Message, time_t t, const std::string &m) throw();
		virtual void on(QueueManagerListener::Finished, QueueItem *item, const string& dir, int64_t avSpeed) throw();
		virtual void on(TimerManagerListener::Second, uint32_t ticks) throw();

		GtkWindow *window;
		TreeView transferView;
		GtkListStore *transferStore;
		GtkTreeSelection *transferSelection;
		GdkPixbuf *uploadPic, *downloadPic;
		GtkTooltips *trayToolTip;
		GtkWidget *trayIcon;
		int64_t lastUpdate, lastUp, lastDown;
		int emptyStatusWidth;
};

#else
class MainWindow;
#endif
