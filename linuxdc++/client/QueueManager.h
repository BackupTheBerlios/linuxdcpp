/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

#if !defined(AFX_QUEUEMANAGER_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)
#define AFX_QUEUEMANAGER_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TimerManager.h"

#include "CriticalSection.h"
#include "Exception.h"
#include "User.h"
#include "File.h"
#include "QueueItem.h"
#include "Singleton.h"
#include "DirectoryListing.h"
#include "MerkleTree.h"

#include "QueueManagerListener.h"
#include "SearchManagerListener.h"
#include "ClientManagerListener.h"

STANDARD_EXCEPTION(QueueException);

class UserConnection;

class DirectoryItem {
public:
	typedef DirectoryItem* Ptr;
	typedef HASH_MULTIMAP<User::Ptr, Ptr, User::HashFunction> DirectoryMap;
	typedef DirectoryMap::iterator DirectoryIter;
	typedef pair<DirectoryIter, DirectoryIter> DirectoryPair;
	
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	DirectoryItem() : priority(QueueItem::DEFAULT) { };
	DirectoryItem(const User::Ptr& aUser, const string& aName, const string& aTarget, 
		QueueItem::Priority p) : name(aName), target(aTarget), priority(p), user(aUser) { };
	~DirectoryItem() { };
	
	User::Ptr& getUser() { return user; };
	void setUser(const User::Ptr& aUser) { user = aUser; };
	
	GETSET(string, name, Name);
	GETSET(string, target, Target);
	GETSET(QueueItem::Priority, priority, Priority);
private:
	User::Ptr user;
};

class ConnectionQueueItem;
class QueueLoader;

