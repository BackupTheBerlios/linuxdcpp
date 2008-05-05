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
 */

#include "UserCommandMenu.hh"
#include <client/FavoriteManager.h>
#include <client/UserCommand.h>
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;

UserCommandMenu::UserCommandMenu(GtkWidget *userCommandMenu, int ctx):
	Entry("User Command Menu", "", TRUE),
	userCommandMenu(userCommandMenu),
	ctx(ctx)
{
}

void UserCommandMenu::addHub(const string &hub)
{
	hubs.push_back(hub);
}

void UserCommandMenu::addHub(const StringList &hubs2)
{
	hubs.insert(hubs.end(), hubs2.begin(), hubs2.end());
}

void UserCommandMenu::addUser(const string &cid)
{
	users.push_back(cid);
}

void UserCommandMenu::cleanMenu_gui()
{
	gtk_container_foreach(GTK_CONTAINER(userCommandMenu), (GtkCallback)gtk_widget_destroy, NULL);
	hubs.clear();
	users.clear();
}

void UserCommandMenu::buildMenu_gui()
{
	UserCommand::List userCommandList = FavoriteManager::getInstance()->getUserCommands(ctx, hubs);

	GtkWidget *menuItem;
	GtkWidget *menu = userCommandMenu;
	bool separator = FALSE; // tracks whether last menu item was a separator

	for (UserCommand::Iter i = userCommandList.begin(); i != userCommandList.end(); ++i)
	{
		UserCommand& uc = *i;

		// Add line separator only if it's not a duplicate
		if (uc.getType() == UserCommand::TYPE_SEPARATOR && !separator)
		{
			menuItem = gtk_separator_menu_item_new();
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
			separator = TRUE;
		}
		else if (uc.getType() == UserCommand::TYPE_RAW || uc.getType() == UserCommand::TYPE_RAW_ONCE)
		{
			string command = uc.getName();
			separator = FALSE;
			menu = userCommandMenu;

			createSubMenu_gui(menu, command);

			// Append the user command to the sub menu
			menuItem = gtk_menu_item_new_with_label(command.c_str());
			g_signal_connect(menuItem, "activate", GCallback(onUserCommandClick_gui), (gpointer)this);
			g_object_set_data_full(G_OBJECT(menuItem), "name", g_strdup(uc.getName().c_str()), g_free);
			g_object_set_data_full(G_OBJECT(menuItem), "command", g_strdup(uc.getCommand().c_str()), g_free);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
		}
	}
}

void UserCommandMenu::createSubMenu_gui(GtkWidget *&menu, std::string &command)
{
	string::size_type i = 0;
	GtkWidget *menuItem;

	// Create subfolders based on path separators in the command
	while ((i = command.find('\\')) != string::npos)
	{
		bool createSubmenu = TRUE;
		GList *menuItems = gtk_container_get_children(GTK_CONTAINER(menu));

		// Search for the sub menu to append the command to
		for (GList *iter = menuItems; iter; iter = iter->next)
		{
			GtkMenuItem *item = (GtkMenuItem *)iter->data;
			if (gtk_menu_item_get_submenu(item) && WulforUtil::getTextFromMenu(item) == command.substr(0, i))
			{
				menu = gtk_menu_item_get_submenu(item);
				createSubmenu = FALSE;
				break;
			}
		}
		g_list_free(menuItems);

		// Couldn't find existing sub menu, so we create one
		if (createSubmenu)
		{
			GtkWidget *subMenu = gtk_menu_new();
			menuItem = gtk_menu_item_new_with_label(command.substr(0, i).c_str());
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), subMenu);
			menu = subMenu;
		}

		command = command.substr(++i);
	}
}

void UserCommandMenu::onUserCommandClick_gui(GtkMenuItem *item, gpointer data)
{
	UserCommandMenu *ucm = (UserCommandMenu *)data;
	string command = (gchar *)g_object_get_data(G_OBJECT(item), "command");
	StringMap params;
	typedef Func3<UserCommandMenu, string, string, StringMap> F3;

	if (WulforManager::get()->getMainWindow()->getUserCommandLines_gui(command, params))
	{
		string commandName = (gchar *)g_object_get_data(G_OBJECT(item), "name");

		for (StringList::const_iterator i = ucm->users.begin(); i != ucm->users.end(); ++i)
		{
			F3 *func = new F3(ucm, &UserCommandMenu::sendUserCommand_client, *i, commandName, params);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void UserCommandMenu::sendUserCommand_client(string cid, string commandName, StringMap params)
{
	if (!cid.empty() && !commandName.empty())
	{
		int id = FavoriteManager::getInstance()->findUserCommand(commandName);
		UserCommand uc;

		if (id == -1 || !FavoriteManager::getInstance()->getUserCommand(id, uc))
			return;

		User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			ClientManager::getInstance()->userCommand(user, uc, params, true);
	}
}

