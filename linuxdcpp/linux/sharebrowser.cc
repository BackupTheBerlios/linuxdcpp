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

#include "sharebrowser.hh"

ShareBrowser::ShareBrowser(User::Ptr user, std::string file):
	BookEntry("List: " + WulforUtil::getNicks(user)),
	listing(user),
	lastDir(""),
	updateFileView(TRUE)
{
	GladeXML *xml = getGladeXML("sharebrowser.glade");

	GtkWidget *window = glade_xml_get_widget(xml, "window");
	box = glade_xml_get_widget(xml, "box");
	gtk_widget_ref(box);
	gtk_container_remove(GTK_CONTAINER(window), box);
	gtk_widget_destroy(window);

	findDialog = glade_xml_get_widget(xml, "findDialog");
	findEntry = GTK_ENTRY(glade_xml_get_widget(xml, "findEntry"));
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(findDialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	mainStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "mainStatus"));
	itemsStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "itemsStatus"));
	sizeStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "sizeStatus"));
	filesStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "filesStatus"));
	totalStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "totalStatus"));

	matchButton = GTK_BUTTON(glade_xml_get_widget(xml, "matchButton"));
	g_signal_connect(G_OBJECT(matchButton), "clicked", G_CALLBACK(onMatchButtonClicked_gui), (gpointer)this);
	findButton = GTK_BUTTON(glade_xml_get_widget(xml, "findButton"));
	g_signal_connect(G_OBJECT(findButton), "clicked", G_CALLBACK(onFindButtonClicked_gui), (gpointer)this);
	nextButton = GTK_BUTTON(glade_xml_get_widget(xml, "nextButton"));
	g_signal_connect(G_OBJECT(nextButton), "clicked", G_CALLBACK(onNextButtonClicked_gui), (gpointer)this);

	// Load icons
	iconFile = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
		GTK_STOCK_FILE, 16, (GtkIconLookupFlags) 0, NULL);
	iconDirectory = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
		GTK_STOCK_DIRECTORY, 16, (GtkIconLookupFlags) 0, NULL);

	// Initialize the file TreeView
	fileView.setView(GTK_TREE_VIEW(glade_xml_get_widget(xml, "fileView")), true, "sharebrowser");
	fileView.insertColumn("Filename", G_TYPE_STRING, TreeView::PIXBUF_STRING, 400, "Icon");
	fileView.insertColumn("Size", G_TYPE_STRING, TreeView::STRINGR, 80);
	fileView.insertColumn("Type", G_TYPE_STRING, TreeView::STRING, 50);
	fileView.insertColumn("TTH", G_TYPE_STRING, TreeView::STRING, 150);
	fileView.insertColumn("Exact Size", G_TYPE_STRING, TreeView::STRINGR, 105);
	fileView.insertHiddenColumn("DL File", G_TYPE_POINTER);
	fileView.insertHiddenColumn("Icon", GDK_TYPE_PIXBUF);
	fileView.insertHiddenColumn("Size Order", G_TYPE_INT64);
	fileView.insertHiddenColumn("File Order", G_TYPE_STRING);
	fileView.finalize();
	fileStore = gtk_list_store_newv(fileView.getColCount(), fileView.getGTypes());
	gtk_tree_view_set_model(fileView.get(), GTK_TREE_MODEL(fileStore));
	g_object_unref(fileStore);
	fileSelection = gtk_tree_view_get_selection(fileView.get());
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(fileView.get()), GTK_SELECTION_MULTIPLE);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), fileView.col("File Order"), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(fileView.get(), fileView.col("Filename")), TRUE);
	gtk_tree_view_set_fixed_height_mode(fileView.get(), TRUE);
	fileView.setSortColumn_gui("Filename", "File Order");
	fileView.setSortColumn_gui("Size", "Size Order");
	fileView.setSortColumn_gui("Exact Size", "Size Order");
	g_signal_connect(G_OBJECT(fileView.get()), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(fileView.get()), "button-release-event", G_CALLBACK(onFileButtonReleased_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(fileView.get()), "key-release-event", G_CALLBACK(onFileKeyReleased_gui), (gpointer)this);

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
	g_signal_connect(G_OBJECT(dirView.get()), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(dirView.get()), "button-release-event", G_CALLBACK(onDirButtonReleased_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(dirView.get()), "key-release-event", G_CALLBACK(onDirKeyReleased_gui), (gpointer)this);

	// Create the popup menus
	dirMenu = GTK_MENU(gtk_menu_new());
	dirDownloadMenu = GTK_MENU(gtk_menu_new());
	dirMenuItems["Download"] = gtk_menu_item_new_with_label("Download");
	gtk_menu_shell_append(GTK_MENU_SHELL(dirMenu), dirMenuItems["Download"]);
	g_signal_connect(G_OBJECT(dirMenuItems["Download"]), "activate", G_CALLBACK(onDownloadDirClicked_gui), (gpointer)this);
	dirMenuItems["DownloadTo"] = gtk_menu_item_new_with_label("Download to...");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(dirMenuItems["DownloadTo"]), GTK_WIDGET(dirDownloadMenu));
	gtk_menu_shell_append(GTK_MENU_SHELL(dirMenu), dirMenuItems["DownloadTo"]);

	fileMenu = GTK_MENU(gtk_menu_new());
	fileDownloadMenu = GTK_MENU(gtk_menu_new());
	fileMenuItems["Download"] = gtk_menu_item_new_with_label("Download");
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), fileMenuItems["Download"]);
	g_signal_connect(G_OBJECT(fileMenuItems["Download"]), "activate", G_CALLBACK(onDownloadClicked_gui), (gpointer)this);
	fileMenuItems["DownloadTo"] = gtk_menu_item_new_with_label("Download to...");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMenuItems["DownloadTo"]), GTK_WIDGET(fileDownloadMenu));
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), fileMenuItems["DownloadTo"]);
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), gtk_separator_menu_item_new());
	fileMenuItems["SearchForAlternates"] = gtk_menu_item_new_with_label("Search for alternates");
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), fileMenuItems["SearchForAlternates"]);
	g_signal_connect(G_OBJECT(fileMenuItems["SearchForAlternates"]), "activate", G_CALLBACK(onSearchAlternatesClicked_gui), (gpointer)this);

	// Set the buttons text to small so that the statusbar isn't too high.
	// This can't be set with glade, needs to be done in code.
	GtkLabel *label;
	label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(matchButton)));
	gtk_label_set_markup(label, "<span size=\"x-small\">Match Queue</span>");
	label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(findButton)));
	gtk_label_set_markup(label, "<span size=\"x-small\">Find</span>");
	label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(nextButton)));
	gtk_label_set_markup(label, "<span size=\"x-small\">Next</span>");

	// Set the buttons in statusbar to same height as statusbar
	GtkRequisition statusReq;
	gtk_widget_size_request(GTK_WIDGET(mainStatus), &statusReq);
	gtk_widget_set_size_request(GTK_WIDGET(matchButton), -1, statusReq.height);
	gtk_widget_set_size_request(GTK_WIDGET(findButton), -1, statusReq.height);
	gtk_widget_set_size_request(GTK_WIDGET(nextButton), -1, statusReq.height);

	try
	{
		listing.loadFile(file);
	}
	catch (const Exception& e)
	{
		setStatus_gui(mainStatus, "Unable to load file list: " + e.getError());
	}

	shareSize = 0;
	shareItems = 0;
	buildDirs_gui(listing.getRoot()->directories, NULL);
	updateStatus_gui();
}

