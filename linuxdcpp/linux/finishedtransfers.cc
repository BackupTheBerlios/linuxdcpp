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

#include "finishedtransfers.hh"

FinishedTransfers::FinishedTransfers(std::string title, GCallback closeCallback):
			BookEntry(title, closeCallback),
			menuCallback(this, &FinishedTransfers::popupMenu_gui),
			removeCallback(this, &FinishedTransfers::removeItems_gui),
			removeAllCallback(this, &FinishedTransfers::removeAll_gui),
			openWithCallback(this, &FinishedTransfers::openWith_gui)
{
	string file = WulforManager::get()->getPath() + "/glade/finishedtransfers.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "finishedTransfers");
	mainBox = glade_xml_get_widget(xml, "mBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);
	
	totalItems = GTK_STATUSBAR(glade_xml_get_widget(xml, "totalItems"));
	totalSize = GTK_STATUSBAR(glade_xml_get_widget(xml, "totalSize"));
	averageSpeed = GTK_STATUSBAR(glade_xml_get_widget(xml, "averageSpeed"));

	openWithDialog = GTK_DIALOG(glade_xml_get_widget(xml, "openWithDialog"));
	openWithEntry = GTK_ENTRY(glade_xml_get_widget(xml, "openWithEntry"));
	gtk_dialog_set_alternative_button_order(openWithDialog, GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	// Initialize transfer treeview
	transferView.setView(GTK_TREE_VIEW(glade_xml_get_widget(xml, "view")), true, "finished");
	transferView.insertColumn("Time", G_TYPE_STRING, TreeView::STRING, 150);
	transferView.insertColumn("Filename", G_TYPE_STRING, TreeView::STRING, 100);
	transferView.insertColumn("Path", G_TYPE_STRING, TreeView::STRING, 200);
	transferView.insertColumn("Nick", G_TYPE_STRING, TreeView::STRING, 100);
	transferView.insertColumn("Hub", G_TYPE_STRING, TreeView::STRING, 200);
	transferView.insertColumn("Size", G_TYPE_STRING, TreeView::STRING, 100);
	transferView.insertColumn("Speed", G_TYPE_STRING, TreeView::STRING, 100);
	transferView.insertColumn("CRC Checked", G_TYPE_STRING, TreeView::STRING, 100);
	transferView.insertHiddenColumn("Target", G_TYPE_STRING);
	transferView.finalize();
	transferStore = gtk_list_store_newv(transferView.getColCount(), transferView.getGTypes());
	gtk_tree_view_set_model(transferView.get(), GTK_TREE_MODEL(transferStore));
	g_object_unref(transferStore);
	transferSelection = gtk_tree_view_get_selection(transferView.get());
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(transferView.get(), transferView.col("Time")), TRUE);

	finishedTransfersMenu = GTK_MENU(gtk_menu_new());
	openWith = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Open with"));
	remove = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Remove"));
	removeAll = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Remove all"));
	gtk_menu_shell_append(GTK_MENU_SHELL(finishedTransfersMenu), GTK_WIDGET(openWith));
	gtk_menu_shell_append(GTK_MENU_SHELL(finishedTransfersMenu), GTK_WIDGET(remove));
	gtk_menu_shell_append(GTK_MENU_SHELL(finishedTransfersMenu), GTK_WIDGET(removeAll));

	items = 0;
	totalBytes = 0;
	totalTime = 0;
	if (getID() == "Finished Downloads")
		getType = false;
	else
		getType = true;
	
	FinishedManager::getInstance()->addListener(this);
	this->updateList(FinishedManager::getInstance()->lockList(getType));
	FinishedManager::getInstance()->unlockList();
	
	menuCallback.connect_after(G_OBJECT(transferView.get()), "button-release-event", NULL);
	removeCallback.connect(G_OBJECT(remove), "activate", NULL);
	removeAllCallback.connect(G_OBJECT(removeAll), "activate", NULL);
	openWithCallback.connect(G_OBJECT(openWith), "activate", NULL);
}

FinishedTransfers::~FinishedTransfers()
{
	FinishedManager::getInstance()->removeListener(this);
	gtk_widget_destroy(GTK_WIDGET(openWithDialog));
}

GtkWidget *FinishedTransfers::getWidget()
{
	return mainBox;
}

void FinishedTransfers::popupMenu_gui(GtkWidget *, GdkEventButton *button, gpointer)
{
	if(button->button != 3) return;
	if (!gtk_tree_selection_get_selected(transferSelection, NULL, NULL)) return;
		
	gtk_menu_popup(finishedTransfersMenu, NULL, NULL, NULL, NULL, 3, button->time);
	gtk_widget_show_all(GTK_WIDGET(finishedTransfersMenu));
}

void FinishedTransfers::removeItems_gui(GtkMenuItem *, gpointer data)
{
	GtkTreeIter it;
	string time;
	FinishedItem *entry;
	std::map<string, FinishedItem*>::iterator iter;
	
	gtk_tree_selection_get_selected(transferSelection, NULL, &it);
	time = transferView.getString(&it, "Time");
	
	iter = finishedList.find(time);
	entry = iter->second;
	finishedList.erase(time);
	
	totalBytes -= entry->getChunkSize();
	totalTime -= entry->getMilliSeconds();
	items--;
	FinishedManager::getInstance()->remove(entry, getType);
	gtk_list_store_remove(transferStore, &it);
	this->updateStatus();
}

void FinishedTransfers::removeAll_gui(GtkMenuItem *, gpointer)
{
	FinishedManager::getInstance()->removeAll(getType);
	gtk_list_store_clear(transferStore);
	totalBytes = 0;
	totalTime = 0;
	items = 0;
	this->updateStatus();
	finishedList.clear();
}

void FinishedTransfers::updateList(FinishedItem::List& list)
{
	for(FinishedItem::List::const_iterator iter = list.begin(); iter != list.end(); iter++){
		this->addEntry(*iter);
	}
}

void FinishedTransfers::addEntry(FinishedItem *entry)
{
	gtk_list_store_append(transferStore, &treeIter);
	gtk_list_store_set(transferStore, &treeIter,
										transferView.col("Filename"), Util::getFileName(entry->getTarget()).c_str(),
										transferView.col("Time"), Util::formatTime("%Y-%m-%d %H:%M:%S", entry->getTime()).c_str(),
										transferView.col("Path"), Util::getFilePath(entry->getTarget()).c_str(),
										transferView.col("Nick"), entry->getUser().c_str(),
										transferView.col("Hub"), entry->getHub().c_str(),
										transferView.col("Size"), Util::formatBytes(entry->getSize()).c_str(),
										transferView.col("Speed"), (Util::formatBytes(entry->getAvgSpeed()) + "/s").c_str(),
										transferView.col("CRC Checked"), "",
										transferView.col("Target"), entry->getTarget().c_str(),
										-1);
	finishedList[Util::formatTime("%Y-%m-%d %H:%M:%S", entry->getTime()).c_str()] = entry;
	totalBytes += entry->getChunkSize();
	totalTime += entry->getMilliSeconds();
	items++;
	this->updateStatus();
}

void FinishedTransfers::updateStatus() 
{
	string status = Util::toString(items) + " Items";
	gtk_statusbar_push(totalItems, 0, status.c_str());
	gtk_statusbar_push(totalSize, 0, Text::toT(Util::formatBytes(totalBytes)).c_str());
	gtk_statusbar_push(averageSpeed, 0, Text::toT(Util::formatBytes((totalTime > 0) ? totalBytes * ((int64_t)1000) / totalTime : 0) + "/s").c_str());
}

void FinishedTransfers::on(AddedDl, FinishedItem* entry) throw(){
	if(getType != false) return;
	addEntry(entry);
}

void FinishedTransfers::on(AddedUl, FinishedItem* entry) throw(){
	if(getType != true) return;
	addEntry(entry);
}

void FinishedTransfers::openWith_gui(GtkMenuItem *item, gpointer data)
{
	GtkTreeIter iter;
	string command;
	string target;
	int ret;

	ret = gtk_dialog_run(openWithDialog);
	gtk_widget_hide(GTK_WIDGET(openWithDialog));

	if (ret == GTK_RESPONSE_OK)
	{
		command = gtk_entry_get_text(openWithEntry);
		gtk_tree_selection_get_selected(transferSelection, NULL, &iter);
		target = transferView.getString(&iter, "Target");

		if (!command.empty() && !target.empty())
		{
			command += Text::utf8ToAcp(" \"" + target + "\"");
			pthread_create(&openWithThread, NULL, &FinishedTransfers::runCommand, (void *)command.c_str());
		}
	}
}

void* FinishedTransfers::runCommand(void *command)
{
	system((char *)command);
	pthread_exit(NULL);
}
