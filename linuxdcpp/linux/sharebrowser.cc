#include "sharebrowser.hh"
#include "wulformanager.hh"
#include "treeviewfactory.hh"

#include <client/Text.h>
#include <iostream>

using namespace std;

ShareBrowser::ShareBrowser(User::Ptr user, std::string file, GCallback closeCallback):
	BookEntry(WulforManager::SHARE_BROWSER, 
		user->getFullNick(), user->getNick(), closeCallback),
	listing(user),
	pressedCallback(this, &ShareBrowser::buttonPressed_gui),
	releasedCallback(this, &ShareBrowser::buttonReleased_gui),
	WIDTH_FILE(400), 
	WIDTH_SIZE(80), 
	WIDTH_TYPE(50), 
	WIDTH_TTH(100)
{
	string gladeFile = WulforManager::get()->getPath() + "/glade/sharebrowser.glade";
	GladeXML *xml = glade_xml_new(gladeFile.c_str(), NULL, NULL);

	GtkWidget *window = glade_xml_get_widget(xml, "window");
	box = glade_xml_get_widget(xml, "box");
	gtk_widget_ref(box);
	gtk_container_remove(GTK_CONTAINER(window), box);
	gtk_widget_destroy(window);

	mainStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "mainStatus"));
	itemsStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "itemsStatus"));
	sizeStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "sizeStatus"));
	filesStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "filesStatus"));
	totalStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "totalStatus"));

	//initiate the file treeview
	fileView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "fileView"));
	fileStore = gtk_list_store_new(5, 
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	gtk_tree_view_set_model(fileView, GTK_TREE_MODEL(fileStore));
	TreeViewFactory f1(fileView);
	f1.addColumn_gui(COLUMN_FILE, "File", TreeViewFactory::STRING, WIDTH_FILE);
	f1.addColumn_gui(COLUMN_SIZE, "Size", TreeViewFactory::STRING, WIDTH_SIZE);
	f1.addColumn_gui(COLUMN_TYPE, "Type", TreeViewFactory::STRING, WIDTH_TYPE);
	f1.addColumn_gui(COLUMN_TTH, "TTH", TreeViewFactory::STRING, WIDTH_TTH);
	gtk_tree_view_insert_column(fileView, gtk_tree_view_column_new(), COLUMN_DL_FILE);
	fileSelection = gtk_tree_view_get_selection(fileView);

	//initiate the dir treeview
	dirView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "dirView"));
	dirStore = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	gtk_tree_view_set_model(dirView, GTK_TREE_MODEL(dirStore));
	TreeViewFactory f2(dirView);
	f2.addColumn_gui(COLUMN_DIR, "", TreeViewFactory::STRING, 200);
	gtk_tree_view_insert_column(dirView, gtk_tree_view_column_new(), COLUMN_DL_DIR);
	dirSelection = gtk_tree_view_get_selection(dirView);

	//connect callbacks
	pressedCallback.connect(G_OBJECT(fileView), "button-press-event", NULL);
	pressedCallback.connect(G_OBJECT(dirView), "button-press-event", NULL);
	releasedCallback.connect_after(G_OBJECT(fileView), "button-release-event", NULL);
	releasedCallback.connect_after(G_OBJECT(dirView), "button-release-event", NULL);
	
	listing.loadFile(file, false);	
	shareSize = 0;
	shareItems = 0;
	buildDirs_gui(listing.getRoot()->directories, NULL);
	updateStatus_gui();
}

GtkWidget *ShareBrowser::getWidget() {
	return box;
}

void ShareBrowser::setStatus_gui(GtkStatusbar *status, std::string msg) {
	gtk_statusbar_pop(status, 0);
	gtk_statusbar_push(status, 0, msg.c_str());
}

void ShareBrowser::setPosition_gui(string pos) {
	cout << "try to set path: " << pos << endl;
}

void ShareBrowser::buildDirs_gui(
	DirectoryListing::Directory::List dirs, GtkTreeIter *iter)
{
	DirectoryListing::Directory::Iter it;
	DirectoryListing::File::Iter file;
	GtkTreeIter newIter;

	for (it = dirs.begin(); it != dirs.end(); it++) {
		gtk_tree_store_append(dirStore, &newIter, iter);
		
		//add the name and check if the name is in UTF8
		if (listing.getUtf8()) {
			gtk_tree_store_set(dirStore, &newIter, 
				COLUMN_DIR, (*it)->getName().c_str(),
				-1);
		} else {
			gtk_tree_store_set(dirStore, &newIter, 
				COLUMN_DIR, Text::acpToUtf8((*it)->getName()).c_str(),
				-1);
		}
		
		gtk_tree_store_set(dirStore, &newIter, 
			COLUMN_DL_DIR, (gpointer)*it,
			-1);

		for (file = (*it)->files.begin(); file != (*it)->files.end(); file++) {
			shareItems++;
			shareSize += (*file)->getSize();
		}		
		
		buildDirs_gui((*it)->directories, &newIter);
	}
}

