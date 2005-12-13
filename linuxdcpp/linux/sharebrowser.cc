
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
	WIDTH_TTH(150),
	WIDTH_EXACT_SIZE(105)
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

	//initiate icons
	iconFile = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
		"gnome-fs-regular", 16, (GtkIconLookupFlags) 0, NULL);
	iconDirectory = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
		"gnome-fs-directory", 16, (GtkIconLookupFlags) 0, NULL);
	
	//initiate the file treeview
	fileView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "fileView"));
	fileStore = gtk_list_store_new(9, 
		G_TYPE_STRING, 		// COLUMN_FILE
		G_TYPE_STRING, 		// COLUMN_SIZE
		G_TYPE_STRING, 		// COLUMN_TYPE
		G_TYPE_STRING, 		// COLUMN_TTH
		G_TYPE_STRING, 		// COLUMN_EXACT_SIZE
		G_TYPE_POINTER, 	// COLUMN_DL_FILE
		GDK_TYPE_PIXBUF, 	// COLUMN_ICON
		G_TYPE_DOUBLE, 		// COLUMN_SIZE_ORDER
		G_TYPE_STRING);		// COLUMN_FILE_ORDER
	gtk_tree_view_set_model(fileView, GTK_TREE_MODEL(fileStore));
	TreeViewFactory fileViewFactory(fileView);
	fileViewFactory.addColumn_gui(COLUMN_FILE, "File", TreeViewFactory::PIXBUF_STRING, WIDTH_FILE, COLUMN_ICON);
	fileViewFactory.addColumn_gui(COLUMN_SIZE, "Size", TreeViewFactory::STRINGR, WIDTH_SIZE);
	fileViewFactory.addColumn_gui(COLUMN_TYPE, "Type", TreeViewFactory::STRING, WIDTH_TYPE);
	fileViewFactory.addColumn_gui(COLUMN_TTH, "TTH", TreeViewFactory::STRING, WIDTH_TTH);
	fileViewFactory.addColumn_gui(COLUMN_EXACT_SIZE, "Exact Size", TreeViewFactory::STRINGR, WIDTH_EXACT_SIZE);
	fileViewFactory.setSortColumn_gui(COLUMN_FILE, COLUMN_FILE_ORDER);
	fileViewFactory.setSortColumn_gui(COLUMN_SIZE, COLUMN_SIZE_ORDER);
	fileViewFactory.setSortColumn_gui(COLUMN_EXACT_SIZE, COLUMN_SIZE_ORDER);
	fileSelection = gtk_tree_view_get_selection(fileView);
//	gtk_tree_view_column_set_sort_order(gtk_tree_view_get_column(fileView, COLUMN_FILE), GTK_SORT_ASCENDING);

	//initiate the dir treeview
	dirView = GTK_TREE_VIEW(glade_xml_get_widget(xml, "dirView"));
	dirStore = gtk_tree_store_new(3, 
		G_TYPE_STRING, 		// COLUMN_DIR
		G_TYPE_POINTER,		// COLUMN_DL_DIR
		GDK_TYPE_PIXBUF);	// COLUMN_ICON_DIR
	gtk_tree_view_set_model(dirView, GTK_TREE_MODEL(dirStore));
	TreeViewFactory dirViewFactory(dirView);
	dirViewFactory.addColumn_gui(COLUMN_DIR, "", TreeViewFactory::PIXBUF_STRING, -1, COLUMN_ICON_DIR);
	dirViewFactory.setSortColumn_gui(COLUMN_DIR, COLUMN_DIR);
	dirSelection = gtk_tree_view_get_selection(dirView);
