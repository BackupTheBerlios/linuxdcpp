#include "treeview.hh"

using namespace std;

TreeView::TreeView(GtkTreeView *view) {
	this->view = view;
	gtk_tree_view_set_headers_clickable(view, TRUE);
}

GtkTreeView *TreeView::get() {
	return view;
}

void TreeView::addColumn_gui(int id, std::string title, type_t type, int width) {
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

	gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(col, width);
	gtk_tree_view_column_set_resizable(col, TRUE);

	//make columns sortable
	if (type == STRING || type == INT) {
		gtk_tree_view_column_set_clickable(col, TRUE);
		gtk_tree_view_column_set_sort_column_id(col, id);
		gtk_tree_view_column_set_sort_indicator(col, TRUE);
	}

	gtk_tree_view_insert_column(view, col, id);
}

