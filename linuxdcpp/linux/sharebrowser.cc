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
	gtk_tree_view_column_set_sort_order(gtk_tree_view_get_column(fileView, COLUMN_FILE), GTK_SORT_ASCENDING);

	//initiate the dir treeview
	dirView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "dirView"));
	dirStore = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	gtk_tree_view_set_model(dirView, GTK_TREE_MODEL(dirStore));
	TreeViewFactory f2(dirView);
	f2.addColumn_gui(COLUMN_DIR, "", TreeViewFactory::STRING, -1);
	gtk_tree_view_insert_column(dirView, gtk_tree_view_column_new(), COLUMN_DL_DIR);
	dirSelection = gtk_tree_view_get_selection(dirView);
	gtk_tree_view_column_set_sort_order(gtk_tree_view_get_column(dirView, COLUMN_DIR), GTK_SORT_ASCENDING);

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

	//connect callbacks
	g_signal_connect(fileView, "button-press-event", G_CALLBACK(buttonPressed), this);
	g_signal_connect(dirView, "button-press-event", G_CALLBACK(buttonPressed), this);
	g_signal_connect(fileView, "button-release-event", G_CALLBACK(buttonReleased), this);
	g_signal_connect(dirView, "button-release-event", G_CALLBACK(buttonReleased), this);

	g_signal_connect(dlDir, "activate", G_CALLBACK(menuClicked), this);
	g_signal_connect(dlDirTo, "activate", G_CALLBACK(menuClicked), this);
	g_signal_connect(dlFile, "activate", G_CALLBACK(menuClicked), this);
	g_signal_connect(dlFileTo, "activate", G_CALLBACK(menuClicked), this);
	g_signal_connect(dlDirDir, "activate", G_CALLBACK(menuClicked), this);
	g_signal_connect(dlDirToDir, "activate", G_CALLBACK(menuClicked), this);

	g_signal_connect(matchButton, "clicked", G_CALLBACK(buttonClicked), this);
	g_signal_connect(findButton, "clicked", G_CALLBACK(buttonClicked), this);
	g_signal_connect(nextButton, "clicked", G_CALLBACK(buttonClicked), this);

	listing.loadFile(file);
	shareSize = 0;
	shareItems = 0;
	buildDirs(listing.getRoot()->directories, NULL);
	updateStatus();
}

ShareBrowser::~ShareBrowser()
{
	if (posDir) gtk_tree_path_free(posDir);
}

GtkWidget *ShareBrowser::getWidget()
{
	return box;
}

void ShareBrowser::setStatus(GtkStatusbar *status, std::string msg)
{
	gtk_statusbar_pop(status, 0);
	gtk_statusbar_push(status, 0, msg.c_str());
}

void ShareBrowser::setPosition(string pos)
{
	cout << "try to set path: " << pos << endl;
}

void ShareBrowser::buildDirs(
	DirectoryListing::Directory::List dirs, GtkTreeIter *iter)
{
	DirectoryListing::Directory::Iter it;
	DirectoryListing::File::Iter file;
	GtkTreeIter newIter;

	std::sort(dirs.begin(), dirs.end(), DirectoryListing::Directory::DirSort());

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
		
		buildDirs((*it)->directories, &newIter);
	}
	
}

void ShareBrowser::updateFiles(bool fromFind)
{
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

	std::sort(files->begin(), files->end(), DirectoryListing::File::FileSort());
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(fileView, COLUMN_FILE), TRUE);
	

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

	updateStatus();
}
	
void ShareBrowser::updateStatus()
{
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

	setStatus(itemsStatus, items);
	setStatus(sizeStatus, size);
	setStatus(filesStatus, files);
	setStatus(totalStatus, total);
}

gboolean ShareBrowser::buttonPressed(
	GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	sb->oldType = event->type;
	sb->oldButton = event->button;
	
	return FALSE;
}

