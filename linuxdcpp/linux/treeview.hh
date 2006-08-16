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

#ifndef WULFOR_TREE_VIEW_HH
#define WULFOR_TREE_VIEW_HH

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include <map>
#include "settingsmanager.hh"
#include "WulforUtil.hh"

class TreeView
{
	public:
		typedef enum
		{
			STRING,
			STRINGR,
			INT,
			BOOL,
			PIXBUF,
			PIXBUF_STRING,
			EDIT_STRING,
			PROGRESS
		} columnType;

		TreeView();
		~TreeView();
		void setView(GtkTreeView *view);
		void setView(GtkTreeView *view, bool padding, const string &name = "");
		GtkTreeView *get();
		void insertColumn(const std::string &title, const GType &gtype, const columnType type, const int width, const string &linkedCol = "");
		void insertHiddenColumn(const std::string &title, const GType &gtype);
		void finalize();
		int getColCount();
		int getRowCount();
		GType* getGTypes();
		void setSortColumn_gui(std::string column, std::string sortColumn);
		int col(const std::string &title);
		void saveSettings();
		string getString(GtkTreeIter *i, std::string column);

		template<class T, class C>
		C getValue(GtkTreeIter *i, std::string column)
		{
			T value;
			GtkTreeModel *m = gtk_tree_view_get_model(view);
			dcassert(gtk_tree_model_get_column_type(m, col(column)) != G_TYPE_STRING);
			gtk_tree_model_get(m, i, col(column), &value, -1);
			return C(value);
		}
		template<class T>
		T getValue(GtkTreeIter *i, std::string column)
		{
			T value;
			GtkTreeModel *m = gtk_tree_view_get_model(view);
			dcassert(gtk_tree_model_get_column_type(m, col(column)) != G_TYPE_STRING);
			gtk_tree_model_get(m, i, col(column), &value, -1);
			return value;
		}

	private:
		class Column
		{
			public:
				Column() {};
				Column(std::string title, int id, GType gtype, TreeView::columnType type, int width, std::string linkedCol = "") :
					title(title), id(id), gtype(gtype), type(type), width(width), pos(id), linkedCol(linkedCol), visible(true) {};
				Column(std::string title, int id, GType gtype) :
					title(title), id(id), gtype(gtype) {};
				std::string title;
				int id;
				GType gtype;
				TreeView::columnType type;
				int width;
				int pos;
				std::string linkedCol;
				bool visible;
				bool operator<(const Column &right) const
				{
					return pos < right.pos;
				}
		};

		void addColumn_gui(Column column);
		void restoreSettings();
		static gboolean popupMenu_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void toggleColumnVisibility(GtkMenuItem *item, gpointer data);

		GtkTreeView *view;
		std::string name; // Used to save settings
		bool padding;
		int count;
		int visibleColumns;
		GtkMenu *menu;
		GType *gtypes;
		hash_map<std::string, GtkWidget*> colMenuItems;

		typedef hash_map<std::string, Column> ColMap;
		typedef hash_map<int, std::string> SortedColMap;
		typedef ColMap::iterator ColIter;
		typedef SortedColMap::iterator SortedColIter;
		ColMap columns;
		SortedColMap sortedColumns;
		ColMap hiddenColumns;
		SortedColMap sortedHiddenColumns;
};

#else
class TreeView;
#endif
