/* 
* Copyright (C) 2004 Jens Oknelid, paskharen@spray.se
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

#include "util.hh"

using namespace Glib;

namespace WUtil
{
	ustring ConvertToUTF8 (const std::string &opsys)
	{
		ustring str = opsys;
		try
		{
			str = locale_to_utf8 (str);
		}
		catch (ConvertError)
		{
			std::string tmp = opsys;
			for (std::string::iterator it=tmp.begin();it != tmp.end(); it++)
				if ((*it & '\x80') != 0)
					(*it) = '?';
			
		str = tmp;
		}
		return str;
	}
	std::string ConvertFromUTF8 (const Glib::ustring &opsys)
	{
		ustring str = opsys;
		std::string result;
		try
		{
			result = locale_from_utf8 (str);
		}
		catch (ConvertError)
		{
			result = opsys.raw ();
			for (std::string::iterator it=result.begin();it != result.end(); it++)
				if ((*it & '\x80') != 0)
					(*it) = '?';
		}
		return result;		
	}
}
