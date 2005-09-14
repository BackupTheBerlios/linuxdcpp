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

#include "hashdialog.hh"
#include "wulformanager.hh"

Hash::Hash()
{
	TimerManager::getInstance()->addListener(this);
	string file = WulforManager::get()->getPath() + "/glade/hash.glade";
	GladeXML *xml = glade_xml_new(file.c_str(), NULL, NULL);

	dialog = GTK_DIALOG(glade_xml_get_widget(xml, "hashDialog"));
	lFile = GTK_LABEL(glade_xml_get_widget(xml, "labelFile"));
	lSpeed = GTK_LABEL(glade_xml_get_widget(xml, "labelSpeed"));
	lTime = GTK_LABEL(glade_xml_get_widget(xml, "labelTime"));
	pProgress = GTK_PROGRESS_BAR(glade_xml_get_widget(xml, "progressbar"));
	
	string tmp;
	startTime = GET_TICK();
	autoClose = false;
	HashManager::getInstance()->getStats(tmp, startBytes, startFiles);
	HashManager::getInstance()->setPriority(Thread::NORMAL);
}

Hash::~Hash()
{
	HashManager::getInstance()->setPriority(Thread::IDLE);
	TimerManager::getInstance()->removeListener(this);
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

gint Hash::run()
{
	return gtk_dialog_run(dialog);
}

void Hash::updateStats()
{
	string file;
	int64_t bytes = 0;
	size_t files = 0;
	u_int32_t tick = GET_TICK();

	HashManager::getInstance()->getStats(file, bytes, files);
	if (bytes > startBytes)
		startBytes = bytes;

	if (files > startFiles)
		startFiles = files;

	if (autoClose && files == 0) 
	{
		gtk_dialog_response(dialog, GTK_RESPONSE_OK);
		return;
	}
		
	double diff = tick - startTime;
	if (diff < 1000 || files == 0 || bytes == 0) 
	{
		gtk_label_set_text (lSpeed, string ("-.-- B/s, " + Util::formatBytes (bytes) + " left").c_str ());
		gtk_label_set_text (lTime, "-:--:-- left");
		gtk_progress_bar_set_text (pProgress, "0%");
		gtk_progress_bar_set_fraction (pProgress, 0.0);
	} 
	else 
	{
		double speedStat = (((double)(startBytes - bytes)) * 1000) / diff;

		gtk_label_set_text (lSpeed, string (Util::formatBytes ((int64_t)speedStat) + "/s, " + Util::formatBytes(bytes) + " left").c_str ());
		
		if(speedStat == 0)
			gtk_label_set_text (lTime,"-:--:-- left");
		else 
		{
			double ss = bytes / speedStat;
			gtk_label_set_text (lTime,string (Util::formatSeconds ((int64_t)ss) + " left").c_str ());
		}
	}

	if (files == 0)
		gtk_label_set_text (lFile, "Done");
	else
		gtk_label_set_text (lFile, file.c_str());

	if (startFiles == 0 || startBytes == 0)
	{
		gtk_progress_bar_set_text (pProgress, "0%");
		gtk_progress_bar_set_fraction (pProgress, 0.0);
	}
	else
	{
		double progress = ((0.5 * (double)(startFiles - files)/(double)startFiles) 
			+ (0.5 * (double)(startBytes - bytes)/(double)startBytes));
		char buf[16];
		sprintf (buf, "%.0f", progress * 100);
		string txt = string(buf) + "%";
		gtk_progress_bar_set_text(pProgress, txt.c_str());
		gtk_progress_bar_set_fraction(pProgress, progress);
	}
}

void Hash::on(TimerManagerListener::Second, u_int32_t tics) throw()
{
	gdk_threads_enter();
	updateStats();
	gdk_threads_leave();
}
