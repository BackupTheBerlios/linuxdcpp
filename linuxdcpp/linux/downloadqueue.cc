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

#include "downloadqueue.hh"

#include <client/ShareManager.h>
#include "search.hh"
#include "wulformanager.hh"

DownloadQueue::DownloadQueue():
	BookEntry("Download Queue", "downloadqueue.glade")
{
	QueueManager::getInstance()->addListener(this);

	// Initialize directory treeview
	dirView.setView(GTK_TREE_VIEW(getWidget("dirView")));
	dirView.insertColumn("Dir", G_TYPE_STRING, TreeView::STRING, -1);
	dirView.insertHiddenColumn("Path", G_TYPE_STRING);
	dirView.finalize();
	dirStore = gtk_tree_store_newv(dirView.getColCount(), dirView.getGTypes());
	gtk_tree_view_set_model(dirView.get(), GTK_TREE_MODEL(dirStore));
	g_object_unref(dirStore);
	dirSelection = gtk_tree_view_get_selection(dirView.get());

	// Initialize file treeview
	fileView.setView(GTK_TREE_VIEW(getWidget("fileView")), TRUE, "downloadqueue");
	fileView.insertColumn("Filename", G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn("Status", G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn("Size", G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn("Downloaded", G_TYPE_STRING, TreeView::STRING, 150);
	fileView.insertColumn("Priority", G_TYPE_STRING, TreeView::STRING, 75);
	fileView.insertColumn("Users", G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn("Path", G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn("Exact Size", G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn("Error", G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn("Added", G_TYPE_STRING, TreeView::STRING, 120);
	fileView.insertColumn("TTH", G_TYPE_STRING, TreeView::STRING, 125);
	fileView.insertHiddenColumn("QueueItem", G_TYPE_POINTER);
	fileView.insertHiddenColumn("Real Size", G_TYPE_INT64);
	fileView.insertHiddenColumn("Download Size", G_TYPE_INT64);
	fileView.finalize();
	fileStore = gtk_list_store_newv(fileView.getColCount(), fileView.getGTypes());
	gtk_tree_view_set_model(fileView.get(), GTK_TREE_MODEL(fileStore));
	g_object_unref(fileStore);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(fileView.get()), GTK_SELECTION_MULTIPLE);
	fileSelection = gtk_tree_view_get_selection(fileView.get());
	fileView.setSortColumn_gui("Size", "Real Size");
	fileView.setSortColumn_gui("Exact Size", "Real Size");
	fileView.setSortColumn_gui("Downloaded", "Download Size");

	// Connect the signals to their callback functions.
	g_signal_connect(getWidget("pausedPriorityItem"), "activate", G_CALLBACK(onDirPriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("lowestPriorityItem"), "activate", G_CALLBACK(onDirPriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("lowPrioritytem"), "activate", G_CALLBACK(onDirPriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("normalPriorityItem"), "activate", G_CALLBACK(onDirPriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("highPriorityItem"), "activate", G_CALLBACK(onDirPriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("highestPriorityItem"), "activate", G_CALLBACK(onDirPriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeDirItem"), "activate", G_CALLBACK(onRemoveDirClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchForAlternatesItem"), "activate", G_CALLBACK(onSearchAlternatesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("filePausedItem"), "activate", G_CALLBACK(onFilePriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("fileLowestPriorityItem"), "activate", G_CALLBACK(onFilePriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("fileLowPriorityItem"), "activate", G_CALLBACK(onFilePriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("fileNormalPriorityItem"), "activate", G_CALLBACK(onFilePriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("fileHighPriorityItem"), "activate", G_CALLBACK(onFilePriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("fileHighestPriorityItem"), "activate", G_CALLBACK(onFilePriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("fileRemoveItem"), "activate", G_CALLBACK(onRemoveFileClicked_gui), (gpointer)this);
 	g_signal_connect(dirView.get(), "button-press-event", G_CALLBACK(onDirButtonPressed_gui), (gpointer)this);
	g_signal_connect(dirView.get(), "button-release-event", G_CALLBACK(onDirButtonReleased_gui), (gpointer)this);
	g_signal_connect(dirView.get(), "key-release-event", G_CALLBACK(onDirKeyReleased_gui), (gpointer)this);
	g_signal_connect(fileView.get(), "button-press-event", G_CALLBACK(onFileButtonPressed_gui), (gpointer)this);
	g_signal_connect(fileView.get(), "button-release-event", G_CALLBACK(onFileButtonReleased_gui), (gpointer)this);
	g_signal_connect(fileView.get(), "key-release-event", G_CALLBACK(onFileKeyReleased_gui), (gpointer)this);

	// Set the pane position
	gtk_paned_set_position(GTK_PANED(getWidget("pane")), WGETI("downloadqueue-pane-position"));

	buildList_gui();
	updateStatus_gui();
}

DownloadQueue::~DownloadQueue()
{
	QueueManager::getInstance()->removeListener(this);

	// Save the pane position
	int downloadqueuePanePosition = gtk_paned_get_position(GTK_PANED(getWidget("pane")));
	WSET("downloadqueue-pane-position", downloadqueuePanePosition);
}

void DownloadQueue::buildDynamicMenu_gui()
{
	int count = gtk_tree_selection_count_selected_rows(fileSelection);
	bool showPMMenu = FALSE;
	bool showReAddMenu = FALSE;
	bool showOtherMenus = FALSE;

	if (count == 1)
	{
		GtkTreeIter iter;
		GList *list = gtk_tree_selection_get_selected_rows(fileSelection, NULL);
		GtkTreePath *path = (GtkTreePath *)g_list_nth_data(list, 0);

		gtk_container_foreach(GTK_CONTAINER(getWidget("browseMenu")), (GtkCallback)gtk_widget_destroy, NULL);
		gtk_container_foreach(GTK_CONTAINER(getWidget("pmMenu")), (GtkCallback)gtk_widget_destroy, NULL);
		gtk_container_foreach(GTK_CONTAINER(getWidget("reAddMenu")), (GtkCallback)gtk_widget_destroy, NULL);
		gtk_container_foreach(GTK_CONTAINER(getWidget("removeMenu")), (GtkCallback)gtk_widget_destroy, NULL);
		gtk_container_foreach(GTK_CONTAINER(getWidget("removeAllMenu")), (GtkCallback)gtk_widget_destroy, NULL);

		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fileStore), &iter, path))
		{
			GtkWidget *menuItem;
			string name;
			QueueItem *item = fileView.getValue<gpointer, QueueItem *>(&iter, "QueueItem");

			for (SourceIter it = item->getSources().begin(); it != item->getSources().end(); ++it)
			{
				name = WulforUtil::getNicks((*it)->getUser());
				showOtherMenus = TRUE;

				menuItem = gtk_menu_item_new_with_label(name.c_str());
				gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("browseMenu")), menuItem);
				g_signal_connect(menuItem, "activate", G_CALLBACK(onGetFileListClicked_gui), (gpointer)this);

				if ((*it)->getUser()->isOnline())
				{
					showPMMenu = TRUE;
					menuItem = gtk_menu_item_new_with_label(name.c_str());
					gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("pmMenu")), menuItem);
					g_signal_connect(menuItem, "activate", G_CALLBACK(onSendPrivateMessageClicked_gui), (gpointer)this);
				}

				menuItem = gtk_menu_item_new_with_label(name.c_str());
				gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("removeMenu")), menuItem);
				g_signal_connect(menuItem, "activate", G_CALLBACK(onRemoveSourceClicked_gui), (gpointer)this);

				menuItem = gtk_menu_item_new_with_label(name.c_str());
				gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("removeAllMenu")), menuItem);
				g_signal_connect(menuItem, "activate", G_CALLBACK(onRemoveUserFromQueueClicked_gui), (gpointer)this);
			}

			for (SourceIter it = item->getBadSources().begin(); it != item->getBadSources().end(); ++it)
			{
				showReAddMenu = TRUE;
				name = WulforUtil::getNicks((*it)->getUser());
				menuItem = gtk_menu_item_new_with_label(name.c_str());
				gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("reAddMenu")), menuItem);
				g_signal_connect(menuItem, "activate", G_CALLBACK(onReAddSourceClicked_gui), (gpointer)this);
			}
		}
		gtk_tree_path_free(path);
		g_list_free(list);

		gtk_widget_show_all(getWidget("browseMenu"));
		gtk_widget_show_all(getWidget("pmMenu"));
		gtk_widget_show_all(getWidget("reAddMenu"));
		gtk_widget_show_all(getWidget("removeMenu"));
		gtk_widget_show_all(getWidget("removeAllMenu"));
		gtk_widget_set_sensitive(getWidget("searchForAlternatesItem"), TRUE);
	}
	else
		gtk_widget_set_sensitive(getWidget("searchForAlternatesItem"), FALSE);

	gtk_widget_set_sensitive(getWidget("getFileListItem"), showOtherMenus);
	gtk_widget_set_sensitive(getWidget("sendPrivateMessageItem"), showPMMenu);
	gtk_widget_set_sensitive(getWidget("reAddSourceItem"), showReAddMenu);
	gtk_widget_set_sensitive(getWidget("removeSourceItem"), showOtherMenus);
	gtk_widget_set_sensitive(getWidget("removeUserFromQueueItem"), showOtherMenus);
}

void DownloadQueue::setDirPriority_gui(string path, QueueItem::Priority priority)
{
	vector<string> targets;
	typedef Func2<DownloadQueue, string, QueueItem::Priority> F2;
	F2 *func;

	getChildren(path, &targets);
	for (size_t i = 0; i < targets.size(); i++)
	{
		func = new F2(this, &DownloadQueue::setPriority_client, targets[i], priority);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void DownloadQueue::onDirPriorityClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dq->dirSelection, NULL, &iter))
	{
		string path = dq->dirView.getString(&iter, "Path");

		if (item == GTK_MENU_ITEM(dq->getWidget("pausedPriorityItem")))
			dq->setDirPriority_gui(path, QueueItem::PAUSED);
		else if (item == GTK_MENU_ITEM(dq->getWidget("lowestPriorityItem")))
			dq->setDirPriority_gui(path, QueueItem::LOWEST);
		else if (item == GTK_MENU_ITEM(dq->getWidget("lowPrioritytem")))
			dq->setDirPriority_gui(path, QueueItem::LOW);
		else if (item == GTK_MENU_ITEM(dq->getWidget("normalPriorityItem")))
			dq->setDirPriority_gui(path, QueueItem::NORMAL);
		else if (item == GTK_MENU_ITEM(dq->getWidget("highPriorityItem")))
			dq->setDirPriority_gui(path, QueueItem::HIGH);
		else if (item == GTK_MENU_ITEM(dq->getWidget("highestPriorityItem")))
			dq->setDirPriority_gui(path, QueueItem::HIGHEST);
	}

}
void DownloadQueue::onFilePriorityClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	string target;
	GtkTreePath *path;
	GtkTreeIter iter;
	typedef Func2<DownloadQueue, string, QueueItem::Priority> F2;
	F2 *func;
	QueueItem::Priority priority;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);
	int count = gtk_tree_selection_count_selected_rows(dq->fileSelection);

	for (int i = 0; i < count; i++)
	{
		path = (GtkTreePath *)g_list_nth_data(list, i);
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
		{
			target = dq->fileView.getValue<gpointer, QueueItem *>(&iter, "QueueItem")->getTarget();

			if (item == GTK_MENU_ITEM(dq->getWidget("filePausedItem")))
				priority = QueueItem::PAUSED;
			else if (item == GTK_MENU_ITEM(dq->getWidget("fileLowestPriorityItem")))
				priority = QueueItem::LOWEST;
			else if (item == GTK_MENU_ITEM(dq->getWidget("fileLowPriorityItem")))
				priority = QueueItem::LOW;
			else if (item == GTK_MENU_ITEM(dq->getWidget("fileNormalPriorityItem")))
				priority = QueueItem::NORMAL;
			else if (item == GTK_MENU_ITEM(dq->getWidget("fileHighPriorityItem")))
				priority = QueueItem::HIGH;
			else if (item == GTK_MENU_ITEM(dq->getWidget("fileHighestPriorityItem")))
				priority = QueueItem::HIGHEST;

			func = new F2(dq, &DownloadQueue::setPriority_client, target, priority);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void DownloadQueue::onRemoveDirClicked_gui(GtkMenuItem *menuitem, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dq->dirSelection, NULL, &iter))
	{
		typedef Func1<DownloadQueue, string> F1;
		F1 *func;
		vector<string> targets;

		dq->getChildren(dq->dirView.getString(&iter, "Path"), &targets);
		for (size_t i = 0; i < targets.size(); i++)
		{
			func = new F1(dq, &DownloadQueue::remove_client, targets[i]);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void DownloadQueue::onRemoveFileClicked_gui(GtkMenuItem *menuitem, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	string target;
	typedef Func1<DownloadQueue, string> F1;
	F1 *func;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);
	int count = gtk_tree_selection_count_selected_rows(dq->fileSelection);

	for (int i = 0; i < count; i++)
	{
		path = (GtkTreePath *)g_list_nth_data(list, i);
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
		{
			target = dq->fileView.getValue<gpointer, QueueItem *>(&iter, "QueueItem")->getTarget();
			func = new F1(dq, &DownloadQueue::remove_client, target);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void DownloadQueue::onSearchAlternatesClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);
	int count = gtk_tree_selection_count_selected_rows(dq->fileSelection);

	dcassert(count == 1);

	path = (GtkTreePath *)g_list_nth_data(list, 0);
	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
	{
		QueueItem *item = dq->fileView.getValue<gpointer, QueueItem *>(&iter, "QueueItem");

		if (item->getTTH() != NULL)
		{
			Search *s = dynamic_cast<Search*>(WulforManager::get()->addSearch_gui());
			s->putValue_gui(item->getTTH()->toBase32(), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
		}
		else
		{
			string target = dq->fileView.getString(&iter, "Filename");
			string searchString = SearchManager::clean(target);

			if (!searchString.empty())
			{
				bool bigFile = (item->getSize() > 10 * 1024 * 1024);
				Search *s = dynamic_cast<Search*>(WulforManager::get()->addSearch_gui());

				if (bigFile)
					s->putValue_gui(searchString, item->getSize() - 1, SearchManager::SIZE_ATLEAST, ShareManager::getInstance()->getType(target));
				else
					s->putValue_gui(searchString, item->getSize() + 1, SearchManager::SIZE_ATMOST, ShareManager::getInstance()->getType(target));
			}
		}
	}
	gtk_tree_path_free(path);
	g_list_free(list);
}

void DownloadQueue::onGetFileListClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);
	int count = gtk_tree_selection_count_selected_rows(dq->fileSelection);

	dcassert(count == 1);

	string name = WulforUtil::getTextFromMenu(item);
	path = (GtkTreePath *)g_list_nth_data(list, 0);
	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
	{
		QueueItem *item = dq->fileView.getValue<gpointer, QueueItem *>(&iter, "QueueItem");
		for (SourceIter it = item->getSources().begin(); it != item->getSources().end(); ++it)
		{
			if (WulforUtil::getNicks((*it)->getUser()) == name)
			{
				typedef Func1<DownloadQueue, const User::Ptr &> F1;
				F1 *func = new F1(dq, &DownloadQueue::addList_client, (*it)->getUser());
				WulforManager::get()->dispatchClientFunc(func);
				break;
			}
		}
	}
	gtk_tree_path_free(path);
	g_list_free(list);
}

void DownloadQueue::onSendPrivateMessageClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);
	int count = gtk_tree_selection_count_selected_rows(dq->fileSelection);

	dcassert(count == 1);

	string name = WulforUtil::getTextFromMenu(item);
	path = (GtkTreePath *)g_list_nth_data(list, 0);
	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
	{
		QueueItem *item = dq->fileView.getValue<gpointer, QueueItem *>(&iter, "QueueItem");
		for (SourceIter it = item->getSources().begin(); it != item->getSources().end(); ++it)
		{
			if (WulforUtil::getNicks((*it)->getUser()) == name)
			{
				WulforManager::get()->addPrivMsg_gui((*it)->getUser());
				break;
			}
		}
	}
	gtk_tree_path_free(path);
	g_list_free(list);
}

void DownloadQueue::onReAddSourceClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);
	int count = gtk_tree_selection_count_selected_rows(dq->fileSelection);

	dcassert(count == 1);

	string name = WulforUtil::getTextFromMenu(item);
	path = (GtkTreePath *)g_list_nth_data(list, 0);
	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
	{
		QueueItem *item = dq->fileView.getValue<gpointer, QueueItem *>(&iter, "QueueItem");
		for (SourceIter it = item->getBadSources().begin(); it != item->getBadSources().end(); ++it)
		{
			if (WulforUtil::getNicks((*it)->getUser()) == name)
			{
				typedef Func2<DownloadQueue, string, User::Ptr&> F2;
				F2 *func = new F2(dq, &DownloadQueue::reAddSource_client, item->getTarget(), (*it)->getUser());
				WulforManager::get()->dispatchClientFunc(func);
				break;
			}
		}
	}
	gtk_tree_path_free(path);
	g_list_free(list);
}

void DownloadQueue::onRemoveSourceClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);
	int count = gtk_tree_selection_count_selected_rows(dq->fileSelection);

	dcassert(count == 1);

	string name = WulforUtil::getTextFromMenu(item);
	path = (GtkTreePath *)g_list_nth_data(list, 0);
	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
	{
		QueueItem *item = dq->fileView.getValue<gpointer, QueueItem *>(&iter, "QueueItem");
		for (SourceIter it = item->getSources().begin(); it != item->getSources().end(); ++it)
		{
			if (WulforUtil::getNicks((*it)->getUser()) == name)
			{
				typedef Func2<DownloadQueue, string, User::Ptr&> F2;
				F2 *func = new F2(dq, &DownloadQueue::removeSource_client, item->getTarget(), (*it)->getUser());
				WulforManager::get()->dispatchClientFunc(func);
				break;
			}
		}
	}
	gtk_tree_path_free(path);
	g_list_free(list);
}

