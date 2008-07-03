/*
 * Copyright © 2004-2008 Jens Oknelid, paskharen@gmail.com
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
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include "WulforUtil.hh"
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <client/ClientManager.h>
#include <client/Util.h>
#include <iostream>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <fcntl.h>

using namespace std;

std::vector<std::string> WulforUtil::charsets;
const std::string WulforUtil::magnetSignature = "magnet:?xt=urn:tree:tiger:";

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

vector<string> WulforUtil::getLocalIPs()
{
	vector<string> addresses;
	struct ifaddrs *ifap;

	if (getifaddrs(&ifap) == 0)
	{
		struct ifaddrs *interface = ifap;

		while (interface)
		{
			struct sockaddr *ifa_addr = interface->ifa_addr;
			if (ifa_addr && ifa_addr->sa_family == AF_INET)
			{
				struct sockaddr_in *addr_in = (struct sockaddr_in *)ifa_addr;
				string address = inet_ntoa(addr_in->sin_addr);

				if (address != "127.0.0.1")
					addresses.push_back(address);
			}
			interface = interface->ifa_next;
		}
		freeifaddrs(ifap);
	}

	return addresses;
}

string WulforUtil::getNicks(const string &cid)
{
	return getNicks(CID(cid));
}

string WulforUtil::getNicks(const CID& cid)
{
	return Util::toString(ClientManager::getInstance()->getNicks(cid));
}

string WulforUtil::getNicks(const User::Ptr& user)
{
	return getNicks(user->getCID());
}

string WulforUtil::getHubNames(const string &cid)
{
	return getHubNames(CID(cid));
}

string WulforUtil::getHubNames(const CID& cid)
{
	StringList hubs = ClientManager::getInstance()->getHubNames(cid);
	if (hubs.empty())
		return _("Offline");
	else
		return Util::toString(hubs);
}

string WulforUtil::getHubNames(const User::Ptr& user)
{
	return getHubNames(user->getCID());
}

StringList WulforUtil::getHubAddress(const CID& cid)
{
	return ClientManager::getInstance()->getHubs(cid);
}

StringList WulforUtil::getHubAddress(const User::Ptr& user)
{
	return getHubAddress(user->getCID());
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
	GError* error = NULL;
	gchar *argv[3];
#if defined(__APPLE__)
	argv[0] = (gchar *)"open";
#elif defined(_WIN32)
	argv[0] = (gchar *)"start";
#else
	argv[0] = (gchar *)"xdg-open";
#endif
	argv[1] = (gchar *)Text::fromUtf8(uri).c_str();
	argv[2] = NULL;

	g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);

	if (error != NULL)
	{
		cerr << "Failed to open URI: " << error->message << endl;
		g_error_free(error);
	}
}

string WulforUtil::makeMagnet(const string &name, const int64_t size, const string &tth)
{
	if (name.empty() || tth.empty())
		return string();

	// other clients can return paths with different separators, so we should catch both cases
	string::size_type i = name.find_last_of("/\\");
	string path = (i != string::npos) ? name.substr(i + 1) : name;

	return magnetSignature + tth + "&xl=" + Util::toString(size) + "&dn=" + Util::encodeURI(path);
}

bool WulforUtil::splitMagnet(const string &magnet, string &name, int64_t &size, string &tth)
{
	name = _("Unknown");
	size = 0;
	tth = _("Unknown");

	if (!isMagnet(magnet.c_str()) || magnet.size() <= magnetSignature.length())
		return FALSE;

	string::size_type nextpos = 0;

	for (string::size_type pos = magnetSignature.length(); pos < magnet.size(); pos = nextpos + 1)
	{
		nextpos = magnet.find('&', pos);
		if (nextpos == string::npos)
			nextpos = magnet.size();

		if (pos == magnetSignature.length())
			tth = magnet.substr(magnetSignature.length(), nextpos - magnetSignature.length());
		else if (magnet.compare(pos, 3, "xl=") == 0)
			size = Util::toInt64(magnet.substr(pos + 3, nextpos - pos - 3));
		else if (magnet.compare(pos, 3, "dn=") == 0)
			name = Util::encodeURI(magnet.substr(pos + 3, nextpos - pos - 3), TRUE);
	}

	return TRUE;
}

bool WulforUtil::isMagnet(const string &text)
{
	return g_ascii_strncasecmp(text.c_str(), magnetSignature.c_str(), magnetSignature.length()) == 0;
}

bool WulforUtil::isLink(const string &text)
{
	return g_ascii_strncasecmp(text.c_str(), "http://", 7) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "https://", 8) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "www.", 4) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "ftp://", 6) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "sftp://", 7) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "irc://", 6) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "ircs://", 7) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "im:", 3) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "mailto:", 7) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "news:", 5) == 0;
}

bool WulforUtil::isHubURL(const string &text)
{
	return g_ascii_strncasecmp(text.c_str(), "dchub://", 8) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "adc://", 6) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "adcs://", 7) == 0;
}

bool WulforUtil::profileIsLocked()
{
	static bool profileIsLocked = false;

	if (profileIsLocked)
		return TRUE;

	// We can't use Util::getConfigPath() since the core has not been started yet.
	// Also, Util::getConfigPath() is utf8 and we need system encoding for g_open().
	char *home = getenv("HOME");
	string configPath = home ? string(home) + "/.dc++/" : "/tmp/";
	string profileLockingFile = configPath + "profile.lck";
	int flags = O_WRONLY | O_CREAT;
	int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	int fd = g_open(profileLockingFile.c_str(), flags, mode);
	if (fd != -1) // read error
	{
		struct flock lock;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_SET;
		struct flock testlock = lock;

		if (fcntl(fd, F_GETLK, &testlock) != -1) // Locking not supported
		{
			if (fcntl(fd, F_SETLK, &lock) == -1)
				profileIsLocked = true;
		}
	}

	return profileIsLocked;
}

