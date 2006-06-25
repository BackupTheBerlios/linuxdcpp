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

#ifndef WULFOR_FINISHED_TRANSFERS
#define WULFOR_FINISHED_TRANSFERS

#include "bookentry.hh"
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

	private:
		// GUI functions
		void updateList_gui(FinishedItem::List& list);
		void addEntry_gui(FinishedItem *entry);
		void updateStatus_gui();

		// GUI callbacks
		static gboolean onPopupMenu_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onRemoveItems_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveAll_gui(GtkMenuItem *item, gpointer data);
		static void onOpenWith_gui(GtkMenuItem *item, gpointer data);

		// Client callbacks
		virtual void on(FinishedManagerListener::AddedDl, FinishedItem* entry) throw();
		virtual void on(FinishedManagerListener::AddedUl, FinishedItem* entry) throw();

		// For open with thread. Not _gui or _client since it runs in its own thread.
		static void *runCommand(void *command);

		GtkWidget *mainBox;
		GtkListStore *transferStore;
		TreeView transferView;
		GtkTreeSelection *transferSelection;
		GtkStatusbar *totalItems, *totalSize, *averageSpeed;
		GtkDialog *openWithDialog;
		GtkEntry *openWithEntry;
		pthread_t openWithThread;
		bool isUpload;
		int items;
		int64_t totalBytes, totalTime;
		GtkMenu *finishedTransfersMenu;
		GtkMenuItem *openWith, *remove, *removeAll;
};

#else
class FinishedTransfers;
#endif
