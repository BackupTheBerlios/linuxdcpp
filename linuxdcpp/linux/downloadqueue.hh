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

#ifndef WULFOR_DOWNLOAD_QUEUE_HH
#define WULFOR_DOWNLOAD_QUEUE_HH

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/QueueManager.h>

#include "bookentry.hh"
#include "treeview.hh"

class DownloadQueue:
	public BookEntry,
	public QueueManagerListener
{
	public:
		DownloadQueue();
		~DownloadQueue();

	private:
		string getNextSubDir(std::string path);
		string getTrailingSubDir(std::string path);
		string getRemainingDir(std::string path);

		// GUI functions
		void buildList_gui();
		void updateStatus_gui();
		void setStatus_gui(std::string text, std::string statusItem);
		void setDirPriority_gui(std::string path, QueueItem::Priority priority);
		void buildDynamicMenu_gui();
		void update_gui();
		void addItem_gui(QueueItem *item);
		void updateItem_gui(QueueItem *item, bool add);
		void removeFile_gui(std::string path);
		void removeDir_gui(std::string path);
		int countFiles_gui(std::string path);
		void addDir_gui(std::string path, GtkTreeIter *row, std::string &current);
		void addFile_gui(QueueItem *item, std::string path);
		void getChildren(std::string path, std::vector<GtkTreeIter> *iter);
		void getChildren(std::string path, std::vector<std::string> *iter);
		static std::string getTextFromMenu(GtkMenuItem *item);

		// GUI callbacks
		static gboolean onDirButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onDirButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onDirKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onFileButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onFileButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onFileKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onRemoveFileClicked_gui(GtkMenuItem *menuitem, gpointer data);
		static void onRemoveDirClicked_gui(GtkMenuItem *menuitem, gpointer data);
		static void onSearchAlternatesClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDirPriorityClicked_gui(GtkMenuItem *item, gpointer data);
		static void onFilePriorityClicked_gui(GtkMenuItem *item, gpointer data);
		static void onGetFileListClicked_gui(GtkMenuItem *item, gpointer data);
		static void onSendPrivateMessageClicked_gui(GtkMenuItem *item, gpointer data);
		static void onReAddSourceClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveSourceClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveUserFromQueueClicked_gui(GtkMenuItem *item, gpointer data);

		// Client functions
		void remove_client(std::string path);
		void setPriority_client(std::string target, QueueItem::Priority p);
		void reAddSource_client(std::string target, User::Ptr &user);
		void addList_client(const User::Ptr &user);
		void removeSource_client(std::string target, User::Ptr &user);
		void removeSources_client(User::Ptr &user);

		// Client callbacks
		virtual void on(QueueManagerListener::Added, QueueItem *item) throw();
		virtual void on(QueueManagerListener::Moved, QueueItem *item) throw();
		virtual void on(QueueManagerListener::Removed, QueueItem *item) throw();
		virtual void on(QueueManagerListener::SourcesUpdated, QueueItem *item) throw();
		virtual void on(QueueManagerListener::StatusUpdated, QueueItem *item) throw();

		// Private variables
		TreeView dirView, fileView;
		GtkTreeStore *dirStore;
		GtkListStore *fileStore;
		hash_map<std::string, GtkTreeIter> dirMap;
		hash_map<std::string, std::vector<QueueItem *> > dirFileMap;
		hash_map<std::string, QueueItem *> fileMap;
		int64_t queueSize;
		int queueItems;
		std::string showingDir;
		GdkEventType dirPrevious;
		GtkTreeSelection *dirSelection;
		GtkTreeSelection *fileSelection;
};

#else
class DownloadQueue;
#endif
