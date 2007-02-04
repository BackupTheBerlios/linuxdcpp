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

#include "WulforUtil.hh"
#include <client/ClientManager.h>

std::vector<std::vector<std::string> > WulforUtil::charsets(16, vector<string>(2));

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

vector<vector<string> >& WulforUtil::getCharsets()
{
	if (charsets[0][0].empty())
	{
		charsets[0][0] = "System default";
		charsets[0][1] = "System default";
		charsets[1][0] = "UTF-8 (Unicode)";
		charsets[1][1] = "UTF-8";
		charsets[2][0] = "CP1252 (Western Europe)";
		charsets[2][1] = "CP1252";
		charsets[3][0] = "ISO-8859-2 (Central Europe)";
		charsets[3][1] = "ISO-8859-2";
		charsets[4][0] = "ISO-8859-7 (Greek)";
		charsets[4][1] = "ISO-8859-7";
		charsets[5][0] = "ISO-8859-8 (Hebrew)";
		charsets[5][1] = "ISO-8859-8";
		charsets[6][0] = "ISO-8859-9 (Turkish)";
		charsets[6][1] = "ISO-8859-9";
		charsets[7][0] = "ISO-2022-JP (Japanese)";
		charsets[7][1] = "ISO-2022-JP";
		charsets[8][0] = "SJIS (Japanese)";
		charsets[8][1] = "SJIS";
		charsets[9][0] = "CP949 (Korean)";
		charsets[9][1] = "CP949";
		charsets[10][0] = "KOI8-R (Cyrillic)";
		charsets[10][1] = "KOI8-R";
		charsets[11][0] = "CP1251 (Cyrillic)";
		charsets[11][1] = "CP1251";
		charsets[12][0] = "CP1256 (Arabic)";
		charsets[12][1] = "CP1256";
		charsets[13][0] = "CP1257 (Baltic)";
		charsets[13][1] = "CP1257";
		charsets[14][0] = "GB18030 (Chinese)";
		charsets[14][1] = "GB18030";
		charsets[15][0] = "TIS-620 (Thai)";
		charsets[15][1] = "TIS-620";
	}
	return charsets;
}
