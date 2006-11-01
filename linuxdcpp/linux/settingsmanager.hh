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

#ifndef WULFOR_SETTINGSMANAGER_HH
#define WULFOR_SETTINGSMANAGER_HH

#include <string>
#include <map>

#define WSET(key, value) WulforSettingsManager::get()->set(key, value)
#define WGETI(key) WulforSettingsManager::get()->getInt(key)
#define WGETS(key) WulforSettingsManager::get()->getString(key)

class WulforSettingsManager
{
	public:
		static WulforSettingsManager *get();

		int getInt(const std::string &key);
		std::string getString(const std::string &key);
		void set(const std::string &key, int value);
		void set(const std::string &key, const std::string &value);

		void load();
		void save();

	private:
		WulforSettingsManager();
		static WulforSettingsManager *ptr;

		std::map<std::string, int> intMap;
		std::map<std::string, std::string> stringMap;
		std::map<std::string, int> defaultInt;
		std::map<std::string, std::string> defaultString;
};

#else
class WulforSettingsManager;
#endif