void ShareBrowser::updateFiles_gui() {
	DirectoryListing::Directory *dir;
	DirectoryListing::File::List *files;
	DirectoryListing::File::Iter it;
	gpointer ptr;
	GtkTreeIter iter;
	
	gtk_tree_selection_get_selected(dirSelection, NULL, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(dirStore), &iter,
		COLUMN_DL_DIR, &ptr,
		-1);
	dir = (DirectoryListing::Directory *)ptr;
	files = &(dir->files);

	gtk_list_store_clear(fileStore);

	currentSize = 0;
	currentItems = 0;
	for (it = files->begin(); it != files->end(); it++) {
		gtk_list_store_append(fileStore, &iter);

		//data needs to be converted to utf8 if it's not in that form
		if (listing.getUtf8()) {
			gtk_list_store_set(fileStore, &iter,
				COLUMN_FILE, Util::getFileName((*it)->getName()).c_str(),
				COLUMN_TYPE, Util::getFileExt((*it)->getName()).c_str(),
				-1);
		} else {
			gtk_list_store_set(fileStore, &iter,
				COLUMN_FILE, Text::acpToUtf8(Util::getFileName((*it)->getName())).c_str(),
				COLUMN_TYPE, Text::acpToUtf8(Util::getFileExt((*it)->getName())).c_str(),
				-1);
		}

		gtk_list_store_set(fileStore, &iter,
			COLUMN_SIZE, Util::formatBytes((*it)->getSize()).c_str(),
			COLUMN_DL_FILE, (gpointer)(*it),
			-1);

		TTHValue *tth;
		if (tth = (*it)->getTTH())
			gtk_list_store_set(fileStore, &iter, COLUMN_TTH, tth->toBase32().c_str(), -1);
		else
			gtk_list_store_set(fileStore, &iter, COLUMN_TTH, "N/A", -1);

		currentSize += (*it)->getSize();
		currentItems++;
	}

	updateStatus_gui();
}
	
void ShareBrowser::updateStatus_gui() {
	string items, files, size, total;

	files = "Files: " + Util::toString(shareItems);
	total = "Total: " + Util::formatBytes(shareSize);
	if (gtk_tree_selection_get_selected(dirSelection, NULL, NULL)) {
		items = "Items: " + Util::toString(currentItems);
		size = "Size: " + Util::formatBytes(currentSize);
	} else {
		items = "Items: 0";
		size = "Size: 0 B";	
	}

	setStatus_gui(itemsStatus, items);
	setStatus_gui(sizeStatus, size);
	setStatus_gui(filesStatus, files);
	setStatus_gui(totalStatus, total);
}

gboolean ShareBrowser::buttonPressed_gui(
	GtkWidget *widget, GdkEventButton *event, gpointer)
{
	oldType = event->type;
	oldButton = event->button;
	
	return FALSE;
}

gboolean ShareBrowser::buttonReleased_gui(
	GtkWidget *widget, GdkEventButton *event, gpointer)
{
	if (oldButton != event->button) return FALSE;

	if (GTK_TREE_VIEW(widget) == fileView) {
		if (!gtk_tree_selection_get_selected(fileSelection, NULL, NULL))
			return FALSE;
	} else {
		if (!gtk_tree_selection_get_selected(dirSelection, NULL, NULL))
			return FALSE;
	}

	//single click left button
	if (event->button == 1 && oldType == GDK_BUTTON_PRESS)
		if (GTK_TREE_VIEW(widget) == dirView) updateFiles_gui();

	//single click right button
	if (event->button == 3 && oldType == GDK_BUTTON_PRESS)
		if (GTK_TREE_VIEW(widget) == dirView)
			;
		else
			;

	//double click left button
	if (event->button == 1 && oldType == GDK_BUTTON2_PRESS)
		if (GTK_TREE_VIEW(widget) == fileView)
			;

	return FALSE;
}
















