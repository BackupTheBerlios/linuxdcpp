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
																		alignmentSpeed (ALIGN_LEFT, ALIGN_CENTER, 0.0, 0.0),
																		alignmentTime (ALIGN_LEFT, ALIGN_CENTER, 0.0, 0.0)
{
	GuiProxy *proxy = GuiProxy::getInstance();
	string tmp;
	int64_t bytes=0;
	
	mw = m;

	set_has_separator (true);
	
	startTime = GET_TICK();
	HashManager::getInstance()->getStats(tmp, bytes, startFiles);

	updateStats();

	HashManager::getInstance()->setPriority(Thread::NORMAL);

	proxy->addListener<HashDialog, TimerManagerListener>(this, TimerManager::getInstance ());

	get_vbox()->pack_start (currentFile, PACK_EXPAND_WIDGET);
	get_vbox()->pack_start (alignmentSpeed, PACK_EXPAND_WIDGET);
	get_vbox()->pack_start (alignmentTime, PACK_EXPAND_WIDGET);
	alignmentSpeed.add (fileSpeed);
	alignmentTime.add (timeLeft);
	//alignment.add (progressBar);
	/*get_vbox()->pack_start (currentFile, PACK_EXPAND_WIDGET);
	get_vbox()->pack_start (fileSpeed, PACK_SHRINK);
	get_vbox()->pack_start (timeLeft, PACK_SHRINK);*/
	get_vbox()->pack_start (progressBar, PACK_EXPAND_WIDGET);

	add_button (GTK_STOCK_OK, 1);
}

HashDialog::~HashDialog ()
{
	HashManager::getInstance()->setPriority(Thread::IDLE);
	GuiProxy::getInstance ()->removeListener<HashDialog>(this);
}

void HashDialog::updateStats ()
{
	string file;
	int64_t bytes = 0;
	size_t files = 0;
	u_int32_t tick = GET_TICK();

	HashManager::getInstance()->getStats(file, bytes, files);
	if(files > startFiles)
		startFiles = files;
	
	double diff = tick - startTime;
	if(diff < 1000 || files == 0)
	{
		fileSpeed.set_text (WUtil::ConvertToUTF8 ("-.-- files/h, " + Util::toString((u_int32_t)files) + " files left."));
		timeLeft.set_text (WUtil::ConvertToUTF8 ("-:--:-- left."));
	}
	else
	{
		double filestat = (((double)(startFiles - files)) * 60 * 60 * 1000) / diff;
		fileSpeed.set_text (WUtil::ConvertToUTF8 (Util::toString (filestat) + " files/h, " + Util::toString ((u_int32_t)files) + " files left."));
		if(filestat == 0)
		{
			timeLeft.set_text (WUtil::ConvertToUTF8 ("-:--:-- left."));
		}
		else
		{
			double fs = files * 60 * 60 / filestat;
			timeLeft.set_text (WUtil::ConvertToUTF8 (Util::formatSeconds ((int64_t)fs) + " left."));
		}
	}
	if(files == 0)
	{
		currentFile.set_text ("Done.");
	}
	else
	{
		currentFile.set_text (file);
	}
	if(startFiles == 0)
	{
		progressBar.set_fraction (0.0);
	}
	else
	{
		progressBar.set_fraction ((double)(((startFiles - files)/(double)startFiles) ));
	}
}

 void HashDialog::on(TimerManagerListener::Second, u_int32_t tics) throw()
 {
	updateStats ();
 }
 