void DownloadQueue::onRemoveUserFromQueueClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);
	int count = gtk_tree_selection_count_selected_rows(dq->fileSelection);

	dcassert(count == 1);

	string name = WulforUtil::getTextFromMenu(item);
	path = (GtkTreePath *)g_list_nth_data(list, 0);
	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
	{
		QueueItem *item = dq->fileView.getValue<gpointer, QueueItem *>(&iter, "QueueItem");
		for (SourceIter it = item->getSources().begin(); it != item->getSources().end(); ++it)
		{
			if (WulforUtil::getNicks((*it)->getUser()) == name)
			{
				typedef Func1<DownloadQueue, User::Ptr&> F1;
				F1 *func = new F1(dq, &DownloadQueue::removeSources_client, (*it)->getUser());
				WulforManager::get()->dispatchClientFunc(func);
				break;
			}
		}
	}
	gtk_tree_path_free(path);
	g_list_free(list);
}

gboolean DownloadQueue::onDirButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	dq->dirPrevious = event->type;

	return FALSE;
}

gboolean DownloadQueue::onDirButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;

	if (dq->dirPrevious == GDK_BUTTON_PRESS)
	{
		if (event->button == 1)
		{
			dq->update_gui();
			dq->updateStatus_gui();
		}
		else if (event->button == 3)
		{
			if (gtk_tree_selection_count_selected_rows(dq->dirSelection) == 1)
			{
				gtk_menu_popup(GTK_MENU(dq->getWidget("dirMenu")), NULL, NULL,
					NULL, NULL, 0, gtk_get_current_event_time());
			}
		}
	}

	return FALSE;
}