//	gtk_tree_view_column_set_sort_order(gtk_tree_view_get_column(dirView, COLUMN_DIR), GTK_SORT_ASCENDING);

	//create popup menus
	dirMenu = GTK_MENU(gtk_menu_new());
	dlDir = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Download"));
	dlDirTo = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Download to..."));
	gtk_menu_shell_append(GTK_MENU_SHELL(dirMenu), GTK_WIDGET(dlDir));
	gtk_menu_shell_append(GTK_MENU_SHELL(dirMenu), GTK_WIDGET(dlDirTo));

	fileMenu = GTK_MENU(gtk_menu_new()); 
	dlFile = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Download"));
	dlFileTo = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Download to..."));
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), GTK_WIDGET(dlFile));
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), GTK_WIDGET(dlFileTo));

	//connect callbacks
	pressedCallback.connect(G_OBJECT(fileView), "button-press-event", NULL);
	pressedCallback.connect(G_OBJECT(dirView), "button-press-event", NULL);
	releasedCallback.connect_after(G_OBJECT(fileView), "button-release-event", NULL);
	releasedCallback.connect_after(G_OBJECT(dirView), "button-release-event", NULL);

	menuCallback.connect(G_OBJECT(dlDir), "activate", NULL);
	menuCallback.connect(G_OBJECT(dlDirTo), "activate", NULL);
	menuCallback.connect(G_OBJECT(dlFile), "activate", NULL);
	menuCallback.connect(G_OBJECT(dlFileTo), "activate", NULL);
	
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
	if (iconFile) g_object_unref(iconFile);
	if (iconDirectory) g_object_unref(iconDirectory);
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
			COLUMN_ICON_DIR, iconDirectory,
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
	DirectoryListing::Directory::List *dirs;
	DirectoryListing::File::List *files;
	DirectoryListing::Directory::Iter it_dir;
	DirectoryListing::File::Iter it_file;
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
	dirs = &(dir->directories);
	files = &(dir->files);

	gtk_list_store_clear(fileStore);

	currentSize = 0;
	currentItems = 0;

	//add dirs to store
	for (it_dir = dirs->begin(); it_dir != dirs->end(); it_dir++) {
		gtk_list_store_append(fileStore, &iter);

		//data needs to be converted to utf8 if it's not in that form
		if (listing.getUtf8()) {
			gtk_list_store_set(fileStore, &iter,
				COLUMN_FILE, Util::getFileName((*it_dir)->getName()).c_str(),
				COLUMN_FILE_ORDER, Util::getFileName("d"+(*it_dir)->getName()).c_str(),
				-1);
		} else {
			gtk_list_store_set(fileStore, &iter,
				COLUMN_FILE, Text::acpToUtf8(Util::getFileName((*it_dir)->getName())).c_str(),
				COLUMN_FILE_ORDER, Text::acpToUtf8(Util::getFileName("d"+(*it_dir)->getName())).c_str(),
				-1);
		}

		gtk_list_store_set(fileStore, &iter,
			COLUMN_ICON, iconDirectory,
			COLUMN_SIZE, Util::formatBytes((*it_dir)->getSize()).c_str(),
			COLUMN_EXACT_SIZE, Util::formatExactSize((*it_dir)->getSize()).c_str(),
			COLUMN_SIZE_ORDER, (*it_dir)->getSize(),
			COLUMN_DL_FILE, (gpointer)(*it_dir),
			-1);

		currentSize += (*it_dir)->getSize();
		currentItems++;
	}

	//add files to store
	for (it_file = files->begin(); it_file != files->end(); it_file++) {
		gtk_list_store_append(fileStore, &iter);

		//If ext is empty we cannot do substr on it
		string ext = Util::getFileExt((*it_file)->getName());
		if (ext.length() > 0) ext = ext.substr(1);

		//data needs to be converted to utf8 if it's not in that form
 		if (listing.getUtf8()) {
			gtk_list_store_set(fileStore, &iter,
				COLUMN_FILE, Util::getFileName((*it_file)->getName()).c_str(),
				COLUMN_TYPE, ext.c_str(),
				COLUMN_FILE_ORDER, Util::getFileName("f"+(*it_file)->getName()).c_str(),
				-1);
		} else {
			gtk_list_store_set(fileStore, &iter,
				COLUMN_FILE, Text::acpToUtf8(Util::getFileName((*it_file)->getName())).c_str(),
				COLUMN_TYPE, Text::acpToUtf8(ext).c_str(),
				COLUMN_FILE_ORDER, Text::acpToUtf8(Util::getFileName("f"+(*it_file)->getName())).c_str(),
				-1);
		}

		gtk_list_store_set(fileStore, &iter,
			COLUMN_ICON, iconFile,
			COLUMN_SIZE, Util::formatBytes((*it_file)->getSize()).c_str(),
			COLUMN_EXACT_SIZE, Util::formatExactSize((*it_file)->getSize()).c_str(),
			COLUMN_SIZE_ORDER, (*it_file)->getSize(),
			COLUMN_DL_FILE, (gpointer)(*it_file),
			-1);

		TTHValue *tth;
		if (tth = (*it_file)->getTTH())
			gtk_list_store_set(fileStore, &iter, COLUMN_TTH, tth->toBase32().c_str(), -1);
		else
			gtk_list_store_set(fileStore, &iter, COLUMN_TTH, "N/A", -1);

		currentSize += (*it_file)->getSize();
		currentItems++;
	}

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), COLUMN_FILE_ORDER, GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(fileView, COLUMN_FILE), TRUE);

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
	GtkTreeIter iter, parent_iter;
	GtkTreePath *path;
	DirectoryListing::File *file;
	DirectoryListing::Directory *dir;
	gpointer ptr, ptr2;
	string target;
	gchar *file_order;

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
		if (GTK_TREE_VIEW(widget) == dirView) {
			gtk_tree_selection_get_selected(dirSelection, NULL, &iter);
			path = gtk_tree_model_get_path(GTK_TREE_MODEL(dirStore), &iter);
			gtk_tree_view_expand_row(dirView, path, FALSE);
			gtk_tree_path_free(path);
		} else {
			gtk_tree_selection_get_selected(fileSelection, NULL, &iter);
			gtk_tree_model_get(GTK_TREE_MODEL(fileStore), &iter,
				COLUMN_DL_FILE, &ptr, -1);
			gtk_tree_model_get(GTK_TREE_MODEL(fileStore), &iter,
				COLUMN_FILE_ORDER, &file_order, -1);

			//if selected item is a directory jump to it
			if ((file_order != NULL) && (file_order[0] == 'd')) {
				dir = (DirectoryListing::Directory *)ptr;

				gtk_tree_selection_get_selected(dirSelection, NULL, &parent_iter);
				gtk_tree_model_iter_children(GTK_TREE_MODEL(dirStore), &iter, &parent_iter);
				gtk_tree_model_get(GTK_TREE_MODEL(dirStore), &iter, COLUMN_DL_DIR, &ptr2, -1);
				
				while ((ptr != ptr2) && gtk_tree_model_iter_next(GTK_TREE_MODEL(dirStore), &iter)) {
					gtk_tree_model_get(GTK_TREE_MODEL(dirStore), &iter, COLUMN_DL_DIR, &ptr2, -1);
				}

				path = gtk_tree_model_get_path(GTK_TREE_MODEL(dirStore), &iter);

				gtk_tree_view_expand_to_path(dirView, path);
				gtk_tree_view_set_cursor(dirView, path, NULL, FALSE);

				gtk_tree_path_free(path);
				
				updateFiles_gui(false);
			} else {
				file = (DirectoryListing::File *)ptr;
		
				if (listing.getUtf8())
					target += Text::utf8ToAcp(Util::getFileName(file->getName()));
				else
					target += Util::getFileName(file->getName());
			
				typedef Func2<ShareBrowser, DirectoryListing::File *, string> F2;
				F2 * func = new F2(this, &ShareBrowser::downloadFile_client, file, target);
				WulforManager::get()->dispatchClientFunc(func);
			}

			if (file_order) g_free(file_order);
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
	gchar *file_order;

	if (item == dlFileTo || item == dlDirTo) {
		GtkWidget *chooser = gtk_file_chooser_dialog_new(
			"Download to...",
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
			COLUMN_DL_FILE, &ptr, 
			COLUMN_FILE_ORDER, &file_order, -1);
		
		if ((file_order != NULL) && (file_order[0] == 'd')) {
			dir = (DirectoryListing::Directory *)ptr;
		
			typedef Func2<ShareBrowser, DirectoryListing::Directory *, string> F2;
			F2 * func = new F2(this, &ShareBrowser::downloadDir_client, dir, target);
			WulforManager::get()->dispatchClientFunc(func);
		} else {
			file = (DirectoryListing::File *)ptr;
		
			if (listing.getUtf8())
				target += Text::utf8ToAcp(Util::getFileName(file->getName()));
			else
				target += Util::getFileName(file->getName());
			
			typedef Func2<ShareBrowser, DirectoryListing::File *, string> F2;
			F2 * func = new F2(this, &ShareBrowser::downloadFile_client, file, target);
			WulforManager::get()->dispatchClientFunc(func);
		}

		if (file_order) g_free(file_order);
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
