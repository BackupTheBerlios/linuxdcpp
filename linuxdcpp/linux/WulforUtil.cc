/*
* Copyright (C) 2004 Jens Oknelid, paskharen@gmail.com
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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "WulforUtil.hh"
#include <client/ClientManager.h>

vector<int> WulforUtil::splitString(const string &str, const string &delimiter)
{
	int loc, len, pos = 0;
	vector<int> array;
	if (!str.empty())
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
	for (string::iterator it = str.begin(); it != str.end(); it++)
		if ((*it) == '\\')
			(*it) = '/';
	return str;
}

string WulforUtil::windowsSeparator(const string &ps)
{
	string str = ps;
	for (string::iterator it = str.begin(); it != str.end(); it++)
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

pair<string, bool> WulforUtil::getHubNames(const CID& cid)
{
	StringList hubs = ClientManager::getInstance()->getHubNames(cid);
	if (hubs.empty())
		return make_pair("Offline", FALSE);
	else
		return make_pair(Util::toString(hubs), TRUE);
}

pair<string, bool> WulforUtil::getHubNames(const User::Ptr& user)
{
	return getHubNames(user->getCID());
}