gboolean DownloadQueue::onDirKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	int count = gtk_tree_selection_count_selected_rows(dq->dirSelection);

	if (count > 0)
	{
		if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
			dq->onRemoveDirClicked_gui(NULL, data);
		else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
			gtk_menu_popup(GTK_MENU(dq->getWidget("dirMenu")), NULL, NULL,
				NULL, NULL, 0, gtk_get_current_event_time());
	}

	return FALSE;
}

gboolean DownloadQueue::onFileButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;

	if (event->button == 3)
	{
		GtkTreePath *path;
		if (gtk_tree_view_get_path_at_pos(dq->fileView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(dq->fileSelection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}
	return FALSE;
}

gboolean DownloadQueue::onFileButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;

	if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
	{
		if (gtk_tree_selection_count_selected_rows(dq->fileSelection) > 0)
		{
			dq->buildDynamicMenu_gui();
			gtk_menu_popup(GTK_MENU(dq->getWidget("fileMenu")), NULL, NULL,
				NULL, NULL, 0, gtk_get_current_event_time());
			return TRUE;
		}
	}
	return FALSE;
}

gboolean DownloadQueue::onFileKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	int count = gtk_tree_selection_count_selected_rows(dq->fileSelection);

	if (count > 0)
	{
		if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		{
			dq->onRemoveFileClicked_gui(NULL, data);
		}
		else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			dq->buildDynamicMenu_gui();
			gtk_menu_popup(GTK_MENU(dq->getWidget("fileMenu")), NULL, NULL,
				NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

void DownloadQueue::buildList_gui()
{
	const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();
	GtkTreeIter row;
	queueItems = 0;
	queueSize = 0;
	for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it)
	{
		string realpath = "/" + getNextSubDir(Util::getFilePath(it->second->getTarget())) + "/";
		if (dirMap.find(realpath) == dirMap.end())
		{
			gtk_tree_store_append(dirStore, &row, NULL);
			gtk_tree_store_set(dirStore, &row,
				dirView.col("Dir"), Text::acpToUtf8(getNextSubDir(Util::getFilePath(it->second->getTarget()))).c_str(),
				dirView.col("Path"), realpath.c_str(),
				-1);
			dirMap[realpath] = row;
			string tmp;
			addDir_gui(getRemainingDir(Util::getFilePath(it->second->getTarget())), &row, tmp);
			fileMap[it->second->getTarget()] = it->second;
			addFile_gui(it->second, tmp);
		}
		else
		{
			string tmp;
			addDir_gui(getRemainingDir(Util::getFilePath(it->second->getTarget())), &dirMap[realpath], tmp);
			fileMap[it->second->getTarget()] = it->second;
			addFile_gui(it->second, tmp);
		}
	}
	gtk_tree_view_expand_all(dirView.get());
	QueueManager::getInstance()->unlockQueue();
}

void DownloadQueue::addDir_gui(string path, GtkTreeIter *row, string &current)
{
	GtkTreeIter newRow;
	string tmp = getNextSubDir(path);
	string rowdata = dirView.getString(row, "Path");
	string realpath = rowdata + tmp + "/";
	if (tmp == "")
	{
		current = rowdata;
		return;
	}

	if (dirMap.find(realpath) == dirMap.end())
	{
		gtk_tree_store_append(dirStore, &newRow, row);
		gtk_tree_store_set(dirStore, &newRow,
			dirView.col("Dir"), Text::acpToUtf8(tmp).c_str(),
			dirView.col("Path"), realpath.c_str(),
			-1);
		dirMap[realpath] = newRow;
		addDir_gui(getRemainingDir(path), &newRow, current);
	}
	else
		addDir_gui(getRemainingDir(path), &dirMap[realpath], current);
}

void DownloadQueue::addFile_gui(QueueItem *item, string path)
{
	queueSize += item->getSize();
	queueItems++;
	dirFileMap[path].push_back(item);
}

string DownloadQueue::getTrailingSubDir(string path)
{
	string dir = "";

	if (!path.empty())
		dir = path.substr(0, path.find_last_of('/', path.size() - 2)) + "/";

	return dir;
}

string DownloadQueue::getNextSubDir(string path)
{
	string dir = path.substr(1, path.find_first_of('/', 1));

	if (!dir.empty())
		dir.erase(dir.size() - 1, 1);

	return dir;
}

string DownloadQueue::getRemainingDir(string path)
{
	return path.substr(path.find_first_of('/', 1), path.size() - path.find_first_of('/', 1));
}

void DownloadQueue::getChildren(string path, vector<GtkTreeIter> *iter)
{
	if (dirMap.find(path) != dirMap.end())
	{
		GtkTreeModel *m = gtk_tree_view_get_model(dirView.get());
		GtkTreeIter it;
		gboolean valid = gtk_tree_model_iter_children(m, &it, &dirMap[path]);

		while (valid)
		{
			iter->push_back(it);
			getChildren(dirView.getString(&it, "Path"), iter);
			valid = gtk_tree_model_iter_next(m, &it);
		}
	}
}

void DownloadQueue::getChildren(string path, vector<string> *target)
{
	if (dirMap.find(path) != dirMap.end())
	{
		if (target->empty())
		{
			for (vector<QueueItem*>::iterator it = dirFileMap[path].begin(); it != dirFileMap[path].end(); ++it)
	 			target->push_back((*it)->getTarget());
		}

		GtkTreeModel *m = gtk_tree_view_get_model(dirView.get());
		GtkTreeIter it;
		gboolean valid = gtk_tree_model_iter_children(m, &it, &dirMap[path]);

		while (valid)
		{
			string lp = dirView.getString(&it, "Path");
			for (vector<QueueItem*>::iterator it2 = dirFileMap[lp].begin(); it2 != dirFileMap[lp].end(); ++it2)
				target->push_back((*it2)->getTarget());
			getChildren(lp, target);
			valid = gtk_tree_model_iter_next(m, &it);
		}
	}
}

void DownloadQueue::updateItem_gui(QueueItem *item, bool add)
{
	GtkTreeIter iter;
	string tmp;

	if (add)
	{
		gtk_list_store_append(fileStore, &iter);
		gtk_list_store_set(fileStore, &iter,
			fileView.col("QueueItem"), (gpointer)item,
			fileView.col("Filename"), Text::acpToUtf8(Util::getFileName(item->getTarget())).c_str(),
			-1);
	}
	else
	{
		bool found = FALSE;
		GtkTreeModel *m = GTK_TREE_MODEL(fileStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

		while (valid)
		{
			if (fileView.getValue<gpointer, QueueItem *>(&iter, "QueueItem") == item)
			{
				found = TRUE;
				break;
			}
			valid = gtk_tree_model_iter_next(m, &iter);
		}

		if (!found)
			return;
	}

	// Users
	int online = 0;

	for (SourceIter it = item->getSources().begin(); it != item->getSources().end(); ++it)
	{
		if (tmp.size() > 0)
			tmp += ", ";

			if ((*it)->getUser()->isOnline())
				online++;

			tmp += WulforUtil::getNicks((*it)->getUser());
	}
	gtk_list_store_set(fileStore, &iter, fileView.col("Users"), string(tmp.empty() ? "No users" : tmp).c_str(), -1);

	// Status
	if (item->getStatus() == QueueItem::STATUS_WAITING)
	{
		char buf[64];
		if (online > 0)
		{
			if (item->getSources().size() == 1)
				snprintf(buf, sizeof(buf), "Waiting (User online)");
			else
				snprintf(buf, sizeof(buf), "Waiting (%d of %lu users online)", online, item->getSources().size());
			gtk_list_store_set(fileStore, &iter, fileView.col("Status"), buf, -1);
		}
		else
		{
			if (item->getSources().size() == 0)
				snprintf(buf, sizeof(buf), "No users to download from");
			else if (item->getSources().size() == 1)
				snprintf(buf, sizeof(buf), "User offline");
			else if (item->getSources().size() == 2)
				snprintf(buf, sizeof(buf), "Both users offline");
			else
				snprintf(buf, sizeof(buf), "All %lu users offline", item->getSources().size());
			gtk_list_store_set(fileStore, &iter, fileView.col("Status"), buf, -1);
		}
	}
	else if (item->getStatus() == QueueItem::STATUS_RUNNING)
		gtk_list_store_set(fileStore, &iter, fileView.col("Status"), "Running...", -1);

	// Size
	gtk_list_store_set(fileStore, &iter,
		fileView.col("Size"), (item->getSize() == -1) ? "Unkown" : Util::formatBytes(item->getSize()).c_str(),
		fileView.col("Exact Size"), (item->getSize() == -1) ? "Unkown" : Util::formatExactSize(item->getSize()).c_str(),
		fileView.col("Real Size"), item->getSize(),
		-1);

	// Downloaded
	if (item->getSize() > 0)
		gtk_list_store_set(fileStore, &iter, fileView.col("Downloaded"),
			string(Util::formatBytes(item->getDownloadedBytes()) + " (" +
			Util::toString((double)item->getDownloadedBytes() * 100.0 / (double)item->getSize()) + "%)").c_str(), -1);
	else
		gtk_list_store_set(fileStore, &iter, fileView.col("Downloaded"), "", -1);

	gtk_list_store_set(fileStore, &iter, fileView.col("Download Size"), item->getSize(), -1);

	// Priority
	switch (item->getPriority())
	{
		case QueueItem::PAUSED:
			gtk_list_store_set(fileStore, &iter, fileView.col("Priority"), "Paused", -1);
			break;
		case QueueItem::LOWEST:
			gtk_list_store_set(fileStore, &iter, fileView.col("Priority"), "Lowest", -1);
			break;
		case QueueItem::LOW:
			gtk_list_store_set(fileStore, &iter, fileView.col("Priority"), "Low", -1);
			break;
		case QueueItem::NORMAL:
			gtk_list_store_set(fileStore, &iter, fileView.col("Priority"), "Normal", -1);
			break;
		case QueueItem::HIGH:
			gtk_list_store_set(fileStore, &iter, fileView.col("Priority"), "High", -1);
			break;
		case QueueItem::HIGHEST:
			gtk_list_store_set(fileStore, &iter, fileView.col("Priority"), "Highest", -1);
			break;
		default:
			gtk_list_store_set(fileStore, &iter, fileView.col("Priority"), "Normal", -1);
	}

	// Path
	gtk_list_store_set(fileStore, &iter, fileView.col("Path"), Util::getFilePath(item->getTarget()).c_str(), -1);

	// Errors
	for (SourceIter it = item->getBadSources().begin(); it != item->getBadSources().end(); ++it)
	{
		if (!(*it)->isSet(QueueItem::Source::FLAG_REMOVED))
		{
			if (tmp.size() > 0)
				tmp += ", ";
			tmp += WulforUtil::getNicks((*it)->getUser());
			tmp += " (";
			if ((*it)->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE))
				tmp += "File not available";
			else if ((*it)->isSet(QueueItem::Source::FLAG_PASSIVE))
				tmp += "Passive user";
			else if ((*it)->isSet(QueueItem::Source::FLAG_ROLLBACK_INCONSISTENCY))
				tmp += "Rollback inconsistency, existing file does not match the one being downloaded";
			else if ((*it)->isSet(QueueItem::Source::FLAG_CRC_FAILED))
				tmp += "CRC32 inconsistency (SFV-Check)";
			else if ((*it)->isSet(QueueItem::Source::FLAG_BAD_TREE))
				tmp += "Downloaded tree does not match TTH root";
			tmp += ")";
		}
	}
	gtk_list_store_set(fileStore, &iter, fileView.col("Error"), string(tmp.empty() ? "No errors" : tmp).c_str(), -1);

	// Added
	gtk_list_store_set(fileStore, &iter, fileView.col("Added"), Util::formatTime("%Y-%m-%d %H:%M", item->getAdded()).c_str(), -1);

	// TTH
	if (item->getTTH() != NULL)
		gtk_list_store_set(fileStore, &iter, fileView.col("TTH"), string(item->getTTH()->toBase32()).c_str(), -1);
}

void DownloadQueue::updateStatus_gui()
{
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dirSelection, NULL, &iter))
	{
		string path = dirView.getString(&iter, "Path");
		if (dirFileMap.find(path) != dirFileMap.end())
		{
			int64_t total = 0;
			int count = 0;

			for (vector<QueueItem *>::iterator it = dirFileMap[path].begin(); it != dirFileMap[path].end(); ++it)
			{
				count++;
				total += ((*it)->getSize() > 0) ? (*it)->getSize() : 0;
			}

			setStatus_gui("Items: " + Util::toString(count), "statusItems");
			setStatus_gui("Size: " + Util::formatBytes(total), "statusFileSize");
			setStatus_gui("Files: " + Util::toString(queueItems), "statusFiles");
			setStatus_gui("Size: " + Util::formatBytes(queueSize), "statusTotalSize");
			return;
		}
	}

	setStatus_gui("Items: 0", "statusItems");
	setStatus_gui("Size: 0 B", "statusFileSize");
	setStatus_gui("Files: " + Util::toString(queueItems), "statusFiles");
	setStatus_gui("Size: " + Util::formatBytes(queueSize), "statusTotalSize");
}

