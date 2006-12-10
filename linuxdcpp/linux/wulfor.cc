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

#include <gtk/gtk.h>
#include <glade/glade.h>

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>

#include "wulformanager.hh"
#include "settingsmanager.hh"
#include <iostream>
#include <signal.h>

void callBack(void* x, const std::string& a)
{
	std::cout << "Loading: " << a << std::endl;
}

int main(int argc, char *argv[])
{
	// Start the DC++ client core
	startup(callBack, NULL);

	WulforSettingsManager::get()->load();
	TimerManager::getInstance()->start();

	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);
	glade_init();

	signal(SIGPIPE, SIG_IGN);

	WulforManager::start();
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	WulforManager::stop();

	WulforSettingsManager::get()->save();
	delete WulforSettingsManager::get();

	std::cout << "Shutting down..." << std::endl;
	shutdown();

	return 0;
}
