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

class ShareBrowser:
	public BookEntry
{
	public:
		//the constructor is to be called from the gui thread
		ShareBrowser(User::Ptr user, std::string file, GCallback closeCallback);
		~ShareBrowser();
	
		GtkWidget *ShareBrowser::getWidget();

		void setStatus(GtkStatusbar *status, std::string msg);
		void setPosition(string pos);
		void buildDirs(
			DirectoryListing::Directory::List dirs, GtkTreeIter *iter);
		void updateFiles(bool fromFind);
		void updateStatus();

		static gboolean buttonPressed(
			GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean buttonReleased(
			GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void buttonClicked(GtkWidget *widget, gpointer data);
		static void menuClicked(GtkMenuItem *item, gpointer data);

		void downloadFile(DirectoryListing::File *file, string target);
		void downloadDir(DirectoryListing::Directory *dir, string target);
		void findNext(bool firstFile);

	private:
		GdkEventType oldType;
		guint oldButton;

		DirectoryListing listing;
		int64_t shareSize;
		int64_t currentSize;
		int shareItems;
		int currentItems;
		std::string search;
		GtkTreePath *posDir;
		DirectoryListing::File::Iter posFile;
	
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
