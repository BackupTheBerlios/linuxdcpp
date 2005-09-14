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

#ifndef WULFOR_MAIN_WINDOW_HH
#define WULFOR_MAIN_WINDOW_HH

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <string>

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/QueueManager.h>
#include <client/TimerManager.h>
#include <client/HubManager.h>
#include <client/DownloadManager.h>
#include <client/UploadManager.h>
#include <client/ConnectionManager.h>
#include <client/LogManager.h>

class MainWindow:
	public QueueManagerListener,
	public TimerManagerListener,
	public DownloadManagerListener,
	public UploadManagerListener,
	public ConnectionManagerListener,
	public LogManagerListener
{
	public:
		MainWindow();
		~MainWindow();

		typedef enum {
			CONNECTION_UL,
			CONNECTION_DL,
			CONNECTION_NA
		} connection_t;

		User::Ptr getSelectedTransfer();
		GtkWindow *getWindow();
		void createWindow();
		
		static gboolean onTransferClicked (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
		static void onGetFileListClicked(GtkMenuItem *item, gpointer user_data);
		static void onMatchQueueClicked(GtkMenuItem *item, gpointer user_data);
		static void onPrivateMessageClicked(GtkMenuItem *item, gpointer user_data);
		static void onAddFavoriteUserClicked(GtkMenuItem *item, gpointer user_data);
		static void onGrantExtraSlotClicked(GtkMenuItem *item, gpointer user_data);
		static void onRemoveUserFromQueueClicked(GtkMenuItem *item, gpointer user_data);
		static void onForceAttemptClicked(GtkMenuItem *item, gpointer user_data);
		static void onCloseConnectionClicked(GtkMenuItem *item, gpointer user_data);

		static void onConnectClicked(GtkWidget *widget, gpointer data);
		static void onPubHubsClicked(GtkWidget *widget, gpointer data);
		static void onSearchClicked(GtkWidget *widget, gpointer data);
		static void onHashClicked(GtkWidget *widget, gpointer data);
		static void onDlQueueClicked(GtkWidget *widget, gpointer data);
		static void onFavHubsClicked(GtkWidget *widget, gpointer data);
		static void onFinishedDLClicked(GtkWidget *widget, gpointer data);
		static void onFinishedULClicked(GtkWidget *widget, gpointer data);
		static void onSettingsClicked(GtkWidget *widget, gpointer data);
		static void onRefreshListClicked(GtkWidget *widget, gpointer data);
		static void onOpenFileListClicked(GtkWidget *widget, gpointer data);
		static void onQuitClicked(GtkWidget *widget, gpointer data);

		static gboolean deleteWindow(GtkWidget *widget, GdkEvent *event, gpointer data);
		
		void autoConnect();
		void autoOpen();
		void startSocket();
		
		void addPage(GtkWidget *page, GtkWidget *label, bool raise);
		void removePage(GtkWidget *page);
		void raisePage(GtkWidget *page);
		GtkWidget *getCurrentPage();
		void setStatus(GtkStatusbar *status, std::string text);

		void updateTransfer(std::string id, connection_t type, ConnectionQueueItem *item, 
			std::string status, std::string time, std::string speed, std::string file, std::string size, std::string path);
		void removeTransfer(std::string id);
		void findTransferId(std::string id, GtkTreeIter *iter);
		std::string getTransferId(ConnectionQueueItem *item);
		std::string getTransferId(Transfer *t);
		void transferComplete(Transfer *t);

		//From Timer manager
		virtual void on(TimerManagerListener::Second, u_int32_t ticks) throw();

		//From Queue Manager
		virtual void on(QueueManagerListener::Finished, QueueItem *item) throw();

		//From Connection manager
		virtual void on(ConnectionManagerListener::Added, ConnectionQueueItem *item) throw();
		virtual void on(ConnectionManagerListener::Removed, ConnectionQueueItem *item) throw();
		virtual void on(ConnectionManagerListener::Failed, ConnectionQueueItem *item, const string &reason) throw();
		virtual void on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem *item) throw();

		//From Download manager
		virtual void on(DownloadManagerListener::Starting, Download *dl) throw();
		virtual void on(DownloadManagerListener::Tick, const Download::List &list) throw();
		virtual void on(DownloadManagerListener::Complete, Download *dl) throw();
		virtual void on(DownloadManagerListener::Failed, Download *dl, const string &reason) throw();

		//From Upload manager
		virtual void on(UploadManagerListener::Starting, Upload *ul) throw();
		virtual void on(UploadManagerListener::Tick, const Upload::List &list) throw();
		virtual void on(UploadManagerListener::Complete, Upload *ul) throw();

		//From Log manager
		virtual void on(LogManagerListener::Message, const string& str) throw();

	private:
		int64_t lastUpdate, lastUp, lastDown;
		int emptyStatusWidth;

		GtkWindow *window;
		GtkDialog *exitDialog, *connectDialog;
		GtkEntry *connectEntry;
		GtkStatusbar *mainStatus, *hubStatus, *slotStatus, 
			*dTotStatus, *uTotStatus, *dlStatus, *ulStatus;
		GtkToolButton *connectButton, *pubHubsButton, *searchButton, *settingsButton, 
			*hashButton, *queueButton, *favHubsButton, *quitButton, *finishedDL_button,
			*finishedUL_button;
		GtkNotebook *book;
		GtkWidget *popupMenu;
		GtkWidget *filelist, *matchQueue, *privateMessage, *addToFavorites, *grantExtraSlot, *removeUser, *forceAttempt, *closeConnection;
		GtkTreeView *transferView;
		GtkListStore *transferStore;
		GtkTreeSelection *transferSel;
		GdkPixbuf *uploadPic, *downloadPic;
		GtkMenuItem *openFList, *openOwnFList, *refreshFList, *openDLdir, *quickConnect, *followRedirect, *reconnectItem,
				*settingsItem, *quitItem, *pubHubsItem, *queueItem, *finishedDL_item, *finishedUL_item, *favHubsItem,
				*favUsersItem, *searchItem, *ADLSearchItem, *searchSpyItem, *networkStatsItem, *hashItem;

		const int WIDTH_TYPE, WIDTH_USER, WIDTH_STATUS, WIDTH_TIMELEFT,
			WIDTH_SPEED, WIDTH_FILENAME, WIDTH_SIZE, WIDTH_PATH;
		
		enum {
			COLUMN_TYPE,
			COLUMN_USER,
			COLUMN_STATUS,
			COLUMN_TIMELEFT,
			COLUMN_SPEED,
			COLUMN_FILENAME,
			COLUMN_SIZE,
			COLUMN_PATH,
			COLUMN_ID,		//not shown in gui
			COLUMN_USERPTR	//likewise ^^
		};
};

#else
class MainWindow;
#endif
