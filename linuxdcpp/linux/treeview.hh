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

#ifndef WULFOR_TREE_VIEW_HH
#define WULFOR_TREE_VIEW_HH

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/SettingsManager.h>
#include "WulforUtil.hh"

class TreeView {
	public:
		typedef enum {
			STRING,
			STRINGR,
			INT,
			BOOL,
			PIXBUF,
			PIXBUF_STRING,
			EDIT_STRING
			//PROGRESS
		} columnType;

		class Column
		{
			public:
				Column(std::string title, int id, GType gtype, TreeView::columnType type, int width, int id_pixbuf = -1) :
					title(title), id(id), gtype(gtype), type(type), width(width), pos(id), id_pixbuf(id_pixbuf) {};
				std::string title;
				int id;
				GType gtype;
				TreeView::columnType type;
				int width;
				int pos;
				int id_pixbuf;
				bool operator<(const Column &right) const
				{
					return pos < right.pos;
				}
		};

		TreeView();
		~TreeView();
		void setView(GtkTreeView *view);
		void setView(GtkTreeView *view, bool padding, SettingsManager::StrSetting orderSetting, SettingsManager::StrSetting widthSetting);
		GtkTreeView *get();
		void insertColumn(const std::string &title, const int id, const GType gtype, const columnType type, const int width, const int id_pixbuf = -1);
		void insertHiddenColumn(const std::string &title, const int id, const GType gtype);
		void finalize();
		int getSize();
		GType* getGTypes();
		void addColumn_gui(Column column);
		void addColumn_gui(int id, std::string title, columnType type, int width, int id_pixbuf=-1);
		void setSortColumn_gui(std::string column, std::string sortColumn);
		int col(const std::string &title);

		template<class T, class C>
		C getValue(GtkTreeIter *i, std::string column)
		{
			T value;
			GtkTreeModel *m = gtk_tree_view_get_model(view);
			gtk_tree_model_get (m, i, col(column), &value, -1);
			return C (value);
		}
		template<class T>
		T getValue(GtkTreeIter *i, std::string column)
		{
			T value;
			GtkTreeModel *m = gtk_tree_view_get_model(view);
			gtk_tree_model_get (m, i, col(column), &value, -1);
			return value;
		}

		int getCount();
		void getColumn(std::string column, std::vector<std::string> *l);

	private:
		void restoreSettings();
		int getColumnWidth(int position);
		std::string getColumnTitle(int position);

		GtkTreeView *view;

		std::list<Column> columns;
		typedef list<Column>::iterator ColIter;
		std::map<std::string, int> currentPosition;
		std::map<std::string, int> defaultPosition;

		std::map<std::string, int> hiddenColumns;
		std::list<GType> hiddenGTypes;

		SettingsManager::StrSetting orderSetting;
		SettingsManager::StrSetting widthSetting;
		bool padding;
};

#else
class TreeView;
#endif
