/* 
* Copyright (C) 2004 Jens Oknelid, paskharen@spray.se
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

#include "treeviewfactory.hh"

using namespace std;

TreeViewFactory::TreeViewFactory(GtkTreeView *view) {
	this->view = view;
	gtk_tree_view_set_headers_clickable(view, TRUE);
}

GtkTreeView *TreeViewFactory::get() {
	return view;
}
void TreeViewFactory::addColumn_gui(int id, std::string title, type_t type, int width) {
	GtkTreeViewColumn *col;
	
	switch (type) {
		case STRING:
			col = gtk_tree_view_column_new_with_attributes(
				title.c_str(), gtk_cell_renderer_text_new(), "text", id, NULL);
			break;
		case INT:
			col = gtk_tree_view_column_new_with_attributes(
				title.c_str(), gtk_cell_renderer_text_new(), "text", id, NULL);
			break;
		case BOOL:
		  	GtkCellRenderer *renderer;
  			renderer = gtk_cell_renderer_toggle_new ();
  			col = gtk_tree_view_column_new_with_attributes (title.c_str (), renderer, "active", id, NULL);
			break;
		case PIXBUF:
			col = gtk_tree_view_column_new_with_attributes(
				title.c_str(), gtk_cell_renderer_pixbuf_new(), "pixbuf", id, NULL);
			break;
		/*
		case PROGRESS:
			col = gtk_tree_view_column_new_with_attributes(
				title.c_str(), gtk_cell_renderer_progress_new(), "progress", currentColumn, NULL);
			break;
		*/
	};

	if (width != -1) {
		gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_fixed_width(col, width);
		gtk_tree_view_column_set_resizable(col, TRUE);
	}

	//make columns sortable
	if (type == STRING || type == INT) {
		gtk_tree_view_column_set_clickable(col, TRUE);
		gtk_tree_view_column_set_sort_column_id(col, id);
		gtk_tree_view_column_set_sort_indicator(col, TRUE);
	}

	gtk_tree_view_insert_column(view, col, id);
}
void TreeViewFactory::setSortColumn_gui(int id, int sortColumn) {
	GtkTreeViewColumn *col;
	col = gtk_tree_view_get_column(view, id);
	gtk_tree_view_column_set_sort_column_id(col, sortColumn);
}
