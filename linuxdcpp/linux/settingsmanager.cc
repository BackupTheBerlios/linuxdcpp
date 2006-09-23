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

#include "settingsmanager.hh"

#include <client/stdinc.h>
#include <client/DCPlusPlus.h>
#include <client/File.h>
#include <client/SimpleXML.h>
#include <client/Util.h>

using namespace std;

WulforSettingsManager *WulforSettingsManager::ptr = NULL;

WulforSettingsManager::WulforSettingsManager()
{
	defaultInt["main-window-maximized"] = 0;
	defaultInt["main-window-size-x"] = 800;
	defaultInt["main-window-size-y"] = 600;
	defaultInt["main-window-pos-x"] = 100;
	defaultInt["main-window-pos-y"] = 100;
	defaultInt["transfer-pane-position"] = 300;
	defaultInt["nick-pane-position"] = 500;
	defaultInt["downloadqueue-pane-position"] = 200;
	defaultInt["sharebrowser-pane-position"] = 200;
	defaultInt["show-tray-icon"] = 1;
	defaultInt["use-stock-icons"] = 0;
	defaultString["downloadqueue-order"] = "";
	defaultString["downloadqueue-width"] = "";
	defaultString["downloadqueue-visibility"] = "";
	defaultString["favoritehubs-order"] = "";
	defaultString["favoritehubs-width"] = "";
	defaultString["favoritehubs-visibility"] = "";
	defaultString["finished-order"] = "";
	defaultString["finished-width"] = "";
	defaultString["finished-visibility"] = "";
	defaultString["hub-order"] = "";
	defaultString["hub-width"] = "";
	defaultString["hub-visibility"] = "";
	defaultString["main-order"] = "";
	defaultString["main-width"] = "";
	defaultString["main-visibility"] = "";
	defaultString["publichubs-order"] = "";
	defaultString["publichubs-width"] = "";
	defaultString["publichubs-visibility"] = "";
	defaultString["search-order"] = "";
	defaultString["search-width"] = "";
	defaultString["search-visibility"] = "";
	defaultString["sharebrowser-order"] = "";
	defaultString["sharebrowser-width"] = "";
	defaultString["sharebrowser-visibility"] = "";
	defaultString["default-charset"] = "CP1252";
}

WulforSettingsManager *WulforSettingsManager::get()
{
	if (ptr == NULL)
		ptr = new WulforSettingsManager;
	return ptr;
}

int WulforSettingsManager::getInt(std::string key)
{
	dcassert(intMap.find(key) != intMap.end() || defaultInt.find(key) != defaultInt.end());

	if (intMap.find(key) == intMap.end())
		return defaultInt[key];
	else
		return intMap[key];
}

string WulforSettingsManager::getString(std::string key)
{
	dcassert(stringMap.find(key) != stringMap.end() || defaultString.find(key) != defaultString.end());

	if (stringMap.find(key) == stringMap.end())
		return defaultString[key];
	else
		return stringMap[key];
}

void WulforSettingsManager::set(std::string key, int value)
{
	dcassert(defaultInt.find(key) != defaultInt.end());
	intMap[key] = value;
}

void WulforSettingsManager::set(std::string key, string value)
{
	dcassert(defaultString.find(key) != defaultString.end());
	stringMap[key] = value;
}

void WulforSettingsManager::load()
{
	string file = Util::getConfigPath() + "LinuxDC++.xml";

	try
	{
		SimpleXML xml;
		xml.fromXML(File(file, File::READ, File::OPEN).read());
		xml.resetCurrentChild();
		xml.stepIn();

		if (xml.findChild("Settings"))
		{
			xml.stepIn();

			map<string, int>::iterator iit;
			for (iit = defaultInt.begin(); iit != defaultInt.end(); iit++)
			{
				if (xml.findChild(iit->first))
					intMap[iit->first] = Util::toInt(xml.getChildData());
				xml.resetCurrentChild();
			}

			map<string, string>::iterator sit;
			for (sit = defaultString.begin(); sit != defaultString.end(); sit++)
			{
				if(xml.findChild(sit->first))
					stringMap[sit->first] = xml.getChildData();
				xml.resetCurrentChild();
			}

			xml.stepOut();
		}
	}
	catch (const Exception&)
	{
	}
}

void WulforSettingsManager::save()
{
	string file = Util::getConfigPath() + "LinuxDC++.xml";

	SimpleXML xml;
	xml.addTag("LinuxDC++");
	xml.stepIn();
	xml.addTag("Settings");
	xml.stepIn();

	map<std::string, int>::iterator iit;
	for (iit = intMap.begin(); iit != intMap.end(); ++iit)
	{
		xml.addTag(iit->first, iit->second);
		xml.addChildAttrib(string("type"), string("int"));
	}

	map<std::string, std::string>::iterator sit;
	for (sit = stringMap.begin(); sit != stringMap.end(); ++sit)
	{
		xml.addTag(sit->first, sit->second);
		xml.addChildAttrib(string("type"), string("string"));
	}

	xml.stepOut();

	try
	{
		File out(file + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		BufferedOutputStream<false> f(&out);
		f.write(SimpleXML::utf8Header);
		xml.toXML(&f);
		f.flush();
		out.close();
		File::deleteFile(file);
		File::renameFile(file + ".tmp", file);
	}
	catch (const FileException &)
	{
	}
}
