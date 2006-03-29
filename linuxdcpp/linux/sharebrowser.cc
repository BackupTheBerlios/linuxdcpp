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

#include "sharebrowser.hh"

ShareBrowser::ShareBrowser(User::Ptr user, std::string file, GCallback closeCallback):
	BookEntry(WulforManager::SHARE_BROWSER, 
		user->getFullNick(), user->getNick(), closeCallback),
	listing(user),
	lastDir(""),
	posDir(NULL)
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
	g_signal_connect(G_OBJECT(matchButton), "clicked", G_CALLBACK(matchButtonClicked_gui), (gpointer)this);
	findButton = GTK_BUTTON(glade_xml_get_widget(xml, "findButton"));
	g_signal_connect(G_OBJECT(findButton), "clicked", G_CALLBACK(findButtonClicked_gui), (gpointer)this);
	nextButton = GTK_BUTTON(glade_xml_get_widget(xml, "nextButton"));
	g_signal_connect(G_OBJECT(nextButton), "clicked", G_CALLBACK(nextButtonClicked_gui), (gpointer)this);

	//initiate icons
	iconFile = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
		"gnome-fs-regular", 16, (GtkIconLookupFlags) 0, NULL);
	iconDirectory = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), 
		"gnome-fs-directory", 16, (GtkIconLookupFlags) 0, NULL);

	//initiate the file treeview
	fileView.setView(GTK_TREE_VIEW(glade_xml_get_widget(xml, "fileView")), true, "sharebrowser");
	fileView.insertColumn("Filename", G_TYPE_STRING, TreeView::PIXBUF_STRING, 400, "Icon");
	fileView.insertColumn("Size", G_TYPE_STRING, TreeView::STRINGR, 80);
	fileView.insertColumn("Type", G_TYPE_STRING, TreeView::STRING, 50);
	fileView.insertColumn("TTH", G_TYPE_STRING, TreeView::STRING, 150);
	fileView.insertColumn("Exact Size", G_TYPE_STRING, TreeView::STRINGR, 105);
	fileView.insertHiddenColumn("DL File", G_TYPE_POINTER);
	fileView.insertHiddenColumn("Icon", GDK_TYPE_PIXBUF);
	fileView.insertHiddenColumn("Size Order", G_TYPE_DOUBLE);
	fileView.insertHiddenColumn("File Order", G_TYPE_STRING);
	fileView.finalize();
	fileStore = gtk_list_store_newv(fileView.getColCount(), fileView.getGTypes());
	gtk_tree_view_set_model(fileView.get(), GTK_TREE_MODEL(fileStore));
	g_object_unref(fileStore);
	fileSelection = gtk_tree_view_get_selection(fileView.get());
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(fileView.get()), GTK_SELECTION_MULTIPLE);
	fileView.setSortColumn_gui("Filename", "File Order");
	fileView.setSortColumn_gui("Size", "Size Order");
	fileView.setSortColumn_gui("Exact Size", "Size Order");

	g_signal_connect(G_OBJECT(fileView.get()), "button_press_event", G_CALLBACK(fileButtonPressed_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(fileView.get()), "popup_menu", G_CALLBACK(filePopupMenu_gui), (gpointer)this);

	// Initialize the directory treeview
	dirView.setView(GTK_TREE_VIEW(glade_xml_get_widget(xml, "dirView")));
	dirView.insertColumn("Dir", G_TYPE_STRING, TreeView::PIXBUF_STRING, -1, "Icon");
	dirView.insertHiddenColumn("DL Dir", G_TYPE_POINTER);
	dirView.insertHiddenColumn("Icon", GDK_TYPE_PIXBUF);
	dirView.finalize();
	dirStore = gtk_tree_store_newv(dirView.getColCount(), dirView.getGTypes());
	gtk_tree_view_set_model(dirView.get(), GTK_TREE_MODEL(dirStore));
	g_object_unref(dirStore);
	dirSelection = gtk_tree_view_get_selection(dirView.get());
	g_signal_connect(G_OBJECT(dirView.get()), "button_press_event", G_CALLBACK(dirButtonPressed_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(dirView.get()), "button_release_event", G_CALLBACK(dirButtonReleased_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(dirView.get()), "popup_menu", G_CALLBACK(dirPopupMenu_gui), (gpointer)this);

	//create popup menus
	dirMenu = GTK_MENU(gtk_menu_new());
	dirDownloadMenu = GTK_MENU(gtk_menu_new());
	
	dirMenuItems["Download"] = gtk_menu_item_new_with_label("Download");
	gtk_menu_shell_append(GTK_MENU_SHELL(dirMenu), dirMenuItems["Download"]);
	g_signal_connect(G_OBJECT(dirMenuItems["Download"]), "activate", G_CALLBACK(downloadDirClicked_gui), (gpointer)this);
	dirMenuItems["DownloadTo"] = gtk_menu_item_new_with_label("Download to...");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(dirMenuItems["DownloadTo"]), GTK_WIDGET(dirDownloadMenu));
	gtk_menu_shell_append(GTK_MENU_SHELL(dirMenu), dirMenuItems["DownloadTo"]);

	fileMenu = GTK_MENU(gtk_menu_new());
	fileDownloadMenu = GTK_MENU(gtk_menu_new());
	
	fileMenuItems["Download"] = gtk_menu_item_new_with_label("Download");
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), fileMenuItems["Download"]);
	g_signal_connect(G_OBJECT(fileMenuItems["Download"]), "activate", G_CALLBACK(downloadClicked_gui), (gpointer)this);
	fileMenuItems["DownloadTo"] = gtk_menu_item_new_with_label("Download to...");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMenuItems["DownloadTo"]), GTK_WIDGET(fileDownloadMenu));
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), fileMenuItems["DownloadTo"]);
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), gtk_separator_menu_item_new());
	fileMenuItems["SearchForAlternates"] = gtk_menu_item_new_with_label("Search for alternates");
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), fileMenuItems["SearchForAlternates"]);
	g_signal_connect(G_OBJECT(fileMenuItems["SearchForAlternates"]), "activate", G_CALLBACK(searchAlternatesClicked_gui), (gpointer)this);
	
	//Set the buttons text to small so that the statusbar isn't too high.
	//This can't be set with glade, needs to be done in code.
	GtkLabel *label;
	label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(matchButton)));
	gtk_label_set_markup(label, "<span size=\"x-small\">Match Queue</span>");
	label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(findButton)));
	gtk_label_set_markup(label, "<span size=\"x-small\">Find</span>");
	label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(nextButton)));
	gtk_label_set_markup(label, "<span size=\"x-small\">Next</span>");
	
	//set the buttons in statusbar to same height as statusbar
	GtkRequisition statusReq;
	gtk_widget_size_request(GTK_WIDGET(mainStatus), &statusReq);
	gtk_widget_set_size_request(GTK_WIDGET(matchButton), -1, statusReq.height);
	gtk_widget_set_size_request(GTK_WIDGET(findButton), -1, statusReq.height);
	gtk_widget_set_size_request(GTK_WIDGET(nextButton), -1, statusReq.height);
	
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
				dirView.col("Dir"), (*it)->getName().c_str(),
				-1);
		} else {
			gtk_tree_store_set(dirStore, &newIter, 
				dirView.col("Dir"), Text::acpToUtf8((*it)->getName()).c_str(),
				-1);
		}
		
		gtk_tree_store_set(dirStore, &newIter, 
			dirView.col("DL Dir"), (gpointer)*it,
			dirView.col("Icon"), iconDirectory,
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
	int64_t size;
	
	//If this function is called from the find stuff the selection is not
	//yet updated sometimes. Thus we need to look at posDir.
	if (fromFind)
		gtk_tree_model_get_iter(GTK_TREE_MODEL(dirStore), &iter, posDir);
	else
		gtk_tree_selection_get_selected(dirSelection, NULL, &iter);

	gtk_tree_model_get(GTK_TREE_MODEL(dirStore), &iter,
		dirView.col("DL Dir"), &ptr,
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
				fileView.col("Filename"), Util::getFileName((*it_dir)->getName()).c_str(),
				fileView.col("File Order"), Util::getFileName("d"+(*it_dir)->getName()).c_str(),
				-1);
		} else {
			gtk_list_store_set(fileStore, &iter,
				fileView.col("Filename"), Text::acpToUtf8(Util::getFileName((*it_dir)->getName())).c_str(),
				fileView.col("File Order"), Text::acpToUtf8(Util::getFileName("d"+(*it_dir)->getName())).c_str(),
				-1);
		}

		size = (*it_dir)->getTotalSize(false);
		gtk_list_store_set(fileStore, &iter,
			fileView.col("Icon"), iconDirectory,
			fileView.col("Size"), Util::formatBytes(size).c_str(),
			fileView.col("Exact Size"), Util::formatExactSize(size).c_str(),
			fileView.col("Size Order"), (gdouble)size,
			fileView.col("DL File"), (gpointer)(*it_dir),
			-1);

		currentSize += size;
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
				fileView.col("Filename"), Util::getFileName((*it_file)->getName()).c_str(),
				fileView.col("Type"), ext.c_str(),
				fileView.col("File Order"), Util::getFileName("f"+(*it_file)->getName()).c_str(),
				-1);
		} else {
			gtk_list_store_set(fileStore, &iter,
				fileView.col("Filename"), Text::acpToUtf8(Util::getFileName((*it_file)->getName())).c_str(),
				fileView.col("Type"), Text::acpToUtf8(ext).c_str(),
				fileView.col("File Order"), Text::acpToUtf8(Util::getFileName("f"+(*it_file)->getName())).c_str(),
				-1);
		}

		size = (*it_file)->getSize();
		gtk_list_store_set(fileStore, &iter,
			fileView.col("Icon"), iconFile,
			fileView.col("Size"), Util::formatBytes(size).c_str(),
			fileView.col("Exact Size"), Util::formatExactSize(size).c_str(),
			fileView.col("Size Order"), (gdouble)size,
			fileView.col("DL File"), (gpointer)(*it_file),
			-1);

		TTHValue *tth;
		if (tth = (*it_file)->getTTH())
			gtk_list_store_set(fileStore, &iter, fileView.col("TTH"), tth->toBase32().c_str(), -1);
		else
			gtk_list_store_set(fileStore, &iter, fileView.col("TTH"), "N/A", -1);

		currentSize += size;
		currentItems++;
	}

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), fileView.col("File Order"), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(fileView.get(), fileView.col("Filename")), TRUE);
    gtk_tree_view_scroll_to_point(fileView.get(), 0, 0);
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

