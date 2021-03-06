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

#ifndef __HASH_DIALOG_HH__
#define __HASH_DIALOG_HH__

#include <gtkmm.h>
#include <string>

#include "mainwindow.hh"
#include "../client/HashManager.h"
#include "../client/TimerManager.h"

class HashDialog : 	public Gtk::Dialog,
								public TimerManagerListener
{
public:
	HashDialog (MainWindow *m);
	~HashDialog ();
	
	void updateStats ();

	//TimerManagerListener
	virtual void on(TimerManagerListener::Second, u_int32_t tics) throw();
	
private:
	int64_t startBytes;
	size_t startFiles;
	u_int32_t startTime;
	Gtk::Frame frame;
	Gtk::VBox box;
	Gtk::Table table;
	Gtk::Label fileLabel, file;
	Gtk::Label speedLabel, speed;
	Gtk::Label timeleftLabel, timeleft;
	Gtk::ProgressBar progressBar;

	MainWindow *mw;
};

#else
class HashDialog;
#endif

