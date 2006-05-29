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

#ifndef WULFOR_UTIL_H
#define WULFOR_UTIL_H

#include <string>
#include <vector>
#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/Util.h>

#include <gtk/gtk.h>

class WulforUtil
{
	public:
		static std::vector<int> splitString(const std::string &str, const std::string &delimiter);
		static std::string linuxSeparator(const std::string &ps);
		static std::string windowsSeparator(const std::string &ps);
		struct HashString
		{
			size_t operator() (const std::string& x) const
			{
				return __gnu_cxx::hash<const char *> () (x.c_str());
			}
		};
};

#endif
