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

#include "privatemessage.hh"

#include <client/ClientManager.h>
#include <client/FavoriteManager.h>
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "search.hh"

using namespace std;

PrivateMessage::PrivateMessage(const string &cid):
	BookEntry(_("PM: ") + WulforUtil::getNicks(cid), "privatemessage.glade"),
	cid(cid),
	historyIndex(0),
	sentAwayMessage(FALSE)
{
	// Intialize the chat window
	if (SETTING(USE_OEM_MONOFONT))
	{
		PangoFontDescription *fontDesc = pango_font_description_new();
		pango_font_description_set_family(fontDesc, "Mono");
		gtk_widget_modify_font(getWidget("text"), fontDesc);
		pango_font_description_free(fontDesc);
	}
	GtkTextIter iter;
	buffer = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(getWidget("text")), buffer);
	gtk_text_buffer_get_end_iter(buffer, &iter);
	mark = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);
	handCursor = gdk_cursor_new(GDK_HAND2);

	if (BOOLSETTING(PRIVATE_MESSAGE_BEEP_OPEN))
		gdk_beep();

	// Connect the signals to their callback functions.
	g_signal_connect(getWidget("entry"), "activate", G_CALLBACK(onSendMessage_gui), (gpointer)this);
	g_signal_connect(getWidget("entry"), "key-press-event", G_CALLBACK(onKeyPress_gui), (gpointer)this);
	g_signal_connect(getWidget("text"), "motion-notify-event", G_CALLBACK(onChatPointerMoved_gui), (gpointer)this);
	g_signal_connect(getWidget("text"), "visibility-notify-event", G_CALLBACK(onChatVisibilityChanged_gui), (gpointer)this);
	g_signal_connect(getWidget("searchMagnetItem"), "activate", G_CALLBACK(onSearchMagnetClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("copyMagnetItem"), "activate", G_CALLBACK(onCopyMagnetClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("magnetPropertiesItem"), "activate", G_CALLBACK(onMagnetPropertiesClicked_gui), (gpointer)this);

	gtk_widget_grab_focus(getWidget("entry"));
	history.push_back("");
	User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
	isBot = user ? user->isSet(User::BOT) : FALSE;

	setLabel_gui(_("PM: ") + WulforUtil::getNicks(cid) + " [" + WulforUtil::getHubNames(cid) + "]");
}

PrivateMessage::~PrivateMessage()
{
	if (handCursor)
	{
		gdk_cursor_unref(handCursor);
		handCursor = NULL;
	}
}

void PrivateMessage::addMessage_gui(string message)
{
	addLine_gui(message);

	if (BOOLSETTING(LOG_PRIVATE_CHAT))
	{
		StringMap params;
		params["message"] = message;
		params["hubNI"] = WulforUtil::getHubNames(cid);
		params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(CID(cid)));
		params["userCID"] = cid;
		params["userNI"] = ClientManager::getInstance()->getNicks(CID(cid))[0];
		params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
		LOG(LogManager::PM, params);
	}

	if (BOOLSETTING(BOLD_PM))
		setBold_gui();

	// Send an away message, but only the first time after setting away mode.
	if (!Util::getAway())
	{
		sentAwayMessage = FALSE;
	}
	else if (!sentAwayMessage && !(BOOLSETTING(NO_AWAYMSG_TO_BOTS) && isBot))
	{
		sentAwayMessage = TRUE;
		typedef Func1<PrivateMessage, string> F1;
		F1 *func = new F1(this, &PrivateMessage::sendMessage_client, Util::getAwayMessage());
		WulforManager::get()->dispatchClientFunc(func);
	}

	if (BOOLSETTING(PRIVATE_MESSAGE_BEEP))
	{
		MainWindow *mw = WulforManager::get()->getMainWindow();
		GdkWindowState state = gdk_window_get_state(mw->getContainer()->window);
	 	if ((state & GDK_WINDOW_STATE_ICONIFIED) || mw->currentPage_gui() != getContainer())
			gdk_beep();
	}
}

void PrivateMessage::addStatusMessage_gui(string message)
{
	addLine_gui("*** " + message);
}

