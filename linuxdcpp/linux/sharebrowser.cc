#include "sharebrowser.hh"
#include "wulformanager.hh"
#include "treeviewfactory.hh"

#include <client/Text.h>
#include <iostream>
#include <sstream>

using namespace std;

ShareBrowser::ShareBrowser(User::Ptr user, std::string file, GCallback closeCallback):
	BookEntry(WulforManager::SHARE_BROWSER, 
		user->getFullNick(), user->getNick(), closeCallback),
	listing(user),
	pressedCallback(this, &ShareBrowser::buttonPressed_gui),
	releasedCallback(this, &ShareBrowser::buttonReleased_gui),
	menuCallback(this, &ShareBrowser::menuClicked_gui),
	buttonCallback(this, &ShareBrowser::buttonClicked_gui),
	posDir(NULL),
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

	matchButton = GTK_BUTTON(glade_xml_get_widget(xml, "matchButton"));
	findButton = GTK_BUTTON(glade_xml_get_widget(xml, "findButton"));
	nextButton = GTK_BUTTON(glade_xml_get_widget(xml, "nextButton"));
	
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
	f2.addColumn_gui(COLUMN_DIR, "", TreeViewFactory::STRING, -1);
	gtk_tree_view_insert_column(dirView, gtk_tree_view_column_new(), COLUMN_DL_DIR);
	dirSelection = gtk_tree_view_get_selection(dirView);

	//create popup menus
	dirMenu = GTK_MENU(gtk_menu_new());
	dlDirDir = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Download"));
	dlDirToDir = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Download to..."));
	gtk_menu_shell_append(GTK_MENU_SHELL(dirMenu), GTK_WIDGET(dlDirDir));
	gtk_menu_shell_append(GTK_MENU_SHELL(dirMenu), GTK_WIDGET(dlDirToDir));

	fileMenu = GTK_MENU(gtk_menu_new()); 
	dlFile = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Download"));
	dlDir = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Download dir"));
	dlFileTo = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Download to..."));
	dlDirTo = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Download dir to..."));
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), GTK_WIDGET(dlFile));
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), GTK_WIDGET(dlDir));
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), GTK_WIDGET(dlFileTo));
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), GTK_WIDGET(dlDirTo));

	//connect callbacks
	pressedCallback.connect(G_OBJECT(fileView), "button-press-event", NULL);
	pressedCallback.connect(G_OBJECT(dirView), "button-press-event", NULL);
	releasedCallback.connect_after(G_OBJECT(fileView), "button-release-event", NULL);
	releasedCallback.connect_after(G_OBJECT(dirView), "button-release-event", NULL);

	menuCallback.connect(G_OBJECT(dlDir), "activate", NULL);
	menuCallback.connect(G_OBJECT(dlDirTo), "activate", NULL);
	menuCallback.connect(G_OBJECT(dlFile), "activate", NULL);
	menuCallback.connect(G_OBJECT(dlFileTo), "activate", NULL);
	menuCallback.connect(G_OBJECT(dlDirDir), "activate", NULL);
	menuCallback.connect(G_OBJECT(dlDirToDir), "activate", NULL);
	
	//Set the buttons text to small so that the statusbar isn't too high.
	//This can't be set with glade, needs tyo be done in code.
	GtkLabel *label;
	label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(matchButton)));
	gtk_label_set_markup(label, "<small>Match Queue</small>");
	label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(findButton)));
	gtk_label_set_markup(label, "<small>Find</small>");
	label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(nextButton)));
	gtk_label_set_markup(label, "<small>Next</small>");
	
	//set the buttons in statusbar to same height as statusbar
	GtkRequisition statusReq;
	gtk_widget_size_request(GTK_WIDGET(mainStatus), &statusReq);
	gtk_widget_set_size_request(GTK_WIDGET(matchButton), -1, statusReq.height);
	gtk_widget_set_size_request(GTK_WIDGET(findButton), -1, statusReq.height);
	gtk_widget_set_size_request(GTK_WIDGET(nextButton), -1, statusReq.height);

	//connect callback to buttons
	buttonCallback.connect(G_OBJECT(matchButton), "clicked", NULL);
	buttonCallback.connect(G_OBJECT(findButton), "clicked", NULL);
	buttonCallback.connect(G_OBJECT(nextButton), "clicked", NULL);
	
	listing.loadFile(file);
	shareSize = 0;
	shareItems = 0;
	buildDirs_gui(listing.getRoot()->directories, NULL);
	updateStatus_gui();
}