gboolean ShareBrowser::buttonReleased(
	GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	GtkTreeIter iter;
	DirectoryListing::File *file;
	gpointer ptr;
	string target;

	if (sb->oldButton != event->button) return FALSE;

	if (GTK_TREE_VIEW(widget) == sb->fileView)
	{
		if (!gtk_tree_selection_get_selected(sb->fileSelection, NULL, NULL))
			return FALSE;
	}
	else
	{
		if (!gtk_tree_selection_get_selected(sb->dirSelection, NULL, NULL))
			return FALSE;
	}

	//single click left button
	if (event->button == 1 && sb->oldType == GDK_BUTTON_PRESS)
		if (GTK_TREE_VIEW(widget) == sb->dirView) sb->updateFiles(false);

	//single click right button
	if (event->button == 3 && sb->oldType == GDK_BUTTON_PRESS)
		if (GTK_TREE_VIEW(widget) == sb->dirView)
		{
			gtk_menu_popup(sb->dirMenu, NULL, NULL, NULL, NULL, 3, event->time);
			gtk_widget_show_all(GTK_WIDGET(sb->dirMenu));
		}
		else
		{
			gtk_menu_popup(sb->fileMenu, NULL, NULL, NULL, NULL, 3, event->time);
			gtk_widget_show_all(GTK_WIDGET(sb->fileMenu));
		}

	//double click left button
	if (event->button == 1 && sb->oldType == GDK_2BUTTON_PRESS)
		if (GTK_TREE_VIEW(widget) == sb->fileView)
		{
			gtk_tree_selection_get_selected(sb->fileSelection, NULL, &iter);
			gtk_tree_model_get(GTK_TREE_MODEL(sb->fileStore), &iter,
				COLUMN_DL_FILE, &ptr, -1);
			file = (DirectoryListing::File *)ptr;
		
			if (sb->listing.getUtf8())
				target += Text::utf8ToAcp(Util::getFileName(file->getName()));
			else
				target += Util::getFileName(file->getName());
			
			sb->downloadFile(file, target);
		}

	return FALSE;
}

void ShareBrowser::menuClicked(GtkMenuItem *item, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	string target;
	gpointer ptr;
	DirectoryListing::File *file;
	DirectoryListing::Directory *dir;
	GtkTreeIter iter;
	int ret;

	if (item == sb->dlFileTo || item == sb->dlDirTo || item == sb->dlDirToDir)
	{
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
	}
	else
	{
		target = Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY));
	}

	if (item == sb->dlFile || item == sb->dlFileTo)
	{
		gtk_tree_selection_get_selected(sb->fileSelection, NULL, &iter);
		gtk_tree_model_get(GTK_TREE_MODEL(sb->fileStore), &iter,
			COLUMN_DL_FILE, &ptr, -1);
		file = (DirectoryListing::File *)ptr;
		
		if (sb->listing.getUtf8())
			target += Text::utf8ToAcp(Util::getFileName(file->getName()));
		else
			target += Util::getFileName(file->getName());
			
		sb->downloadFile(file, target);
	}
	else
	{
		gtk_tree_selection_get_selected(sb->dirSelection, NULL, &iter);
		gtk_tree_model_get(GTK_TREE_MODEL(sb->dirStore), &iter,
			COLUMN_DL_DIR, &ptr, -1);
		dir = (DirectoryListing::Directory *)ptr;
		
		sb->downloadDir(dir, target);
	}
}

void ShareBrowser::downloadFile(DirectoryListing::File *file, string target)
{
	try {
		listing.download(file, target, false, false);
	} catch(const Exception& e) {
		setStatus(mainStatus, e.getError());
	}
}

void ShareBrowser::downloadDir(DirectoryListing::Directory *dir, string target)
{
	try {
		listing.download(dir, target, false);
	} catch(const Exception& e) {
		setStatus(mainStatus, e.getError());
	}
}

void ShareBrowser::buttonClicked(GtkWidget *widget, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	GtkButton *button = GTK_BUTTON(widget);

	if (button == sb->matchButton)
	{
		ostringstream stream;
		int matched;

		matched = QueueManager::getInstance()->matchListing(&sb->listing);
		stream << "Matched " << matched << " files";

		sb->setStatus(sb->mainStatus, stream.str());
	}

	if (button == sb->findButton)
	{
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

		sb->search = text;
		sb->findNext(true);
	}

	if (button == sb->nextButton)
	{
		sb->findNext(false);
	}
}

void ShareBrowser::findNext(bool firstFile)
{
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
				setStatus(mainStatus, "No files found");
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
				updateFiles(true);
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



