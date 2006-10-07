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

#include "finishedtransfers.hh"

FinishedTransfers::FinishedTransfers(std::string title):
	BookEntry(title, "finishedtransfers.glade"),
	items(0),
	totalBytes(0),
	totalTime(0)
{
	FinishedManager::getInstance()->addListener(this);

	// Configure the dialog
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("openWithDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	// Initialize transfer treeview
	transferView.setView(GTK_TREE_VIEW(getWidget("view")), true, "finished");
	transferView.insertColumn("Time", G_TYPE_STRING, TreeView::STRING, 150);
	transferView.insertColumn("Filename", G_TYPE_STRING, TreeView::STRING, 100);
	transferView.insertColumn("Path", G_TYPE_STRING, TreeView::STRING, 200);
	transferView.insertColumn("Nick", G_TYPE_STRING, TreeView::STRING, 100);
	transferView.insertColumn("Hub", G_TYPE_STRING, TreeView::STRING, 200);
	transferView.insertColumn("Size", G_TYPE_STRING, TreeView::STRING, 100);
	transferView.insertColumn("Speed", G_TYPE_STRING, TreeView::STRING, 100);
	transferView.insertColumn("CRC Checked", G_TYPE_STRING, TreeView::STRING, 100);
	transferView.insertHiddenColumn("Target", G_TYPE_STRING);
	transferView.insertHiddenColumn("FinishedItem", G_TYPE_POINTER);
	transferView.finalize();
	transferStore = gtk_list_store_newv(transferView.getColCount(), transferView.getGTypes());
	gtk_tree_view_set_model(transferView.get(), GTK_TREE_MODEL(transferStore));
	g_object_unref(transferStore);
	transferSelection = gtk_tree_view_get_selection(transferView.get());
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(transferView.get(), transferView.col("Time")), TRUE);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(transferStore), transferView.col("Time"), GTK_SORT_ASCENDING);
	gtk_tree_view_set_fixed_height_mode(transferView.get(), TRUE);

	// Connect the signals to their callback functions.
	g_signal_connect(getWidget("openWithItem"), "activate", G_CALLBACK(onOpenWith_gui), (gpointer)this);
	g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveItems_gui), (gpointer)this);
	g_signal_connect(getWidget("removeAllItem"), "activate", G_CALLBACK(onRemoveAll_gui), (gpointer)this);
	g_signal_connect(transferView.get(), "button-release-event", G_CALLBACK(onPopupMenu_gui), (gpointer)this);
	g_signal_connect(transferView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);

	// Update the list of finished items.
	isUpload = (getID() == string("Finished Uploads")) ? TRUE : FALSE;
	updateList_gui(FinishedManager::getInstance()->lockList(isUpload));
	FinishedManager::getInstance()->unlockList();
}

FinishedTransfers::~FinishedTransfers()
{
	FinishedManager::getInstance()->removeListener(this);
	gtk_widget_destroy(getWidget("openWithDialog"));
}

gboolean FinishedTransfers::onPopupMenu_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;

	if (event->button == 3 && gtk_tree_selection_get_selected(ft->transferSelection, NULL, NULL))
	{
		gtk_menu_popup(GTK_MENU(ft->getWidget("menu")), NULL, NULL, NULL, NULL, event->button, event->time);
		gtk_widget_show_all(ft->getWidget("menu"));
	}
	return FALSE;
}

gboolean FinishedTransfers::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;

	if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		onOpenWith_gui(NULL, data);
	else if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		onRemoveItems_gui(NULL, data);
	else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
	{
		gtk_menu_popup(GTK_MENU(ft->getWidget("menu")), NULL, NULL, NULL, NULL, 1, event->time);
		gtk_widget_show_all(ft->getWidget("menu"));
	}
	return FALSE;
}

void FinishedTransfers::onRemoveItems_gui(GtkMenuItem *item, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	GtkTreeIter iter;
	FinishedItem *entry;

	if (gtk_tree_selection_get_selected(ft->transferSelection, NULL, &iter))
	{
		entry = ft->transferView.getValue<gpointer, FinishedItem *>(&iter, "FinishedItem");

		ft->totalBytes -= entry->getChunkSize();
		ft->totalTime -= entry->getMilliSeconds();
		ft->items--;
		FinishedManager::getInstance()->remove(entry, ft->isUpload);
		gtk_list_store_remove(ft->transferStore, &iter);
		ft->updateStatus_gui();
	}
}