void DownloadQueue::update_gui()
{
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dirSelection, NULL, &iter))
	{
		showingDir = dirView.getString(&iter, "Path");
		gtk_list_store_clear(fileStore);
		if (dirFileMap.find(showingDir) == dirFileMap.end())
			return;

		for (vector<QueueItem *>::iterator it = dirFileMap[showingDir].begin(); it != dirFileMap[showingDir].end(); ++it)
			updateItem_gui(*it, TRUE);
	}
}

void DownloadQueue::setStatus_gui(string text, string statusItem)
{
	gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusItem)), 0);
	if (statusItem == "statusMain")
		gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusItem)), 0, string("[" + Util::getShortTimeString() + "] " + text).c_str());
	else
		gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusItem)), 0, text.c_str());
}

int DownloadQueue::countFiles_gui(string path)
{
	if (!dirFileMap[path].empty())
		return 1;

	vector<GtkTreeIter> iter;
	getChildren(path, &iter);
	for (size_t i = 0; i < iter.size(); i++)
	{
		string rp = dirView.getString(&iter[i], "Path");
		if (dirFileMap.find(rp) == dirFileMap.end())
		{
			if (!dirFileMap[rp].empty())
				return 1;
		}
		else
			return 1;
	}

	return 0;
}

void DownloadQueue::removeDir_gui(string path)
{
	if (path == "" || path == "/")
		return;

	if (countFiles_gui(path) == 0)
	{
		if (showingDir == path)
			gtk_list_store_clear (fileStore);

		dirFileMap.erase (dirFileMap.find(path));
		if (dirMap.find(path) != dirMap.end())
		{
			gtk_tree_store_remove(dirStore, &dirMap[path]);
			dirMap.erase(dirMap.find(path));
		}
		removeDir_gui(getTrailingSubDir(path));
	}
	else
		return;
}