void PrivateMessage::addLine_gui(const string &message)
{
	GtkTextIter iter;
	GtkAdjustment *adj;
	bool setBottom;
	string line = "";
	vector<int> magnets;
	string magnet;
	string::size_type start, end;
	GtkTextTag *tag;
	GtkTextIter i_start, i_end;

	adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(getWidget("scroll")));
	setBottom = gtk_adjustment_get_value(adj) >= (adj->upper - adj->page_size);

	if (BOOLSETTING(TIME_STAMPS))
		line = "[" + Util::getShortTimeString() + "] ";
	line += message + "\n";

	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_buffer_insert(buffer, &iter, line.c_str(), line.size());

	// check for magnet links in line
	magnets = WulforUtil::findMagnets(line);
	gtk_text_buffer_get_end_iter(buffer, &iter);

	for (string::size_type i = 0; i < magnets.size(); i += 2)
	{
		start = magnets[i];
		end = magnets[i + 1];
		magnet = line.substr(start, end - start);

		// check for the magnet in our buffer
		tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer), magnet.c_str());

		if (!tag)
		{
			tag = gtk_text_buffer_create_tag(buffer, magnet.c_str(), "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, NULL);
			g_signal_connect(tag, "event", G_CALLBACK(onMagnetTagEvent_gui), (gpointer)this);
		}

		i_start = i_end = iter;
		gtk_text_iter_backward_chars(&i_start, g_utf8_strlen(line.c_str() + start, -1));
		gtk_text_iter_backward_chars(&i_end, g_utf8_strlen(line.c_str() + end, -1));
		gtk_text_buffer_apply_tag(buffer, tag, &i_start, &i_end);
	}

	if (gtk_text_buffer_get_line_count(buffer) > maxLines)
	{
		GtkTextIter next;
		gtk_text_buffer_get_start_iter(buffer, &iter);
		gtk_text_buffer_get_iter_at_line(buffer, &next, 1);
		gtk_text_buffer_delete(buffer, &iter, &next);
	}

	if (setBottom)
	{
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_move_mark(buffer, mark, &iter);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(getWidget("text")), mark, 0, FALSE, 0, 0);
	}
}

