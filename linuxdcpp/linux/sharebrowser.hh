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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef WULFOR_SHARE_BROWSER_HH
#define WULFOR_SHARE_BROWSER_HH

#include "bookentry.hh"
#include "treeview.hh"
#include "wulformanager.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/DirectoryListing.h>
#include <client/Text.h>
#include <client/User.h>

class ShareBrowser:
	public BookEntry
{
	public:
		ShareBrowser(User::Ptr user, std::string file);
		~ShareBrowser();

		// From BookEntry
		GtkWidget *getWidget();

		// GUI function
		void setPosition_gui(std::string pos);

	private:
		// GUI functions
		void buildDirs_gui(DirectoryListing::Directory::List dir, GtkTreeIter *iter);
		void updateFiles_gui(DirectoryListing::Directory *dir);
		void updateStatus_gui();
		void setStatus_gui(GtkStatusbar *status, std::string msg);
		void findNext_gui(bool firstFile);
		void downloadSelectedFiles_gui(std::string target);
		void downloadSelectedDirs_gui(std::string target);
		void buildDirDownloadMenu_gui();
		void buildFileDownloadMenu_gui();
		void fileViewSelected_gui();

		// GUI callbacks
		static gboolean onFileButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onFileKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onFilePopupMenu_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onDirButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onDirKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onDirPopupMenu_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onMatchButtonClicked_gui(GtkWidget *widget, gpointer data);
		static void onFindButtonClicked_gui(GtkWidget *widget, gpointer);
		static void onNextButtonClicked_gui(GtkWidget *widget, gpointer);
		static void onDownloadClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadToClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadFavoriteClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadDirClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadDirToClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDownloadFavoriteDirClicked_gui(GtkMenuItem *item, gpointer data);
		static void onSearchAlternatesClicked_gui(GtkMenuItem *item, gpointer data);

		// Client functions
		void downloadFile_client(DirectoryListing::File *file, std::string target);
		void matchQueue_client();
		void downloadDir_client(DirectoryListing::Directory *dir, std::string target);

		GdkEventType oldType;
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
		hash_map<std::string, GtkWidget*> dirMenuItems;
		hash_map<std::string, GtkWidget*> fileMenuItems;
};

#else
class ShareBrowser;
#endif
