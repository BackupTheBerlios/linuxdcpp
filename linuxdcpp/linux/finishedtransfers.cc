#include "finishedtransfers.hh"
#include "wulformanager.hh"

using namespace std;

FinishedTransfers::FinishedTransfers(int type, std::string title, GCallback closeCallback):
	BookEntry(type, title, title, closeCallback),
	items(0),
	totalBytes(0),
	totalTime(0),
	isUpload(type == WulforManager::FINISHED_UPLOADS)
{
	string file = WulforManager::get()->getPath() + "/glade/finishedtransfers.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "finishedTransfers");
	mainBox = glade_xml_get_widget(xml, "mBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);
	
	transferView = GTK_TREE_VIEW (glade_xml_get_widget (xml, "view"));
	transferStore = gtk_list_store_new (9, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(transferView, GTK_TREE_MODEL (transferStore));
	transferSelection = gtk_tree_view_get_selection(transferView);
	
	totalItems = GTK_STATUSBAR(glade_xml_get_widget(xml, "totalItems"));
	totalSize = GTK_STATUSBAR(glade_xml_get_widget(xml, "totalSize"));
	averageSpeed = GTK_STATUSBAR(glade_xml_get_widget(xml, "averageSpeed"));

	openDialog = GTK_DIALOG(glade_xml_get_widget(xml, "openWithDialog"));
	openEntry = GTK_ENTRY(glade_xml_get_widget(xml, "openWithEntry"));
	
	TreeViewFactory fc(transferView);
	fc.addColumn_gui(COLUMN_TIME, "Time", TreeViewFactory::STRING, 150);
	fc.addColumn_gui(COLUMN_FILENAME, "Filename", TreeViewFactory::STRING, 100);
	fc.addColumn_gui(COLUMN_PATH, "Path", TreeViewFactory::STRING, 200);
	fc.addColumn_gui(COLUMN_NICK, "Nick", TreeViewFactory::STRING, 100);
	fc.addColumn_gui(COLUMN_HUB, "Hub", TreeViewFactory::STRING, 200);
	fc.addColumn_gui(COLUMN_SIZE, "Size", TreeViewFactory::STRING, 100);
	fc.addColumn_gui(COLUMN_SPEED, "Speed", TreeViewFactory::STRING, 100);
	fc.addColumn_gui(COLUMN_CRC, "CRC Checked", TreeViewFactory::STRING, 100);
	
	finishedTransfersMenu = GTK_MENU(gtk_menu_new());
	openItem = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Open with"));
	removeItem = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Remove"));
	removeAllItem = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Remove all"));
	gtk_menu_shell_append(GTK_MENU_SHELL(finishedTransfersMenu), GTK_WIDGET(openItem));
	gtk_menu_shell_append(GTK_MENU_SHELL(finishedTransfersMenu), GTK_WIDGET(removeItem));
	gtk_menu_shell_append(GTK_MENU_SHELL(finishedTransfersMenu), GTK_WIDGET(removeAllItem));

	FinishedManager::getInstance()->addListener(this);
	updateList(FinishedManager::getInstance()->lockList(isUpload));
	FinishedManager::getInstance()->unlockList();
	
	g_signal_connect_after(transferView, "button-release-event", G_CALLBACK(popupMenu), this);
	g_signal_connect(removeItem, "activate", G_CALLBACK(removeItems), this);
	g_signal_connect(removeAllItem, "activate", G_CALLBACK(removeAll), this);
	g_signal_connect(openItem, "activate", G_CALLBACK(openWith), this);
}

FinishedTransfers::~FinishedTransfers()
{
	FinishedManager::getInstance()->removeListener(this);
	gtk_widget_destroy(GTK_WIDGET(openDialog));
}

GtkWidget *FinishedTransfers::getWidget()
{
	return mainBox;
}

void FinishedTransfers::popupMenu(GtkWidget *, GdkEventButton *button, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	
	if (button->button != 3) return;
	if (!gtk_tree_selection_get_selected(ft->transferSelection, NULL, NULL)) return;
		
	gtk_menu_popup(ft->finishedTransfersMenu, NULL, NULL, NULL, NULL, 3, button->time);
	gtk_widget_show_all(GTK_WIDGET(ft->finishedTransfersMenu));
}

void FinishedTransfers::removeItems(GtkMenuItem *, gpointer data)
{
	GtkTreeIter it;
	const char *time;
	FinishedItem *entry;
	std::map<string, FinishedItem*>::iterator iter;
	FinishedTransfers *ft = (FinishedTransfers *)data;
	
	gtk_tree_selection_get_selected(ft->transferSelection, NULL, &it);
	gtk_tree_model_get(GTK_TREE_MODEL(ft->transferStore), &it, COLUMN_TIME, &time, -1);
	
	ft->totalBytes -= entry->getChunkSize();
	ft->totalTime -= entry->getMilliSeconds();
	ft->items--;
	FinishedManager::getInstance()->remove(entry, ft->isUpload);
	gtk_list_store_remove(ft->transferStore, &it);
	ft->updateStatus();
}

void FinishedTransfers::removeAll(GtkMenuItem *, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	FinishedManager::getInstance()->removeAll(ft->isUpload);
	gtk_list_store_clear(ft->transferStore);
	ft->totalBytes = 0;
	ft->totalTime = 0;
	ft->items = 0;
	ft->updateStatus();
}

void FinishedTransfers::updateList(FinishedItem::List& list)
{
	FinishedItem::List::const_iterator iter;
	for (iter = list.begin(); iter != list.end(); iter++) {
		addEntry(*iter);
	}
}

void FinishedTransfers::addEntry(FinishedItem *entry)
{
	//target ( = path to file) is always in local codepage, so it needs converting
	string target = Text::acpToUtf8(entry->getTarget());

	gtk_list_store_append(transferStore, &treeIter);
	gtk_list_store_set(transferStore, &treeIter,
		COLUMN_FILENAME, Util::getFileName(target).c_str(),
		COLUMN_TIME, Util::formatTime("%Y-%m-%d %H:%M:%S", entry->getTime()).c_str(),
		COLUMN_PATH, Util::getFilePath(target).c_str(),
		COLUMN_NICK, entry->getUser().c_str(),
		COLUMN_HUB, entry->getHub().c_str(),
		COLUMN_SIZE, Util::formatBytes(entry->getSize()).c_str(),
		COLUMN_SPEED, (Util::formatBytes(entry->getAvgSpeed()) + "/s").c_str(),
		COLUMN_CRC, "",
		COLUMN_TARGET, target.c_str(),
		-1);
	totalBytes += entry->getChunkSize();
	totalTime += entry->getMilliSeconds();
	items++;
	updateStatus();
}

void FinishedTransfers::updateStatus() 
{
	string status = Util::toString(items) + " Items";
	gtk_statusbar_push(totalItems, 0, status.c_str());
	gtk_statusbar_push(totalSize, 0, Text::toT(Util::formatBytes(totalBytes)).c_str());
	if (totalTime > 0) {
		gtk_statusbar_push(averageSpeed, 0, Text::toT(Util::formatBytes(totalBytes * ((int64_t)1000) / totalTime) + "/s").c_str());
	} else {
		gtk_statusbar_push(averageSpeed, 0, Text::toT(Util::formatBytes(totalBytes * ((int64_t)100)) + "/s").c_str());
	}
}

void FinishedTransfers::openWith(GtkMenuItem *, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	MainWindow *mainWin = WulforManager::get()->getMainWindow();
	GtkTreeIter iter;
	string command;
	const char *target;
	int ret;

	gtk_widget_show_all(GTK_WIDGET(ft->openDialog));
	ret = gtk_dialog_run(ft->openDialog);
	command = gtk_entry_get_text(ft->openEntry);
	gtk_widget_hide(GTK_WIDGET(ft->openDialog));

	if (ret != GTK_RESPONSE_ACCEPT) return;
		
	gtk_tree_selection_get_selected(ft->transferSelection, NULL, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(ft->transferStore), &iter, 
		COLUMN_TARGET, &target, 
		-1);
	pid_t pid = fork();
	if (pid == 0) {
		system(Text::toT(command + " \"" + target + "\"").c_str());
		exit(EXIT_SUCCESS);
	}
}

void FinishedTransfers::on(AddedDl, FinishedItem* entry) throw()
{
	gdk_threads_enter();
	if (!isUpload) addEntry(entry);
	gdk_threads_leave();
}

void FinishedTransfers::on(AddedUl, FinishedItem* entry) throw()
{
	gdk_threads_enter();
	if (isUpload) addEntry(entry);
	gdk_threads_leave();
}
