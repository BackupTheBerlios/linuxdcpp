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

#include "treeview.hh"

using namespace std;

TreeView::TreeView(string name):
	view(NULL)
{
	this->name = name;
}

TreeView::~TreeView() {
	string widths, order, isVisible;
	GtkTreeViewColumn *col;
	int pos;
	
	assert(view);

	for (pos = 0, col = gtk_tree_view_get_column(view, pos);
		col != NULL;
		pos++, col = gtk_tree_view_get_column(view, pos))
	{
		widths += Util::toString(gtk_tree_view_column_get_width(col)) + ", ";
		order += Util::toString(defaultPosition[title]) + ",";
		if (gtk_tree_view_column_get_visible(col))
			isVisible += "1,";
		else
			isVisible += "0,";
	}

	//remove last ,
	if (pos > 1) {
		widths.erase(widths.size() - 1, 1);
		order.erase(order.size() - 1, 1);
		isVisible.erase(isVisible.size() - 1, 1);
	}

	WulforSettingsManager::get()->set(name + "-order", order);
	WulforSettingsManager::get()->set(name + "-widths", widths);
	WulforSettingsManager::get()->set(name + "-visible", isVisible);
}



//This can't be set in the constructor since the view is loaded by libglade usually
void TreeView::setView_gui(GtkTreeView *view) {
	this->view = view;
	gtk_tree_view_set_headers_clickable(view, TRUE);
}

GtkTreeView *TreeView::get() {
	return view;
}

void TreeView::addColumn_gui(int id, std::string title, type_t type, int width, int id_pixbuf) {
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkCellRenderer *renderer_pixbuf;

	assert(view);

	switch (type) {
		case STRING:
			col = gtk_tree_view_column_new_with_attributes(
				title.c_str(), gtk_cell_renderer_text_new(), "text", id, NULL);
			break;
		case STRINGR:
			renderer = gtk_cell_renderer_text_new();
			g_object_set(renderer, "xalign", 1.0, NULL);
			col = gtk_tree_view_column_new_with_attributes(title.c_str(), renderer, "text", id, NULL);
			gtk_tree_view_column_set_alignment(col, 1.0);

			break;
		case INT:
			col = gtk_tree_view_column_new_with_attributes(
				title.c_str(), gtk_cell_renderer_text_new(), "text", id, NULL);
			break;
		case BOOL:
  			renderer = gtk_cell_renderer_toggle_new();
  			col = gtk_tree_view_column_new_with_attributes(title.c_str (), renderer, "active", id, NULL);
			break;
		case PIXBUF:
			col = gtk_tree_view_column_new_with_attributes(
				title.c_str(), gtk_cell_renderer_pixbuf_new(), "pixbuf", id, NULL);
			break;
		case PIXBUF_STRING:
			renderer = gtk_cell_renderer_text_new();
			renderer_pixbuf = gtk_cell_renderer_pixbuf_new();
			col = gtk_tree_view_column_new();
			gtk_tree_view_column_set_title(col, title.c_str());
			gtk_tree_view_column_pack_start(col, renderer_pixbuf, false);
			gtk_tree_view_column_add_attribute(col, renderer_pixbuf, "pixbuf", id_pixbuf);
			gtk_tree_view_column_pack_start(col, renderer, true);
			gtk_tree_view_column_add_attribute(col, renderer, "text", id);

			break;
		case EDIT_STRING:
			renderer = gtk_cell_renderer_text_new();
 			g_object_set(renderer, "editable", TRUE, NULL);
			col = gtk_tree_view_column_new_with_attributes(title.c_str(), renderer, "text", id, NULL);
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

void TreeView::setSortColumn_gui(int id, int sortColumn) {
	GtkTreeViewColumn *col;
	col = gtk_tree_view_get_column(view, id);
	gtk_tree_view_column_set_sort_column_id(col, sortColumn);
}