void FinishedTransfers::onRemoveAll_gui(GtkMenuItem *item, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;

	FinishedManager::getInstance()->removeAll(ft->isUpload);
	gtk_list_store_clear(ft->transferStore);
	ft->totalBytes = 0;
	ft->totalTime = 0;
	ft->items = 0;
	ft->updateStatus_gui();
}

void FinishedTransfers::updateList_gui(FinishedItem::List& list)
{
	for (FinishedItem::List::const_iterator iter = list.begin(); iter != list.end(); iter++)
		addEntry_gui(*iter);
}

void FinishedTransfers::addEntry_gui(FinishedItem *entry)
{
	GtkTreeIter iter;
	gtk_list_store_append(transferStore, &iter);
	gtk_list_store_set(transferStore, &iter,
		transferView.col("Filename"), Util::getFileName(entry->getTarget()).c_str(),
		transferView.col("Time"), Util::formatTime("%Y-%m-%d %H:%M:%S", entry->getTime()).c_str(),
		transferView.col("Path"), Util::getFilePath(entry->getTarget()).c_str(),
		transferView.col("Nick"), entry->getUser().c_str(),
		transferView.col("Hub"), entry->getHub().c_str(),
		transferView.col("Size"), Util::formatBytes(entry->getSize()).c_str(),
		transferView.col("Speed"), (Util::formatBytes(entry->getAvgSpeed()) + "/s").c_str(),
		transferView.col("CRC Checked"), entry->getCrc32Checked() ? "Yes" : "No",
		transferView.col("Target"), entry->getTarget().c_str(),
		transferView.col("FinishedItem"), (gpointer)entry,
		-1);

	totalBytes += entry->getChunkSize();
	totalTime += entry->getMilliSeconds();
	items++;
	updateStatus_gui();

	if (!isUpload && BOOLSETTING(BOLD_FINISHED_DOWNLOADS) || isUpload && BOOLSETTING(BOLD_FINISHED_UPLOADS))
		setBold_gui();
}

void FinishedTransfers::updateStatus_gui()
{
	string status = Util::toString(items) + " Items";
	gtk_statusbar_push(GTK_STATUSBAR(getWidget("totalItems")), 0, status.c_str());
	gtk_statusbar_push(GTK_STATUSBAR(getWidget("totalSize")), 0, Text::utf8ToAcp(Util::formatBytes(totalBytes)).c_str());
	gtk_statusbar_push(GTK_STATUSBAR(getWidget("averageSpeed")), 0, Text::utf8ToAcp(Util::formatBytes((totalTime > 0) ? totalBytes * ((int64_t)1000) / totalTime : 0) + "/s").c_str());
}

void FinishedTransfers::onOpenWith_gui(GtkMenuItem *item, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	GtkTreeIter iter;
	string command;
	string target;
	int ret;

	ret = gtk_dialog_run(GTK_DIALOG(ft->getWidget("openWithDialog")));
	gtk_widget_hide(ft->getWidget("openWithDialog"));

	if (ret == GTK_RESPONSE_OK)
	{
		command = gtk_entry_get_text(GTK_ENTRY(ft->getWidget("openWithEntry")));
		gtk_tree_selection_get_selected(ft->transferSelection, NULL, &iter);
		target = ft->transferView.getString(&iter, "Target");

		if (!command.empty() && !target.empty())
		{
			command += Text::utf8ToAcp(" \"" + target + "\"");
			g_spawn_command_line_async(command.c_str(), NULL);
		}
	}
}

void FinishedTransfers::on(FinishedManagerListener::AddedDl, FinishedItem *entry) throw()
{
	if (!isUpload)
	{
		typedef Func1<FinishedTransfers, FinishedItem *> F1;
		F1 *f1 = new F1(this, &FinishedTransfers::addEntry_gui, entry);
		WulforManager::get()->dispatchGuiFunc(f1);
	}
}

void FinishedTransfers::on(FinishedManagerListener::AddedUl, FinishedItem *entry) throw()
{
	if (isUpload)
	{
		typedef Func1<FinishedTransfers, FinishedItem *> F1;
		F1 *f1 = new F1(this, &FinishedTransfers::addEntry_gui, entry);
		WulforManager::get()->dispatchGuiFunc(f1);
	}
}
