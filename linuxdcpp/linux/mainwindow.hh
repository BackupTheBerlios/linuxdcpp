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

class MainWindow:
	public QueueManagerListener,
	public TimerManagerListener
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

		//client functions
		void startSocket();
		virtual void on(TimerManagerListener::Second, u_int32_t ticks) throw();
		virtual void on(QueueManagerListener::Finished, QueueItem *item) throw();

	private:
		//callbacks (to be called from the gui)
		static void pubHubs_callback(GtkWidget *widget, gpointer data);
		static void quit_callback(GtkWidget *widget, gpointer data);

		int64_t lastUpdate, lastUp, lastDown;

		GtkWindow *window;
		GtkStatusbar *mainStatus, *hubStatus, *slotStatus, *dTotStatus, *uTotStatus,
			*dlStatus, *ulStatus;
		GtkNotebook *book;
		GtkTreeView *transfers;
};

#else
class MainWindow;
#endif