void DownloadQueue::removeFile_gui(string path)
{
	if (dirFileMap.find(path) != dirFileMap.end())
	{
		if (dirFileMap[path].empty())
			removeDir_gui(path);

		update_gui();
		updateStatus_gui();
	}
}

void DownloadQueue::addItem_gui(QueueItem *item)
{
	GtkTreeIter row;
	GtkTreeIter iter;

	string realpath = "/" + getNextSubDir(Util::getFilePath(item->getTarget())) + "/";
	if (dirMap.find(realpath) == dirMap.end())
	{
		gtk_tree_store_append(dirStore, &row, NULL);
		gtk_tree_store_set(dirStore, &row,
			dirView.col("Dir"), getNextSubDir(Util::getFilePath(item->getTarget())).c_str(),
			dirView.col("Path"), realpath.c_str(),
			-1);
		dirMap[realpath] = row;
		string tmp;
		addDir_gui(getRemainingDir(Util::getFilePath(item->getTarget())), &row, tmp);
		fileMap[item->getTarget()] = item;
		addFile_gui(item, tmp);
	}
	else
	{
		string tmp;
		addDir_gui(getRemainingDir(Util::getFilePath(item->getTarget())), &dirMap[realpath], tmp);
		fileMap[item->getTarget()] = item;
		addFile_gui(item, tmp);
	}
	gtk_tree_view_expand_all(dirView.get());

	if (BOOLSETTING(BOLD_QUEUE))
		setBold_gui();

	if (gtk_tree_selection_get_selected(dirSelection, NULL, &iter))
	{
		if (showingDir == dirView.getString(&iter, "Path"))
			updateItem_gui(item, TRUE);
	}
}