ShareBrowser::~ShareBrowser()
{
	if (iconFile)
		g_object_unref(iconFile);
	if (iconDirectory)
		g_object_unref(iconDirectory);

	gtk_widget_destroy(findDialog);
}

GtkWidget *ShareBrowser::getWidget()
{
	return box;
}

void ShareBrowser::setPosition_gui(string pos)
{
	cout << "try to set path: " << pos << endl;
}

void ShareBrowser::buildDirs_gui(DirectoryListing::Directory::List dirs, GtkTreeIter *iter)
{
	DirectoryListing::Directory::Iter it;
	DirectoryListing::File::Iter file;
	GtkTreeIter newIter;

	std::sort(dirs.begin(), dirs.end(), DirectoryListing::Directory::DirSort());

	for (it = dirs.begin(); it != dirs.end(); it++)
	{
		gtk_tree_store_append(dirStore, &newIter, iter);

		// Add the name and check if it is utf-8.
		if (listing.getUtf8())
			gtk_tree_store_set(dirStore, &newIter, dirView.col("Dir"), (*it)->getName().c_str(), -1);
		else
			gtk_tree_store_set(dirStore, &newIter, dirView.col("Dir"), Text::acpToUtf8((*it)->getName()).c_str(), -1);

		gtk_tree_store_set(dirStore, &newIter,
			dirView.col("DL Dir"), (gpointer)*it,
			dirView.col("Icon"), iconDirectory,
			-1);

		std::sort((*it)->files.begin(), (*it)->files.end(), DirectoryListing::File::FileSort());
		for (file = (*it)->files.begin(); file != (*it)->files.end(); file++)
		{
			shareItems++;
			shareSize += (*file)->getSize();
		}

		buildDirs_gui((*it)->directories, &newIter);
	}

}

