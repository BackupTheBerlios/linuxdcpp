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

TreeView::TreeView(): view(NULL)
{
}

TreeView::~TreeView()
{
	string columnOrder, columnWidth, title;

	for(int i = 0; i < columns.size(); i++)
	{
		title = getColumnTitle(i);
		if (title.empty())
			return; // error: a col was moved to the right of the padding col
		columnOrder += Util::toString(defaultPosition[title]) + ",";
	}
	columnOrder.erase(columnOrder.size() - 1, 1);

	for(int i = 0; i < columns.size(); i++)
		columnWidth += Util::toString(getColumnWidth(i)) + ",";
	columnWidth.erase(columnWidth.size() - 1, 1);

	SettingsManager::getInstance()->set(orderSetting, columnOrder);
	SettingsManager::getInstance()->set(widthSetting, columnWidth);
}

void TreeView::setView(GtkTreeView *view)
{
	this->view = view;
	gtk_tree_view_set_headers_clickable(view, TRUE);
}

void TreeView::setView(GtkTreeView *view, bool padding, SettingsManager::StrSetting orderSetting, SettingsManager::StrSetting widthSetting)
{
	this->view = view;
	this->padding = padding;
	this->orderSetting = orderSetting;
	this->widthSetting = widthSetting;
	gtk_tree_view_set_headers_clickable(view, TRUE);
}

GtkTreeView *TreeView::get()
{
	return view;
}

void TreeView::insertColumn(const string &title, const int id, const GType gtype, const columnType type, const int width, const int id_pixbuf)
{
	columns.push_back(Column(title, id, gtype, type, width, id_pixbuf));
	defaultPosition[title] = currentPosition[title] = id;
}

void TreeView::insertHiddenColumn(const string &title, const int id, const GType gtype)
{
	hiddenColumns[title] = id;
	hiddenGTypes.push_back(gtype);
}

void TreeView::finalize()
{
	restoreSettings();
	columns.sort();

	for (ColIter iter = columns.begin(); iter != columns.end(); iter++)
		addColumn_gui(*iter);

	if (padding)
		gtk_tree_view_insert_column(view, gtk_tree_view_column_new(), columns.size() + hiddenColumns.size());
}

int TreeView::getCount() {
	GtkTreeIter it;
	GtkTreeModel *m = gtk_tree_view_get_model(view);

	if (!gtk_tree_model_get_iter_first(m, &it)) return 0;

	int count = 0;
	while (1) {
		count++;
		if (!gtk_tree_model_iter_next (m, &it))
			return count;
	}
}

void TreeView::getColumn(string column, std::vector<std::string> *l) {
	GtkTreeModel *m = gtk_tree_view_get_model(view);
	GtkTreeIter it;

	if (!gtk_tree_model_get_iter_first(m, &it)) return;
		
	while (1) {
		l->push_back(getValue<gchar*,std::string>(&it, column));
		if (!gtk_tree_model_iter_next (m, &it))	break;
	}
}

int TreeView::getSize()
{
	return columns.size() + hiddenColumns.size();
}

GType* TreeView::getGTypes()
{
	int i = 0;
	GType *gtypes = new GType[columns.size() + hiddenColumns.size()];

	for (ColIter iter = columns.begin(); iter != columns.end(); iter++)
		gtypes[i++] = iter->gtype;
	for (list<GType>::const_iterator iter = hiddenGTypes.begin(); iter != hiddenGTypes.end(); iter++)
		gtypes[i++] = *iter;

	return gtypes;
}

void TreeView::addColumn_gui(Column column)
{
	addColumn_gui(column.pos, column.title, column.type, column.width, column.id_pixbuf);
}

void TreeView::addColumn_gui(int id, std::string title, columnType type, int width, int id_pixbuf)
{
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkCellRenderer *renderer_pixbuf;

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

	gtk_tree_view_column_set_reorderable(col, true);

	gtk_tree_view_insert_column(view, col, id);
}

void TreeView::setSortColumn_gui(string column, string sortColumn) {
	GtkTreeViewColumn *gtkColumn;
	gtkColumn = gtk_tree_view_get_column(view, col(column));
	gtk_tree_view_column_set_sort_column_id(gtkColumn, col(sortColumn));
}

int TreeView::getColumnWidth(int position)
{
	return gtk_tree_view_column_get_width(gtk_tree_view_get_column(view, position));
}

string TreeView::getColumnTitle(int position)
{
	return string(gtk_tree_view_column_get_title(gtk_tree_view_get_column(view, position)));
}

int TreeView::col(const string &title)
{
	return (currentPosition.find(title) != currentPosition.end()) ? currentPosition[title] : hiddenColumns[title];
}

void TreeView::restoreSettings()
{
	vector<int> columnOrder = WulforUtil::splitString(SettingsManager::getInstance()->get(orderSetting), ",");
	vector<int> columnWidth = WulforUtil::splitString(SettingsManager::getInstance()->get(widthSetting), ",");

	if (!columnOrder.empty() && columnOrder.size() == columns.size() && 
		!columnWidth.empty() && columnWidth.size() == columns.size())
	{
		for (ColIter iter = columns.begin(); iter != columns.end(); iter++)
		{
			for (int j = 0; j < columns.size(); j++)
			{
				if (iter->id == columnOrder.at(j))
				{
					iter->pos = j;
					currentPosition[iter->title] = j;
					iter->width = columnWidth.at(j);
					break;
				}
			}
		}
	}
}