void DownloadQueue::on(QueueManagerListener::Added, QueueItem *item) throw()
{
	WulforManager::get()->dispatchGuiFunc(new Func1<DownloadQueue, QueueItem*>(this, &DownloadQueue::addItem_gui, item));
}

void DownloadQueue::on(QueueManagerListener::Moved, QueueItem *item) throw()
{
	///@todo: implement
	if (BOOLSETTING(BOLD_QUEUE))
		WulforManager::get()->dispatchGuiFunc(new Func0<DownloadQueue>(this, &DownloadQueue::setBold_gui));
}

void DownloadQueue::on(QueueManagerListener::Removed, QueueItem *item) throw()
{
	string path = Util::getFilePath(item->getTarget());

	if (dirFileMap.find(path) != dirFileMap.end())
	{
		for (vector<QueueItem *>::iterator it = dirFileMap[path].begin(); it != dirFileMap[path].end(); ++it)
		{
			if (*it == item)
			{
				queueSize -= (*it)->getSize();
				queueItems--;
				dirFileMap[path].erase(it);
				break;
			}
		}
	}

	WulforManager::get()->dispatchGuiFunc(new Func1<DownloadQueue, string>(this, &DownloadQueue::removeFile_gui, path));
	if (BOOLSETTING(BOLD_QUEUE))
		WulforManager::get()->dispatchGuiFunc(new Func0<DownloadQueue>(this, &DownloadQueue::setBold_gui));
}