void ShareBrowser::buildDirDownloadMenu_gui()
{
	GtkWidget *menuItem;

	gtk_container_foreach(GTK_CONTAINER(dirDownloadMenu), (GtkCallback)gtk_widget_destroy, NULL);

	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if (spl.size() > 0)
	{
		for (StringPairIter i = spl.begin(); i != spl.end(); i++)
		{
			menuItem = gtk_menu_item_new_with_label(i->second.c_str());
			g_object_set_data_full(G_OBJECT(menuItem), "fav", g_strdup(i->first.c_str()), g_free);
			g_signal_connect(G_OBJECT(menuItem), "activate", G_CALLBACK(onDownloadFavoriteDirClicked_gui), (gpointer)this);
			gtk_menu_shell_append(GTK_MENU_SHELL(dirDownloadMenu), menuItem);
		}
		menuItem = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(dirDownloadMenu), menuItem);
	}

	menuItem = gtk_menu_item_new_with_label("Browse...");
	g_signal_connect(G_OBJECT(menuItem), "activate", G_CALLBACK(onDownloadDirToClicked_gui), (gpointer)this);
	gtk_menu_shell_append(GTK_MENU_SHELL(dirDownloadMenu), menuItem);
}

void ShareBrowser::buildFileDownloadMenu_gui()
{
	GtkWidget *menuItem;

	gtk_container_foreach(GTK_CONTAINER(fileDownloadMenu), (GtkCallback)gtk_widget_destroy, NULL);

	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if (spl.size() > 0)
	{
		for (StringPairIter i = spl.begin(); i != spl.end(); i++)
		{
			menuItem = gtk_menu_item_new_with_label(i->second.c_str());
			g_object_set_data_full(G_OBJECT(menuItem), "fav", g_strdup(i->first.c_str()), g_free);
			g_signal_connect(G_OBJECT(menuItem), "activate", G_CALLBACK(onDownloadFavoriteClicked_gui), (gpointer)this);
			gtk_menu_shell_append(GTK_MENU_SHELL(fileDownloadMenu), menuItem);
		}
		menuItem = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(fileDownloadMenu), menuItem);
	}

	menuItem = gtk_menu_item_new_with_label("Browse...");
	g_signal_connect(G_OBJECT(menuItem), "activate", G_CALLBACK(onDownloadToClicked_gui), (gpointer)this);
	gtk_menu_shell_append(GTK_MENU_SHELL(fileDownloadMenu), menuItem);
}

