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

#ifndef WULFOR_MAIN_WINDOW
#define WULFOR_MAIN_WINDOW

#include <gtkmm.h>
#include <string>

#include "privatemsg.hh"
#include "bookentry.hh"

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"
#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/ConnectionManager.h"
#include "../client/QueueManager.h"
#include "../client/TimerManager.h"

enum {
	STATUS_MAIN,
	STATUS_HUBS,
	STATUS_SLOTS,
	STATUS_DL,
	STATUS_UL,
	STATUS_DL_SPEED,
	STATUS_UL_SPEED,
	NUM_STATUS
};

class TransferColumns: public Gtk::TreeModel::ColumnRecord {
	public:
		TransferColumns() {
			add(user);
			add(status);
			add(speed);
		}

		Gtk::TreeModelColumn<Glib::ustring> user, status, speed;
};

class MainWindow:
	public Gtk::Window,
	public DownloadManagerListener, 
	public UploadManagerListener,
	public ConnectionManagerListener,
	public TimerManagerListener
//	public QueueManagerListener
{
	public:
		MainWindow();
		void setStatus(std::string text, int num);
		void startSocket();

		bool addPage(BookEntry *page);
		BookEntry *getPage(BookEntry *page);

		//DownloadManagerListener
		void on(DownloadManagerListener::Starting, 
			Download *dl) throw();
		void on(DownloadManagerListener::Tick, 
			const Download::List &list) throw();
		void on(DownloadManagerListener::Complete, Download *dl) throw();
		void on(DownloadManagerListener::Failed, Download *dl, 
			const string &reason) throw();
	
		//UploadManagerListener
		//I was too lazy to get this working...
		
		//ConnectionManagerListener
		void on(ConnectionManagerListener::Added, 
			ConnectionQueueItem *item) throw();
		void on(ConnectionManagerListener::Connected, 
			ConnectionQueueItem *item) throw();
		void on(ConnectionManagerListener::Removed, 
			ConnectionQueueItem *item) throw();
		void on(ConnectionManagerListener::Failed, 
			ConnectionQueueItem *item, const string &reason) throw();
		void on(ConnectionManagerListener::StatusChanged, 
			ConnectionQueueItem *item) throw();

		//TimerManagerListener
		void on(TimerManagerListener::Second, u_int32_t tics) throw();
			
	private:	
		void pubHubsClicked();
		void searchClicked();
		void settingsClicked();
		void exitClicked();
		bool on_delete_event(GdkEventAny *e);

		int64_t lastUpdate, lastUp, lastDown;
						
		Gtk::VBox mainBox;
		Gtk::HBox statusBox, toolBox;
		Gtk::Toolbar leftBar, rightBar;
		Gtk::VPaned pane;
		
		Gtk::Notebook book;
		Gtk::Statusbar status[NUM_STATUS];
		Gtk::Image pubHubsIcon, exitIcon, settingsIcon, searchIcon;
		
		Gtk::TreeView transferList;
		Glib::RefPtr<Gtk::ListStore> transferStore;
		TransferColumns columns;
};

#else
class MainWindow;
#endif
