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
#include "util.hh"
#include "transfer.hh"

#include "hashdialog.hh"

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

class MainWindow:
	public Gtk::Window,
	public TimerManagerListener
//	public QueueManagerListener
{
	public:
		MainWindow();
		void setStatus(std::string text, int num);
		void startSocket();

		bool addPage(BookEntry *page);
		BookEntry *getPage(BookEntry *page);
		BookEntry *findPage (Glib::ustring text);

		//TimerManagerListener
		virtual void on(TimerManagerListener::Second, u_int32_t tics) throw();

		void quit();

	private:	
		void pubHubsClicked();
		void searchClicked();
		void settingsClicked();
		void exitClicked();
		void hashClicked();
		bool on_delete_event(GdkEventAny *e);

		int64_t lastUpdate, lastUp, lastDown;
						
		Gtk::VBox mainBox;
		Gtk::HBox statusBox, toolBox;
		Gtk::Toolbar leftBar, rightBar;
		Gtk::VPaned pane;
		
		Gtk::Notebook book;
		Gtk::Statusbar status[NUM_STATUS];
		Gtk::Image pubHubsIcon, exitIcon, settingsIcon, searchIcon, hashIcon;

		HashDialog *hashProgress;
		bool showingHash;

};

#else
class MainWindow;
#endif
