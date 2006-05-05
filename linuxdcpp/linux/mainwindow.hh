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
#include <iostream>
#include <sstream>
#include <iomanip>
 
#include "callback.hh"
#include "func.hh"
#include "selecter.hh"
#include "settingsdialog.hh"
#include "treeview.hh"
#include "wulformanager.hh"

#include <client/Client.h>
#include <client/ConnectionManager.h>
#include <client/DCPlusPlus.h>
#include <client/DownloadManager.h>
#include <client/Exception.h>
#include <client/HubManager.h>
#include <client/LogManager.h>
#include <client/QueueManager.h>
#include <client/SearchManager.h>
#include <client/SettingsManager.h>
#include <client/Socket.h>
#include <client/stdinc.h>
#include <client/TimerManager.h>
#include <client/UploadManager.h>
#include <client/version.h>

using namespace std;

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

		GtkWindow *getWindow();

		//gui functions
		void createWindow_gui();
		void setStatus_gui(GtkStatusbar *status, std::string text);
		void setStats_gui(std::string hub, std::string slot, 
			std::string dTot, std::string uTot, std::string dl, std::string ul);
		void addShareBrowser_gui(User::Ptr user, 
			std::string searchString, std::string listName);
			
		void addPage_gui(GtkWidget *page, GtkWidget *label, bool raise);
		void removePage_gui(GtkWidget *page);
		void raisePage_gui(GtkWidget *page);
		GtkWidget *currentPage_gui();
		void appendWindowItem(GtkWidget *page, std::string title);
		void removeWindowItem(GtkWidget *page);
		void modifyWindowItem(GtkWidget *page, std::string title);

		void autoOpen_gui();
		void openHub_gui(string server, string nick, string desc, string password);
		void popup (GdkEventButton *event, gpointer user_data);

		// Transferview related functions
		User::Ptr getSelectedTransfer_gui ();

		void connectClicked_gui(GtkWidget *widget, gpointer data);
		void pubHubsClicked_gui(GtkWidget *widget, gpointer data);
		void dlQueueClicked_gui(GtkWidget *widget, gpointer data);
		void settingsClicked_gui(GtkWidget *widget, gpointer data);
		void favHubsClicked_gui(GtkWidget *widget, gpointer data);
		void searchClicked_gui(GtkWidget *widget, gpointer data);
		void hashClicked_gui(GtkWidget *widget, gpointer data);
		void aboutClicked_gui(GtkWidget *widget, gpointer data);
		void quitClicked_gui(GtkWidget *widget, gpointer data);
		void finishedDLclicked_gui(GtkWidget *widget, gpointer data);
		void finishedULclicked_gui(GtkWidget *widget, gpointer data);
		void openFList_gui(GtkWidget *widget, gpointer data);
		void refreshFList_gui(GtkWidget *widget, gpointer data);

		gboolean deleteWindow_gui(
			GtkWidget *widget, GdkEvent *event, gpointer data);
		void switchPage_gui (GtkNotebook *notebook, 
			GtkNotebookPage *page, guint page_num, gpointer user_data);

		//client functions
		void autoConnect_client();
		void startSocket_client();
		void transferComplete_client(Transfer *t);

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
		class TransferItem
		{
			public:
			TransferItem(const User::Ptr& user, bool isDownload)
			{
				this->user = user;
				this->isDownload = isDownload;
				nicks = user->getNick();
				hubs = user->getLastHubName();
				failed = FALSE;
			}

			void setFile(std::string file) { this->file = file; update["file"] = TRUE; }
			void setPath(std::string path) { this->path = path; update["path"] = TRUE; }
			void setStatus(std::string status, bool failed = FALSE)
			{
				if (!this->failed)
					this->status = status;
				update["status"] = TRUE;
				this->failed = failed;
			}
			void setTime(std::string time) { this->time = time; update["time"] = TRUE; }
			void setSortOrder(std::string sortOrder) { this->sortOrder = sortOrder; update["sortOrder"] = TRUE; }
			void setSize(int64_t size) { this->size = size; update["size"] = TRUE; }
			void setSpeed(int64_t speed) { this->speed = speed; update["speed"] = TRUE; }
			void setProgress(int progress) { this->progress = progress; update["progress"] = TRUE; }

			map<std::string, bool> update;
			User::Ptr user;
			bool isDownload;
			GtkTreeRowReference *rowRef;
			std::string file;
			std::string path;
			std::string nicks;
			std::string hubs;
			std::string status;
			std::string time;
			std::string sortOrder;
			int64_t size;
			int64_t speed;
			int progress;
			bool failed;
		};

		typedef pair<User::Ptr, bool> UserID;
		std::map<UserID, TransferItem *> transferMap;
		void updateTransfer_gui(TransferItem *item);
		void removeTransfer_gui(UserID id);
		TransferItem* getTransferItem(UserID id);

		Callback2<MainWindow, void, GtkWidget *>
			connectCallback, pubHubsCallback, dlQueueCallback,
			settingsCallback, favHubsCallback, searchCallback,
			hashCallback, quitCallback, finishedDL_Callback,
			finishedUL_Callback, openFListCallback, refreshFListCallback,
			aboutCallback;
		Callback3<MainWindow, gboolean, GtkWidget *, GdkEvent *>
			deleteCallback;
		Callback4<MainWindow, void, GtkNotebook *, GtkNotebookPage *, guint>
			switchPageCallback;
		static gboolean transferClicked_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
		static void onGetFileListClicked_gui (GtkMenuItem *item, gpointer user_data);
		static void onMatchQueueClicked_gui (GtkMenuItem *item, gpointer user_data);
		static void onPrivateMessageClicked_gui (GtkMenuItem *item, gpointer user_data);
		static void onAddFavoriteUserClicked_gui (GtkMenuItem *item, gpointer user_data);
		static void onGrantExtraSlotClicked_gui (GtkMenuItem *item, gpointer user_data);
		static void onRemoveUserFromQueueClicked_gui (GtkMenuItem *item, gpointer user_data);
		static void onForceAttemptClicked_gui (GtkMenuItem *item, gpointer user_data);
		static void onCloseConnectionClicked_gui (GtkMenuItem *item, gpointer user_data);
		static void raisePage(GtkMenuItem *item, gpointer data);

		int64_t lastUpdate, lastUp, lastDown;
		int emptyStatusWidth;
		TreeView transferView;

		GtkWindow *window;
		GtkPaned *transferPane;
		GtkDialog *exitDialog, *connectDialog, *flistDialog, *aboutDialog;
		GtkEntry *connectEntry;
		GtkStatusbar *mainStatus, *hubStatus, *slotStatus, 
			*dTotStatus, *uTotStatus, *dlStatus, *ulStatus;
		GtkToolButton *connectButton, *pubHubsButton, *searchButton, *settingsButton, 
			*hashButton, *queueButton, *favHubsButton, *quitButton, *finishedDL_button,
			*finishedUL_button;
		GtkNotebook *book;
		GtkWidget *popupMenu;
		GtkWidget *filelist, *matchQueue, *privateMessage, *addToFavorites, *grantExtraSlot, *removeUser, *forceAttempt, *closeConnection;
		GtkListStore *transferStore;
		GtkTreeSelection *transferSel;
		GdkPixbuf *uploadPic, *downloadPic;
		GtkMenuItem *openFList, *openOwnFList, *refreshFList, *openDLdir, *quickConnect, *followRedirect, *reconnectItem,
				*settingsItem, *quitItem, *pubHubsItem, *queueItem, *finishedDL_item, *finishedUL_item, *favHubsItem,
				*favUsersItem, *searchItem, *ADLSearchItem, *searchSpyItem, *networkStatsItem, *hashItem, *aboutItem;
		map<GtkWidget *, GtkWidget *> windowMenuItems;
		GtkWidget *windowMenu;

		// Convenience thing for the updateTransfer_gui function.
		typedef Func1 <MainWindow, TransferItem *> UFunc;
};

#else
class MainWindow;
#endif