class QueueManager : public Singleton<QueueManager>, public Speaker<QueueManagerListener>, private TimerManagerListener, 
	private SearchManagerListener, private ClientManagerListener
{
public:
	
	/** Add a file to the queue. */
	void add(const string& aFile, int64_t aSize, User::Ptr aUser, 
		const string& aTarget, const TTHValue* root, 
		int aFlags = QueueItem::FLAG_RESUME, QueueItem::Priority p = QueueItem::DEFAULT, 
		const string& aTempTarget = Util::emptyString, bool addBad = true) throw(QueueException, FileException);
	
	/** Add a user's filelist to the queue. */
	void addList(const User::Ptr& aUser, int aFlags) throw(QueueException, FileException) {
		string x = aUser->getNick();
		string::size_type i = 0;
		while((i = x.find(PATH_SEPARATOR), i) != string::npos)
			x[i] = '_';
		string file = Util::getAppPath() + "FileLists" + PATH_SEPARATOR_STR + x;
		// We use the searchString to store the start viewing directory for file lists
		add(USER_LIST_NAME, -1, aUser, file, NULL, 
			QueueItem::FLAG_USER_LIST | aFlags,  QueueItem::DEFAULT, 
			Util::emptyString, true);
	}

	/** Readd a source that was removed */
	void readd(const string& target, User::Ptr& aUser) throw(QueueException);

	/** Add a directory to the queue (downloads filelist and matches the directory). */
	void addDirectory(const string& aDir, const User::Ptr& aUser, const string& aTarget, QueueItem::Priority p = QueueItem::DEFAULT) throw();
	
	int matchListing(DirectoryListing* dl) throw();

	/** Move the target location of a queued item. Running items are silently ignored */
	void move(const string& aSource, const string& aTarget) throw();

	void remove(const string& aTarget) throw();
	void removeSource(const string& aTarget, User::Ptr& aUser, int reason, bool removeConn = true) throw();
	void removeSources(User::Ptr& aUser, int reason) throw();
	
	void setPriority(const string& aTarget, QueueItem::Priority p) throw();
	
	void getTargetsBySize(StringList& sl, int64_t aSize, const string& suffix) throw() {
		Lock l(cs);
		QueueItem::List ql;
		fileQueue.find(ql, aSize, suffix);
		for(QueueItem::Iter i = ql.begin(); i != ql.end(); ++i) {
			sl.push_back((*i)->getTarget());
		}
	}

	void getTargetsByRoot(StringList& sl, const TTHValue& tth) {
		Lock l(cs);
		QueueItem::List ql;
		fileQueue.find(ql, &tth);
		for(QueueItem::Iter i = ql.begin(); i != ql.end(); ++i) {
			sl.push_back((*i)->getTarget());
		}
	}

	QueueItem::StringMap& lockQueue() throw() { cs.enter(); return fileQueue.getQueue(); } ;
	void unlockQueue() throw() { cs.leave(); };

	Download* getDownload(User::Ptr& aUser) throw();
	void putDownload(Download* aDownload, bool finished = false) throw();

	bool hasDownload(const User::Ptr& aUser, QueueItem::Priority minPrio = QueueItem::LOWEST) throw() {
		Lock l(cs);
		return (userQueue.getNext(aUser, minPrio) != NULL);
	}
	
	void loadQueue() throw();
	void saveQueue() throw();
	
	GETSET(u_int32_t, lastSave, LastSave);
	GETSET(string, queueFile, QueueFile);
private:

	/** All queue items by target */
	class FileQueue {
	public:
		FileQueue() : lastInsert(queue.end()) { };
		~FileQueue() {
			for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i)
				delete i->second;
		}
		void add(QueueItem* qi);
		QueueItem* add(const string& aTarget, int64_t aSize, 
			int aFlags, QueueItem::Priority p, const string& aTempTarget, int64_t aDownloaded,
			u_int32_t aAdded, const TTHValue* root) throw(QueueException, FileException);

		QueueItem* find(const string& target);
		void find(QueueItem::List& sl, int64_t aSize, const string& ext);
		void find(QueueItem::List& ql, const TTHValue* tth);

		QueueItem* findAutoSearch(StringList& recent);
		size_t getSize() { return queue.size(); };
		QueueItem::StringMap& getQueue() { return queue; };
		void move(QueueItem* qi, const string& aTarget);
		void remove(QueueItem* qi) {
			if(lastInsert != queue.end() && Util::stricmp(*lastInsert->first, qi->getTarget()) == 0)
				lastInsert = queue.end();
			queue.erase(const_cast<string*>(&qi->getTarget()));
			delete qi;
		}

	private:
		QueueItem::StringMap queue;
		/** A hint where to insert an item... */
		QueueItem::StringIter lastInsert;
	};

	/** All queue items indexed by user (this is a cache for the FileQueue really...) */
	class UserQueue {
	public:
		void add(QueueItem* qi);
		void add(QueueItem* qi, const User::Ptr& aUser);
		QueueItem* getNext(const User::Ptr& aUser, QueueItem::Priority minPrio = QueueItem::LOWEST);
		QueueItem* getRunning(const User::Ptr& aUser);
		void setRunning(QueueItem* qi, const User::Ptr& aUser);
		void setWaiting(QueueItem* qi);
		QueueItem::UserListMap& getList(int p) { return userQueue[p]; };
		void remove(QueueItem* qi);
		void remove(QueueItem* qi, const User::Ptr& aUser);

		QueueItem::UserMap& getRunning() { return running; };
		bool isRunning(const User::Ptr& aUser) const { 
			return (running.find(aUser) != running.end());
		};
	private:
		/** QueueItems by priority and user (this is where the download order is determined) */
		QueueItem::UserListMap userQueue[QueueItem::LAST];
		/** Currently running downloads, a QueueItem is always either here or in the userQueue */
		QueueItem::UserMap running;
	};

	friend class QueueLoader;
	friend class Singleton<QueueManager>;
	
	QueueManager();
	virtual ~QueueManager();
	
	CriticalSection cs;
	
	/** QueueItems by target */
	FileQueue fileQueue;
	/** QueueItems by user */
	UserQueue userQueue;
	/** Directories queued for downloading */
	DirectoryItem::DirectoryMap directories;
	/** Recent searches list, to avoid searching for the same thing too often */
	StringList recent;
	/** The queue needs to be saved */
	bool dirty;
	/** Next search */
	u_int32_t nextSearch;
	
	static const string USER_LIST_NAME;
	static const string TEMP_EXTENSION;
	static string getTempName(const string& /*aFileName*/, const TTHValue* /*aRoot*/);

	/** Sanity check for the target filename */
	static string checkTarget(const string& aTarget, int64_t aSize, int& flags) throw(QueueException, FileException);
	/** Add a source to an existing queue item */
	bool addSource(QueueItem* qi, const string& aFile, User::Ptr aUser, Flags::MaskType addBad, bool utf8) throw(QueueException, FileException);

	int QueueManager::matchFiles(DirectoryListing::Directory* dir) throw();

	void removeAll(QueueItem* q);
	void load(SimpleXML* aXml);

	void setDirty() {
		if(!dirty) {
			dirty = true;
			lastSave = GET_TICK();
		}
	}

	// TimerManagerListener
	virtual void on(TimerManagerListener::Second, u_int32_t aTick) throw();
	virtual void on(TimerManagerListener::Minute, u_int32_t aTick) throw();
	
	// SearchManagerListener
	virtual void on(SearchManagerListener::SR, SearchResult*) throw();

	// ClientManagerListener
	virtual void on(ClientManagerListener::UserUpdated, const User::Ptr& aUser) throw();
};

#endif // !defined(AFX_QUEUEMANAGER_H__07D44A33_1277_482D_AFB4_05E3473B4379__INCLUDED_)

/**
 * @file
 * $Id: QueueManager.h,v 1.2 2004/11/12 16:29:31 phase Exp $
 */