void ShareBrowser::buildDownloadMenus_gui(int menu) {
	menuUserData.clear();
	if(menu == 1) {
		for(vector<GtkWidget*>::iterator it=dirDownloadItems.begin(); it != dirDownloadItems.end(); it++)
			gtk_container_remove(GTK_CONTAINER (dirDownloadMenu), *it);
		dirDownloadItems.clear();	
			
		StringPairList spl = HubManager::getInstance()->getFavoriteDirs();
		if (spl.size() > 0)
		{
			for(StringPairIter i = spl.begin(); i != spl.end(); i++) 
			{
				dirDownloadItems.push_back(gtk_menu_item_new_with_label(i->second.c_str()));
				gtk_menu_shell_append(GTK_MENU_SHELL(dirDownloadMenu), dirDownloadItems.back());
				menuUserData.push_back(new userData(this, i->first));
				g_signal_connect(G_OBJECT(dirDownloadItems.back()), "activate", G_CALLBACK(downloadFavoriteDirClicked_gui), (gpointer)menuUserData.back());
			}
			dirDownloadItems.push_back(gtk_separator_menu_item_new());
			gtk_menu_shell_append(GTK_MENU_SHELL(dirDownloadMenu), dirDownloadItems.back());
		}
		dirDownloadItems.push_back(gtk_menu_item_new_with_label("Browse..."));
		gtk_menu_shell_append(GTK_MENU_SHELL(dirDownloadMenu), dirDownloadItems.back());
		
		g_signal_connect(G_OBJECT(dirDownloadItems.back()), "activate", G_CALLBACK(downloadDirToClicked_gui), (gpointer)this);
		gtk_widget_show_all(GTK_WIDGET(dirDownloadMenu));
	}
	else if(menu == 2) {
		for(vector<GtkWidget*>::iterator it=fileDownloadItems.begin(); it != fileDownloadItems.end(); it++)
			gtk_container_remove(GTK_CONTAINER(fileDownloadMenu), *it);
		fileDownloadItems.clear();
						
		StringPairList spl = HubManager::getInstance()->getFavoriteDirs();
		if (spl.size() > 0) 
		{
			for(StringPairIter i = spl.begin(); i != spl.end(); i++) 
			{
				fileDownloadItems.push_back(gtk_menu_item_new_with_label(i->second.c_str()));
				gtk_menu_shell_append(GTK_MENU_SHELL(fileDownloadMenu), fileDownloadItems.back());
				menuUserData.push_back(new userData(this, i->first));
				g_signal_connect(G_OBJECT(fileDownloadItems.back()), "activate", G_CALLBACK(downloadFavoriteClicked_gui), (gpointer)menuUserData.back());
			}
			fileDownloadItems.push_back(gtk_separator_menu_item_new());
			gtk_menu_shell_append(GTK_MENU_SHELL(fileDownloadMenu), fileDownloadItems.back());
		}
		fileDownloadItems.push_back(gtk_menu_item_new_with_label("Browse..."));
		gtk_menu_shell_append(GTK_MENU_SHELL(fileDownloadMenu), fileDownloadItems.back());
		g_signal_connect(G_OBJECT(fileDownloadItems.back()), "activate", G_CALLBACK(downloadToClicked_gui), (gpointer)this);
		gtk_widget_show_all(GTK_WIDGET(fileDownloadMenu));
	}
}

