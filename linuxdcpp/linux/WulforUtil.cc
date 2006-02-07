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
			pos = loc + 1;
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
