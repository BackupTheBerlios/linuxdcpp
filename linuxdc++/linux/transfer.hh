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

#ifndef WULFOR_TRANSFER
#define WULFOR_TRANSFER

#include <gtkmm.h>
#include <string>

#include "util.hh"
#include "sharebrowser.hh"

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"
#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/CriticalSection.h"
#include "../client/ConnectionManager.h"
#include "../client/ClientManager.h"
#include "../client/QueueManager.h"


class CTransfer : 	public DownloadManagerListener,
							public UploadManagerListener,
							public ConnectionManagerListener,
							public SigC::Object
{
	public:
		CTransfer ();
		~CTransfer ();

		enum {
			MENU_FIRST,
			MENU_CLOSE_TRANSFER = MENU_FIRST,
			MENU_BROWSE_FILELIST,
			MENU_REMOVE_FROM_QUEUE,
			MENU_LAST
		};
			
			

		static CTransfer *getInstance ();
		static void startup ();
		void shutdown ();

		Gtk::ScrolledWindow &getTransferScroll ();
		
		friend class TransferItem;
		
	private:
		class TransferItem
		{
			public:
				typedef HASH_MAP<ConnectionQueueItem*, TransferItem*, PointerHash<ConnectionQueueItem> > Map;
				typedef Map::iterator MapIter;
				
				enum Status {
					STATUS_RUNNING,
					STATUS_WAITING
				};
				enum Types {
					TYPE_DOWNLOAD,
					TYPE_UPLOAD
				};

				enum {
					COLUMN_FIRST,
					COLUMN_USER = COLUMN_FIRST,
					COLUMN_HUB,
					COLUMN_STATUS,
					COLUMN_TIMELEFT,
					COLUMN_SPEED,
					COLUMN_FILE,
					COLUMN_SIZE,
					COLUMN_PATH,
					COLUMN_IP,
					COLUMN_RATIO,
					COLUMN_LAST
				};
				
				enum {
					MASK_USER = 1 << COLUMN_USER,
					MASK_HUB = 1 << COLUMN_HUB,
					MASK_STATUS = 1 << COLUMN_STATUS,
					MASK_TIMELEFT = 1 << COLUMN_TIMELEFT,
					MASK_SPEED = 1 << COLUMN_SPEED,
					MASK_FILE = 1 << COLUMN_FILE,
					MASK_SIZE = 1 << COLUMN_SIZE,
					MASK_PATH = 1 << COLUMN_PATH,
					MASK_IP = 1 << COLUMN_IP,
					MASK_RATIO = 1 << COLUMN_RATIO,
				};
				
				TransferItem(const User::Ptr& u, Types t = TYPE_DOWNLOAD, Status s = STATUS_WAITING,
						 int64_t p = 0, int64_t sz = 0, int st = 0, int a = 0) : user(u), type(t),
				status(s), pos(p), size(sz), start(st), actual(a), speed(0), timeLeft(0) { };

				User::Ptr user;
				Types type;
				Status status;
				int64_t pos;
				int64_t size;
				int64_t start;
				int64_t actual;
				int64_t speed;
				int64_t timeLeft;
				string statusString;
				string file;
				string path;
				string IP;
				u_int32_t updateMask;

				void update();
				void disconnect();
				void remove();

				double getRatio () { return (pos > 0) ? (double)actual/(double)pos : 1.0; }
		};

		class TransferColumns: public Gtk::TreeModel::ColumnRecord
		{
			public:
				TransferColumns() {
					add(user);
					add(hub);
					add(status);
					add(timeleft);
					add(speed);
					add(file);
					add(size);
					add(path);
					add(ip);
					add(ratio);
					add(direction);
				}

				Gtk::TreeModelColumn<Glib::ustring> 	user,
				hub,
				status,
				timeleft,
				speed,
				file,
				size,
				path,
				ip,
				ratio;

				Gtk::TreeModelColumn<TransferItem::Types> direction; // Hidden from the user.
		};
		
		TransferItem::Map transfer;
		
		//Connection manager.
		virtual void on(ConnectionManagerListener::Added, ConnectionQueueItem *item) throw();
		virtual void on(ConnectionManagerListener::Removed, ConnectionQueueItem *item) throw();
		virtual void on(ConnectionManagerListener::Failed, ConnectionQueueItem *item, const string &reason) throw();
		virtual void on(ConnectionManagerListener::StatusChanged, 	ConnectionQueueItem *item) throw();

		// Download manager
		virtual void on(DownloadManagerListener::Starting, Download *dl) throw();
		virtual void on(DownloadManagerListener::Tick, const Download::List &list) throw();
		virtual void on(DownloadManagerListener::Complete, Download *dl) throw() { onTransferComplete (dl, false); }
		virtual void on(DownloadManagerListener::Failed, Download *dl, const string &reason) throw();

		// Upload manager
		virtual void on(UploadManagerListener::Starting, Upload *ul) throw();
		virtual void on(UploadManagerListener::Tick, const Upload::List &list) throw();
		virtual void on(UploadManagerListener::Complete, Upload *ul) throw() { onTransferComplete (ul, true); }

		void onTransferComplete (Transfer *t, bool upload);

		Gtk::ScrolledWindow transferScroll;
		
		Gtk::TreeView transferList;
		Glib::RefPtr<Gtk::ListStore> transferStore;
		Gtk::TreeModel::iterator findTransfer (TransferItem *t);
		TransferItem *findTransfer (Gtk::TreeModel::iterator i);
		TransferItem* bruteFindTransfer ();
		TransferColumns columns;

		Gtk::Menu popupMenu;
		Gtk::MenuItem menuSeparator;
		Gtk::MenuItem menuForce;
		Gtk::MenuItem menuRemoveTransfer;
		Gtk::MenuItem menuRemoveQueue;
		Gtk::MenuItem menuBrowse;
  		void showPopupMenu(GdkEventButton* event);
		void forceClicked ();
		void removeTransferClicked ();
		void browseClicked ();
		void removeQueueClicked ();
		
		u_int32_t itemMask;

		CriticalSection cs;
		
		static CTransfer *instance;
};

#endif
