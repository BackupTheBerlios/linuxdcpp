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

#ifndef _HASH_MANAGER
#define _HASH_MANAGER

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <iostream>

#include "Singleton.h"
#include "MerkleTree.h"
#include "Thread.h"
#include "CriticalSection.h"
#include "Semaphore.h"
#include "TimerManager.h"
#include "Util.h"
#include "FastAlloc.h"

class HashManagerListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> TTHDone;

	virtual void on(TTHDone, const string& /* fileName */, TTHValue* /* root */) throw() = 0;
};

class HashLoader;

class HashManager : public Singleton<HashManager>, public Speaker<HashManagerListener>,
	private TimerManagerListener 
{
public:
	HashManager() {
		TimerManager::getInstance()->addListener(this);
	}
	virtual ~HashManager() {
		TimerManager::getInstance()->removeListener(this);
		hasher.join();
	}

	/**
	 * Check if the TTH tree associated with the filename is current.
	 */
	void checkTTH(const string& aFileName, int64_t aSize, u_int32_t aTimeStamp);

	void stopHashing(const string& baseDir) {
		hasher.stopHashing(baseDir);
	}

	void setPriority(Thread::Priority p) {
		hasher.setThreadPriority(p);
	}
	/**
	 * Retrieves TTH root or queue's file for hashing.
	 * @return TTH root if available, otherwise NULL
	 */
	TTHValue* getTTH(const string& aFileName);

	bool getTree(const string& aFileName, const TTHValue* root, TigerTree& tt);

	void addTree(const string& aFileName, const TigerTree& tt) {
		hashDone(aFileName, tt, -1);
	}

	void getStats(string& curFile, int64_t& bytesLeft, size_t& filesLeft) {
		hasher.getStats(curFile, bytesLeft, filesLeft);
	}

	/**
	 * Rebuild hash data file
	 */
	void rebuild() {
		hasher.scheduleRebuild();
	}

	void startup() {
		hasher.start();
		store.load();
	}

	void shutdown() {
		hasher.shutdown();
		hasher.join();
		Lock l(cs);
		store.save();
	}

private:

	class Hasher : public Thread {
	public:
		enum { MIN_BLOCK_SIZE = 64*1024 };
		Hasher() : stop(false), running(false), total(0), rebuild(false) { }

		void hashFile(const string& fileName, int64_t size) {
			Lock l(cs);
			if(w.insert(make_pair(fileName, size)).second) {
				s.signal();
				total += size;
			}
		}

		void stopHashing(const string& baseDir) {
			Lock l(cs);
			for(WorkIter i = w.begin(); i != w.end(); ) {
				if(Util::strnicmp(baseDir, i->first, baseDir.length()) == 0) {
					total -= i->second;
					w.erase(i++);
				} else {
					++i;
				}
			}
		}

		virtual int run();
#ifdef _WIN32
		bool fastHash(const string& fname, u_int8_t* buf, TigerTree& tth, int64_t size);
#endif
		void getStats(string& curFile, int64_t& bytesLeft, size_t& filesLeft) {
			Lock l(cs);
			curFile = file;
			filesLeft = w.size();
			if(running)
				filesLeft++;
			// Just in case...
			if(total < 0)
				total = 0;
			bytesLeft = total;
		}
		void shutdown() {
			stop = true;
			s.signal();
		}
		void scheduleRebuild() {
			rebuild = true;
			s.signal();
		}

	private:
		// Case-sensitive (faster), it is rather unlikely that case changes, and if it does it's harmless.
		// set because it's sorted (to avoid random hash order that would create quite strange shares while hashing)
		typedef map<string, int64_t> WorkMap;	
		typedef WorkMap::iterator WorkIter;

		WorkMap w;
		CriticalSection cs;
		Semaphore s;

		bool stop;
		bool running;
		bool rebuild;
		int64_t total;
		string file;
	};

	friend class Hasher;

	class HashStore {
	public:
		HashStore();
		void addFile(const string& aFileName, const TigerTree& tth, bool aUsed);

		void load();
		void save();

		void rebuild();

		bool checkTTH(const string& aFileName, int64_t aSize, u_int32_t aTimeStamp) {
			TTHIter i = indexTTH.find(aFileName);
			if(i != indexTTH.end()) {
				if(i->second->getSize() != aSize || i->second->getTimeStamp() != aTimeStamp) {
					delete i->second;
					indexTTH.erase(i);
					dirty = true;
					return false;
				}
				return true;
			} 
			return false;
		}

		TTHValue* getTTH(const string& aFileName) {
			TTHIter i = indexTTH.find(aFileName);
			if(i != indexTTH.end()) {
				i->second->setUsed(true);
				return &(i->second->getRoot());
			}
			return NULL;
		}

		bool getTree(const string& aFileName, const TTHValue* root, TigerTree& tth);
		bool isDirty() { return dirty; };
	private:
		class FileInfo : public FastAlloc<FileInfo> {
		public:
			FileInfo(const TTHValue& aRoot, int64_t aSize, int64_t aIndex, size_t aBlockSize, u_int32_t aTimeStamp, bool aUsed) :
			  root(aRoot), size(aSize), index(aIndex), blockSize(aBlockSize), timeStamp(aTimeStamp), used(aUsed) { }

			TTHValue& getRoot() { return root; }
			void setRoot(const TTHValue& aRoot) { root = aRoot; }
		private:
			TTHValue root;
			GETSET(int64_t, size, Size)
			GETSET(int64_t, index, Index);
			GETSET(size_t, blockSize, BlockSize);
			GETSET(u_int32_t, timeStamp, TimeStamp);
			GETSET(bool, used, Used);
		};

		typedef HASH_MAP_X(string, FileInfo*, noCaseStringHash, noCaseStringEq, noCaseStringLess) TTHMap;
		typedef TTHMap::iterator TTHIter;

		friend class HashLoader;

		TTHMap indexTTH;

		string indexFile;
		string dataFile;

		bool dirty;

		void createDataFile(const string& name);
		int64_t addLeaves(const TigerTree::MerkleList& leaves);
	};

	friend class HashLoader;

	Hasher hasher;
	HashStore store;

	CriticalSection cs;

	void hashDone(const string& aFileName, const TigerTree& tth, int64_t speed);

	virtual void on(TimerManagerListener::Minute, u_int32_t) throw() {
		Lock l(cs);
		store.save();
	}
};

#endif // _HASH_MANAGER

/**
 * @file
 * $Id: HashManager.h,v 1.2 2004/11/12 16:29:31 phase Exp $
 */
