/*
 * Copyright © 2004-2007 Jens Oknelid, paskharen@gmail.com
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

#include "hub.hh"

#include <client/FavoriteManager.h>
#include <client/HashManager.h>
#include <client/ShareManager.h>
#include <client/UserCommand.h>
#include "privatemessage.hh"
#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;

Hub::Hub(const string &address):
	BookEntry(_("Hub: ") + address, "hub.glade"),
	client(NULL),
	historyIndex(0),
	totalShared(0)
{
	// Configure the dialog
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("passwordDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	// Initialize nick treeview
	nickView.setView(GTK_TREE_VIEW(getWidget("nickView")), true, "hub");
	nickView.insertColumn("Nick", G_TYPE_STRING, TreeView::PIXBUF_STRING, 100, "Icon");
	nickView.insertColumn("Shared", G_TYPE_STRING, TreeView::STRING, 75);
	nickView.insertColumn("Description", G_TYPE_STRING, TreeView::STRING, 85);
	nickView.insertColumn("Tag", G_TYPE_STRING, TreeView::STRING, 100);
	nickView.insertColumn("Connection", G_TYPE_STRING, TreeView::STRING, 85);
	nickView.insertColumn("eMail", G_TYPE_STRING, TreeView::STRING, 90);
	nickView.insertHiddenColumn("Shared Bytes", G_TYPE_INT64);
	nickView.insertHiddenColumn("Icon", GDK_TYPE_PIXBUF);
	nickView.insertHiddenColumn("Nick Order", G_TYPE_STRING);
	nickView.insertHiddenColumn("CID", G_TYPE_STRING);
	nickView.finalize();
	nickStore = gtk_list_store_newv(nickView.getColCount(), nickView.getGTypes());
	gtk_tree_view_set_model(nickView.get(), GTK_TREE_MODEL(nickStore));
	g_object_unref(nickStore);
	nickSelection = gtk_tree_view_get_selection(nickView.get());
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(nickView.get()), GTK_SELECTION_MULTIPLE);
	nickView.setSortColumn_gui("Nick", "Nick Order");
	nickView.setSortColumn_gui("Shared", "Shared Bytes");
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), nickView.col("Nick Order"), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(nickView.get(), nickView.col("Nick")), TRUE);
	gtk_tree_view_set_fixed_height_mode(nickView.get(), TRUE);

	// Initialize the chat window
	if (BOOLSETTING(USE_OEM_MONOFONT))
	{
		PangoFontDescription *fontDesc = pango_font_description_new();
		pango_font_description_set_family(fontDesc, "Mono");
		gtk_widget_modify_font(getWidget("chatText"), fontDesc);
		pango_font_description_free(fontDesc);
	}
	chatBuffer = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(getWidget("chatText")), chatBuffer);
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(chatBuffer, &iter);
	chatMark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, FALSE);

	// Load the icons for the nick list
	string path = WulforManager::get()->getPath() + "/pixmaps/";
	string icon = path + "normal.png";
	userIcons["normal"] = gdk_pixbuf_new_from_file(icon.c_str(), NULL);
	icon = path + "normal-op.png";
	userIcons["normal-op"] = gdk_pixbuf_new_from_file(icon.c_str(), NULL);
	icon = path + "normal-fw.png";
	userIcons["normal-fw"] = gdk_pixbuf_new_from_file(icon.c_str(), NULL);
	icon = path + "normal-fw-op.png";
	userIcons["normal-fw-op"] = gdk_pixbuf_new_from_file(icon.c_str(), NULL);
	icon = path + "dc++.png";
	userIcons["dc++"] = gdk_pixbuf_new_from_file(icon.c_str(), NULL);
	icon = path + "dc++-op.png";
	userIcons["dc++-op"] = gdk_pixbuf_new_from_file(icon.c_str(), NULL);
	icon = path + "dc++-fw.png";
	userIcons["dc++-fw"] = gdk_pixbuf_new_from_file(icon.c_str(), NULL);
	icon = path + "dc++-fw-op.png";
	userIcons["dc++-fw-op"] = gdk_pixbuf_new_from_file(icon.c_str(), NULL);

	// Connect the signals to their callback functions.
	g_signal_connect(nickView.get(), "button-press-event", G_CALLBACK(onNickListButtonPress_gui), (gpointer)this);
	g_signal_connect(nickView.get(), "button-release-event", G_CALLBACK(onNickListButtonRelease_gui), (gpointer)this);
	g_signal_connect(nickView.get(), "key-release-event", G_CALLBACK(onNickListKeyRelease_gui), (gpointer)this);
	g_signal_connect(getWidget("chatEntry"), "activate", G_CALLBACK(onSendMessage_gui), (gpointer)this);
	g_signal_connect(getWidget("chatEntry"), "key-press-event", G_CALLBACK(onEntryKeyPress_gui), (gpointer)this);
	g_signal_connect(getWidget("browseItem"), "activate", G_CALLBACK(onBrowseItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("matchItem"), "activate", G_CALLBACK(onMatchItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("msgItem"), "activate", G_CALLBACK(onMsgItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("grantItem"), "activate", G_CALLBACK(onGrantItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeUserItem"), "activate", G_CALLBACK(onRemoveUserItemClicked_gui), (gpointer)this);

	gtk_widget_set_sensitive(getWidget("favoriteUserItem"), FALSE); // Not implemented yet
	gtk_widget_grab_focus(getWidget("chatEntry"));

	// Set the pane position
	int panePosition = WGETI("nick-pane-position");
	if (panePosition > 10)
		gtk_paned_set_position(GTK_PANED(getWidget("pane")), panePosition);

	GtkWidget *menuItem = gtk_menu_item_new_with_label(_("User commands"));
	subMenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), subMenu);
	gtk_menu_append(GTK_MENU(getWidget("nickMenu")), menuItem);
	history.push_back("");
}

Hub::~Hub()
{
	if (client)
	{
		client->removeListener(this);
		client->disconnect(TRUE);
		ClientManager::getInstance()->putClient(client);
		client = NULL;
	}

	hash_map<string, GdkPixbuf *>::iterator it;
	for (it = userIcons.begin(); it != userIcons.end(); ++it)
		g_object_unref(G_OBJECT(it->second));

	// Save the pane position
	int panePosition = gtk_paned_get_position(GTK_PANED(getWidget("pane")));
	if (panePosition > 10)
		WSET("nick-pane-position", panePosition);

	gtk_widget_destroy(getWidget("passwordDialog"));
}

void Hub::setStatus_gui(string statusBar, string text)
{
	if (!statusBar.empty() && !text.empty())
	{
		gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusBar)), 0);
		gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusBar)), 0, text.c_str());
	}
}

bool Hub::findUser_gui(const string &nick, GtkTreeIter *iter)
{
	if (userMap.find(nick) != userMap.end())
	{
		GtkTreeModel *m = GTK_TREE_MODEL(nickStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, iter);

		while (valid)
		{
			if (nick == nickView.getString(iter, "Nick"))
				return TRUE;

			valid = gtk_tree_model_iter_next(m, iter);
		}
	}

	return FALSE;
}


void Hub::updateUser_gui(StringMap params)
{
	GtkTreeIter iter;
	int64_t shared = Util::toInt64(params["Shared Bytes"]);

	if (findUser_gui(params["Nick"], &iter))
	{
		totalShared += shared - nickView.getValue<int64_t>(&iter, "Shared Bytes");

		gtk_list_store_set(nickStore, &iter,
			nickView.col("Nick"), params["Nick"].c_str(),
			nickView.col("Shared"), params["Shared"].c_str(),
			nickView.col("Description"), params["Description"].c_str(),
			nickView.col("Tag"), params["Tag"].c_str(),
 			nickView.col("Connection"), params["Connection"].c_str(),
			nickView.col("eMail"), params["eMail"].c_str(),
			nickView.col("Shared Bytes"), shared,
 			nickView.col("Icon"), userIcons[params["Icon"]],
			nickView.col("Nick Order"), params["Nick Order"].c_str(),
			-1);
	}
	else
	{
		totalShared += shared;
		userMap[params["Nick"]] = params["CID"];

		if (BOOLSETTING(SHOW_JOINS))
		{
			addStatusMessage_gui(params["Nick"] + _(" has joined"));
		}
		else if (BOOLSETTING(FAV_SHOW_JOINS))
		{
			typedef Func1<Hub, string> F1;
			F1 *func = new F1(this, &Hub::checkFavoriteUserJoin_client, params["CID"]);
			WulforManager::get()->dispatchClientFunc(func);
		}

		gtk_list_store_insert_with_values(nickStore, &iter, userMap.size(),
			nickView.col("Nick"), params["Nick"].c_str(),
			nickView.col("Shared"), params["Shared"].c_str(),
			nickView.col("Description"), params["Description"].c_str(),
			nickView.col("Tag"), params["Tag"].c_str(),
 			nickView.col("Connection"), params["Connection"].c_str(),
			nickView.col("eMail"), params["eMail"].c_str(),
			nickView.col("Shared Bytes"), shared,
 			nickView.col("Icon"), userIcons[params["Icon"]],
			nickView.col("Nick Order"), params["Nick Order"].c_str(),
			nickView.col("CID"), params["CID"].c_str(),
			-1);
	}

	setStatus_gui("statusUsers", Util::toString(userMap.size()) + _(" Users"));
	setStatus_gui("statusShared", Util::formatBytes(totalShared));
}

void Hub::removeUser_gui(string nick)
{
	GtkTreeIter iter;

	if (findUser_gui(nick, &iter))
	{
		totalShared -= nickView.getValue<int64_t>(&iter, "Shared Bytes");
		gtk_list_store_remove(nickStore, &iter);
		userMap.erase(nick);
		setStatus_gui("statusUsers", Util::toString(userMap.size()) + _(" Users"));
		setStatus_gui("statusShared", Util::formatBytes(totalShared));
	}
}

void Hub::clearNickList_gui()
{
	gtk_list_store_clear(nickStore);
	userMap.clear();
	totalShared = 0;
	setStatus_gui("statusUsers", _("0 Users"));
	setStatus_gui("statusShared", "0 B");
}

void Hub::popupNickMenu_gui()
{
	gtk_container_foreach(GTK_CONTAINER(subMenu), (GtkCallback)gtk_widget_destroy, NULL);

	// Add user commands.
	::UserCommand::List list = FavoriteManager::getInstance()->
		getUserCommands(::UserCommand::CONTEXT_CHAT, StringList(1, client->getHubUrl()));

	if (!list.empty())
	{
		GtkWidget *menuItem;

		for (::UserCommand::Iter i = list.begin(); i != list.end(); ++i)
		{
			::UserCommand& uc = *i;
			if (uc.getType() == ::UserCommand::TYPE_SEPARATOR)
			{
				menuItem = gtk_separator_menu_item_new();
				gtk_menu_shell_append(GTK_MENU_SHELL(subMenu), menuItem);
			}
			else
			{
				menuItem = gtk_menu_item_new_with_label(uc.getName().c_str());
				g_signal_connect(menuItem, "activate", G_CALLBACK(onUserCommandClick_gui), (gpointer)this);
				g_object_set_data_full(G_OBJECT(menuItem), "command", g_strdup(uc.getCommand().c_str()), g_free);
				gtk_menu_shell_append(GTK_MENU_SHELL(subMenu), menuItem);
			}
		}
	}

	gtk_menu_popup(GTK_MENU(getWidget("nickMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	gtk_widget_show_all(getWidget("nickMenu"));
}

void Hub::getPassword_gui()
{
	gint ret;

	ret = gtk_dialog_run(GTK_DIALOG(getWidget("passwordDialog")));
	gtk_widget_hide(getWidget("passwordDialog"));

	if (ret == GTK_RESPONSE_OK)
	{
		string password = gtk_entry_get_text(GTK_ENTRY(getWidget("passwordEntry")));
		typedef Func1<Hub, string> F1;
		F1 *func = new F1(this, &Hub::setPassword_client, password);
		WulforManager::get()->dispatchClientFunc(func);
	}
	else
		client->disconnect(TRUE);
}

void Hub::addStatusMessage_gui(string message)
{
	if (!message.empty())
	{
		setStatus_gui("statusMain", message);

		if (BOOLSETTING(STATUS_IN_CHAT))
		{
			string line = "*** " + message;
			addMessage_gui(line);
		}
	}
}

void Hub::addMessage_gui(string message)
{
	if (message.empty())
		return;

	GtkTextIter iter;
	GtkAdjustment *adj;
	bool setBottom;
	string line = "";

	if (BOOLSETTING(TIME_STAMPS))
		line = "[" + Util::getShortTimeString() + "] ";
	line += message + "\n";

	adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(getWidget("chatScroll")));
	setBottom = gtk_adjustment_get_value(adj) >= (adj->upper - adj->page_size);

	gtk_text_buffer_get_end_iter(chatBuffer, &iter);
	gtk_text_buffer_insert(chatBuffer, &iter, line.c_str(), line.size());

	if (gtk_text_buffer_get_line_count(chatBuffer) > maxLines)
	{
		GtkTextIter next;
		gtk_text_buffer_get_start_iter(chatBuffer, &iter);
		gtk_text_buffer_get_iter_at_line(chatBuffer, &next, 1);
		gtk_text_buffer_delete(chatBuffer, &iter, &next);
	}

	if (setBottom)
	{
		gtk_text_buffer_get_end_iter(chatBuffer, &iter);
		gtk_text_buffer_move_mark(chatBuffer, chatMark, &iter);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(getWidget("chatText")), chatMark, 0, FALSE, 0, 0);
	}

	if (BOOLSETTING(BOLD_HUB))
		setBold_gui();
}

void Hub::addPrivateMessage_gui(string cid, string msg)
{
	if (!cid.empty())
	{
		User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
		{
			BookEntry *entry = WulforManager::get()->addPrivMsg_gui(user, !BOOLSETTING(POPUNDER_PM));
			if (!msg.empty())
				dynamic_cast< ::PrivateMessage *>(entry)->addMessage_gui(msg);
		}
	}
}

void Hub::onSendMessage_gui(GtkEntry *entry, gpointer data)
{
	string text = gtk_entry_get_text(entry);
	if (text.empty())
		return;

	gtk_entry_set_text(entry, "");
	Hub *hub = (Hub *)data;
	typedef Func1<Hub, string> F1;
	F1 *func;

	// Store line in chat history
	hub->history.pop_back();
	hub->history.push_back(text);
	hub->history.push_back("");
	hub->historyIndex = hub->history.size() - 1;
	if (hub->history.size() > maxHistory + 1)
		hub->history.erase(hub->history.begin());

	// Process special commands
	if (text[0] == '/')
	{
		string command, param;
		string::size_type separator = text.find_first_of(' ');
		if (separator != string::npos && text.size() > separator + 1)
		{
			command = text.substr(1, separator - 1);
			param = text.substr(separator + 1);
		}
		else
		{
			command = text.substr(1);
		}
		std::transform(command.begin(), command.end(), command.begin(), (int(*)(int))tolower);

		if (command == _("away"))
		{
			if (Util::getAway() && param.empty())
			{
				Util::setAway(FALSE);
				Util::setManualAway(FALSE);
				hub->addStatusMessage_gui(_("Away mode off"));
			}
			else
			{
				Util::setAway(TRUE);
				Util::setManualAway(TRUE);
				Util::setAwayMessage(param);
				hub->addStatusMessage_gui(_("Away mode on: ") + Util::getAwayMessage());
			}
		}
		else if (command == _("back"))
		{
			Util::setAway(FALSE);
			hub->addStatusMessage_gui(_("Away mode off"));
		}
		else if (command == _("clear"))
		{
			GtkTextIter startIter, endIter;
			gtk_text_buffer_get_start_iter(hub->chatBuffer, &startIter);
			gtk_text_buffer_get_end_iter(hub->chatBuffer, &endIter);
			gtk_text_buffer_delete(hub->chatBuffer, &startIter, &endIter);
		}
		else if (command == _("close"))
		{
			/// @todo: figure out why this sometimes closes and reopens the tab
			WulforManager::get()->deleteEntry_gui(hub);
		}
		else if (command == _("favorite") || command == _("fav"))
		{
			WulforManager::get()->dispatchClientFunc(new Func0<Hub>(hub, &Hub::addAsFavorite_client));
		}
		else if (command == _("getlist"))
		{
			if (hub->userMap.find(param) != hub->userMap.end())
			{
				typedef Func2<Hub, string, bool> F2;
				F2 *f2 = new F2(hub, &Hub::getFileList_client, hub->userMap[param], FALSE);
				WulforManager::get()->dispatchClientFunc(f2);
			}
			else
				hub->addStatusMessage_gui(_("User not found"));
		}
		else if (command == _("grant"))
		{
			if (hub->userMap.find(param) != hub->userMap.end())
			{
				func = new F1(hub, &Hub::grantSlot_client, hub->userMap[param]);
				WulforManager::get()->dispatchClientFunc(func);
			}
			else
				hub->addStatusMessage_gui(_("User not found"));
		}
		else if (command == _("help"))
		{
			hub->addStatusMessage_gui(_("Available commands: /away <message>, /back, /clear, /close, /favorite, "\
				 "/getlist <nick>, /grant <nick>, /help, /join <address>, /pm <nick>, /rebuild, /refresh, /userlist"));
		}
		else if (command == _("join") && !param.empty())
		{
			if (BOOLSETTING(JOIN_OPEN_NEW_WINDOW))
			{
				// Assumption: new hub is same encoding as current hub.
				WulforManager::get()->addHub_gui(param, hub->client->getEncoding());
			}
			else
			{
				func = new F1(hub, &Hub::redirect_client, param);
				WulforManager::get()->dispatchClientFunc(func);
			}
		}
		else if (command == _("pm"))
		{
			if (hub->userMap.find(param) != hub->userMap.end())
				hub->addPrivateMessage_gui(hub->userMap[param], "");
			else
				hub->addStatusMessage_gui(_("User not found"));
		}
		else if (command == _("rebuild"))
		{
			WulforManager::get()->dispatchClientFunc(new Func0<Hub>(hub, &Hub::rebuildHashData_client));
		}
		else if (command == _("refresh"))
		{
			WulforManager::get()->dispatchClientFunc(new Func0<Hub>(hub, &Hub::refreshFileList_client));
		}
		else if (command == _("userlist"))
		{
			if (GTK_WIDGET_VISIBLE(hub->getWidget("scrolledwindow2")))
				gtk_widget_hide(hub->getWidget("scrolledwindow2"));
			else
				gtk_widget_show_all(hub->getWidget("scrolledwindow2"));
		}
		else if (BOOLSETTING(SEND_UNKNOWN_COMMANDS))
		{
			func = new F1(hub, &Hub::sendMessage_client, text);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else
		{
			hub->addStatusMessage_gui(_("Unknown command '") + text + _("': type /help for a list of available commands"));
		}
	}
	else
	{
		func = new F1(hub, &Hub::sendMessage_client, text);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

gboolean Hub::onNickListButtonPress_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (event->type == GDK_BUTTON_PRESS || event->type == GDK_2BUTTON_PRESS)
		hub->oldType = event->type;

	if (event->button == 3)
	{
		GtkTreePath *path;
		if (gtk_tree_view_get_path_at_pos(hub->nickView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(hub->nickSelection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}

	return FALSE;
}

gboolean Hub::onNickListButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		if (event->button == 1 && hub->oldType == GDK_2BUTTON_PRESS)
		{
			hub->onBrowseItemClicked_gui(NULL, data);
		}
		else if (event->button == 2 && event->type == GDK_BUTTON_RELEASE)
		{
			hub->onMsgItemClicked_gui(NULL, data);
		}
		else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
		{
			hub->popupNickMenu_gui();
		}
	}

	return FALSE;
}

gboolean Hub::onNickListKeyRelease_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			hub->popupNickMenu_gui();
		}
		else if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		{
			hub->onBrowseItemClicked_gui(NULL, data);
		}
	}

	return FALSE;
}

gboolean Hub::onEntryKeyPress_gui(GtkWidget *entry, GdkEventKey *event, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (event->keyval == GDK_Up || event->keyval == GDK_KP_Up)
	{
		size_t index = hub->historyIndex - 1;
		if (index >= 0 && index < hub->history.size())
		{
			hub->historyIndex = index;
			gtk_entry_set_text(GTK_ENTRY(entry), hub->history[index].c_str());
		}
		return TRUE;
	}
	else if (event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
	{
		size_t index = hub->historyIndex + 1;
		if (index >= 0 && index < hub->history.size())
		{
			hub->historyIndex = index;
			gtk_entry_set_text(GTK_ENTRY(entry), hub->history[index].c_str());
		}
		return TRUE;
	}
	else if (event->keyval == GDK_Tab || event->keyval == GDK_ISO_Left_Tab)
	{
		string current;
		string::size_type start, end;
		string text(gtk_entry_get_text(GTK_ENTRY(entry)));
		int curpos = gtk_editable_get_position(GTK_EDITABLE(entry));

		// Allow tab to focus other widgets if entry is empty
		if (curpos <= 0 && text.empty())
			return FALSE;

		// Erase ": " at the end of the nick.
		if (curpos > 2 && text.substr(curpos - 2, 2) == ": ")
		{
			text.erase(curpos - 2, 2);
			curpos -= 2;
		}

		start = text.rfind(' ', curpos - 1);
		end = text.find(' ', curpos - 1);

		// Text to match starts at the beginning
		if (start == string::npos)
			start = 0;
		else
			++start;

		if (start < end)
		{
			current = text.substr(start, end - start);

			if (hub->completionKey.empty() || Text::toLower(current).find(Text::toLower(hub->completionKey)) != 0)
				hub->completionKey = current;

			GtkTreeIter iter;
			bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(hub->nickStore), &iter);
			bool useNext = (current == hub->completionKey);
			string key = Text::toLower(hub->completionKey);
			string complete = hub->completionKey;

			while (valid)
			{
				string nick = hub->nickView.getString(&iter, "Nick");
				if (useNext && Text::toLower(nick).find(key) == 0)
				{
					complete = nick;
					if (start <= 0)
						complete.append(": ");

					break;
				}

				if (nick == current)
					useNext = TRUE;

				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(hub->nickStore),&iter);
			}

			text.replace(start, end - start, complete);
			gtk_entry_set_text(GTK_ENTRY(entry), text.c_str());
			gtk_editable_set_position(GTK_EDITABLE(entry), start + complete.length());
		}
		else
			hub->completionKey.clear();

		return TRUE;
	}

	hub->completionKey.clear();
	return FALSE;
}

void Hub::onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func2<Hub, string, bool> F2;
		F2 *func;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				func = new F2(hub, &Hub::getFileList_client, cid, FALSE);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::onMatchItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func2<Hub, string, bool> F2;
		F2 *func;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				func = new F2(hub, &Hub::getFileList_client, cid, TRUE);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::onMsgItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				WulforManager::get()->addPrivMsg_gui(ClientManager::getInstance()->getUser(CID(cid)));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::onUserCommandClick_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;
	GtkTreeIter iter;
	GtkTreePath *path;
	string cid;
	string commandName = WulforUtil::getTextFromMenu(item);
	string command = (gchar *)g_object_get_data(G_OBJECT(item), "command");
	GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);
	MainWindow *mw = WulforManager::get()->getMainWindow();

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
		{
			cid = hub->nickView.getString(&iter, "CID");
			StringMap params;
			if (mw->getUserCommandLines_gui(command, params))
			{
				typedef Func3<Hub, string, string, StringMap> F3;
				F3 *func = new F3(hub, &Hub::sendUserCommand_client, cid, commandName, params);
				WulforManager::get()->dispatchClientFunc(func);
			}
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void Hub::onGrantItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func1<Hub, string> F1;
		F1 *func;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				func = new F1(hub, &Hub::grantSlot_client, cid);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::onRemoveUserItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;

	if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
	{
		string cid;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func1<Hub, string> F1;
		F1 *func;
		GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
			{
				cid = hub->nickView.getString(&iter, "CID");
				func = new F1(hub, &Hub::removeUserFromQueue_client, cid);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void Hub::connectClient_client(string address, string encoding)
{
	dcassert(client == NULL);
	client = ClientManager::getInstance()->getClient(address);
	client->setEncoding(encoding);
	client->addListener(this);
	client->connect();
}

void Hub::setPassword_client(string password)
{
	if (client && !password.empty())
	{
		client->setPassword(password);
		client->password(password);
	}
}

void Hub::sendMessage_client(string message)
{
	if (client && !message.empty())
		client->hubMessage(message);
}

void Hub::sendUserCommand_client(string cid, string commandName, StringMap params)
{
	if (!cid.empty() && !commandName.empty())
	{
		int id = FavoriteManager::getInstance()->findUserCommand(commandName);
		::UserCommand uc;

		if (id == -1 || !FavoriteManager::getInstance()->getUserCommand(id, uc))
			return;

		User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			ClientManager::getInstance()->userCommand(user, uc, params, true);
	}
}

void Hub::getFileList_client(string cid, bool match)
{
	string message;

	if (!cid.empty())
	{
		try
		{
			User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
			if (user)
			{
				if (match)
					QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
				else
					QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
			}
			else
				message = _("User not found");
		}
		catch (const Exception &e)
		{
			message = e.getError();
			LogManager::getInstance()->message(message);
		}
	}

	if (!message.empty())
	{
		typedef Func1<Hub, string> F1;
		F1 *func = new F1(this, &Hub::addStatusMessage_gui, message);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::grantSlot_client(string cid)
{
	string message = _("User not found");

	if (!cid.empty())
	{
		User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
		{
			UploadManager::getInstance()->reserveSlot(user);
			message = _("Slot granted to ") + WulforUtil::getNicks(user);
		}
	}

	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::addStatusMessage_gui, message);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::removeUserFromQueue_client(std::string cid)
{
	if (!cid.empty())
	{
		User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
	}
}

void Hub::redirect_client(string address)
{
	if (!address.empty())
	{
		if (ClientManager::getInstance()->isConnected(address))
		{
			string error = _("Unable to connect: already connected to the requested hub");
			typedef Func1<Hub, string> F1;
			F1 *f1 = new F1(this, &Hub::addStatusMessage_gui, error);
			WulforManager::get()->dispatchGuiFunc(f1);
			return;
		}

		if (BOOLSETTING(AUTO_FOLLOW))
		{
			// the client is dead, long live the client!
			string encoding = client->getEncoding();
			client->removeListener(this);
			ClientManager::getInstance()->putClient(client);

			Func0<Hub> *func = new Func0<Hub>(this, &Hub::clearNickList_gui);
			WulforManager::get()->dispatchGuiFunc(func);

			client = ClientManager::getInstance()->getClient(address);
			client->setEncoding(encoding);
			client->addListener(this);
			client->connect();
		}
	}
}

void Hub::rebuildHashData_client()
{
	HashManager::getInstance()->rebuild();
}

void Hub::refreshFileList_client()
{
	try
	{
		ShareManager::getInstance()->setDirty();
		ShareManager::getInstance()->refresh(true);
	}
	catch (const ShareException& e)
	{
	}
}

void Hub::addAsFavorite_client()
{
	typedef Func1<Hub, string> F1;
	F1 *func;

	FavoriteHubEntry *existingHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());

	if (!existingHub)
	{
		FavoriteHubEntry aEntry;
		aEntry.setServer(client->getHubUrl());
		aEntry.setName(client->getHubName());
		aEntry.setDescription(client->getHubDescription());
		aEntry.setConnect(FALSE);
		aEntry.setNick(client->getMyNick());
		FavoriteManager::getInstance()->addFavorite(aEntry);
		func = new F1(this, &Hub::addStatusMessage_gui, _("Favorite hub added"));
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else
	{
		func = new F1(this, &Hub::addStatusMessage_gui, _("Favorite hub already exists"));
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::reconnect_client()
{
	Func0<Hub> *func = new Func0<Hub>(this, &Hub::clearNickList_gui);
	WulforManager::get()->dispatchGuiFunc(func);

	if (client)
		client->reconnect();
}

void Hub::checkFavoriteUserJoin_client(string cid)
{
	User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user && FavoriteManager::getInstance()->isFavoriteUser(user))
	{
		string message = WulforUtil::getNicks(user) + _(" has joined");
		typedef Func1<Hub, std::string> F1;
		F1 *func = new F1(this, &Hub::addStatusMessage_gui, message);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::getParams_client(StringMap &params, Identity &id)
{
	if (id.getUser()->isSet(User::DCPLUSPLUS))
		params["Icon"] = "dc++";
	else
		params["Icon"] = "normal";

	if (id.getUser()->isSet(User::PASSIVE))
		params["Icon"] += "-fw";

	if (id.isOp())
	{
		params["Icon"] += "-op";
		params["Nick Order"] = "o" + id.getNick();
	}
	else
	{
		params["Nick Order"] = "u" + id.getNick();
	}

	params["Nick"] = id.getNick();
	params["Shared"] = Util::formatBytes(id.getBytesShared());
	params["Description"] = id.getDescription();
	params["Tag"] = id.getTag();
	params["Connection"] = id.getConnection();
	params["eMail"] = id.getEmail();
	params["Shared Bytes"] = Util::toString(id.getBytesShared());
	params["CID"] = id.getUser()->getCID().toBase32();
}

void Hub::on(ClientListener::Connecting, Client *) throw()
{
	typedef Func1<Hub, string> F1;
	F1 *f1 = new F1(this, &Hub::addStatusMessage_gui, _("Connecting to ") + client->getHubUrl() + "...");
	WulforManager::get()->dispatchGuiFunc(f1);
}

void Hub::on(ClientListener::Connected, Client *) throw()
{
	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::addStatusMessage_gui, _("Connected"));
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::UserUpdated, Client *, const OnlineUser &user) throw()
{
	Identity id = user.getIdentity();

	if (!id.isHidden())
	{
		StringMap params;
		getParams_client(params, id);
		Func1<Hub, StringMap> *func = new Func1<Hub, StringMap>(this, &Hub::updateUser_gui, params);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::UsersUpdated, Client *, const OnlineUser::List &list) throw()
{
	Identity id;
	typedef Func1<Hub, StringMap> F1;
	F1 *func;

	for (OnlineUser::List::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		id = (*it)->getIdentity();
		if (!id.isHidden())
		{
			StringMap params;
			getParams_client(params, id);
			func = new F1(this, &Hub::updateUser_gui, params);
			WulforManager::get()->dispatchGuiFunc(func);
		}
	}
}

void Hub::on(ClientListener::UserRemoved, Client *, const OnlineUser &user) throw()
{
	string nick = user.getIdentity().getNick();
	typedef Func1<Hub, string> F1;
	F1 *func;

	if (BOOLSETTING(SHOW_JOINS) || (BOOLSETTING(FAV_SHOW_JOINS) &&
		FavoriteManager::getInstance()->isFavoriteUser(user.getUser())))
	{
		func = new F1(this, &Hub::addStatusMessage_gui, nick + _(" has quit"));
		WulforManager::get()->dispatchGuiFunc(func);
	}

	func = new F1(this, &Hub::removeUser_gui, nick);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::Redirect, Client *, const string &address) throw()
{
	// redirect_client() crashes unless I put it into the dispatcher (why?)
	Func1<Hub, string> *func = new Func1<Hub, string>(this, &Hub::redirect_client, address);
	WulforManager::get()->dispatchClientFunc(func);
}

void Hub::on(ClientListener::Failed, Client *, const string &reason) throw()
{
	Func0<Hub> *f0 = new Func0<Hub>(this, &Hub::clearNickList_gui);
	WulforManager::get()->dispatchGuiFunc(f0);

	typedef Func1<Hub, string> F1;
	F1 *f1 = new F1(this, &Hub::addStatusMessage_gui, _("Connect failed: ") + reason);
	WulforManager::get()->dispatchGuiFunc(f1);
}

void Hub::on(ClientListener::GetPassword, Client *) throw()
{
	if (!client->getPassword().empty())
		client->password(client->getPassword());
	else
	{
		Func0<Hub> *func = new Func0<Hub>(this, &Hub::getPassword_gui);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::HubUpdated, Client *) throw()
{
	typedef Func1<Hub, string> F1;
	typedef Func2<MainWindow, GtkWidget *, string> F2;
	string hubName = _("Hub: ");

	if (client->getHubName().empty())
		hubName += client->getAddress() + ":" + Util::toString(client->getPort());
	else
		hubName += client->getHubName();

	if (!client->getHubDescription().empty())
		hubName += " - " + client->getHubDescription();

	F1 *func1 = new F1(this, &BookEntry::setLabel_gui, hubName);
	WulforManager::get()->dispatchGuiFunc(func1);

	F2 *func2 = new F2(WulforManager::get()->getMainWindow(), &MainWindow::modifyWindowItem, getWindowItem(), hubName);
	WulforManager::get()->dispatchGuiFunc(func2);
}

void Hub::on(ClientListener::Message, Client *, const OnlineUser &from, const string &message) throw()
{
	if (!message.empty())
	{
		string line = "<" + from.getIdentity().getNick() + "> " + message;

		if (BOOLSETTING(FILTER_MESSAGES))
		{
			if ((message.find("Hub-Security") != string::npos && message.find("was kicked by") != string::npos) ||
				(message.find("is kicking") != string::npos && message.find("because:") != string::npos))
			{
				typedef Func1<Hub, string> F1;
				F1 *func = new F1(this, &Hub::addStatusMessage_gui, line);
				WulforManager::get()->dispatchGuiFunc(func);
				return;
			}
		}

		if (BOOLSETTING(LOG_MAIN_CHAT))
		{
			StringMap params;
			params["message"] = line;
			client->getHubIdentity().getParams(params, "hub", false);
			params["hubURL"] = client->getHubUrl();
			client->getMyIdentity().getParams(params, "my", true);
			LOG(LogManager::CHAT, params);
		}

		typedef Func1<Hub, string> F1;
		F1 *func = new F1(this, &Hub::addMessage_gui, line);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::StatusMessage, Client *, const string &message) throw()
{
	if (!message.empty())
	{
		if (BOOLSETTING(FILTER_MESSAGES))
		{
			if ((message.find("Hub-Security") != string::npos && message.find("was kicked by") != string::npos) ||
				(message.find("is kicking") != string::npos && message.find("because:") != string::npos))
			{
				typedef Func1<Hub, string> F1;
				F1 *func = new F1(this, &Hub::addStatusMessage_gui, message);
				WulforManager::get()->dispatchGuiFunc(func);
				return;
			}
		}

		if (BOOLSETTING(LOG_STATUS_MESSAGES))
		{
			StringMap params;
			client->getHubIdentity().getParams(params, "hub", FALSE);
			params["hubURL"] = client->getHubUrl();
			client->getMyIdentity().getParams(params, "my", TRUE);
			params["message"] = message;
			LOG(LogManager::STATUS, params);
		}

		typedef Func1<Hub, string> F1;
		F1 *func = new F1(this, &Hub::addMessage_gui, message);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::PrivateMessage, Client *, const OnlineUser &from,
	const OnlineUser& to, const OnlineUser& replyTo, const string &msg) throw()
{
	string error;
	const OnlineUser& user = (replyTo.getUser() == ClientManager::getInstance()->getMe()) ? to : replyTo;
	string line = "<" + from.getIdentity().getNick() + "> " + msg;

	if (user.getIdentity().isHub() && BOOLSETTING(IGNORE_HUB_PMS))
	{
		error = _("Ignored private message from hub");
		typedef Func1<Hub, string> F1;
		F1 *func = new F1(this, &Hub::addStatusMessage_gui, error);
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else if (user.getIdentity().isBot() && BOOLSETTING(IGNORE_BOT_PMS))
	{
		error = _("Ignored private message from bot ") + user.getIdentity().getNick();
		typedef Func1<Hub, string> F1;
		F1 *func = new F1(this, &Hub::addStatusMessage_gui, error);
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else
	{
		typedef Func2<Hub, string, string> F2;
		F2 *func = new F2(this, &Hub::addPrivateMessage_gui, user.getUser()->getCID().toBase32(), line);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::NickTaken, Client *) throw()
{
	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::addStatusMessage_gui, _("Nick already taken"));
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::SearchFlood, Client *, const string &msg) throw()
{
	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::addStatusMessage_gui, _("Search spam from ") + msg);
	WulforManager::get()->dispatchGuiFunc(func);
}