void PrivateMessage::onSendMessage_gui(GtkEntry *entry, gpointer data)
{
	string text = gtk_entry_get_text(entry);
	if (text.empty())
		return;

	PrivateMessage *pm = (PrivateMessage *)data;
	gtk_entry_set_text(entry, "");

	// Store line in chat history
	pm->history.pop_back();
	pm->history.push_back(text);
	pm->history.push_back("");
	pm->historyIndex = pm->history.size() - 1;
	if (pm->history.size() > maxHistory + 1)
		pm->history.erase(pm->history.begin());

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
				pm->addStatusMessage_gui(_("Away mode off"));
				pm->sentAwayMessage = FALSE;
			}
			else
			{
				Util::setAway(TRUE);
				Util::setManualAway(TRUE);
				Util::setAwayMessage(param);
				pm->addStatusMessage_gui(_("Away mode on: ") + Util::getAwayMessage());
			}
		}
		else if (command == _("back"))
		{
			Util::setAway(FALSE);
			pm->addStatusMessage_gui(_("Away mode off"));
		}
		else if (command == _("clear"))
		{
			GtkTextIter startIter, endIter;
			gtk_text_buffer_get_start_iter(pm->buffer, &startIter);
			gtk_text_buffer_get_end_iter(pm->buffer, &endIter);
			gtk_text_buffer_delete(pm->buffer, &startIter, &endIter);
		}
		else if (command == _("close"))
		{
			WulforManager::get()->deleteEntry_gui(pm);
		}
		else if (command == _("favorite") || text == _("fav"))
		{
			typedef Func0<PrivateMessage> F0;
			F0 *func = new F0(pm, &PrivateMessage::addFavoriteUser_client);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else if (command == _("getlist"))
		{
			typedef Func0<PrivateMessage> F0;
			F0 *func = new F0(pm, &PrivateMessage::getFileList_client);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else if (command == _("grant"))
		{
			typedef Func0<PrivateMessage> F0;
			F0 *func = new F0(pm, &PrivateMessage::grantSlot_client);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else if (command == _("help"))
		{
			pm->addStatusMessage_gui(_("Available commands: /away <message>, /back, /clear, /close, /favorite, /getlist, /grant, /help"));
		}
		else
		{
			pm->addStatusMessage_gui(_("Unknown command ") + text + _(": type /help for a list of available commands"));
		}
	}
	else
	{
		typedef Func1<PrivateMessage, string> F1;
		F1 *func = new F1(pm, &PrivateMessage::sendMessage_client, text);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

gboolean PrivateMessage::onKeyPress_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	string text;
	size_t index;

	if (event->keyval == GDK_Up || event->keyval == GDK_KP_Up)
	{
		index = pm->historyIndex - 1;
		if (index >= 0 && index < pm->history.size())
		{
			text = pm->history[index];
			pm->historyIndex = index;
			gtk_entry_set_text(GTK_ENTRY(widget), text.c_str());
		}
		return TRUE;
	}
	else if (event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
	{
		index = pm->historyIndex + 1;
		if (index >= 0 && index < pm->history.size())
		{
			text = pm->history[index];
			pm->historyIndex = index;
			gtk_entry_set_text(GTK_ENTRY(widget), text.c_str());
		}
		return TRUE;
	}

	return FALSE;
}

gboolean PrivateMessage::onMagnetTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	if (event->type == GDK_BUTTON_PRESS)
	{
		pm->selectedMagnet = tag->name;

		switch (event->button.button)
		{
			case 1:
				// Search for magnet
				onSearchMagnetClicked_gui(NULL, data);
				break;
			case 3:
				// Popup magnet context menu
				gtk_widget_show_all(pm->getWidget("magnetMenu"));
				gtk_menu_popup(GTK_MENU(pm->getWidget("magnetMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
				break;
		}
		return TRUE;
	}
	else if (event->type == GDK_MOTION_NOTIFY)
	{
		// Change to a hand cursor when the cursor first moves over the magnet.
		if (!pm->aboveMagnet)
		{
			pm->aboveMagnet = TRUE;
			gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(textView), GTK_TEXT_WINDOW_TEXT), pm->handCursor);
		}
		return TRUE;
	}

	return FALSE;
}

gboolean PrivateMessage::onChatPointerMoved_gui(GtkWidget* widget, GdkEventMotion* event, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	if (pm->aboveMagnet)
	{
		pm->aboveMagnet = FALSE;
		gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT), NULL);
	}

	return FALSE;
}

gboolean PrivateMessage::onChatVisibilityChanged_gui(GtkWidget* widget, GdkEventVisibility* event, gpointer data)
{
	return onChatPointerMoved_gui(widget, NULL, data);
}

void PrivateMessage::onSearchMagnetClicked_gui(GtkMenuItem *item, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	string name;
	int64_t size;
	string tth;

	if (WulforUtil::splitMagnet(pm->selectedMagnet, name, size, tth))
	{
		Search *s = dynamic_cast<Search *>(WulforManager::get()->addSearch_gui());
		s->putValue_gui(tth, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
	}
}

void PrivateMessage::onCopyMagnetClicked_gui(GtkMenuItem *item, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), pm->selectedMagnet.c_str(), pm->selectedMagnet.length());
}

void PrivateMessage::onMagnetPropertiesClicked_gui(GtkMenuItem *item, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;

	WulforManager::get()->getMainWindow()->openMagnetDialog_gui(pm->selectedMagnet);
}

void PrivateMessage::sendMessage_client(std::string message)
{
	User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
	if (user && user->isOnline())
	{
		ClientManager::getInstance()->privateMessage(user, message);
	}
	else
	{
		typedef Func1<PrivateMessage, string> F1;
		F1 *func = new F1(this, &PrivateMessage::addStatusMessage_gui, _("User went offline"));
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void PrivateMessage::addFavoriteUser_client()
{
	User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
	if (user)
	{
		FavoriteManager::getInstance()->addFavoriteUser(user);
	}
	else
	{
		typedef Func1<PrivateMessage, string> F1;
		F1 *func = new F1(this, &PrivateMessage::addStatusMessage_gui, _("Added user to favorites list"));
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void PrivateMessage::getFileList_client()
{
	try
	{
		User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	}
	catch (const Exception& e)
	{
		typedef Func1<PrivateMessage, string> F1;
		F1 *func = new F1(this, &PrivateMessage::addStatusMessage_gui, e.getError());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void PrivateMessage::grantSlot_client()
{
	User::Ptr user = ClientManager::getInstance()->findUser(CID(cid));
	if (user)
	{
		UploadManager::getInstance()->reserveSlot(user);
	}
	else
	{
		typedef Func1<PrivateMessage, string> F1;
		F1 *func = new F1(this, &PrivateMessage::addStatusMessage_gui, _("Slot granted"));
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

