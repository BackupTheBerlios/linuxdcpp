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

#include "WulforUtil.hh"
#include <glib/gi18n.h>
#include <client/ClientManager.h>

std::vector<std::string> WulforUtil::charsets;

vector<int> WulforUtil::splitString(const string &str, const string &delimiter)
{
	string::size_type loc, len, pos = 0;
	vector<int> array;

	if (!str.empty() && !delimiter.empty())
	{
		while ((loc = str.find(delimiter, pos)) != string::npos)
		{
			len = loc - pos;
			array.push_back(Util::toInt(str.substr(pos, len)));
			pos = loc + delimiter.size();
		}
		len = str.size() - pos;
		array.push_back(Util::toInt(str.substr(pos, len)));
	}
	return array;
}

string WulforUtil::linuxSeparator(const string &ps)
{
	string str = ps;
	for (string::iterator it = str.begin(); it != str.end(); ++it)
		if ((*it) == '\\')
			(*it) = '/';
	return str;
}

string WulforUtil::windowsSeparator(const string &ps)
{
	string str = ps;
	for (string::iterator it = str.begin(); it != str.end(); ++it)
		if ((*it) == '/')
			(*it) = '\\';
	return str;
}

string WulforUtil::getNicks(const CID& cid)
{
	return Util::toString(ClientManager::getInstance()->getNicks(cid));
}

string WulforUtil::getNicks(const User::Ptr& user)
{
	return getNicks(user->getCID());
}

string WulforUtil::getHubNames(const CID& cid)
{
	StringList hubs = ClientManager::getInstance()->getHubNames(cid);
	if (hubs.empty())
		return "Offline";
	else
		return Util::toString(hubs);
}

string WulforUtil::getHubNames(const User::Ptr& user)
{
	return getHubNames(user->getCID());
}

string WulforUtil::getTextFromMenu(GtkMenuItem *item)
{
	string text;
	GtkWidget *child = gtk_bin_get_child(GTK_BIN(item));

	if (child && GTK_IS_LABEL(child))
		text = gtk_label_get_text(GTK_LABEL(child));

	return text;
}

vector<string>& WulforUtil::getCharsets()
{
	if (charsets.size() == 0)
	{
		charsets.push_back(_("System default"));
		charsets.push_back(_("UTF-8 (Unicode)"));
		charsets.push_back(_("CP1252 (Western Europe)"));
		charsets.push_back(_("ISO-8859-2 (Central Europe)"));
		charsets.push_back(_("ISO-8859-7 (Greek)"));
		charsets.push_back(_("ISO-8859-8 (Hebrew)"));
		charsets.push_back(_("ISO-8859-9 (Turkish)"));
		charsets.push_back(_("ISO-2022-JP (Japanese)"));
		charsets.push_back(_("SJIS (Japanese)"));
		charsets.push_back(_("CP949 (Korean)"));
		charsets.push_back(_("KOI8-R (Cyrillic)"));
		charsets.push_back(_("CP1251 (Cyrillic)"));
		charsets.push_back(_("CP1256 (Arabic)"));
		charsets.push_back(_("CP1257 (Baltic)"));
		charsets.push_back(_("GB18030 (Chinese)"));
		charsets.push_back(_("TIS-620 (Thai)"));
	}
	return charsets;
}

void WulforUtil::openURI(const std::string &uri)
{
	gchar *argv[3];
	argv[0] = "gnome-open";
	argv[1] = (gchar *)uri.c_str();
	argv[2] = NULL;

	g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
}
