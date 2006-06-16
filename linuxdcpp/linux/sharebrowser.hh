/* 
 * Copyright © 2004-2006 Jens Oknelid, paskharen@gmail.com
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

#include <sstream>

#include "bookentry.hh"
#include "callback.hh"
#include "treeview.hh"
#include "wulformanager.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/DirectoryListing.h>
#include <client/Text.h>
#include <client/User.h>

using namespace std;

class ShareBrowser:
	public BookEntry
{
	public:
		//the constructor is to be called from the gui thread
		ShareBrowser(User::Ptr user, std::string file);
		~ShareBrowser();
	
		// From BookEntry
		GtkWidget *getWidget();

		//these functions should only be called from the client thread
		void matchQueue_client();
		void findNext_gui(bool firstFile);
	
		//these functions should only be called from the gui thread
		void setStatus_gui(GtkStatusbar *status, std::string msg);
		void setPosition_gui(std::string pos);

	private:
		//only call these from gui thread
		static gboolean fileButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
		static gboolean filePopupMenu_gui(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
		static gboolean dirButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
		static gboolean dirButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
		static gboolean dirPopupMenu_gui(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
		static void matchButtonClicked_gui(GtkWidget *widget, gpointer user_data);
		static void findButtonClicked_gui(GtkWidget *widget, gpointer);
		static void nextButtonClicked_gui(GtkWidget *widget, gpointer);
		static void downloadClicked_gui(GtkMenuItem *item, gpointer user_data);
		static void downloadToClicked_gui(GtkMenuItem *item, gpointer user_data);
		static void downloadFavoriteClicked_gui(GtkMenuItem *item, gpointer user_data);
		static void downloadDirClicked_gui(GtkMenuItem *item, gpointer user_data);
		static void downloadDirToClicked_gui(GtkMenuItem *item, gpointer user_data);
		static void downloadFavoriteDirClicked_gui(GtkMenuItem *item, gpointer user_data);
		static void searchAlternatesClicked_gui(GtkMenuItem *item, gpointer user_data);

		void buildDirs_gui(DirectoryListing::Directory::List dir, GtkTreeIter *iter);
		void updateFiles_gui(bool fromFind);
		void updateStatus_gui();

		//only call these from client thread
		void downloadSelectedFiles_gui(std::string target);
		void downloadFile_client(DirectoryListing::File *file, std::string target);
		void downloadSelectedDirs_gui(std::string target);
		void downloadDir_client(DirectoryListing::Directory *dir, std::string target);

		void buildDownloadMenus_gui(int menu);

		GdkEventType oldType;
		guint oldButton;

		DirectoryListing listing;
		std::string lastDir;
		int64_t shareSize;
		int64_t currentSize;
		int shareItems;
		int currentItems;
		std::string search;
		GtkTreePath *posDir;
		DirectoryListing::File::Iter posFile;
		TreeView dirView, fileView;
	
		GtkStatusbar *mainStatus, *itemsStatus, *sizeStatus, *filesStatus, *totalStatus;
		GtkListStore *fileStore;
		GtkTreeStore *dirStore;
		GtkTreeSelection *fileSelection, *dirSelection;
		GtkWidget *box;
		GtkButton *matchButton, *findButton, *nextButton;
		GdkPixbuf *iconFile, *iconDirectory;
		
		GtkMenu *fileMenu, *dirMenu, *fileDownloadMenu, *dirDownloadMenu;
		std::map<string, GtkWidget*> dirMenuItems;
		std::map<string, GtkWidget*> fileMenuItems;
		std::vector<GtkWidget*> fileDownloadItems, dirDownloadItems;
		typedef pair<ShareBrowser*, string> userData;
		std::vector<userData*> menuUserData;
};

#else
class ShareBrowser;
#endif