gboolean ShareBrowser::fileButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	ShareBrowser *sb = (ShareBrowser*)user_data;
	if(event->type == GDK_BUTTON_PRESS) {
		if(event->button == 3) {
			sb->buildDownloadMenus_gui(2);
			gtk_menu_popup(sb->fileMenu, NULL, NULL, NULL, NULL,
						   (event != NULL) ? 1 : 0,
						   gdk_event_get_time((GdkEvent*)event));
			gtk_widget_show_all(GTK_WIDGET(sb->fileMenu));
			return TRUE;
		}
	}
	
	if(event->type == GDK_2BUTTON_PRESS) {
		if(event->button == 1) {
			gpointer ptr, ptr2;
			DirectoryListing::Directory *dir;
			string file_order;
			GtkTreeIter iter, parent_iter;
			GtkTreePath *path;
			GtkTreeModel *m = GTK_TREE_MODEL(sb->fileStore);
			GList *list = gtk_tree_selection_get_selected_rows(sb->fileSelection, &m);
			gint count = gtk_tree_selection_count_selected_rows(sb->fileSelection);
			GList *tmp = g_list_first(list);
			
			if(count < 1 || count > 1)
				return FALSE;
			
			if(!tmp)
				return FALSE;

			if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(sb->fileStore), &iter, (GtkTreePath*)tmp->data))
				return FALSE;
				
			ptr = sb->fileView.getValue<gpointer>(&iter, "DL File");
			file_order = sb->fileView.getString(&iter, "File Order");
			
			if((!file_order.empty()) && (file_order[0] == 'd')) {
				dir = (DirectoryListing::Directory *)ptr;

				gtk_tree_selection_get_selected(sb->dirSelection, NULL, &parent_iter);
				gtk_tree_model_iter_children(GTK_TREE_MODEL(sb->dirStore), &iter, &parent_iter);
				ptr2 = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
				
				while ((ptr != ptr2) && gtk_tree_model_iter_next(GTK_TREE_MODEL(sb->dirStore), &iter)) {
					ptr2 = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
				}

				path = gtk_tree_model_get_path(GTK_TREE_MODEL(sb->dirStore), &iter);

				gtk_tree_view_expand_to_path(sb->dirView.get(), path);
				gtk_tree_view_set_cursor(sb->dirView.get(), path, NULL, FALSE);

				gtk_tree_path_free(path);
				
				sb->updateFiles_gui(false);
			}
			else {
				sb->downloadSelectedFiles_gui(Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY)));
			}
		}
	}
	
	return FALSE;
}

