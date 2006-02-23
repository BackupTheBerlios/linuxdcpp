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

TreeView::TreeView()
{
	view = NULL;
	count = 0;
	padding = false;
}

TreeView::~TreeView()
{
	if (!name.empty() && GTK_IS_TREE_VIEW(view))
		saveSettings();
}

void TreeView::setView(GtkTreeView *view)
{
	this->view = view;
	gtk_tree_view_set_headers_clickable(view, TRUE);
}

void TreeView::setView(GtkTreeView *view, bool padding, const string &name)
{
	this->view = view;
	this->padding = padding;
	this->name = name;
	gtk_tree_view_set_headers_clickable(view, TRUE);
}

GtkTreeView *TreeView::get()
{
	return view;
}

void TreeView::insertColumn(const string &title, const GType &gtype, const columnType type, const int width, const string &linkedCol)
{
	// All insertColumn's have to be called before any insertHiddenColumn's.
	g_assert(hiddenColumns.size() == 0);

	// Title must be unique.
	g_assert(!title.empty() && columns.find(title) == columns.end());

	columns[title] = Column(title, count, gtype, type, width, linkedCol);
	sortedColumns[count] = title;
	++count;
}

void TreeView::insertHiddenColumn(const string &title, const GType &gtype)
{
	// Title must be unique.
	g_assert(!title.empty() && hiddenColumns.find(title) == hiddenColumns.end()	&& columns.find(title) == columns.end());

	hiddenColumns[title] = Column(title, count, gtype);
	sortedHiddenColumns[count] = title;
	++count;
}

void TreeView::finalize()
{
	g_assert(count > 0);

	Column col;
	menu = GTK_MENU(gtk_menu_new());
	visibleColumns = columns.size();

	if (!name.empty())
		restoreSettings();

	for (SortedColIter iter = sortedColumns.begin(); iter != sortedColumns.end(); iter++)
	{
		col = columns[iter->second];
		addColumn_gui(col);
		
		//Add entry to popup menu for hiding and showing the column. Don't add empty titles
		if (col.title != "" && col.title != " ")
		{
			colMenuItems[col.title] = gtk_check_menu_item_new_with_label(col.title.c_str());
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(colMenuItems[col.title]), col.visible);
			g_signal_connect(G_OBJECT(colMenuItems[col.title]), "activate", G_CALLBACK(toggleColumnVisibility), (gpointer)this);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), colMenuItems[col.title]);
		}

		if (!col.visible)
			visibleColumns--;
	}

	if (padding)
		gtk_tree_view_insert_column(view, gtk_tree_view_column_new(), count);
}

// This is the total number of columns, including hidden columns.
int TreeView::getColCount()
{
	return count;
}

int TreeView::getRowCount()
{
	GtkTreeIter it;
	GtkTreeModel *m = gtk_tree_view_get_model(view);
	int numRows = 0;

	if (!gtk_tree_model_get_iter_first(m, &it))
		return numRows;

	do
	{
		numRows++;
	} while (gtk_tree_model_iter_next(m, &it));

	return numRows;
}

GType* TreeView::getGTypes()
{
	int i = 0;
	GType *gtypes = new GType[count]; // @todo: fix memory leak

	for (SortedColIter iter = sortedColumns.begin(); iter != sortedColumns.end(); iter++)
		gtypes[i++] = columns[iter->second].gtype;
	for (SortedColIter iter = sortedHiddenColumns.begin(); iter != sortedHiddenColumns.end(); iter++)
		gtypes[i++] = hiddenColumns[iter->second].gtype;

	return gtypes;
}

void TreeView::addColumn_gui(Column column)
{
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkCellRenderer *renderer_pixbuf;

	switch (column.type)
	{
		case STRING:
			col = gtk_tree_view_column_new_with_attributes(
				column.title.c_str(), gtk_cell_renderer_text_new(), "text", column.pos, NULL);
			break;
		case STRINGR:
			renderer = gtk_cell_renderer_text_new();
			g_object_set(renderer, "xalign", 1.0, NULL);
			col = gtk_tree_view_column_new_with_attributes(column.title.c_str(), renderer, "text", column.pos, NULL);
			gtk_tree_view_column_set_alignment(col, 1.0);
			break;
		case INT:
			col = gtk_tree_view_column_new_with_attributes(
				column.title.c_str(), gtk_cell_renderer_text_new(), "text", column.pos, NULL);
			break;
		case BOOL:
  			renderer = gtk_cell_renderer_toggle_new();
  			col = gtk_tree_view_column_new_with_attributes(column.title.c_str (), renderer, "active", column.pos, NULL);
			break;
		case PIXBUF:
			col = gtk_tree_view_column_new_with_attributes(
				column.title.c_str(), gtk_cell_renderer_pixbuf_new(), "pixbuf", column.pos, NULL);
			break;
		case PIXBUF_STRING:
			g_assert(!column.linkedCol.empty());
			renderer = gtk_cell_renderer_text_new();
			renderer_pixbuf = gtk_cell_renderer_pixbuf_new();
			col = gtk_tree_view_column_new();
			gtk_tree_view_column_set_title(col, column.title.c_str());
			gtk_tree_view_column_pack_start(col, renderer_pixbuf, false);
			gtk_tree_view_column_add_attribute(col, renderer_pixbuf, "pixbuf", TreeView::col(column.linkedCol));
			gtk_tree_view_column_pack_start(col, renderer, true);
			gtk_tree_view_column_add_attribute(col, renderer, "text", column.pos);
			break;
		case EDIT_STRING:
			renderer = gtk_cell_renderer_text_new();
 			g_object_set(renderer, "editable", TRUE, NULL);
			col = gtk_tree_view_column_new_with_attributes(column.title.c_str(), renderer, "text", column.pos, NULL);
			break;
		case PROGRESS:
			g_assert(!column.linkedCol.empty());
			col = gtk_tree_view_column_new_with_attributes(
				column.title.c_str(), gtk_cell_renderer_progress_new(), "text", column.id, "value", TreeView::col(column.linkedCol), NULL);
			break;
	};

	// If columns are too small, they can't be manipulated
	if (column.width >= 20)
	{
		gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_fixed_width(col, column.width);
		gtk_tree_view_column_set_resizable(col, TRUE);
	}

	//make columns sortable
	if (column.type == STRING || column.type == INT || column.type == PROGRESS)
	{
		gtk_tree_view_column_set_sort_column_id(col, column.pos);
		gtk_tree_view_column_set_sort_indicator(col, TRUE);
	}

	gtk_tree_view_column_set_clickable(col, TRUE);
	gtk_tree_view_column_set_reorderable(col, true);
	gtk_tree_view_column_set_visible(col, column.visible);

	gtk_tree_view_insert_column(view, col, column.pos);

	// Breaks GTK+ API, but is the only way to attach a signal to a gtktreeview column header. See GTK bug #141937.
	// Replace when GTK adds a way to add a signal to the entire header (remove visibleColumns var, too).
	g_signal_connect(G_OBJECT(col->button), "button-release-event", G_CALLBACK(popupMenu_gui), (gpointer)this);
}