ShareBrowser::~ShareBrowser() {
	if (posDir) gtk_tree_path_free(posDir);
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

void ShareBrowser::updateFiles_gui(bool fromFind) {
	DirectoryListing::Directory *dir;
	DirectoryListing::File::List *files;
	DirectoryListing::File::Iter it;
	gpointer ptr;
	GtkTreeIter iter;
	
	//If this function is called from the find stuff the selection is not
	//yet updated sometimes. Thus we need to look at posDir.
	if (fromFind)
		gtk_tree_model_get_iter(GTK_TREE_MODEL(dirStore), &iter, posDir);
	else
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
	GtkTreeIter iter;
	DirectoryListing::File *file;
	gpointer ptr;
	string target;

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
		if (GTK_TREE_VIEW(widget) == dirView) updateFiles_gui(false);

	//single click right button
	if (event->button == 3 && oldType == GDK_BUTTON_PRESS)
		if (GTK_TREE_VIEW(widget) == dirView) {
			gtk_menu_popup(dirMenu, NULL, NULL, NULL, NULL, 3, event->time);
			gtk_widget_show_all(GTK_WIDGET(dirMenu));
		} else {
			gtk_menu_popup(fileMenu, NULL, NULL, NULL, NULL, 3, event->time);
			gtk_widget_show_all(GTK_WIDGET(fileMenu));
		}

	//double click left button
	if (event->button == 1 && oldType == GDK_2BUTTON_PRESS)
		if (GTK_TREE_VIEW(widget) == fileView) {
			gtk_tree_selection_get_selected(fileSelection, NULL, &iter);
			gtk_tree_model_get(GTK_TREE_MODEL(fileStore), &iter,
				COLUMN_DL_FILE, &ptr, -1);
			file = (DirectoryListing::File *)ptr;
		
			if (listing.getUtf8())
				target += Text::utf8ToAcp(Util::getFileName(file->getName()));
			else
				target += Util::getFileName(file->getName());
			
			typedef Func2<ShareBrowser, DirectoryListing::File *, string> F2;
			F2 * func = new F2(this, &ShareBrowser::downloadFile_client, file, target);
			WulforManager::get()->dispatchClientFunc(func);
		}

	return FALSE;
}

void ShareBrowser::menuClicked_gui(GtkMenuItem *item, gpointer) {
	string target;
	gpointer ptr;
	DirectoryListing::File *file;
	DirectoryListing::Directory *dir;
	GtkTreeIter iter;
	int ret;

	if (item == dlFileTo || item == dlDirTo || item == dlDirToDir) {
		GtkWidget *chooser = gtk_file_chooser_dialog_new(
			"Download to",
			WulforManager::get()->getMainWindow()->getWindow(),
			GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL);

		ret = gtk_dialog_run(GTK_DIALOG(chooser));
		target = Text::toT(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser))) + Text::toT("/");
		gtk_widget_destroy(GTK_WIDGET(chooser));
		if (ret == GTK_RESPONSE_CANCEL) return;
	} else {
		target = Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY));
	}

	if (item == dlFile || item == dlFileTo) {
		gtk_tree_selection_get_selected(fileSelection, NULL, &iter);
		gtk_tree_model_get(GTK_TREE_MODEL(fileStore), &iter,
			COLUMN_DL_FILE, &ptr, -1);
		file = (DirectoryListing::File *)ptr;
		
		if (listing.getUtf8())
			target += Text::utf8ToAcp(Util::getFileName(file->getName()));
		else
			target += Util::getFileName(file->getName());
			
		typedef Func2<ShareBrowser, DirectoryListing::File *, string> F2;
		F2 * func = new F2(this, &ShareBrowser::downloadFile_client, file, target);
		WulforManager::get()->dispatchClientFunc(func);
	} else {
		gtk_tree_selection_get_selected(dirSelection, NULL, &iter);
		gtk_tree_model_get(GTK_TREE_MODEL(dirStore), &iter,
			COLUMN_DL_DIR, &ptr, -1);
		dir = (DirectoryListing::Directory *)ptr;
		
		typedef Func2<ShareBrowser, DirectoryListing::Directory *, string> F2;
		F2 * func = new F2(this, &ShareBrowser::downloadDir_client, dir, target);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void ShareBrowser::downloadFile_client(DirectoryListing::File *file, string target) {
	try {
		listing.download(file, target, false, false);
	} catch(const Exception& e) {
		typedef Func2<ShareBrowser, GtkStatusbar *, string> F2;
		F2 *func = new F2(this, &ShareBrowser::setStatus_gui, mainStatus, e.getError());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void ShareBrowser::downloadDir_client(DirectoryListing::Directory *dir, string target) {
	try {
		listing.download(dir, target, false);
	} catch(const Exception& e) {
		typedef Func2<ShareBrowser, GtkStatusbar *, string> F2;
		F2 *func = new F2(this, &ShareBrowser::setStatus_gui, mainStatus, e.getError());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void ShareBrowser::buttonClicked_gui(GtkWidget *widget, gpointer) {
	GtkButton *button = GTK_BUTTON(widget);

	if (button == matchButton) {
		typedef Func0<ShareBrowser> F0;
		F0 *f = new F0(this, &ShareBrowser::matchQueue_client);
		WulforManager::get()->dispatchClientFunc(f);
	}

	if (button == findButton) {
		string text;
		gint ret;
		GtkDialog *dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(
			"Find files",
			WulforManager::get()->getMainWindow()->getWindow(),
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL));

		GtkWidget *frame = gtk_frame_new("Enter text to search for");
		GtkWidget *entry = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(dialog->vbox), frame, TRUE, TRUE, 5);
		gtk_container_add(GTK_CONTAINER(frame), entry);

		gtk_widget_show_all(GTK_WIDGET(dialog));
		ret = gtk_dialog_run(dialog);
		text = gtk_entry_get_text(GTK_ENTRY(entry));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		if (ret == GTK_RESPONSE_CANCEL) return;

		search = text;
		findNext_gui(true);
	}

	if (button == nextButton) {
		findNext_gui(false);
	}
}

void ShareBrowser::findNext_gui(bool firstFile) {
	DirectoryListing::Directory *dir;
	DirectoryListing::File::List *files;
	gpointer ptr;
	GtkTreeIter iter;
	bool found, updatedPosDir = false;
	
	if (search == "") return;

	if (firstFile) {
		if (posDir) gtk_tree_path_free(posDir);
		posDir = gtk_tree_path_new_first();
		updatedPosDir = true;
	}

	if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(dirStore), &iter, posDir)) 
		return;
	gtk_tree_model_get(GTK_TREE_MODEL(dirStore), &iter,
		COLUMN_DL_DIR, &ptr,
		-1);
	dir = (DirectoryListing::Directory *)ptr;
	files = &(dir->files);
	
	if (firstFile) posFile = files->begin();
	else posFile++;

	//This goes through all directories and files to find search
	while (true) {
		//This is in case we need to scan next directory
		//While is in case directories are empty
		while (posFile == files->end()) {
			updatedPosDir = true;

			//Searching for unvisided nodes, first children, then sibilings 
			found = true;
			gtk_tree_path_down(posDir);
			if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(dirStore), 
				&iter, posDir))
			{
				gtk_tree_path_up(posDir);
				gtk_tree_path_next(posDir);
				if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(dirStore), 
					&iter, posDir))
				{
					found = false;
				}
			}

			//Now searching upwards through the tree
			while (!found) {
				if (!gtk_tree_path_up(posDir)) break;

				gtk_tree_path_next(posDir);
				found = gtk_tree_model_get_iter(
					GTK_TREE_MODEL(dirStore), &iter, posDir);
			}

			if (!found) {
				setStatus_gui(mainStatus, "No files found");
				return;
			}
				
			gtk_tree_model_get(GTK_TREE_MODEL(dirStore), &iter,
				COLUMN_DL_DIR, &ptr,
				-1);
			dir = (DirectoryListing::Directory *)ptr;
			files = &(dir->files);
			posFile = files->begin();
		}

		string filename = (*posFile)->getName();
		if (!listing.getUtf8())
			filename = Text::acpToUtf8(filename);

		//This is if we find the file, we need to select it and 
		//expand its dir in the dir view
		if (filename.find(search, 0) != string::npos) {
			if (updatedPosDir) {
				gtk_tree_view_expand_row(dirView, posDir, FALSE);
				gtk_tree_view_set_cursor(dirView, posDir, NULL, FALSE);
				updateFiles_gui(true);
			}
			
			//Select the file in the file view
			gtk_tree_model_get_iter_first(GTK_TREE_MODEL(fileStore), &iter);
			while (true) {
				assert(gtk_list_store_iter_is_valid(fileStore, &iter));

				gtk_tree_model_get(GTK_TREE_MODEL(fileStore), &iter,
					COLUMN_DL_FILE, &ptr, -1);
				DirectoryListing::File *file = (DirectoryListing::File *)ptr;
				if (file == *posFile) {
					GtkTreePath *path;
					path = gtk_tree_model_get_path(GTK_TREE_MODEL(fileStore), &iter);
					gtk_tree_view_set_cursor(fileView, path, NULL, FALSE);
					gtk_tree_path_free(path);
					return;
				}
				
				gtk_tree_model_iter_next(GTK_TREE_MODEL(fileStore), &iter);
			}
		}

		posFile++;
	}
}

void ShareBrowser::matchQueue_client() {
	ostringstream stream;
	int matched;

	matched = QueueManager::getInstance()->matchListing(&listing);
	stream << "Matched " << matched << " files";

	typedef Func2<ShareBrowser, GtkStatusbar *, string> F2;
	F2 *f = new F2(this, &ShareBrowser::setStatus_gui, mainStatus, stream.str());
	WulforManager::get()->dispatchGuiFunc(f);
}