gboolean ShareBrowser::filePopupMenu_gui(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	gtk_menu_popup(((ShareBrowser*)user_data)->fileMenu, NULL, NULL, NULL, NULL,
				   (event != NULL) ? 1 : 0,
				   gdk_event_get_time((GdkEvent*)event));
	gtk_widget_show_all(GTK_WIDGET(((ShareBrowser*)user_data)->fileMenu));
}

gboolean ShareBrowser::dirButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	((ShareBrowser*)user_data)->oldType = event->type;
	((ShareBrowser*)user_data)->oldButton = event->button;
	
	return FALSE;
}

gboolean ShareBrowser::dirButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	ShareBrowser *sb = (ShareBrowser*)user_data;
	GtkTreeIter iter;
	GtkTreePath *path;
	if(sb->oldButton != event->button)
		return FALSE;

	GtkTreeSelection *selection = gtk_tree_view_get_selection(sb->dirView.get());
	
	if(!gtk_tree_selection_get_selected(selection, NULL, NULL))
		return FALSE;
		
	if(event->button == 1 && sb->oldType == GDK_BUTTON_PRESS)
		sb->updateFiles_gui(false);
		
	if(event->button == 3 && sb->oldType == GDK_BUTTON_PRESS) {
		sb->buildDownloadMenus_gui(1);
		gtk_menu_popup(sb->dirMenu, NULL, NULL, NULL, NULL,
					   (event != NULL) ? 1 : 0,
					   gdk_event_get_time((GdkEvent*)event));
		gtk_widget_show_all(GTK_WIDGET(sb->dirMenu));
	}
	
	if(event->button == 1 && sb->oldType == GDK_2BUTTON_PRESS) {
		gtk_tree_selection_get_selected(selection, NULL, &iter);
		path = gtk_tree_model_get_path(GTK_TREE_MODEL(sb->dirStore), &iter);
		gtk_tree_view_expand_row(sb->dirView.get(), path, FALSE);
		gtk_tree_path_free(path);
	}
	
	return FALSE;
}