void ShareBrowser::updateFiles_gui(DirectoryListing::Directory *dir)
{
	DirectoryListing::Directory::List *dirs = &(dir->directories);
	DirectoryListing::Directory::Iter it_dir;
	DirectoryListing::File::List *files = &(dir->files);
	DirectoryListing::File::Iter it_file;
	gpointer ptr;
	GtkTreeIter iter;
	int64_t size;
	gint sortColumn;
	GtkSortType sortType;

	currentSize = 0;
	currentItems = 0;

	gtk_list_store_clear(fileStore);

	gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(fileStore), &sortColumn, &sortType);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, sortType);

	// Add directories to the store.
	for (it_dir = dirs->begin(); it_dir != dirs->end(); it_dir++)
	{
		gtk_list_store_append(fileStore, &iter);

		// Data needs to be converted to utf-8 if it's not in that form.
		if (listing.getUtf8())
		{
			gtk_list_store_set(fileStore, &iter,
				fileView.col("Filename"), Util::getFileName((*it_dir)->getName()).c_str(),
				fileView.col("File Order"), Util::getFileName("d"+(*it_dir)->getName()).c_str(),
				-1);
		}
		else
		{
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
			fileView.col("Size Order"), size,
			fileView.col("Type"), "Folder",
			fileView.col("DL File"), (gpointer)(*it_dir),
			-1);

		currentSize += size;
		currentItems++;
	}

	// Add files to the store.
	for (it_file = files->begin(); it_file != files->end(); it_file++)
	{
		gtk_list_store_append(fileStore, &iter);

		// If ext is empty we cannot do substr on it.
		string ext = Util::getFileExt((*it_file)->getName());
		if (ext.length() > 0)
			ext = ext.substr(1);

		// Data needs to be converted to utf-8 if it's not in that form.
 		if (listing.getUtf8())
 		{
			gtk_list_store_set(fileStore, &iter,
				fileView.col("Filename"), Util::getFileName((*it_file)->getName()).c_str(),
				fileView.col("Type"), ext.c_str(),
				fileView.col("File Order"), Util::getFileName("f"+(*it_file)->getName()).c_str(),
				-1);
		}
		else
		{
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
			fileView.col("Size Order"), size,
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

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), sortColumn, sortType);
	gtk_tree_view_scroll_to_point(fileView.get(), 0, 0);
	updateStatus_gui();
	updateFileView = TRUE;
}

void ShareBrowser::updateStatus_gui()
{
	string items, files, size, total;

	files = "Files: " + Util::toString(shareItems);
	total = "Total: " + Util::formatBytes(shareSize);
	if (gtk_tree_selection_get_selected(dirSelection, NULL, NULL))
	{
		items = "Items: " + Util::toString(currentItems);
		size = "Size: " + Util::formatBytes(currentSize);
	}
	else
	{
		items = "Items: 0";
		size = "Size: 0 B";
	}

	setStatus_gui(itemsStatus, items);
	setStatus_gui(sizeStatus, size);
	setStatus_gui(filesStatus, files);
	setStatus_gui(totalStatus, total);
}

void ShareBrowser::setStatus_gui(GtkStatusbar *status, std::string msg)
{
	gtk_statusbar_pop(status, 0);
	gtk_statusbar_push(status, 0, msg.c_str());
}

void ShareBrowser::fileViewSelected_gui()
{
	gpointer ptr;
	string fileOrder;
	GtkTreeIter iter, parentIter;
	GtkTreeModel *m = GTK_TREE_MODEL(fileStore);
	GList *list = gtk_tree_selection_get_selected_rows(fileSelection, NULL);
	GtkTreePath *path = (GtkTreePath *)g_list_nth_data(list, 0);

	if (!gtk_tree_model_get_iter(m, &iter, path))
		return;

	gtk_tree_path_free(path);
	ptr = fileView.getValue<gpointer>(&iter, "DL File");
	fileOrder = fileView.getString(&iter, "File Order");

	if (fileOrder[0] == 'd' && gtk_tree_selection_get_selected(dirSelection, NULL, &parentIter))
	{
		m = GTK_TREE_MODEL(dirStore);
		gboolean valid = gtk_tree_model_iter_children(m, &iter, &parentIter);

		while (valid && ptr != dirView.getValue<gpointer>(&iter, "DL Dir"))
			valid = gtk_tree_model_iter_next(m, &iter);

		path = gtk_tree_model_get_path(m, &iter);
		gtk_tree_view_expand_to_path(dirView.get(), path);
		gtk_tree_view_set_cursor(dirView.get(), path, NULL, FALSE);
		gtk_tree_path_free(path);

		updateFiles_gui((DirectoryListing::Directory *)ptr);
	}
	else
		downloadSelectedFiles_gui(Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY)));

	g_list_free(list);
}

