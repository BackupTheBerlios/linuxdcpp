/* 
* Copyright (C) 2004 Jens Oknelid, paskharen@spray.se
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
#include <client/DownloadManager.h>
#include <client/UploadManager.h>
#include <client/ConnectionManager.h>

#include "func.hh"

class MainWindow:
	public QueueManagerListener,
	public TimerManagerListener,
	public DownloadManagerListener,
	public UploadManagerListener,
	public ConnectionManagerListener
{
	public:
		MainWindow();
		~MainWindow();

		GtkWindow *getWindow();

		//gui functions
		void createWindow_gui();
		void setStatus_gui(std::string status);
		void setStats_gui(std::string hub, std::string slot, 
			std::string dTot, std::string uTot, std::string dl, std::string ul);
		void addShareBrowser_gui(User::Ptr user, 
			std::string searchString, std::string listName);
			
		void addPage_gui(GtkWidget *page, GtkWidget *label, bool raise);
		void removePage_gui(GtkWidget *page);
		void raisePage_gui(GtkWidget *page);

		typedef enum {
			CONNECTION_UL,
			CONNECTION_DL,
			CONNECTION_NA
		} connection_t;

		void updateTransfer_gui(std::string id, connection_t type, std::string user, std::string status, 
			std::string time, std::string speed, std::string file, std::string size, std::string path);
		void removeTransfer_gui(std::string id);
		void findId_gui(std::string id, GtkTreeIter *iter);

		//client functions
		void startSocket_client();
		std::string getId_client(ConnectionQueueItem *item);
		std::string getId_client(Transfer *t);
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

	private:
		//callbacks (to be called from the gui)
		static void pubHubs_callback(GtkWidget *widget, gpointer data);
		static void quit_callback(GtkWidget *widget, gpointer data);

		int64_t lastUpdate, lastUp, lastDown;

		GtkWindow *window;
		GtkStatusbar *mainStatus, *hubStatus, *slotStatus, 
			*dTotStatus, *uTotStatus, *dlStatus, *ulStatus;
		GtkToolButton *pubHubsButton, *searchButton, *settingsButton, 
			*hashButton, *queueButton, *favHubsButton, *quitButton;
		GtkNotebook *book;
		GtkTreeView *transferView;
		GtkListStore *transferStore;
		GtkTreeSelection *transferSel;
		GdkPixbuf *uploadPic, *downloadPic;

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
			COLUMN_ID		//hidden from user
		};
		
		//conviniece thing for the updateTransfer_gui function
		typedef Func9 <MainWindow, std::string, connection_t, std::string, std::string, 
			std::string, std::string, std::string, std::string, std::string> UFunc;
};

#else
class MainWindow;
#endif