gboolean ShareBrowser::dirPopupMenu_gui(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
	gtk_menu_popup(((ShareBrowser*)user_data)->dirMenu, NULL, NULL, NULL, NULL,
				   (event != NULL) ? 1 : 0,
				   gdk_event_get_time((GdkEvent*)event));
	gtk_widget_show_all(GTK_WIDGET(((ShareBrowser*)user_data)->dirMenu));
}

void ShareBrowser::matchButtonClicked_gui(GtkWidget *widget, gpointer user_data) {
	typedef Func0<ShareBrowser> F0;
	F0 *f = new F0((ShareBrowser*)user_data, &ShareBrowser::matchQueue_client);
	WulforManager::get()->dispatchClientFunc(f);
}

void ShareBrowser::findButtonClicked_gui(GtkWidget *widget, gpointer user_data) {
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
	if(ret == GTK_RESPONSE_CANCEL)
		return;

	((ShareBrowser*)user_data)->search = text;
	((ShareBrowser*)user_data)->findNext_gui(true);
}

void ShareBrowser::nextButtonClicked_gui(GtkWidget *widget, gpointer user_data) {
	((ShareBrowser*)user_data)->findNext_gui(false);
}

void ShareBrowser::downloadClicked_gui(GtkMenuItem *item, gpointer user_data) {
	((ShareBrowser*)user_data)->downloadSelectedFiles_gui(Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY)));
}

void ShareBrowser::downloadToClicked_gui(GtkMenuItem *item, gpointer user_data) {
	ShareBrowser *sb = (ShareBrowser*)user_data;
	GtkWidget *chooser = gtk_file_chooser_dialog_new("Choose location",
												WulforManager::get()->getMainWindow()->getWindow(),
												GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
												GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
												GTK_STOCK_OPEN, GTK_RESPONSE_OK,
												NULL);								
	if(sb->lastDir != "")
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), sb->lastDir.c_str());
	else
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY)).c_str());			
	gint response = gtk_dialog_run(GTK_DIALOG (chooser));
	if(response == GTK_RESPONSE_OK) {
		string path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(chooser));
		if(path[path.length ()-1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sb->lastDir = path;
		
		sb->downloadSelectedFiles_gui(path);
	}
	gtk_widget_hide(chooser);
	chooser = NULL;	
}