void ShareBrowser::downloadSelectedFiles_gui(string target)
{
	gpointer ptr;
	string fileOrder;
	string filename;
	GtkTreeIter iter;
	GtkTreePath *path;
	DirectoryListing::File *file;
	DirectoryListing::Directory *dir;
	GtkTreeModel *m = GTK_TREE_MODEL(fileStore);
	GList *list = gtk_tree_selection_get_selected_rows(fileSelection, NULL);
	gint count = gtk_tree_selection_count_selected_rows(fileSelection);

	for (int i = 0; i < count; i++)
	{
		path = (GtkTreePath *)g_list_nth_data(list, i);
		if (!gtk_tree_model_get_iter(m, &iter, path))
		{
			gtk_tree_path_free(path);
			continue;
		}


		ptr = fileView.getValue<gpointer>(&iter, "DL File");
		fileOrder = fileView.getString(&iter, "File Order");

		if (fileOrder[0] == 'd')
		{
			dir = (DirectoryListing::Directory *)ptr;

			typedef Func2<ShareBrowser, DirectoryListing::Directory *, string> F2;
			F2 * func = new F2(this, &ShareBrowser::downloadDir_client, dir, target);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else
		{
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
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void ShareBrowser::downloadSelectedDirs_gui(string target)
{
	DirectoryListing::Directory *dir;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dirSelection, NULL, &iter))
	{
		dir = dirView.getValue<gpointer, DirectoryListing::Directory *>(&iter, "DL Dir");

		typedef Func2<ShareBrowser, DirectoryListing::Directory *, string> F2;
		F2 * func = new F2(this, &ShareBrowser::downloadDir_client, dir, target);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void ShareBrowser::filePopupMenu_gui()
{
	buildFileDownloadMenu_gui();
	gtk_menu_popup(fileMenu, NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	gtk_widget_show_all(GTK_WIDGET(fileMenu));
}

void ShareBrowser::dirPopupMenu_gui()
{
	buildDirDownloadMenu_gui();
	gtk_menu_popup(dirMenu, NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	gtk_widget_show_all(GTK_WIDGET(dirMenu));
}

/*
 * Searches the directories iteratively for the requested pattern. Uses a pre-order
 * traversal method, with the exception that it searches the parent's dir name first.
 * Instead of keeping track of the last directory its search ended at, it counts
 * the number of matches and re-searches the listing, skipping matches until it 
 * reaches the newest one. Slightly slower, but simpler.
 */
void ShareBrowser::find_gui()
{
	string name;
	bool findLeafNode = TRUE;
	int cursorPos, hits = 0;
	DirectoryListing::Directory *dir;
	DirectoryListing::File::Iter file;
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(dirStore);
	GtkTreePath *dirPath = gtk_tree_path_new_first();

	if (gtk_tree_path_get_depth(dirPath) == 0 || !gtk_tree_model_get_iter(m, &iter, dirPath))
	{
		gtk_tree_path_free(dirPath);
		return;
	}

	while (TRUE)
	{
		// Drill down until we reach a leaf node (e.g. a dir with no child dirs).
		if (findLeafNode)
		{
			do
			{
				name = Text::toLower(dirView.getString(&iter, "Dir"));
				// We found a matching directory name.
				if (name.find(search, 0) != string::npos && hits++ == skipHits)
				{
					skipHits = hits;
					gtk_tree_view_expand_to_path(dirView.get(), dirPath);
					gtk_tree_view_set_cursor(dirView.get(), dirPath, NULL, FALSE);
					dir = dirView.getValue<gpointer, DirectoryListing::Directory *>(&iter, "DL Dir");
					updateFiles_gui(dir);
					gtk_widget_grab_focus(GTK_WIDGET(dirView.get()));
					updateFileView = FALSE;
					gtk_tree_path_free(dirPath);
					setStatus_gui(mainStatus, "Found a match");
					return;
				}
				gtk_tree_path_down(dirPath);
			}
			while (gtk_tree_model_get_iter(m, &iter, dirPath));
		}

		// Come back up one directory. If we can't, then we've returned to the root and are done.
		if (!gtk_tree_path_up(dirPath) || gtk_tree_path_get_depth(dirPath) == 0 ||
			!gtk_tree_model_get_iter(m, &iter, dirPath))
		{
			setStatus_gui(mainStatus, "No matches");
			gtk_tree_path_free(dirPath);
			return;
		}

		// Search the files that are contained in this directory.
		dir = dirView.getValue<gpointer, DirectoryListing::Directory *>(&iter, "DL Dir");
		std::sort(dir->files.begin(), dir->files.end(), DirectoryListing::File::FileSort());

		for (file = dir->files.begin(), cursorPos = dir->directories.size(); file != dir->files.end(); file++, cursorPos++)
		{
			name = (*file)->getName();
			if (!listing.getUtf8())
				name = Text::acpToUtf8(name);
			name = Text::toLower(name);

			// We found a matching file. Update the cursors and the fileView if necessary.
			if (name.find(search, 0) != string::npos && hits++ == skipHits)
			{
				if (updateFileView)
				{
					gtk_tree_view_expand_to_path(dirView.get(), dirPath);
					gtk_tree_view_set_cursor(dirView.get(), dirPath, NULL, FALSE);
					updateFiles_gui(dir);
					updateFileView = FALSE;
				}

				skipHits = hits;
				// Keeping track of the current index allows us to quickly get the path to the file.
				GtkTreePath *path = gtk_tree_path_new_from_string(Util::toString(cursorPos).c_str());
				gtk_tree_view_set_cursor(fileView.get(), path, NULL, FALSE);
				gtk_widget_grab_focus(GTK_WIDGET(fileView.get()));
				gtk_tree_path_free(path);
				gtk_tree_path_free(dirPath);
				setStatus_gui(mainStatus, "Found a match");
				return;
			}
		}
		updateFileView = TRUE;

		// Determine if we are to go to the next sibling or back to the parent dir.
		gtk_tree_path_next(dirPath);
		if (!gtk_tree_model_get_iter(m, &iter, dirPath))
			findLeafNode = FALSE;
		else
			findLeafNode = TRUE;
	}
}

gboolean ShareBrowser::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	((ShareBrowser *)data)->oldType = event->type;
	return FALSE;
}

gboolean ShareBrowser::onFileButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	gint count = gtk_tree_selection_count_selected_rows(sb->fileSelection);

	if (count > 0 && event->type == GDK_BUTTON_RELEASE && event->button == 3)
		sb->filePopupMenu_gui();
	else if (count == 1 && sb->oldType == GDK_2BUTTON_PRESS && event->button == 1)
		sb->fileViewSelected_gui();

	return FALSE;
}

gboolean ShareBrowser::onFileKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	gint count = gtk_tree_selection_count_selected_rows(sb->fileSelection);

	if (count > 0 && (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK)))
		sb->filePopupMenu_gui();
	else if (count == 1 && (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter))
		sb->fileViewSelected_gui();

	return FALSE;
}

gboolean ShareBrowser::onDirButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	GtkTreeIter iter;
	GtkTreePath *path;
	gpointer ptr;

	if (!gtk_tree_selection_get_selected(sb->dirSelection, NULL, &iter))
		return FALSE;

	if (event->button == 1 && sb->oldType == GDK_2BUTTON_PRESS)
	{
		path = gtk_tree_model_get_path(GTK_TREE_MODEL(sb->dirStore), &iter);
		if (gtk_tree_view_row_expanded(sb->dirView.get(), path))
			gtk_tree_view_collapse_row(sb->dirView.get(), path);
		else
			gtk_tree_view_expand_row(sb->dirView.get(), path, FALSE);
		gtk_tree_path_free(path);
	}
	else if (event->button == 1 && event->type == GDK_BUTTON_RELEASE)
	{
		ptr = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
		sb->updateFiles_gui((DirectoryListing::Directory *)ptr);
	}
	else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
	{
		ptr = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
		sb->updateFiles_gui((DirectoryListing::Directory *)ptr);
		sb->dirPopupMenu_gui();
	}

	return FALSE;
}

gboolean ShareBrowser::onDirKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	GtkTreeIter iter;
	GtkTreePath *path;
	gpointer ptr;

	if (!gtk_tree_selection_get_selected(sb->dirSelection, NULL, &iter))
		return FALSE;

	if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
	{
		path = gtk_tree_model_get_path(GTK_TREE_MODEL(sb->dirStore), &iter);
		if (gtk_tree_view_row_expanded(sb->dirView.get(), path))
			gtk_tree_view_collapse_row(sb->dirView.get(), path);
		else
			gtk_tree_view_expand_row(sb->dirView.get(), path, FALSE);
		gtk_tree_path_free(path);
	}
	else if (event->keyval == GDK_Up || event->keyval == GDK_KP_Up ||
		event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
	{
		ptr = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
		sb->updateFiles_gui((DirectoryListing::Directory *)ptr);
	}
	else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
	{
		ptr = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
		sb->updateFiles_gui((DirectoryListing::Directory *)ptr);
		sb->dirPopupMenu_gui();
	}

	return FALSE;
}

void ShareBrowser::onMatchButtonClicked_gui(GtkWidget *widget, gpointer data)
{
	typedef Func0<ShareBrowser> F0;
	F0 *f0 = new F0((ShareBrowser*)data, &ShareBrowser::matchQueue_client);
	WulforManager::get()->dispatchClientFunc(f0);
}

void ShareBrowser::onFindButtonClicked_gui(GtkWidget *widget, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	gint ret;

	gtk_widget_grab_focus(GTK_WIDGET(sb->findEntry));
	ret = gtk_dialog_run(GTK_DIALOG(sb->findDialog));
	gtk_widget_hide(sb->findDialog);

	if (ret == GTK_RESPONSE_OK)
	{
		string text = gtk_entry_get_text(sb->findEntry);
		if (!text.empty())
		{
			sb->search = text;
			sb->skipHits = 0;
			sb->find_gui();
		}
		else
		{
			sb->setStatus_gui(sb->mainStatus, "No matches");
		}
	}
}

void ShareBrowser::onNextButtonClicked_gui(GtkWidget *widget, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	if (!sb->search.empty())
		sb->find_gui();
	else
		sb->setStatus_gui(sb->mainStatus, "No search text entered");
}

void ShareBrowser::onDownloadClicked_gui(GtkMenuItem *item, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	sb->downloadSelectedFiles_gui(Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY)));
}

