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

#include "hashdialog.hh"
#include "guiproxy.hh"

using namespace Gtk;

HashDialog::HashDialog (MainWindow *m) : 	Dialog ("Hash progress", *m, true), startBytes (0), startTime (GET_TICK ()), startFiles (0),
																		frame ("Hashprogress"),
																		table (3, 2),
																		fileLabel ("File:", 0.0, 0.5), file ("", 1.0, 0.5),
																		speedLabel ("Speed:", 0.0, 0.5), speed ("", 1.0, 0.5),
																		timeleftLabel ("Timeleft:", 0.0, 0.5), timeleft ("", 1.0, 0.5)
{
	GuiProxy *proxy = GuiProxy::getInstance();
	string tmp;

	mw = m;

	set_has_separator (true);
	
	startTime = GET_TICK();
	HashManager::getInstance()->getStats(tmp, startBytes, startFiles);

	updateStats();

	HashManager::getInstance()->setPriority(Thread::NORMAL);

	proxy->addListener<HashDialog, TimerManagerListener>(this, TimerManager::getInstance ());

	file.set_justify (JUSTIFY_LEFT);
	speed.set_justify (JUSTIFY_LEFT);
	timeleft.set_justify (JUSTIFY_LEFT);

	set_border_width (8);
	get_vbox()->pack_start (frame, PACK_EXPAND_WIDGET);
	frame.add (box);
	box.pack_start (table, PACK_EXPAND_WIDGET);
	table.set_spacings (2);
	table.set_border_width (8);
	table.attach(fileLabel, 0, 1, 0, 1);
	table.attach(file, 1, 2, 0, 1, FILL, SHRINK);
	table.attach(speedLabel, 0, 1, 1, 2);
	table.attach(speed, 1, 2, 1, 2, FILL, SHRINK);
	table.attach(timeleftLabel, 0, 1, 2, 3);
	table.attach(timeleft, 1, 2, 2, 3, FILL, SHRINK);
	box.pack_start (progressBar, PACK_EXPAND_WIDGET);
	add_button (GTK_STOCK_CLOSE, 1);
}

HashDialog::~HashDialog ()
{
	HashManager::getInstance()->setPriority(Thread::IDLE);
	GuiProxy::getInstance ()->removeListener<HashDialog>(this);
}

void HashDialog::updateStats ()
{
	string cfile;
	int64_t bytes = 0;
	size_t files = 0;
	u_int32_t tick = GET_TICK();

	HashManager::getInstance()->getStats(cfile, bytes, files);
	if (bytes > startBytes)
		startBytes = bytes;
		
	if(files > startFiles)
		startFiles = files;
	
	double diff = tick - startTime;
	if(files == 0 || bytes == 0 )
	{
		frame.set_label ("Hashprogress - " + Util::toString((u_int32_t)files) +" files left");
		speed.set_text ("-.-- B/s, " + Util::formatBytes(bytes) + " left");
		timeleft.set_text ("-:--:-- left");
		progressBar.set_fraction (0.0);
	}
	else
	{
		double speedStat = (((double)(startBytes - bytes)) * 1000) / diff;
		frame.set_label ("Hashprogress - " + Util::toString((u_int32_t)files) +" files left");
		speed.set_text (Util::formatBytes((int64_t)speedStat) + "/s, " + Util::formatBytes(bytes) + " left");
		if(speedStat == 0)
		{
			timeleft.set_text ("-:--:-- left");
		}
		else
		{
			double ss = bytes / speedStat;
			timeleft.set_text (Util::formatSeconds ((int64_t)ss) + " left");
		}
	}
	if(files == 0)
	{
		file.set_text ("Done");
	}
	else
	{
		file.set_text (Util::getFileName (cfile));
	}
	if(startFiles == 0 || startBytes == 0)
	{
		progressBar.set_fraction (0.0);
	}
	else
	{
		progressBar.set_fraction ((double)(((startBytes - bytes)/(double)startBytes) ));
	}
}

 void HashDialog::on(TimerManagerListener::Second, u_int32_t tics) throw()
 {
	updateStats ();
 }
 
