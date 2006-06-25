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

#include "privatemessage.hh"
#include "wulformanager.hh"

using namespace std;

PrivateMessage::PrivateMessage(User::Ptr user):
	BookEntry("PM: " + WulforUtil::getNicks(user)),
	user(user)
{
	GladeXML *xml = getGladeXML("privatemessage.glade");

	GtkWidget *window = glade_xml_get_widget(xml, "window");
	box = glade_xml_get_widget(xml, "box");
	gtk_widget_ref(box);
	gtk_container_remove(GTK_CONTAINER(window), box);
	gtk_widget_destroy(window);

	GtkWidget *entry = glade_xml_get_widget(xml, "entry");
	text = GTK_TEXT_VIEW(glade_xml_get_widget(xml, "text"));

	if (SETTING(USE_OEM_MONOFONT))
	{
		PangoFontDescription *font_desc;
		font_desc = pango_font_description_from_string("Mono 10");
		gtk_widget_modify_font(GTK_WIDGET(text), font_desc);
		pango_font_description_free(font_desc);
	}

	scroll = GTK_SCROLLED_WINDOW(glade_xml_get_widget(xml, "scroll"));

	buffer = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(text, buffer);
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(buffer, &iter);
	mark = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);

	g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(sendMessage_gui), (gpointer)this);
	g_signal_connect(G_OBJECT(entry), "key-press-event", G_CALLBACK(onKeyPress_gui), (gpointer)this);

	gtk_widget_grab_focus(entry);

	history.push_back("");
}

GtkWidget *PrivateMessage::getWidget()
{
	return box;
}

void PrivateMessage::addMessage_gui(string message)
{
	string line = "";

	if (BOOLSETTING(LOG_PRIVATE_CHAT))
	{
		StringMap params;
		params["message"] = Text::acpToUtf8(message);
		params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(user->getCID()));
		params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(user->getCID()));
		params["userCID"] = user->getCID().toBase32();
		params["userNI"] = ClientManager::getInstance()->getNicks(user->getCID())[0];
		params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
		LOG(LogManager::PM, params);
	}

	if (BOOLSETTING(TIME_STAMPS))
		line = "[" + Util::getShortTimeString() + "] ";
	line += message + "\n";

	addLine(line);
}

void PrivateMessage::addStatusMessage_gui(string message)
{
	string line = "";

	if (BOOLSETTING(TIME_STAMPS))
		line = "[" + Util::getShortTimeString() + "] ";
	line += "*** " + message + "\n";

	addLine(line);
}

void PrivateMessage::addLine(string line)
{
	GtkTextIter iter;
	GtkAdjustment *adj;
	bool setBottom;

	adj = gtk_scrolled_window_get_vadjustment(scroll);
	setBottom = gtk_adjustment_get_value(adj) >= (adj->upper - adj->page_size);

	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_buffer_insert(buffer, &iter, line.c_str(), line.size());

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
		gtk_text_view_scroll_to_mark(text, mark, 0, FALSE, 0, 0);
	}

	if (BOOLSETTING(BOLD_PM))
		setBold_gui();
}

void PrivateMessage::sendMessage_gui(GtkEntry *entry, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	string text = gtk_entry_get_text(entry);

	if (!text.empty())
	{
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
			if (text == "/clear")
			{
				GtkTextIter startIter, endIter;
				gtk_text_buffer_get_start_iter(pm->buffer, &startIter);
				gtk_text_buffer_get_end_iter(pm->buffer, &endIter);
				gtk_text_buffer_delete(pm->buffer, &startIter, &endIter);
			}
			else if (text == "/close")
			{
				WulforManager::get()->deleteBookEntry_gui((BookEntry *)pm);
			}
			else if (text == "/favorite" || text == "/fav")
			{
				FavoriteManager::getInstance()->addFavoriteUser(pm->user);
				pm->addStatusMessage_gui("Added user to favorites list");
			}
			else if (text == "/getlist")
			{
				try
				{
					QueueManager::getInstance()->addList(pm->user, QueueItem::FLAG_CLIENT_VIEW);
				}
				catch (const Exception& e)
				{
					pm->addStatusMessage_gui(Text::acpToUtf8(e.getError()));
				}
			}
			else if (text == "/grant")
			{
				UploadManager::getInstance()->reserveSlot(pm->user);
				pm->addStatusMessage_gui("Slot granted");
			}
			else if (text == "/help")
			{
				pm->addStatusMessage_gui("Available commands: /clear, /close, /favorite, /getlist, /grant, /help");
			}
			else
			{
				pm->addStatusMessage_gui("Unknown command " + text + ": type /help for a list of valid commands");
			}
		}
		else if (pm->user->isOnline())
		{
			typedef Func1<PrivateMessage, string> F1;
			F1 *func = new F1(pm, &PrivateMessage::sendMessage_client, text);
			WulforManager::get()->dispatchClientFunc(func);
		}
		else
		{
			pm->addStatusMessage_gui("User went offline");
		}
	}
}

gboolean PrivateMessage::onKeyPress_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	PrivateMessage *pm = (PrivateMessage *)data;
	string text;

	if (event->keyval == GDK_Up || event->keyval == GDK_KP_Up)
	{
		if (pm->historyIndex - 1 >= 0)
		{
			pm->historyIndex -= 1;
			text = pm->history[pm->historyIndex];
			gtk_entry_set_text(GTK_ENTRY(widget), text.c_str());
		}
		return TRUE;
	}
	else if (event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
	{
		if (pm->historyIndex + 1 < pm->history.size())
		{
			pm->historyIndex += 1;
			text = pm->history[pm->historyIndex];
			gtk_entry_set_text(GTK_ENTRY(widget), text.c_str());
		}
		return TRUE;
	}

	return FALSE;
}

void PrivateMessage::sendMessage_client(std::string message)
{
	ClientManager::getInstance()->privateMessage(user, message);
}