void ShareBrowser::onDownloadToClicked_gui(GtkMenuItem *item, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	GtkWidget *chooser = gtk_file_chooser_dialog_new("Choose location",
		WulforManager::get()->getMainWindow()->getWindow(),
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_OK,
		NULL);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(chooser), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	if (!sb->lastDir.empty())
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), sb->lastDir.c_str());
	else
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY)).c_str());

	gint response = gtk_dialog_run(GTK_DIALOG (chooser));
	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(chooser));
		if (path[path.length () - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sb->lastDir = path;

		sb->downloadSelectedFiles_gui(path);
	}
	gtk_widget_destroy(chooser);
}

void ShareBrowser::onDownloadFavoriteClicked_gui(GtkMenuItem *item, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	string target = string((gchar *)g_object_get_data(G_OBJECT(item), "fav"));
	sb->downloadSelectedFiles_gui(target);
}

void ShareBrowser::onDownloadDirClicked_gui(GtkMenuItem *item, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	sb->downloadSelectedDirs_gui(Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY)));
}

void ShareBrowser::onDownloadDirToClicked_gui(GtkMenuItem *item, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	GtkWidget *chooser = gtk_file_chooser_dialog_new("Choose location",
		WulforManager::get()->getMainWindow()->getWindow(),
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_OK,
		NULL);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(chooser), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	if (!sb->lastDir.empty())
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), sb->lastDir.c_str());
	else
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), Text::utf8ToAcp(SETTING(DOWNLOAD_DIRECTORY)).c_str());

	gint response = gtk_dialog_run(GTK_DIALOG(chooser));
	if (response == GTK_RESPONSE_OK)
	{
		string path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(chooser));
		if (path[path.length() - 1] != PATH_SEPARATOR)
			path += PATH_SEPARATOR;
		sb->lastDir = path;

		sb->downloadSelectedDirs_gui(path);
	}
	gtk_widget_destroy(chooser);
}

