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

#ifndef WULFOR_UTIL_H
#define WULFOR_UTIL_H

#include <gtk/gtk.h>
#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/CID.h>
#include <client/User.h>

class WulforUtil
{
	public:
		static std::vector<int> splitString(const std::string &str, const std::string &delimiter);
		static std::string linuxSeparator(const std::string &ps);
		static std::string windowsSeparator(const std::string &ps);
		static std::string getNicks(const CID& cid);
		static std::string getNicks(const User::Ptr& user);
		static std::string getHubNames(const CID& cid);
		static std::string getHubNames(const User::Ptr& user);
		static std::string getTextFromMenu(GtkMenuItem *item);
};

#endif
