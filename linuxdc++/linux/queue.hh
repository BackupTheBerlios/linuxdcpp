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

#ifndef WULFOR_QUEUE
#define WULFOR_QUEUE

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"
#include "../client/QueueManager.h"
#include "../client/User.h"

#include <gtkmm.h>
#include <iostream>

#include "bookentry.hh"
#include "mainwindow.hh"
#include "util.hh"
#include "search.hh"
#include "privatemsg.hh"

class DownloadQueue : 	public QueueManagerListener,
										public BookEntry
{
public:
	DownloadQueue (MainWindow *mw);
	~DownloadQueue();

	void setStatus (std::string text, int num);

	bool operator== (BookEntry &b);
private:
	enum {
		COLUMN_FIRST,
		COLUMN_TARGET = COLUMN_FIRST,
		COLUMN_STATUS,
		COLUMN_SIZE,
		COLUMN_DOWNLOADED,
		COLUMN_PRIORITY,
		COLUMN_USERS,
		COLUMN_PATH,
		COLUMN_EXACT_SIZE,
		COLUMN_ERRORS,
		COLUMN_ADDED,
		COLUMN_TTH,
		COLUMN_LAST
	};
	enum {
		STATUS_FIRST,
		STATUS_MAIN = STATUS_FIRST,
		STATUS_ITEMS,
		STATUS_FILE_SIZE,
		STATUS_FILES,
		STATUS_TOTAL_SIZE,
		STATUS_LAST
	};
	enum Tasks {
		ADD_ITEM,
		REMOVE_ITEM,
		UPDATE_ITEM
	};

 	class QueueItemInfo;
	class FileColumns: public Gtk::TreeModel::ColumnRecord
	{
		public:
			FileColumns()
			{
				add(target);
				add (status);
				add(filesize);
				add (downloaded);
				add (priority);
				add (users);
				add(path);
				add (exactsize);
				add (added);
				add (errors);
				add (tth);
				add (item); // Hidden from user.
			}
			Gtk::TreeModelColumn<Glib::ustring> 	target,
																			status,
																			filesize,
																			downloaded,
																			priority,
																			users,
																			path,
																			exactsize,
																			added,
																			errors,
																			tth;
			Gtk::TreeModelColumn<QueueItemInfo*> item; // Hidden from user.
	};

	class DirColumns: public Gtk::TreeModel::ColumnRecord
	{
		public:
			DirColumns()
			{
				add(name);
				add(realpath);
			}
			Gtk::TreeModelColumn<Glib::ustring> name, realpath;
	};

	friend class QueueItemInfo;
	
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
			for(QueueItem::Source::Iter i = aQI->getSources().begin(); i != aQI->getSources().end(); ++i) {
				sources.push_back(SourceInfo(*(*i)));
			}
			for(QueueItem::Source::Iter i = aQI->getBadSources().begin(); i != aQI->getBadSources().end(); ++i) {
				badSources.push_back(SourceInfo(*(*i)));
			}
		};

		~QueueItemInfo() { };

		void remove() { QueueManager::getInstance()->remove(getTarget()); }
		void update (DownloadQueue *dq, bool add);
		

		SourceList& getSources() { return sources; };
		SourceList& getBadSources() { return badSources; };

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

	virtual void on(QueueManagerListener::Added, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::Finished, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::Moved, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::Removed, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::SourcesUpdated, QueueItem* aQI) throw() { updateFiles (aQI); }
	virtual void on(QueueManagerListener::StatusUpdated, QueueItem* aQI) throw() { updateFiles (aQI); }
	void updateFiles (QueueItem *aQI);
	void updateStatus ();
		
	CriticalSection cs;

	int64_t queueSize;
	int queueItems;
	Glib::ustring showingDir;

	FileColumns fileColumns;
	DirColumns dirColumns;
	
	Gtk::VBox mainBox;
	Gtk::HBox statusBox;

	Gtk::HPaned pane;
	Gtk::ScrolledWindow fileScroll, dirScroll;
	Gtk::TreeView dirView, fileView;
	Glib::RefPtr<Gtk::TreeStore> dirStore;
	Glib::RefPtr<Gtk::ListStore> fileStore;
	Gtk::Statusbar statusBar[STATUS_LAST];

	Gtk::Menu dirMenu, fileMenu, priorityDirMenu, priorityFileMenu, browseMenu, pmMenu, readdMenu, removeMenu, removeAllMenu;
	Gtk::MenuItem smFilePrio, smDirPrio, smBrowse, smPM, smReAdd, smRemove, smRemoveAll;

	/* Menu actions */
	void searchAlternates ();
	void searchTTH ();
	void browseList (QueueItemInfo::SourceInfo *s);
	void sendPM (QueueItemInfo::SourceInfo *s);
	void readdSource (QueueItemInfo *i, QueueItemInfo::SourceInfo *s);
	void removeSource (QueueItemInfo *i, QueueItemInfo::SourceInfo *s);
	void removeUser (QueueItemInfo *i, QueueItemInfo::SourceInfo *s);
	
	MainWindow *window;

	static int columnSize[COLUMN_LAST];

	/* Mouseclicks */
	void update ();
	void rebuildMenu ();
	void removeDirClicked ();
	void removeFileClicked ();
	void buttonPressedDir (GdkEventButton *event);
	void buttonReleasedDir (GdkEventButton *event);
	void buttonPressedFile (GdkEventButton *event);
	void buttonReleasedFile (GdkEventButton *event);
	
	GdkEventType filePrevious, dirPrevious;
	
	/* Queue info */
	void buildList (const QueueItem::StringMap& l);
	void addDir (string path, Gtk::TreeModel::Row row, Glib::ustring &current);
	void removeDir (Glib::ustring path);
	void removeFile (Glib::ustring target);
	void removeFiles (Glib::ustring path);
	void setFilePriority (Glib::ustring target, QueueItem::Priority p);
	void setDirPriority (Glib::ustring path, QueueItem::Priority p);
	int countFiles (Glib::ustring path);
	void addFile (QueueItemInfo *i, Glib::ustring path);
	string getNextSubDir (string path);
	string getTrailingSubDir (string path);
	string getRemainingDir (string path);
	void getChildren (Glib::ustring path, vector<Gtk::TreeModel::iterator> *iter);
	std::map<Glib::ustring, Gtk::TreeModel::Row> dirMap;
	std::map<Glib::ustring, std::vector<QueueItemInfo*> > dirFileMap;
	std::map<Glib::ustring, QueueItem*> fileMap;
	
};
#else
class Queue;
#endif