void ShareBrowser::downloadFavoriteClicked_gui(GtkMenuItem *item, gpointer user_data) {
	userData *data = (userData*)user_data;

	data->first->downloadSelectedFiles_gui(data->second);
	
	delete data;
}

void ShareBrowser::downloadDirClicked_gui(GtkMenuItem *item, gpointer user_data) {
	((ShareBrowser*)user_data)->downloadSelectedDirs_gui(Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY)));
}

void ShareBrowser::downloadDirToClicked_gui(GtkMenuItem *item, gpointer user_data) {
	ShareBrowser *sb = (ShareBrowser*)user_data;
	GtkWidget *chooser = gtk_file_chooser_dialog_new("Choose location",
												WulforManager::get()->getMainWindow()->getWindow(),
												GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
												GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
												GTK_STOCK_OPEN, GTK_RESPONSE_OK,
												NULL);								
	if(sb->lastDir != "")
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), sb->lastDir.c_str());
	else
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY)).c_str());			
	gint response = gtk_dialog_run(GTK_DIALOG (chooser));
	if(response == GTK_RESPONSE_OK) {
		string path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(chooser));
		if(path[path.length ()-1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sb->lastDir = path;
		
		sb->downloadSelectedDirs_gui(path);
	}
	gtk_widget_hide(chooser);
	chooser = NULL;	
}
void ShareBrowser::downloadFavoriteDirClicked_gui(GtkMenuItem *item, gpointer user_data) {
	userData *data = (userData*)user_data;
	
	data->first->downloadSelectedDirs_gui(data->second);
	
	delete data;
}

void ShareBrowser::downloadSelectedDirs_gui(string target) {
	gpointer ptr;
	DirectoryListing::Directory *dir;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(dirView.get());
	gtk_tree_selection_get_selected(selection, NULL, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(dirStore), &iter,
					   dirView.col("DL Dir"), &ptr, -1);
	dir = (DirectoryListing::Directory *)ptr;
		
	typedef Func2<ShareBrowser, DirectoryListing::Directory *, string> F2;
	F2 * func = new F2(this, &ShareBrowser::downloadDir_client, dir, target);
	WulforManager::get()->dispatchClientFunc(func);
}

