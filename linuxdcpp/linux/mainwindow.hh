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
#include <ext/hash_map>

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
		std::string getID() { return "Main Window"; }

		// GUI functions
		void addPage_gui(GtkWidget *page, GtkWidget *label, bool raise = TRUE);
		void removePage_gui(GtkWidget *page);
		void raisePage_gui(GtkWidget *page);
		GtkWidget *currentPage_gui();
		void appendWindowItem(GtkWidget *page, std::string title);
		void removeWindowItem(GtkWidget *page);
		void modifyWindowItem(GtkWidget *page, std::string title);
		void autoOpen_gui();

		// Client functions
		void autoConnect_client();

	private:
		// GUI functions
		void createWindow_gui();
		void createTrayIcon_gui();
		void updateTrayToolTip_gui(std::string);
		void setStatus_gui(GtkStatusbar *status, std::string text);
		void setStats_gui(std::string hub, std::string slot, 
			std::string dTot, std::string uTot, std::string dl, std::string ul);
		void addShareBrowser_gui(User::Ptr user, std::string searchString, std::string listName);
		void openHub_gui(string server, string nick, string desc, string password);
		User::Ptr getSelectedTransfer_gui();
		void popup_gui(GtkWidget *menu, GdkEventButton *event);

		// Client functions
		void startSocket_client();
		void transferComplete_client(Transfer *t);
		void refreshFileList_client();
		void openOwnList_client();

		// GUI Callbacks
		static gboolean transferClicked_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onGetFileListClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMatchQueueClicked_gui(GtkMenuItem *item, gpointer data);
		static void onPrivateMessageClicked_gui(GtkMenuItem *item, gpointer data);
		static void onAddFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data);
		static void onGrantExtraSlotClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveUserFromQueueClicked_gui(GtkMenuItem *item, gpointer data);
		static void onForceAttemptClicked_gui(GtkMenuItem *item, gpointer data);
		static void onCloseConnectionClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRaisePage_gui(GtkMenuItem *item, gpointer data);
		static void connectClicked_gui(GtkWidget *widget, gpointer data);
		static void pubHubsClicked_gui(GtkWidget *widget, gpointer data);
		static void dlQueueClicked_gui(GtkWidget *widget, gpointer data);
		static void settingsClicked_gui(GtkWidget *widget, gpointer data);
		static void favHubsClicked_gui(GtkWidget *widget, gpointer data);
		static void searchClicked_gui(GtkWidget *widget, gpointer data);
		static void hashClicked_gui(GtkWidget *widget, gpointer data);
		static void quitClicked_gui(GtkWidget *widget, gpointer data);
		static void aboutClicked_gui(GtkWidget *widget, gpointer data);
		static void finishedDLclicked_gui(GtkWidget *widget, gpointer data);
		static void finishedULclicked_gui(GtkWidget *widget, gpointer data);
		static void openFileList_gui(GtkWidget *widget, gpointer data);
		static void openOwnList_gui(GtkWidget *widget, gpointer data);
		static void refreshFileList_gui(GtkWidget *widget, gpointer data);
		static gboolean deleteWindow_gui(GtkWidget *widget, GdkEvent *event, gpointer data);
		static void switchPage_gui(GtkNotebook *notebook, GtkNotebookPage *page, guint num, gpointer data);
		static void onTrayIconClicked_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onToggleWindowVisibility_gui(GtkMenuItem *item, gpointer data);

		// From Timer manager
		virtual void on(TimerManagerListener::Second, u_int32_t ticks) throw();

		// From Queue Manager
		virtual void on(QueueManagerListener::Finished, QueueItem *item) throw();

		// From Connection manager
		virtual void on(ConnectionManagerListener::Added, ConnectionQueueItem *item) throw();
		virtual void on(ConnectionManagerListener::Removed, ConnectionQueueItem *item) throw();
		virtual void on(ConnectionManagerListener::Failed, ConnectionQueueItem *item, const string &reason) throw();
		virtual void on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem *item) throw();

		// From Download manager
		virtual void on(DownloadManagerListener::Starting, Download *dl) throw();
		virtual void on(DownloadManagerListener::Tick, const Download::List &list) throw();
		virtual void on(DownloadManagerListener::Complete, Download *dl) throw();
		virtual void on(DownloadManagerListener::Failed, Download *dl, const string &reason) throw();

		// From Upload manager
		virtual void on(UploadManagerListener::Starting, Upload *ul) throw();
		virtual void on(UploadManagerListener::Tick, const Upload::List &list) throw();
		virtual void on(UploadManagerListener::Complete, Upload *ul) throw();

		// From Log manager
		virtual void on(LogManagerListener::Message, const string& str) throw();

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

			typedef enum
			{
				MASK_FILE = 1 << 0,
				MASK_PATH = 1 << 1,
				MASK_STATUS = 1 << 2,
				MASK_TIME = 1 << 3,
				MASK_SORT_ORDER = 1 << 4,
				MASK_SIZE = 1 << 5,
				MASK_SPEED = 1 << 6,
				MASK_PROGRESS = 1 << 7,
			} Mask;

			bool isSet(Mask m) { return updateMask & m; }
			void setFile(std::string file) { this->file = file; updateMask |= MASK_FILE; }
			void setPath(std::string path) { this->path = path; updateMask |= MASK_PATH; }
			void setStatus(std::string status, bool failed = FALSE)
			{
				if (!this->failed)
					this->status = status;
				this->failed = failed;
				updateMask |= MASK_STATUS;
			}
			void setTime(std::string time) { this->time = time; updateMask |= MASK_TIME; }
			void setSortOrder(std::string sortOrder) { this->sortOrder = sortOrder; updateMask |= MASK_SORT_ORDER; }
			void setSize(int64_t size) { this->size = size; updateMask |= MASK_SIZE; }
			void setSpeed(int64_t speed) { this->speed = speed; updateMask |= MASK_SPEED; }
			void setProgress(int progress) { this->progress = progress; updateMask |= MASK_PROGRESS; }

			u_int32_t updateMask;
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
		struct HashPair
		{
			size_t operator() (const UserID u) const
			{
				return u.first * 23 + u.second;
			}
		};
		hash_map<UserID, TransferItem *, HashPair> transferMap;
		TransferItem* getTransferItem(UserID id);

		// More GUI funcs (can't be declared above because of TransferItem class declaration.
		void insertTransfer_gui(TransferItem *item);
		void updateTransfer_gui(TransferItem *item);
		void removeTransfer_gui(TransferItem *item);

		int64_t lastUpdate, lastUp, lastDown;
		int emptyStatusWidth;
		TreeView transferView;
		GtkWindow *window;
		GtkPaned *transferPane;
		GtkDialog *exitDialog, *connectDialog, *flistDialog, *aboutDialog;
		GtkEntry *connectEntry;
		GtkStatusbar *mainStatus, *hubStatus, *slotStatus, 
			*dTotStatus, *uTotStatus, *dlStatus, *ulStatus;
		GtkNotebook *book;
		GtkWidget *popupMenu;
		GtkWidget *filelist, *matchQueue, *privateMessage, *addToFavorites,
			*grantExtraSlot, *removeUser, *forceAttempt, *closeConnection;
		GtkListStore *transferStore;
		GtkTreeSelection *transferSel;
		GdkPixbuf *uploadPic, *downloadPic;
		map<GtkWidget *, GtkWidget *> windowMenuItems;
		GtkWidget *windowMenu;
		GtkWidget *trayMenu;
		GtkTooltips *trayToolTip;
		GtkWidget *trayIcon;

		// Convenience thing for the updateTransfer_gui function.
		typedef Func1 <MainWindow, TransferItem *> UFunc;
};

#else
class MainWindow;
#endif
