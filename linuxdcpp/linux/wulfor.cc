/*
 * Copyright © 2004-2008 Jens Oknelid, paskharen@gmail.com
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
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <glib/gi18n.h>

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>

#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include <iostream>
#include <signal.h>

void callBack(void* x, const std::string& a)
{
	std::cout << "Loading: " << a << std::endl;
}

int main(int argc, char *argv[])
{
	// Initialize i18n support
	bindtextdomain("linuxdcpp", _DATADIR "/locale");
	textdomain("linuxdcpp");
	bind_textdomain_codeset("linuxdcpp", "UTF-8");

	// Check if profile is locked
	if (WulforUtil::profileIsLocked())
	{
		gtk_init(&argc, &argv);
		string message = _("Only one instance of LinuxDC++ is allowed per profile");

		GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, message.c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		return -1;
	}

	// Start the DC++ client core
	startup(callBack, NULL);

	TimerManager::getInstance()->start();

	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);
	glade_init();

	signal(SIGPIPE, SIG_IGN);

	WulforSettingsManager::newInstance();
	WulforManager::start();
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	WulforManager::stop();
	WulforSettingsManager::deleteInstance();

	std::cout << "Shutting down..." << std::endl;
	shutdown();

	return 0;
}

