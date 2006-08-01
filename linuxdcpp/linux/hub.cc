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

using namespace std;

Hub::Hub(string address):
	BookEntry("Hub: " + address)
{
	TimerManager::getInstance()->addListener(this);

	GladeXML *xml = getGladeXML("hub.glade");

	// Initialize mainbox
	GtkWidget *window = glade_xml_get_widget(xml, "hubWindow");
	mainBox = glade_xml_get_widget(xml, "hubBox");
	gtk_widget_ref(mainBox);
	gtk_container_remove(GTK_CONTAINER(window), mainBox);
	gtk_widget_destroy(window);

	// Get widgets from glade xml.
	chatText = GTK_TEXT_VIEW(glade_xml_get_widget(xml, "chatText"));
	nickPane = GTK_PANED(glade_xml_get_widget(xml, "pane"));
	passwordDialog = GTK_DIALOG(glade_xml_get_widget(xml, "passwordDialog"));
	passwordEntry = GTK_ENTRY(glade_xml_get_widget(xml, "passwordEntry"));
	chatEntry = GTK_ENTRY(glade_xml_get_widget(xml, "chatEntry"));
	chatText = GTK_TEXT_VIEW(glade_xml_get_widget(xml, "chatText"));
	chatScroll = GTK_SCROLLED_WINDOW(glade_xml_get_widget(xml, "chatScroll"));
	mainStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusMain"));
	usersStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusUsers"));
	sharedStatus = GTK_STATUSBAR(glade_xml_get_widget(xml, "statusShared"));
	scrolledwindow2 = glade_xml_get_widget(xml, "scrolledwindow2");

	gtk_dialog_set_alternative_button_order(passwordDialog, GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	if (BOOLSETTING(USE_OEM_MONOFONT))
	{
		PangoFontDescription *font_desc;
		font_desc = pango_font_description_from_string("Mono 10");
		gtk_widget_modify_font(GTK_WIDGET(chatText), font_desc);
		pango_font_description_free(font_desc);
	}
	int nickPanePosition = WulforSettingsManager::get()->getInt("nick-pane-position");
	gtk_paned_set_position(nickPane, nickPanePosition);

	// Initialize nick treeview
	nickView.setView(GTK_TREE_VIEW(glade_xml_get_widget(xml, "nickView")), true, "hub");
	nickView.insertColumn("Nick", G_TYPE_STRING, TreeView::PIXBUF_STRING, 100, "Icon");
	nickView.insertColumn("Shared", G_TYPE_STRING, TreeView::STRING, 75);
	nickView.insertColumn("Description", G_TYPE_STRING, TreeView::STRING, 75);
	nickView.insertColumn("Tag", G_TYPE_STRING, TreeView::STRING, 100);
	nickView.insertColumn("Connection", G_TYPE_STRING, TreeView::STRING, 75);
	nickView.insertColumn("eMail", G_TYPE_STRING, TreeView::STRING, 100);
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
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(nickView.get(), nickView.col("Nick")), TRUE);
	gtk_tree_view_set_fixed_height_mode(nickView.get(), TRUE);
	sorted = FALSE;

	// Initialize chat
	chatBuffer = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(chatText, chatBuffer);
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(chatBuffer, &iter);
	chatMark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, FALSE);

	// Initialize nick completion
	completion = gtk_entry_completion_new();
	gtk_entry_completion_set_inline_completion(completion, FALSE);
	gtk_entry_set_completion(chatEntry, completion);
	g_object_unref(completion);
	gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(nickStore));
	gtk_entry_completion_set_text_column(completion, nickView.col("Nick"));

	// Initialize nick popup menu
	nickMenu = GTK_MENU(glade_xml_get_widget(xml, "nickMenu"));
	GtkWidget *browseItem = glade_xml_get_widget(xml, "browseItem");
	g_signal_connect(G_OBJECT(browseItem), "activate", G_CALLBACK(onBrowseItemClicked_gui), (gpointer)this);
	GtkWidget *msgItem = glade_xml_get_widget(xml, "msgItem");
	g_signal_connect(G_OBJECT(msgItem), "activate", G_CALLBACK(onMsgItemClicked_gui), (gpointer)this);
	GtkWidget *grantItem = glade_xml_get_widget(xml, "grantItem");
	g_signal_connect(G_OBJECT(grantItem), "activate", G_CALLBACK(onGrantItemClicked_gui), (gpointer)this);

	// Load icons for the nick list
	string icon, path = WulforManager::get()->getPath() + "/pixmaps/";
	icon = path + "normal.png";
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

	// Connect callbacks
	g_signal_connect(chatEntry, "activate", G_CALLBACK(onSendMessage_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(nickView.get()), "button-press-event", G_CALLBACK(onNickListButtonPress_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(nickView.get()), "button-release-event", G_CALLBACK(onNickListButtonRelease_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(nickView.get()), "key-release-event", G_CALLBACK(onNickListKeyRelease_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(chatEntry), "key-press-event", G_CALLBACK(onEntryKeyPress_gui), (gpointer)this);

	gtk_widget_grab_focus(GTK_WIDGET(chatEntry));

	client = NULL;
	history.push_back("");
	historyIndex = 0;
}

Hub::~Hub()
{
	TimerManager::getInstance()->removeListener(this);

	if (client)
	{
		client->removeListener(this);
		client->disconnect(TRUE);
		ClientManager::getInstance()->putClient(client);
		client = NULL;
	}

	hash_map<string, GdkPixbuf *>::iterator it;
	for (it = userIcons.begin(); it != userIcons.end(); it++)
		g_object_unref(G_OBJECT(it->second));

	int nickPanePosition = gtk_paned_get_position(nickPane);
	WulforSettingsManager::get()->set("nick-pane-position", nickPanePosition);

	gtk_widget_destroy(GTK_WIDGET(passwordDialog));
}

GtkWidget *Hub::getWidget()
{
	return mainBox;
}

void Hub::setStatus_gui(GtkStatusbar *status, string text)
{
	if (!text.empty())
	{
		gtk_statusbar_pop(status, 0);
		gtk_statusbar_push(status, 0, text.c_str());
	}
}

void Hub::findUser_gui(string nick, GtkTreeIter *iter)
{
	dcassert(idMap.find(nick) != idMap.end());

	GtkTreeModel *m = GTK_TREE_MODEL(nickStore);
	gboolean valid = gtk_tree_model_get_iter_first(m, iter);

	while (valid)
	{
		if (nick == nickView.getString(iter, "Nick"))
			valid = FALSE;
		else
			valid = gtk_tree_model_iter_next(m, iter);
	}
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


	if (idMap.find(nick) != idMap.end())
	{
		findUser_gui(nick, &iter);
	}
	else
	{
		gtk_list_store_append(nickStore, &iter);
		idMap[nick] = id;

		if (BOOLSETTING(SHOW_JOINS) || (BOOLSETTING(FAV_SHOW_JOINS) &&
			FavoriteManager::getInstance()->isFavoriteUser(id.getUser())))
		{
			addStatusMessage_gui(nick + " has joined");
		}
	}

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
		nickOrder = "u" + nick;

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

	usersUpdated = TRUE;
}

void Hub::removeUser_gui(string nick)
{
	if (idMap.find(nick) != idMap.end())
	{
		GtkTreeIter iter;
		findUser_gui(nick, &iter);
		gtk_list_store_remove(nickStore, &iter);
		idMap.erase(nick);
	}
}

void Hub::clearNickList_gui()
{
	gtk_list_store_clear(nickStore);
	idMap.clear();
}

void Hub::getPassword_gui()
{
	gint ret;

	gtk_widget_show_all(GTK_WIDGET(passwordDialog));
	ret = gtk_dialog_run(passwordDialog);
	gtk_widget_hide(GTK_WIDGET(passwordDialog));

	if (ret == GTK_RESPONSE_OK)
	{
		string password = gtk_entry_get_text(passwordEntry);
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
		setStatus_gui(mainStatus, message);

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

	adj = gtk_scrolled_window_get_vadjustment(chatScroll);
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
		gtk_text_view_scroll_to_mark(chatText, chatMark, 0, FALSE, 0, 0);
	}

	if (BOOLSETTING(BOLD_HUB))
		setBold_gui();
}

void Hub::addPrivateMessage_gui(Identity id, string msg)
{
	string nick = id.getNick();

	if (id.getUser()->isOnline())
	{
		WulforManager::get()->addPrivMsg_gui(id.getUser(), !BOOLSETTING(POPUNDER_PM))->addMessage_gui(msg);
	}
	else
	{
		if (BOOLSETTING(IGNORE_OFFLINE))
			addStatusMessage_gui("Ignored private message from " + nick);
		else
			WulforManager::get()->addPrivMsg_gui(id.getUser(), !BOOLSETTING(POPUNDER_PM))->addMessage_gui(msg);
	}
}

void Hub::sortList_gui()
{
	gint sortColumn;
	GtkSortType sortType;

	gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(nickStore), &sortColumn, &sortType);

	if (sortColumn == GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID)
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), nickView.col("Nick Order"), sortType);
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
			func = new F1(hub, &Hub::getFileList_client, param);
			WulforManager::get()->dispatchClientFunc(func);
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
			if (GTK_WIDGET_VISIBLE(hub->scrolledwindow2))
				gtk_widget_hide(hub->scrolledwindow2);
			else
				gtk_widget_show_all(hub->scrolledwindow2);
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

			typedef Func1<Hub, string> F1;
			F1 *func = new F1(hub, &Hub::getFileList_client, nick);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else if (event->button == 2 && event->type == GDK_BUTTON_RELEASE)
		{
			string nick = hub->nickView.getString(&iter, "Nick");
			WulforManager::get()->addPrivMsg_gui(hub->idMap[nick].getUser());
		}
		else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
		{
			gtk_menu_popup(hub->nickMenu, NULL, NULL, NULL, NULL, 0, event->time);
			gtk_widget_show_all(GTK_WIDGET(hub->nickMenu));
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
			gtk_menu_popup(hub->nickMenu, NULL, NULL, NULL, NULL, 0, event->time);
			gtk_widget_show_all(GTK_WIDGET(hub->nickMenu));
		}
		else if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		{
			string nick = hub->nickView.getString(&iter, "Nick");

			typedef Func1<Hub, string> F1;
			F1 *func = new F1(hub, &Hub::getFileList_client, nick);
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

		typedef Func1<Hub, string> F1;
		F1 *func = new F1(hub, &Hub::getFileList_client, nick);
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

void Hub::connectClient_client(string address, string nick, string desc, string password)
{
	dcassert(client == NULL);
	client = ClientManager::getInstance()->getClient(address);

	if (nick.empty())
		client->getMyIdentity().setNick(SETTING(NICK));
	else
		client->getMyIdentity().setNick(nick);

	if (!desc.empty())
		client->getMyIdentity().setDescription(desc);

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

void Hub::getFileList_client(string nick)
{
	string message;

	if (idMap.find(nick) != idMap.end())
	{
		try
		{
			QueueManager::getInstance()->addList(idMap[nick].getUser(), QueueItem::FLAG_CLIENT_VIEW);
		}
		catch (const Exception& e)
		{
			message = e.getError();
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

	for (OnlineUser::List::const_iterator it = list.begin(); it != list.end(); it++)
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

	F2 *func2 = new F2(WulforManager::get()->getMainWindow(), &MainWindow::modifyWindowItem, getWidget(), hubName);
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
			params["message"] = message;
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

void Hub::on(TimerManagerListener::Second, u_int32_t tics) throw()
{
	if (usersUpdated)
	{
		string users = Util::toString(idMap.size()) + " Users";

		typedef Func2<Hub, GtkStatusbar *, string> F2;
		F2 *f2 = new F2(this, &Hub::setStatus_gui, usersStatus, users);
		WulforManager::get()->dispatchGuiFunc(f2);

		int64_t totalShared = 0;
		hash_map<string, Identity>::const_iterator iter;
		for (iter = idMap.begin(); iter != idMap.end(); iter++)
			totalShared += iter->second.getBytesShared();

		f2 = new F2(this, &Hub::setStatus_gui, sharedStatus, Util::formatBytes(totalShared));
		WulforManager::get()->dispatchGuiFunc(f2);

		usersUpdated = FALSE;
	}
}

/*
 * Sets the userlist to sorted at the first minute marker. Can't have it sorted when
 * first joining since GTK+ is very slow when inserting many rows into a sorted GtkTreeView.
 */
void Hub::on(TimerManagerListener::Minute, u_int32_t tics) throw()
{
	if (!sorted)
	{
		Func0<Hub> *f = new Func0<Hub>(this, &Hub::sortList_gui);
		WulforManager::get()->dispatchGuiFunc(f);

		sorted = TRUE;
	}
}
