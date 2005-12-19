#include "finishedtransfers.hh"
#include "func.hh"
#include "wulformanager.hh"

using namespace std;

FinishedTransfers::FinishedTransfers(int type, std::string title, GCallback closeCallback):
			BookEntry(type, title, title, closeCallback),
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
	
	transferView = GTK_TREE_VIEW (glade_xml_get_widget (xml, "view"));
	transferStore = gtk_list_store_new (9, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(transferView, GTK_TREE_MODEL (transferStore));
	transferSelection = gtk_tree_view_get_selection(transferView);
	
	totalItems = GTK_STATUSBAR(glade_xml_get_widget(xml, "totalItems"));
	totalSize = GTK_STATUSBAR(glade_xml_get_widget(xml, "totalSize"));
	averageSpeed = GTK_STATUSBAR(glade_xml_get_widget(xml, "averageSpeed"));
	
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
	openWith = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Open with"));
	remove = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Remove"));
	removeAll = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Remove all"));
	gtk_menu_shell_append(GTK_MENU_SHELL(finishedTransfersMenu), GTK_WIDGET(openWith));
	gtk_menu_shell_append(GTK_MENU_SHELL(finishedTransfersMenu), GTK_WIDGET(remove));
	gtk_menu_shell_append(GTK_MENU_SHELL(finishedTransfersMenu), GTK_WIDGET(removeAll));

	items = 0;
	totalBytes = 0;
	totalTime = 0;
	if(type == WulforManager::get()->FINISHED_DOWNLOADS)
			getType = false;
	else getType = true;
	
	FinishedManager::getInstance()->addListener(this);
	this->updateList(FinishedManager::getInstance()->lockList(getType));
	FinishedManager::getInstance()->unlockList();
	
	menuCallback.connect_after(G_OBJECT(transferView), "button-release-event", NULL);
	removeCallback.connect(G_OBJECT(remove), "activate", NULL);
	removeAllCallback.connect(G_OBJECT(removeAll), "activate", NULL);
	openWithCallback.connect(G_OBJECT(openWith), "activate", NULL);
}

FinishedTransfers::~FinishedTransfers()
{
	FinishedManager::getInstance()->removeListener(this);
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

void FinishedTransfers::removeItems_gui(GtkMenuItem *, gpointer)
{
	GtkTreeIter it;
	const char *time;
	FinishedItem *entry;
	std::map<string, FinishedItem*>::iterator iter;
	
	gtk_tree_selection_get_selected(transferSelection, NULL, &it);
	gtk_tree_model_get(GTK_TREE_MODEL(transferStore), &it, COLUMN_TIME, &time, -1);
	
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
										COLUMN_FILENAME, Util::getFileName(entry->getTarget()).c_str(),
										COLUMN_TIME, Util::formatTime("%Y-%m-%d %H:%M:%S", entry->getTime()).c_str(),
										COLUMN_PATH, Util::getFilePath(entry->getTarget()).c_str(),
										COLUMN_NICK, entry->getUser().c_str(),
										COLUMN_HUB, entry->getHub().c_str(),
										COLUMN_SIZE, Util::formatBytes(entry->getSize()).c_str(),
										COLUMN_SPEED, (Util::formatBytes(entry->getAvgSpeed()) + "/s").c_str(),
										COLUMN_CRC, "",
										COLUMN_TARGET, entry->getTarget().c_str(),
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

void FinishedTransfers::openWith_gui(GtkMenuItem *, gpointer)
{
	MainWindow *mainWin = WulforManager::get()->getMainWindow();
	GtkWidget *entry, *box, *label;
	GtkTreeIter iter;
	string command;
	const char *target;
	int ret;

	GtkWidget *dialog = gtk_dialog_new_with_buttons ("Open with: ",
		mainWin->getWindow(),
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		NULL);

	entry = gtk_entry_new();
	box = gtk_hbox_new(false, 5);
	label = gtk_label_new("Open with");
	gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(box), entry, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), box, TRUE, TRUE, 5);
	gtk_widget_show_all(dialog);

	ret = gtk_dialog_run(GTK_DIALOG(dialog));
	command = gtk_entry_get_text(GTK_ENTRY(entry));
	gtk_widget_destroy(dialog);
	if (ret != GTK_RESPONSE_ACCEPT) return;
		
	gtk_tree_selection_get_selected(transferSelection, NULL, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(transferStore), &iter, COLUMN_TARGET, &target, -1);
	pid_t pid= fork();
	if(pid == 0){
		system(Text::toT(command + " \"" + target + "\"").c_str());
		exit(EXIT_SUCCESS);
	}
}