void ShareBrowser::onDownloadFavoriteDirClicked_gui(GtkMenuItem *item, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	string target = string((gchar *)g_object_get_data(G_OBJECT(item), "fav"));
	sb->downloadSelectedDirs_gui(target);
}

void ShareBrowser::onSearchAlternatesClicked_gui(GtkMenuItem *item, gpointer data)
{
	ShareBrowser *sb = (ShareBrowser *)data;
	string target;
	bool bigFile;
	GtkTreeIter iter;
	GtkTreePath *path;
	gpointer ptr;
	string fileOrder;
	DirectoryListing::File *file;
	GtkTreeModel *m = GTK_TREE_MODEL(sb->fileStore);
	GList *list = gtk_tree_selection_get_selected_rows(sb->fileSelection, NULL);
	gint count = gtk_tree_selection_count_selected_rows(sb->fileSelection);

	for (int i = 0; i < count; i++)
	{
		path = (GtkTreePath *)g_list_nth_data(list, i);
		if (!gtk_tree_model_get_iter(m, &iter, path))
		{
			gtk_tree_path_free(path);
			continue;
		}

		ptr = sb->fileView.getValue<gpointer>(&iter, "DL File");
		fileOrder = sb->fileView.getString(&iter, "File Order");

		if (fileOrder[0] == 'f')
		{
			file = (DirectoryListing::File *)ptr;
			Search *s = WulforManager::get()->addSearch_gui();

			if (file->getTTH())
				s->putValue(file->getTTH()->toBase32(), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
			else
			{
				bigFile = (file->getSize() > 10 * 1024 * 1024);
				if (sb->listing.getUtf8())
					target = Util::getFileName(file->getName());
				else
					target = Text::acpToUtf8(Util::getFileName(file->getName()));

				if (!target.empty())
				{
					if (bigFile)
						s->putValue(SearchManager::clean(target), file->getSize()-1, SearchManager::SIZE_ATLEAST, ShareManager::getInstance()->getType(target));
					else
						s->putValue(SearchManager::clean(target), file->getSize()+1, SearchManager::SIZE_ATMOST, ShareManager::getInstance()->getType(target));
				}
			}
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void ShareBrowser::downloadFile_client(DirectoryListing::File *file, string target)
{
	try
	{
		listing.download(file, target, FALSE, FALSE);
	}
	catch (const Exception& e)
	{
		typedef Func2<ShareBrowser, GtkStatusbar *, string> F2;
		F2 *func = new F2(this, &ShareBrowser::setStatus_gui, mainStatus, e.getError());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void ShareBrowser::downloadDir_client(DirectoryListing::Directory *dir, string target)
{
	try
	{
		listing.download(dir, target, FALSE);
	}
	catch (const Exception& e)
	{
		typedef Func2<ShareBrowser, GtkStatusbar *, string> F2;
		F2 *func = new F2(this, &ShareBrowser::setStatus_gui, mainStatus, e.getError());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void ShareBrowser::matchQueue_client()
{
	int matched = QueueManager::getInstance()->matchListing(listing);
	string message = "Matched " + Util::toString(matched) + " files";

	typedef Func2<ShareBrowser, GtkStatusbar *, string> F2;
	F2 *f = new F2(this, &ShareBrowser::setStatus_gui, mainStatus, message);
	WulforManager::get()->dispatchGuiFunc(f);
}