void ShareBrowser::downloadSelectedFiles_gui(string target) {
	gpointer ptr;
	DirectoryListing::File *file;
	DirectoryListing::Directory *dir;
	string file_order;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(fileView.get());
	GtkTreeModel *m = GTK_TREE_MODEL(fileStore);
	GList *list = gtk_tree_selection_get_selected_rows(selection, &m);
	GList *tmp = g_list_first(list);
	vector<GtkTreeIter> iters;
	GtkTreeIter tmpiter;

	if(!tmp)
		return;

	while(1) {
		if(gtk_tree_model_get_iter(m, &tmpiter, (GtkTreePath*)tmp->data))
			iters.push_back(tmpiter);
	
		tmp = g_list_next(tmp);
		if(!tmp)
			break;
	}
	g_list_free(tmp);
	g_list_free(list);
	for (int i=0;i<iters.size ();i++) {
		ptr = fileView.getValue<gpointer>(&iters[i], "DL File");
		file_order = fileView.getString(&iters[i], "File Order");
		if ((!file_order.empty()) && (file_order[0] == 'd')) {
			dir = (DirectoryListing::Directory *)ptr;
	
			typedef Func2<ShareBrowser, DirectoryListing::Directory *, string> F2;
			F2 * func = new F2(this, &ShareBrowser::downloadDir_client, dir, target);
			WulforManager::get()->dispatchClientFunc(func);
		} else {
			file = (DirectoryListing::File *)ptr;
			string filename;
			if (listing.getUtf8())
				filename = Util::getFileName(file->getName());
			else
				filename = Text::acpToUtf8(Util::getFileName(file->getName()));
		
			typedef Func2<ShareBrowser, DirectoryListing::File *, string> F2;
			F2 * func = new F2(this, &ShareBrowser::downloadFile_client, file, target + filename);
			WulforManager::get()->dispatchClientFunc(func);
		}
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

void ShareBrowser::searchAlternatesClicked_gui(GtkMenuItem *item, gpointer user_data) {
	ShareBrowser *sb = (ShareBrowser*)user_data;
	gpointer ptr;
	DirectoryListing::File *file;
	string file_order;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(sb->fileView.get());
	GtkTreeModel *m = GTK_TREE_MODEL(sb->fileStore);
	GList *list = gtk_tree_selection_get_selected_rows(selection, &m);
	GList *tmp = g_list_first(list);
	vector<GtkTreeIter> iters;
	GtkTreeIter tmpiter;

	if(!tmp)
		return;

	while(1) {
		if(gtk_tree_model_get_iter(m, &tmpiter, (GtkTreePath*)tmp->data))
			iters.push_back(tmpiter);
	
		tmp = g_list_next(tmp);
		if(!tmp)
			break;
	}
	g_list_free(tmp);
	g_list_free(list);
	for (int i=0;i<iters.size ();i++) {
		ptr = sb->fileView.getValue<gpointer>(&iters[i], "DL File");
		file_order = sb->fileView.getString(&iters[i], "File Order");
		string target;

		if ((!file_order.empty()) && (file_order[0] == 'd')) {
			continue;
		} else {
			file = (DirectoryListing::File *)ptr;
			Search *s = WulforManager::get ()->addSearch_gui();
			
			if(file->getTTH()) {
				s->putValue(file->getTTH()->toBase32(), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
			}
			else {
				bool bigFile = (file->getSize() > 10*1024*1024);
				if(sb->listing.getUtf8())
					target = Util::getFileName(file->getName());
				else
					target = Text::acpToUtf8(Util::getFileName(file->getName()));
					
				if(!target.empty()) {
					if(bigFile)
						s->putValue(SearchManager::clean(target), file->getSize()-1, SearchManager::SIZE_ATLEAST, ShareManager::getInstance()->getType(target));
					else
						s->putValue(SearchManager::clean(target), file->getSize()+1, SearchManager::SIZE_ATMOST, ShareManager::getInstance()->getType(target));				
				}
			}
		}
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
	if(gtk_tree_path_get_depth(posDir) == 0)
		return;
	if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(dirStore), &iter, posDir)) 
		return;
	gtk_tree_model_get(GTK_TREE_MODEL(dirStore), &iter,
		dirView.col("DL Dir"), &ptr,
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
			if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(dirStore), &iter, posDir)) {
				gtk_tree_path_up(posDir);
				gtk_tree_path_next(posDir);
				if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(dirStore), &iter, posDir)) {
					found = false;
				}
			}

			//Now searching upwards through the tree
			while (!found) {
				if(!gtk_tree_path_up(posDir))
					break;
				if(gtk_tree_path_get_depth(posDir) == 0)
					break;

				gtk_tree_path_next(posDir);
				found = gtk_tree_model_get_iter(GTK_TREE_MODEL(dirStore), &iter, posDir);
			}

			if (!found) {
				setStatus_gui(mainStatus, "No files found");
				return;
			}
				
			gtk_tree_model_get(GTK_TREE_MODEL(dirStore), &iter, dirView.col("DL Dir"), &ptr, -1);
			dir = (DirectoryListing::Directory *)ptr;
			files = &(dir->files);
			posFile = files->begin();
		}

		string filename = (*posFile)->getName();
		if (!listing.getUtf8())
			filename = Text::acpToUtf8(filename);

		//This is if we find the file, we need to select it and 
		//expand its dir in the dir view
		if(filename.find(search, 0) != string::npos) {
			if(updatedPosDir) {
				gtk_tree_view_expand_row(dirView.get(), posDir, FALSE);
				gtk_tree_view_set_cursor(dirView.get(), posDir, NULL, FALSE);
				updateFiles_gui(true);
			}
			
			//Select the file in the file view
			gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(fileStore), &iter);
			while (valid)
			{

				gtk_tree_model_get(GTK_TREE_MODEL(fileStore), &iter,
					fileView.col("DL File"), &ptr, -1);
				DirectoryListing::File *file = (DirectoryListing::File *)ptr;
				if (file == *posFile) {
					GtkTreePath *path;
					path = gtk_tree_model_get_path(GTK_TREE_MODEL(fileStore), &iter);
					gtk_tree_view_set_cursor(fileView.get(), path, NULL, FALSE);
					gtk_tree_path_free(path);
					return;
				}
				
				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(fileStore), &iter);
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
