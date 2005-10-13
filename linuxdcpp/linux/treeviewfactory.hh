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

class TreeViewFactory {
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
		} type_t;
	
		TreeViewFactory(GtkTreeView *view);
		GtkTreeView *get();
		void addColumn_gui(int id, std::string title, type_t type, int width, int id_pixbuf=-1);
		void setSortColumn_gui(int id, int sortColumn);

		template<class T, class C>
		static C getValue (GtkTreeModel *m, GtkTreeIter *i, gint c)
		{
			T value;
			gtk_tree_model_get (m, i, c, &value, -1);
			return C (value);
		}
		template<class T>
		static T getValue (GtkTreeModel *m, GtkTreeIter *i, gint c)
		{
			T value;
			gtk_tree_model_get (m, i, c, &value, -1);
			return value;
		}
		static int getCount (GtkTreeModel *m)
		{
			GtkTreeIter it;
			if (!gtk_tree_model_get_iter_first (m, &it))
				return 0;
			int count=0;
			while (1)
			{
				count++;
				if (!gtk_tree_model_iter_next (m, &it))
					return count;
			}
		}
		static void getColumn (GtkTreeModel *m, gint c, std::vector<std::string> *l)
		{
			GtkTreeIter it;
			if (!gtk_tree_model_get_iter_first (m, &it))
				return;
		
			while (1)
			{
				l->push_back (getValue<gchar*,std::string>(m, &it, c));
				if (!gtk_tree_model_iter_next (m, &it))
					break;
			}
		}
	private:
		GtkTreeView *view;
};

#else
class TreeViewFactory;
#endif