void TreeView::setSortColumn_gui(string column, string sortColumn)
{
	GtkTreeViewColumn *gtkColumn;
	gtkColumn = gtk_tree_view_get_column(view, col(column));
	gtk_tree_view_column_set_sort_column_id(gtkColumn, col(sortColumn));
}

int TreeView::col(const string &title)
{
	return (columns.find(title) != columns.end()) ? columns[title].pos : hiddenColumns[title].id;
}

gboolean TreeView::popupMenu_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	TreeView *tv = (TreeView*)data;

	if (event->button == 3)
	{
		gtk_menu_popup(tv->menu, NULL, NULL, NULL, NULL, (event != NULL) ? event->button : 0, gdk_event_get_time((GdkEvent*)event));
		gtk_widget_show_all(GTK_WIDGET(tv->menu));
		return true;
	}
	else
		return false;
}

void TreeView::toggleColumnVisibility(GtkMenuItem *item, gpointer data)
{
	TreeView *tv = (TreeView*)data;
	GtkTreeViewColumn *column;
	gboolean visible;
	SortedColIter iter;
	string title = string(gtk_label_get_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(item)))));

	// Function col(title) doesn't work here, so we have to find column manually.
	for (iter = tv->sortedColumns.begin(); iter != tv->sortedColumns.end(); iter++)
	{
		column = gtk_tree_view_get_column(tv->view, iter->first);
		if (string(gtk_tree_view_column_get_title(column)) == title)
			break;
	}

	visible = !gtk_tree_view_column_get_visible(column);

	// Can't let number of visible columns fall below 1, otherwise there's no way to unhide columns.
	if (!visible && tv->visibleColumns <= 1)
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(tv->colMenuItems[title]), true);
		return;
	}

	gtk_tree_view_column_set_visible(column, visible);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(tv->colMenuItems[title]), visible);

	if (visible)
	{
		tv->visibleColumns++;
		// Seems to be a bug in gtk where sometimes columns are unresizable after being made visible
		gtk_tree_view_column_set_resizable(column, TRUE);
	}
	else
		tv->visibleColumns--;
}

void TreeView::restoreSettings()
{
	vector<int> columnOrder, columnWidth, columnVisibility;
	columnOrder = WulforUtil::splitString(WGETS(name + "-order"), ",");
	columnWidth = WulforUtil::splitString(WGETS(name + "-width"), ",");
	columnVisibility = WulforUtil::splitString(WGETS(name + "-visibility"), ",");

	if (!columnOrder.empty() && columnOrder.size() == columns.size() && 
		!columnWidth.empty() && columnWidth.size() == columns.size() && 
		!columnVisibility.empty() && columnVisibility.size() == columns.size())
	{
		for (ColIter iter = columns.begin(); iter != columns.end(); iter++)
		{
			for (int i = 0; i < columns.size(); i++)
			{
				if (iter->second.id == columnOrder.at(i))
				{
					iter->second.pos = i;
					sortedColumns[i] = iter->second.title;
					iter->second.width = columnWidth.at(i);
					iter->second.visible = columnVisibility.at(i);
					break;
				}
			}
		}
	}
}

void TreeView::saveSettings()
{
	string columnOrder, columnWidth, columnVisibility, title;
	GtkTreeViewColumn *col;

	g_assert(columns.size() > 0);

	for (int i = 0; i < columns.size(); i++)
	{
		col = gtk_tree_view_get_column(view, i);
		title = string(gtk_tree_view_column_get_title(col));

		// A col was moved to the right of the padding col
		if (title.empty())
			return;

		columnOrder += Util::toString(columns[title].id) + ",";
		columnWidth += Util::toString(gtk_tree_view_column_get_width(col)) + ",";
		columnVisibility += Util::toString(gtk_tree_view_column_get_visible(col)) + ",";
	}

	columnOrder.erase(columnOrder.size() - 1, 1);
	columnWidth.erase(columnWidth.size() - 1, 1);
	columnVisibility.erase(columnVisibility.size() - 1, 1);

	WSET(name + "-order", columnOrder);
	WSET(name + "-width", columnWidth);
	WSET(name + "-visibility", columnVisibility);
}
