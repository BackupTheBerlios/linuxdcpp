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

#ifndef WULFOR_SHARE_BROWSER_HH
#define WULFOR_SHARE_BROWSER_HH

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <string>

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/User.h>
#include <client/DirectoryListing.h>

#include "bookentry.hh"
#include "callback.hh"

class ShareBrowser:
	public BookEntry
{
	public:
		//the constructor is to be called from the gui thread
		ShareBrowser(User::Ptr user, std::string file, GCallback closeCallback);
	
		GtkWidget *getWidget();
	
		//these functions should only be called from the gui thread
		void setStatus_gui(GtkStatusbar *status, std::string msg);
		void setPosition_gui(std::string pos);

	private:
		//only call these from gui thread
		gboolean buttonPressed_gui(GtkWidget *, GdkEventButton *, gpointer);
		gboolean buttonReleased_gui(GtkWidget *, GdkEventButton *, gpointer);
		void menuClicked_gui(GtkMenuItem *item, gpointer);

		void buildDirs_gui(DirectoryListing::Directory::List dir, GtkTreeIter *iter);
		void updateFiles_gui();
		void updateStatus_gui();

		//only call these from client thread
		void downloadFile_client(DirectoryListing::File *file, std::string target);
		void downloadDir_client(DirectoryListing::Directory *dir, std::string target);

		Callback3<ShareBrowser, gboolean, GtkWidget *, GdkEventButton *> 
			pressedCallback, releasedCallback;
		Callback2<ShareBrowser, void, GtkMenuItem *> menuCallback;
		GdkEventType oldType;
		guint oldButton;

		DirectoryListing listing;
		int64_t shareSize;
		int64_t currentSize;
		int shareItems;
		int currentItems;
	
		GtkStatusbar *mainStatus, *itemsStatus, *sizeStatus, *filesStatus, *totalStatus;
		GtkTreeView *dirView, *fileView;
		GtkListStore *fileStore;
		GtkTreeStore *dirStore;
		GtkTreeSelection *fileSelection, *dirSelection;
		GtkWidget *box;
		GtkButton *matchButton, *findButton, *nextButton;
		
		GtkMenu *fileMenu, *dirMenu;
		GtkMenuItem *dlDir, *dlFile, *dlDirTo, *dlFileTo, *dlDirDir, *dlDirToDir;
		
		enum {
			COLUMN_FILE,
			COLUMN_SIZE,
			COLUMN_TYPE,
			COLUMN_TTH,
			COLUMN_DL_FILE
		};

		enum {
			COLUMN_DIR,
			COLUMN_DL_DIR
		};

		const int WIDTH_FILE, WIDTH_SIZE, WIDTH_TYPE, WIDTH_TTH;
};

#else
class ShareBrowser;
#endif
