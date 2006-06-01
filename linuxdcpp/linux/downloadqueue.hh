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

#ifndef WULFOR_DOWNLOAD_QUEUE_HH
#define WULFOR_DOWNLOAD_QUEUE_HH

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <iostream>
#include <sstream>
#include <ext/hash_map>

#include "bookentry.hh"
#include "search.hh"
#include "treeview.hh"
#include "wulformanager.hh"
#include "wulformanager.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/QueueManager.h>
#include <client/User.h>

using namespace std;

class DownloadQueue:
	public BookEntry,
	public QueueManagerListener
{
public:
	// Must be called from gui-thread
	DownloadQueue();
	~DownloadQueue ();

	// From BookEntry
	GtkWidget *getWidget();

	void buildList_gui ();
	void updateStatus_gui ();

	virtual void on(QueueManagerListener::Added, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::Finished, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::Moved, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::Removed, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::SourcesUpdated, QueueItem* aQI) throw() { updateFiles_gui (aQI); }
	virtual void on(QueueManagerListener::StatusUpdated, QueueItem* aQI) throw() { updateFiles_gui (aQI); }

private:
	enum {
		STATUS_FIRST,
		STATUS_MAIN = STATUS_FIRST,
		STATUS_ITEMS,
		STATUS_FILE_SIZE,
		STATUS_FILES,
		STATUS_TOTAL_SIZE,
		STATUS_LAST
	};

	class QueueItemInfo;

	GtkWidget *mainBox;
	vector<GtkWidget*> statusbar;
	pthread_mutex_t queueLock;

	// TreeView related stuff
	TreeView dirView, fileView;
	GtkTreeStore *dirStore;
	GtkListStore *fileStore;
	static gboolean dir_onButtonPressed_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
	static gboolean dir_onButtonReleased_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
  	static gboolean dir_onPopupMenu_gui (GtkWidget *widget, gpointer user_data);
	static gboolean file_onButtonPressed_gui (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
  	static gboolean file_onPopupMenu_gui (GtkWidget *widget, gpointer user_data);
	void dir_popup_menu_gui (GdkEventButton *event, gpointer user_data);
	void file_popup_menu_gui (GdkEventButton *event, gpointer user_data);

	// Set statusbar
	void setStatus_gui (string text, int num);

	// Popup-Menus
	GtkMenu *dirMenu, *fileMenu, *dirPriority, *filePriority;
	GtkMenu *browseMenu, *pmMenu, *readdMenu, *removeMenu, *removeallMenu;
	hash_map<std::string, GtkWidget *, WulforUtil::HashString> dirItems;
	hash_map<std::string, GtkWidget *, WulforUtil::HashString> fileItems;
	std::vector<GtkWidget*> browseItems;
	std::vector<GtkWidget*> readdItems;
	std::vector<GtkWidget*> pmItems;
	std::vector<GtkWidget*> removeItems;
	std::vector<GtkWidget*> removeallItems;
	void buildStaticMenu_gui ();	
	void buildDynamicMenu_gui ();

	static void removeFileClicked_gui (GtkMenuItem *menuitem, gpointer user_data);
	static void removeDirClicked_gui (GtkMenuItem *menuitem, gpointer user_data);
	void remove_client (string path);
	
	// Search
	static void onSearchAlternatesClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onSearchByTTHClicked_gui (GtkMenuItem *item, gpointer user_data);
	
	//Priority
	static void setDirPriorityClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void setFilePriorityClicked_gui (GtkMenuItem *item, gpointer user_data);
	void setPriority_client (string target, QueueItem::Priority p);
	void setDirPriority_gui (string path, QueueItem::Priority p);
	
	// Dynamic menu
	static void onGetFileListClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onSendPrivateMessageClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onReAddSourceClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onRemoveSourceClicked_gui (GtkMenuItem *item, gpointer user_data);
	static void onRemoveUserFromQueueClicked_gui (GtkMenuItem *item, gpointer user_data);

	// Updates
	void update_gui ();
	void updateItem_gui (QueueItemInfo *i, bool add);
	void updateFiles_gui (QueueItem *aQI);
	void removeFile_gui (string target);
	void removeDir_gui (string path);
	int countFiles_gui (string path);

	// Queue-handling functions and datastorage
	void addDir_gui (string path, GtkTreeIter *row, string &current);
	void addFile_gui (QueueItemInfo *i, string path);
	string getNextSubDir (string path);
	string getTrailingSubDir (string path);
	string getRemainingDir (string path);
	void getChildren (string path, vector<GtkTreeIter> *iter);
	void getChildren (string path, vector<string> *iter);
	hash_map<std::string, GtkTreeIter, WulforUtil::HashString> dirMap;
	hash_map<std::string, std::vector<QueueItemInfo*>, WulforUtil::HashString> dirFileMap;
	hash_map<std::string, QueueItem *, WulforUtil::HashString> fileMap;

	int64_t queueSize;
	int queueItems;
	string showingDir;
	CriticalSection cs;
	GdkEventType dirPrevious;
	static string getTextFromMenu (GtkMenuItem *item);
	
	void contentUpdated ();

	class QueueItemInfo : public Flags
	{
	public:

		struct SourceInfo : public Flags {
			explicit SourceInfo(const QueueItem::Source& s) : Flags(s), user(s.getUser()) { };

			SourceInfo& operator=(const QueueItem::Source& s) {
				*((Flags*)this) = s;
				user = s.getUser();
			}
			User::Ptr& getUser() { return user; };

			User::Ptr user;
		};

		typedef vector<SourceInfo> SourceList;
		typedef SourceList::iterator SourceIter;

		QueueItemInfo(QueueItem* aQI) :
			Flags(*aQI),
			target(aQI->getTarget()),
			path(Util::getFilePath(aQI->getTarget())),
			size(aQI->getSize()), 
			downloadedBytes(aQI->getDownloadedBytes()), 
			added(aQI->getAdded()), 
			tth(aQI->getTTH()), 
			priority(aQI->getPriority()), 
			status(aQI->getStatus())
		{ 
			QueueItem::Source::Iter i;
			for(i = aQI->getSources().begin(); i != aQI->getSources().end(); ++i) {
				sources.push_back(SourceInfo(*(*i)));
			}
			for(i = aQI->getBadSources().begin(); i != aQI->getBadSources().end(); ++i) {
				badSources.push_back(SourceInfo(*(*i)));
			}
		};

		~QueueItemInfo() { };

		void remove() { QueueManager::getInstance()->remove(getTarget()); }
		void update (DownloadQueue *dq, bool add);
		

		SourceList& getSources() { return sources; };
		SourceList& getBadSources() { return badSources; };

		void getUserList (string text)
		{
			for(SourceIter i = sources.begin(); i != sources.end(); ++i)
				if(i->getUser()->getFullNick () == text)
				{
					try 
					{
						QueueManager::getInstance()->addList(i->getUser(), QueueItem::FLAG_CLIENT_VIEW);
					} 
					catch(const Exception&) 
					{
					}
					break;
				}
		}
		void sendPrivateMessage (string text);
		void reAddSource (string text, DownloadQueue *q)
		{
			for(SourceIter i = sources.begin(); i != sources.end(); ++i)
				if(i->getUser()->getFullNick () == text)
				{
					try 
					{
						QueueManager::getInstance()->readd(getTarget(), i->getUser());
					} 
					catch(const Exception& e) 
					{
						q->setStatus_gui (e.getError (), STATUS_MAIN);
					}
					break;
				}
		}
		void removeSource (string text)
		{
			for(SourceIter i = sources.begin(); i != sources.end(); ++i)
				if(i->getUser()->getFullNick () == text)
				{
					QueueManager::getInstance()->removeSource(getTarget(), i->getUser(), QueueItem::Source::FLAG_REMOVED);
					break;
				}
		}
		void removeSources (string text)
		{
			for(SourceIter i = sources.begin(); i != sources.end(); ++i)
				if(i->getUser()->getFullNick () == text)			
				{
					QueueManager::getInstance()->removeSources(i->getUser(), QueueItem::Source::FLAG_REMOVED);
					break;
				}
		}
		
		bool isSource(const User::Ptr& u) {
			for(SourceIter i = sources.begin(); i != sources.end(); ++i) {
				if(i->getUser() == u)
					return true;
			}
			return false;
		}
		bool isBadSource(const User::Ptr& u) {
			for(SourceIter i = badSources.begin(); i != badSources.end(); ++i) {
				if(i->getUser() == u)
					return true;
			}
			return false;
		}
		
		GETSET(string, target, Target);
		GETSET(string, path, Path);
		GETSET(int64_t, size, Size);
		GETSET(int64_t, downloadedBytes, DownloadedBytes);
		GETSET(u_int32_t, added, Added);
		GETSET(QueueItem::Priority, priority, Priority);
		GETSET(QueueItem::Status, status, Status);
		GETSET(TTHValue*, tth, TTH);

		friend class DownloadQueue;

	private:

		QueueItemInfo(const QueueItemInfo&);
		QueueItemInfo& operator=(const QueueItemInfo&);
		
		SourceList sources;
		SourceList badSources;

	};
};

#else
class DownloadQueue;
#endif