void DownloadQueue::on(QueueManagerListener::SourcesUpdated, QueueItem *item) throw()
{
	typedef Func2<DownloadQueue, QueueItem *, bool> F2;
	F2 *func = new F2(this, &DownloadQueue::updateItem_gui, item, FALSE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void DownloadQueue::on(QueueManagerListener::StatusUpdated, QueueItem *item) throw()
{
	typedef Func2<DownloadQueue, QueueItem *, bool> F2;
	F2 *func = new F2(this, &DownloadQueue::updateItem_gui, item, FALSE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void DownloadQueue::reAddSource_client(string target, User::Ptr &user)
{
	try
	{
		QueueManager::getInstance()->readd(target, user);
	}
	catch (const Exception& e)
	{
		setStatus_gui(e.getError(), "statusMain");
	}
}

void DownloadQueue::addList_client(const User::Ptr &user)
{
	try
	{
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	}
	catch(const Exception& e)
	{
		setStatus_gui(e.getError(), "statusMain");
	}
}

void DownloadQueue::removeSource_client(string target, User::Ptr &user)
{
	QueueManager::getInstance()->removeSource(target, user, QueueItem::Source::FLAG_REMOVED);
}

void DownloadQueue::removeSources_client(User::Ptr &user)
{
	QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
}

void DownloadQueue::setPriority_client(string target, QueueItem::Priority p)
{
	QueueManager::getInstance()->setPriority(target, p);
}

void DownloadQueue::remove_client(string path)
{
	QueueManager::getInstance()->remove(path);
}
