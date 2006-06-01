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

#ifndef WULFOR_FINISHED_TRANSFERS
#define WULFOR_FINISHED_TRANSFERS

#include <ext/hash_map>

#include "bookentry.hh"
#include "callback.hh"
#include "func.hh"
#include "wulformanager.hh"

#include <client/FinishedManager.h>

class FinishedTransfers:
	public BookEntry,
	public FinishedManagerListener
{
	public:
	
		FinishedTransfers(std::string title);
		~FinishedTransfers();
	
		// From BookEntry
		GtkWidget *getWidget();
	
		void popupMenu_gui(GtkWidget *, GdkEventButton *, gpointer);
		void removeItems_gui(GtkMenuItem *, gpointer);
		void removeAll_gui(GtkMenuItem *, gpointer);
		void updateList(FinishedItem::List& list);
		void addEntry(FinishedItem *entry);
		void updateStatus();
		void openWith_gui(GtkMenuItem *, gpointer);
	
		//from FinishedManagerListener
		void on(AddedDl, FinishedItem* entry) throw();
		void on(AddedUl, FinishedItem* entry) throw();
	
	private:
		GtkWidget *mainBox;
		GtkListStore *transferStore;
		TreeView transferView;
		GtkTreeSelection *transferSelection;
		GtkTreeIter treeIter;
		GtkStatusbar *totalItems, *totalSize, *averageSpeed;
		GtkDialog *openWithDialog;
		GtkEntry *openWithEntry;
		pthread_t openWithThread;
		static void *runCommand(void *command);

		int items;
		bool getType;
		int64_t totalBytes, totalTime;
		hash_map<std::string, FinishedItem *, WulforUtil::HashString> finishedList;
		
		GtkMenu *finishedTransfersMenu;
		GtkMenuItem *openWith, *remove, *removeAll;
	
		Callback3<FinishedTransfers, void, GtkWidget *, GdkEventButton *> menuCallback;
		Callback2<FinishedTransfers, void, GtkMenuItem *> removeCallback, removeAllCallback, openWithCallback;	
};
#else
class FinishedTransfers;
#endif
