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

#include "hub.hh"

#include <client/FavoriteManager.h>
#include <client/HashManager.h>
#include <client/ShareManager.h>
#include "privatemessage.hh"
#include "settingsmanager.hh"
#include "wulformanager.hh"

using namespace std;

Hub::Hub(string address):
	BookEntry("Hub: " + address, "hub.glade")
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
	nickView.finalize();
	nickStore = gtk_list_store_newv(nickView.getColCount(), nickView.getGTypes());
	gtk_tree_view_set_model(nickView.get(), GTK_TREE_MODEL(nickStore));
	g_object_unref(nickStore);
	nickSelection = gtk_tree_view_get_selection(nickView.get());
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

	// Initialize nick completion
	completion = gtk_entry_completion_new();
	gtk_entry_completion_set_inline_completion(completion, FALSE);
	gtk_entry_set_completion(GTK_ENTRY(getWidget("chatEntry")), completion);
	g_object_unref(completion);
	gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(nickStore));
	gtk_entry_completion_set_text_column(completion, nickView.col("Nick"));

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

	gtk_widget_set_sensitive(getWidget("favoriteUserItem"), FALSE); // Not implemented yet
	gtk_widget_grab_focus(getWidget("chatEntry"));

	int nickPanePosition = WulforSettingsManager::get()->getInt("nick-pane-position");
	gtk_paned_set_position(GTK_PANED(getWidget("pane")), nickPanePosition);

	client = NULL;
	history.push_back("");
	historyIndex = 0;
	totalShared = 0;
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

	int nickPanePosition = gtk_paned_get_position(GTK_PANED(getWidget("pane")));
	WulforSettingsManager::get()->set("nick-pane-position", nickPanePosition);

	gtk_widget_destroy(GTK_WIDGET(GTK_DIALOG(getWidget("passwordDialog"))));
}

void Hub::setStatus_gui(string statusBar, string text)
{
	if (!text.empty())
	{
		gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusBar)), 0);
		gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusBar)), 0, text.c_str());
	}
}

