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

#ifndef WULFOR_PUBLIC_HUBS_HH
#define WULFOR_PUBLIC_HUBS_HH

#include <gtk/gtk.h>
#include <glade/glade.h>
#include "bookentry.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/HubManager.h>

class PublicHubs: 
	public BookEntry,
	public HubManagerListener
{
	public:
		//constructor must be called from gui thread
		PublicHubs(GCallback closeCallback);
		~PublicHubs();

		GtkWidget *getWidget();

		//only to be called from the gui thread
		void filterHubs_gui();
		void connect_gui();


	private:
		static void filter_callback(GtkWidget *widget, gpointer data);
		static void connect_callback(GtkWidget *widget, gpointer data);

		GtkWidget *mainBox;
		GtkEntry *filterEntry, *connectEntry;
		//GtkButton *filterButton, *connectButton;
		GtkTreeView *hubView;
};

#else
class PublicHubs;
#endif