bool Hub::findUser_gui(string nick, GtkTreeIter *iter)
{
	if (idMap.find(nick) != idMap.end())
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


void Hub::updateUser_gui(Identity id)
{
	GtkTreeIter iter;
	string nickOrder, icon;

	string nick = id.getNick();
	int64_t shared = id.getBytesShared();
	string description = id.getDescription();
	string tag = id.getTag();
	string connection = id.getConnection();
	string email = id.getEmail();

	if (id.getUser()->isSet(User::DCPLUSPLUS))
		icon = "dc++";
	else
		icon = "normal";

	if (id.getUser()->isSet(User::PASSIVE))
		icon += "-fw";

	if (id.isOp())
	{
		icon += "-op";
		nickOrder = "o" + nick;
	}
	else
	{
		nickOrder = "u" + nick;
	}

	if (findUser_gui(nick, &iter))
	{
		totalShared += shared - idMap[nick].getBytesShared();
		idMap[nick] = id;

		gtk_list_store_set(nickStore, &iter,
			nickView.col("Nick"), nick.c_str(),
			nickView.col("Shared"), Util::formatBytes(shared).c_str(),
			nickView.col("Description"), description.c_str(),
			nickView.col("Tag"), tag.c_str(),
			nickView.col("Connection"), connection.c_str(),
			nickView.col("eMail"), email.c_str(),
			nickView.col("Shared Bytes"), shared,
			nickView.col("Icon"), userIcons[icon],
			nickView.col("Nick Order"), nickOrder.c_str(),
			-1);
	}
	else
	{
		idMap[nick] = id;
		totalShared += shared;

		if (BOOLSETTING(SHOW_JOINS) || (BOOLSETTING(FAV_SHOW_JOINS) &&
			FavoriteManager::getInstance()->isFavoriteUser(id.getUser())))
		{
			addStatusMessage_gui(nick + " has joined");
		}

		gtk_list_store_insert_with_values(nickStore, &iter, idMap.size(),
			nickView.col("Nick"), nick.c_str(),
			nickView.col("Shared"), Util::formatBytes(shared).c_str(),
			nickView.col("Description"), description.c_str(),
			nickView.col("Tag"), tag.c_str(),
			nickView.col("Connection"), connection.c_str(),
			nickView.col("eMail"), email.c_str(),
			nickView.col("Shared Bytes"), shared,
			nickView.col("Icon"), userIcons[icon],
			nickView.col("Nick Order"), nickOrder.c_str(),
			-1);
	}

	setStatus_gui("statusUsers", Util::toString(idMap.size()) + " Users");
	setStatus_gui("statusShared", Util::formatBytes(totalShared));
}

void Hub::removeUser_gui(string nick)
{
	GtkTreeIter iter;

	if (findUser_gui(nick, &iter))
	{
		totalShared -= idMap[nick].getBytesShared();
		gtk_list_store_remove(nickStore, &iter);
		idMap.erase(nick);
		setStatus_gui("statusUsers", Util::toString(idMap.size()) + " Users");
		setStatus_gui("statusShared", Util::formatBytes(totalShared));
	}
}

void Hub::clearNickList_gui()
{
	gtk_list_store_clear(nickStore);
	idMap.clear();
	totalShared = 0;
	setStatus_gui("statusUsers", "0 Users");
	setStatus_gui("statusShared", Util::formatBytes(totalShared));
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

void Hub::addPrivateMessage_gui(Identity id, string msg)
{
	string nick = id.getNick();
	BookEntry *entry;

	if (id.getUser()->isOnline())
	{
		entry = WulforManager::get()->addPrivMsg_gui(id.getUser(), !BOOLSETTING(POPUNDER_PM));
		dynamic_cast< ::PrivateMessage *>(entry)->addMessage_gui(msg);
	}
	else
	{
		if (BOOLSETTING(IGNORE_OFFLINE))
		{
			addStatusMessage_gui("Ignored private message from " + nick);
		}
		else
		{
			entry = WulforManager::get()->addPrivMsg_gui(id.getUser(), !BOOLSETTING(POPUNDER_PM));
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

		if (command == "away")
		{
			if (Util::getAway())
			{
				Util::setAway(FALSE);
				Util::setManualAway(FALSE);
				hub->addStatusMessage_gui("Away mode off");
			}
			else
			{
				Util::setAway(TRUE);
				Util::setManualAway(TRUE);
				if (!param.empty())
					Util::setAwayMessage(param);
				hub->addStatusMessage_gui("Away mode on: " + Util::getAwayMessage());
			}
		}
		else if (command == "back")
		{
			Util::setAway(FALSE);
			hub->addStatusMessage_gui("Away mode off");
		}
		else if (command == "clear")
		{
			GtkTextIter startIter, endIter;
			gtk_text_buffer_get_start_iter(hub->chatBuffer, &startIter);
			gtk_text_buffer_get_end_iter(hub->chatBuffer, &endIter);
			gtk_text_buffer_delete(hub->chatBuffer, &startIter, &endIter);
		}
		else if (command == "close")
		{
			/// @todo: figure out why this sometimes closes and reopens the tab
			WulforManager::get()->deleteBookEntry_gui((BookEntry *)hub);
		}
		else if (command == "favorite" || command == "fav")
		{
			WulforManager::get()->dispatchClientFunc(new Func0<Hub>(hub, &Hub::addAsFavorite_client));
		}
		else if (command == "getlist")
		{
			typedef Func2<Hub, string, bool> F2;
			F2 *f2 = new F2(hub, &Hub::getFileList_client, param, FALSE);
			WulforManager::get()->dispatchClientFunc(f2);
		}
		else if (command == "grant")
		{
			func = new F1(hub, &Hub::grantSlot_client, param);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else if (command == "help")
		{
			hub->addStatusMessage_gui("Available commands: /away <message>, /back, /clear, /close, /favorite, " \
				 "/getlist <nick>, /grant <nick>, /help, /join <address>, /pm <nick>, /rebuild, /refresh, /userlist");
		}
		else if (command == "join" && !param.empty())
		{
			if (BOOLSETTING(JOIN_OPEN_NEW_WINDOW))
			{
				WulforManager::get()->addHub_gui(param);
			}
			else
			{
				func = new F1(hub, &Hub::redirect_client, param);
				WulforManager::get()->dispatchClientFunc(func);
			}
		}
		else if (command == "pm")
		{
			if (hub->idMap.find(param) != hub->idMap.end())
				WulforManager::get()->addPrivMsg_gui(hub->idMap[param].getUser());
			else
				hub->addStatusMessage_gui("User not found");
		}
		else if (command == "rebuild")
		{
			WulforManager::get()->dispatchClientFunc(new Func0<Hub>(hub, &Hub::rebuildHashData_client));
		}
		else if (command == "refresh")
		{
			WulforManager::get()->dispatchClientFunc(new Func0<Hub>(hub, &Hub::refreshFileList_client));
		}
		else if (command == "userlist")
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
			hub->addStatusMessage_gui("Unknown command '" + text + "': type /help for a list of valid commands");
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

	return FALSE;
}

gboolean Hub::onNickListButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Hub *hub = (Hub *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(hub->nickSelection, NULL, &iter))
	{
		if (event->button == 1 && hub->oldType == GDK_2BUTTON_PRESS)
		{
			string nick = hub->nickView.getString(&iter, "Nick");

			typedef Func2<Hub, string, bool> F2;
			F2 *func = new F2(hub, &Hub::getFileList_client, nick, FALSE);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else if (event->button == 2 && event->type == GDK_BUTTON_RELEASE)
		{
			string nick = hub->nickView.getString(&iter, "Nick");
			WulforManager::get()->addPrivMsg_gui(hub->idMap[nick].getUser());
		}
		else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
		{
			gtk_menu_popup(GTK_MENU(hub->getWidget("nickMenu")), NULL, NULL, NULL, NULL, 0, event->time);
			gtk_widget_show_all(hub->getWidget("nickMenu"));
		}
	}

	return FALSE;
}

gboolean Hub::onNickListKeyRelease_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	Hub *hub = (Hub *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(hub->nickSelection, NULL, &iter))
	{
		if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			gtk_menu_popup(GTK_MENU(hub->getWidget("nickMenu")), NULL, NULL, NULL, NULL, 0, event->time);
			gtk_widget_show_all(hub->getWidget("nickMenu"));
		}
		else if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		{
			string nick = hub->nickView.getString(&iter, "Nick");

			typedef Func2<Hub, string, bool> F2;
			F2 *func = new F2(hub, &Hub::getFileList_client, nick, FALSE);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}

	return FALSE;
}

gboolean Hub::onEntryKeyPress_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	Hub *hub = (Hub *)data;
	GtkEntry *chatEntry = GTK_ENTRY(widget);
	string text;
	size_t index;

	if (event->keyval == GDK_Up || event->keyval == GDK_KP_Up)
	{
		index = hub->historyIndex - 1;
		if (index >= 0 && index < hub->history.size())
		{
			text = hub->history[index];
			hub->historyIndex = index;
			gtk_entry_set_text(chatEntry, text.c_str());
		}
		return TRUE;
	}
	else if (event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
	{
		index = hub->historyIndex + 1;
		if (index >= 0 && index < hub->history.size())
		{
			text = hub->history[index];
			hub->historyIndex = index;
			gtk_entry_set_text(chatEntry, text.c_str());
		}
		return TRUE;
	}

	return FALSE;
}

void Hub::onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;
	GtkTreeIter iter;
	string nick;

	if (gtk_tree_selection_get_selected(hub->nickSelection, NULL, &iter))
	{
		string nick = hub->nickView.getString(&iter, "Nick");

		typedef Func2<Hub, string, bool> F2;
		F2 *func = new F2(hub, &Hub::getFileList_client, nick, FALSE);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void Hub::onMatchItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;
	GtkTreeIter iter;
	string nick;

	if (gtk_tree_selection_get_selected(hub->nickSelection, NULL, &iter))
	{
		string nick = hub->nickView.getString(&iter, "Nick");

		typedef Func2<Hub, string, bool> F2;
		F2 *func = new F2(hub, &Hub::getFileList_client, nick, TRUE);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void Hub::onMsgItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(hub->nickSelection, NULL, &iter))
	{
		string nick = hub->nickView.getString(&iter, "Nick");
		WulforManager::get()->addPrivMsg_gui(hub->idMap[nick].getUser());
	}
}

void Hub::onGrantItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;
	GtkTreeIter iter;
	string nick;

	if (gtk_tree_selection_get_selected(hub->nickSelection, NULL, &iter))
	{
		string nick = hub->nickView.getString(&iter, "Nick");

		typedef Func1<Hub, string> F1;
		F1 *func = new F1(hub, &Hub::grantSlot_client, nick);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void Hub::onRemoveUserItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	Hub *hub = (Hub *)data;
	GtkTreeIter iter;
	string nick;

	if (gtk_tree_selection_get_selected(hub->nickSelection, NULL, &iter))
	{
		string nick = hub->nickView.getString(&iter, "Nick");

		typedef Func1<Hub, string> F1;
		F1 *func = new F1(hub, &Hub::removeUserFromQueue_client, nick);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void Hub::connectClient_client(string address, string nick, string desc, string password)
{
	dcassert(client == NULL);
	client = ClientManager::getInstance()->getClient(address);

	if (!nick.empty())
		client->getMyIdentity().setNick(nick);
	else
		client->getMyIdentity().setNick(SETTING(NICK));

	if (!desc.empty())
		client->getMyIdentity().setDescription(desc);
	else
		client->getMyIdentity().setDescription(SETTING(DESCRIPTION));

	client->addListener(this);
	client->setPassword(password);
	client->connect();
}

void Hub::setPassword_client(string password)
{
	if (client)
	{
		client->setPassword(password);
		client->password(password);
	}
}

void Hub::sendMessage_client(string message)
{
	if (client)
		client->hubMessage(message);
}

void Hub::getFileList_client(string nick, bool match)
{
	string message;

	if (idMap.find(nick) != idMap.end())
	{
		try
		{
			if (match)
				QueueManager::getInstance()->addList(idMap[nick].getUser(), QueueItem::FLAG_MATCH_QUEUE);
			else
				QueueManager::getInstance()->addList(idMap[nick].getUser(), QueueItem::FLAG_CLIENT_VIEW);
		}
		catch (const Exception& e)
		{
			message = e.getError();
			LogManager::getInstance()->message(message);
		}
	}
	else
	{
		message = "User not found";
	}

	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::addStatusMessage_gui, message);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::grantSlot_client(string nick)
{
	string message;

	if (idMap.find(nick) != idMap.end())
	{
		UploadManager::getInstance()->reserveSlot(idMap[nick].getUser());
		message = "Slot granted to " + nick;
	}
	else
	{
		message = "User not found";
	}

	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::addStatusMessage_gui, message);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::removeUserFromQueue_client(std::string nick)
{
	if (idMap.find(nick) != idMap.end())
		QueueManager::getInstance()->removeSource(idMap[nick].getUser(), QueueItem::Source::FLAG_REMOVED);
}

void Hub::redirect_client(string address)
{
	if (!address.empty())
	{
		if (ClientManager::getInstance()->isConnected(address))
		{
			string error = "Unable to connect: already connected to the requested hub";
			typedef Func1<Hub, string> F1;
			F1 *f1 = new F1(this, &Hub::addStatusMessage_gui, error);
			WulforManager::get()->dispatchGuiFunc(f1);
			return;
		}

		if (BOOLSETTING(AUTO_FOLLOW))
		{
			// the client is dead, long live the client!
			client->removeListener(this);
			ClientManager::getInstance()->putClient(client);

			Func0<Hub> *func = new Func0<Hub>(this, &Hub::clearNickList_gui);
			WulforManager::get()->dispatchGuiFunc(func);

			client = ClientManager::getInstance()->getClient(address);
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
		func = new F1(this, &Hub::addStatusMessage_gui, "Favorite hub added");
		WulforManager::get()->dispatchGuiFunc(func);
	}
	else
	{
		func = new F1(this, &Hub::addStatusMessage_gui, "Favorite hub already exists");
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::reconnect_client()
{
	Func0<Hub> *func = new Func0<Hub>(this, &Hub::clearNickList_gui);
	WulforManager::get()->dispatchGuiFunc(func);

	if (client)
	{
		client->disconnect(FALSE);
		client->connect();
	}
}

void Hub::on(ClientListener::Connecting, Client *) throw()
{
	typedef Func1<Hub, string> F1;
	F1 *f1 = new F1(this, &Hub::addStatusMessage_gui, "Connecting");
	WulforManager::get()->dispatchGuiFunc(f1);
}

void Hub::on(ClientListener::Connected, Client *) throw()
{
	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::addStatusMessage_gui, "Connected");
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::BadPassword, Client *) throw()
{
	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::addStatusMessage_gui, "Invalid password");
	WulforManager::get()->dispatchGuiFunc(func);

	client->setPassword("");
}

void Hub::on(ClientListener::UserUpdated, Client *, const OnlineUser &user) throw()
{
	Identity id = user.getIdentity();

	if (!id.isHidden())
	{
		Func1<Hub, Identity> *func = new Func1<Hub, Identity>(this, &Hub::updateUser_gui, id);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void Hub::on(ClientListener::UsersUpdated, Client *, const OnlineUser::List &list) throw()
{
	Identity id;
	typedef Func1<Hub, Identity> F1;
	F1 *func;

	for (OnlineUser::List::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		id = (*it)->getIdentity();
		if (!id.isHidden())
		{
			func = new F1(this, &Hub::updateUser_gui, id);
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
		func = new F1(this, &Hub::addStatusMessage_gui, nick + " has quit");
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
	F1 *f1 = new F1(this, &Hub::addStatusMessage_gui, "Connect failed: " + reason);
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
	string hubName = "Hub: ";

	if (client->getHubName().empty())
		hubName += client->getAddress() + ":" + Util::toString(client->getPort());
	else
		hubName += client->getHubName();

	F1 *func1 = new F1(this, &BookEntry::setLabel_gui, hubName);
	WulforManager::get()->dispatchGuiFunc(func1);

	F2 *func2 = new F2(WulforManager::get()->getMainWindow(), &MainWindow::modifyWindowItem, getWidget("mainBox"), hubName);
	WulforManager::get()->dispatchGuiFunc(func2);
}

void Hub::on(ClientListener::Message, Client *, const OnlineUser &from, const string &message) throw()
{
	if (!message.empty())
	{
		string line = "<" + from.getIdentity().getNick() + "> " + message;

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

void Hub::on(ClientListener::PrivateMessage,	Client *, const OnlineUser &from,
	const OnlineUser& to, const OnlineUser& replyTo, const string &msg) throw()
{
	const OnlineUser& user = (replyTo.getUser() == ClientManager::getInstance()->getMe()) ? to : replyTo;
	string line = "<" + from.getIdentity().getNick() + "> " + msg;

	typedef Func2<Hub, Identity, string> F2;
	F2 *func = new F2(this, &Hub::addPrivateMessage_gui, user.getIdentity(), line);
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::NickTaken, Client *) throw()
{
	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::addStatusMessage_gui, "Nick already taken");
	WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::SearchFlood, Client *, const string &msg) throw()
{
	typedef Func1<Hub, string> F1;
	F1 *func = new F1(this, &Hub::addStatusMessage_gui, "Search spam from " + msg);
	WulforManager::get()->dispatchGuiFunc(func);
}
