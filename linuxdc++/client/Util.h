/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

#if !defined(AFX_UTIL_H__1758F242_8D16_4C50_B40D_E59B3DD63913__INCLUDED_)
#define AFX_UTIL_H__1758F242_8D16_4C50_B40D_E59B3DD63913__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#endif

#include "Text.h"

/** Evaluates op(pair<T1, T2>.first, compareTo) */
template<class T1, class T2, class op = equal_to<T1> >
class CompareFirst {
public:
	CompareFirst(const T1& compareTo) : a(compareTo) { };
	bool operator()(const pair<T1, T2>& p) { return op()(p.first, a); };
private:
	const T1& a;
};

/** Evaluates op(pair<T1, T2>.second, compareTo) */
template<class T1, class T2, class op = equal_to<T2> >
class CompareSecond {
public:
	CompareSecond(const T2& compareTo) : a(compareTo) { };
	bool operator()(const pair<T1, T2>& p) { return op()(p.second, a); };
private:
	const T2& a;
};

template<class T>
struct PointerHash {
#if _MSC_VER < 1300 
	enum {bucket_size = 4}; 
	enum {min_buckets = 8}; 
#else 
	static const size_t bucket_size = 4; 
	static const size_t min_buckets = 8; 
#endif // _MSC_VER == 1200
	size_t operator()(const T* a) const { return ((size_t)a)/sizeof(T); };
	bool operator()(const T* a, const T* b) { return a < b; };
};
template<>
struct PointerHash<void> {
	size_t operator()(const void* a) const { return ((size_t)a)>>2; };
};

/** 
 * Compares two values
 * @return -1 if v1 < v2, 0 if v1 == v2 and 1 if v1 > v2
 */
template<typename T1>
inline int compare(const T1& v1, const T1& v2) { return (v1 < v2) ? -1 : ((v1 == v2) ? 0 : 1); }

class Flags {
	public:
		typedef int MaskType;

		Flags() : flags(0) { };
		Flags(const Flags& rhs) : flags(rhs.flags) { };
		Flags(MaskType f) : flags(f) { };
		bool isSet(MaskType aFlag) const { return (flags & aFlag) == aFlag; };
		bool isAnySet(MaskType aFlag) const { return (flags & aFlag) != 0; };
		void setFlag(MaskType aFlag) { flags |= aFlag; };
		void unsetFlag(MaskType aFlag) { flags &= ~aFlag; };
		Flags& operator=(const Flags& rhs) { flags = rhs.flags; return *this; };
	private:
		MaskType flags;
};

template<typename T>
class AutoArray {
	typedef T* TPtr;
	typedef T& TRef;
public:
	explicit AutoArray(TPtr t) : p(t) { };
	explicit AutoArray(size_t size) : p(new T[size]) { };
	~AutoArray() { delete[] p; };
	operator TPtr() { return p; };
	AutoArray& operator=(TPtr t) { delete[] p; p = t; return *this; };
private:
	AutoArray(const AutoArray&);
	AutoArray& operator=(const AutoArray&);

	TPtr p;
};

class Util  
{
public:
	static tstring emptyStringT;
	static string emptyString;
	static wstring emptyStringW;

	static void initialize();

	static string getAppPath() { return appPath; }
	static string getAppName() {
#ifdef _WIN32
		TCHAR buf[MAX_PATH+1];
		DWORD x = GetModuleFileName(NULL, buf, MAX_PATH);
		return Text::wideToUtf8(wstring(buf, x));
#else // _WIN32
		char buf[PATH_MAX + 1];
		char* path = getenv("_");
		if (!path) {
			if (readlink("/proc/self/exe", buf, sizeof (buf)) == -1) {
				return emptyString;
			}
			path = buf;
		}
		return string(path);
#endif // _WIN32
	}	

	static string getTempPath() {
#ifdef _WIN32
		TCHAR buf[MAX_PATH + 1];
		DWORD x = GetTempPath(MAX_PATH, buf);
		return Text::wideToUtf8(wstring(buf, x));
#else
		return "/tmp/";
#endif
	}

	static string getDataPath() {
#ifdef _WIN32
		return getAppPath();
#else
		char* home = getenv("HOME");
		if (home) {
			return string(home) + "/.dc++/";
		}
		return emptyString;
#endif
	}

	static string translateError(int aError) {
#ifdef _WIN32
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			aError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);
		string tmp = Text::wideToUtf8((LPCTSTR)lpMsgBuf);
		// Free the buffer.
		LocalFree( lpMsgBuf );
		string::size_type i;

		while( (i = tmp.find_last_of("\r\n")) != string::npos) {
			tmp.erase(i, 1);
		}
		return tmp;
#else // _WIN32
		return strerror(aError);
#endif // _WIN32
	}

	static string getFilePath(const string& path) {
		string::size_type i = path.rfind(PATH_SEPARATOR);
		return (i != string::npos) ? path.substr(0, i + 1) : path;
	}
	static string getFileName(const string& path) {
		string::size_type i = path.rfind(PATH_SEPARATOR);
		return (i != string::npos) ? path.substr(i + 1) : path;
	}
	static string getFileExt(const string& path) {
		string::size_type i = path.rfind('.');
		return (i != string::npos) ? path.substr(i) : Util::emptyString;
	}
	static string getLastDir(const string& path) {
		string::size_type i = path.rfind(PATH_SEPARATOR);
		if(i == string::npos)
			return Util::emptyString;
		string::size_type j = path.rfind(PATH_SEPARATOR, i-1);
		return (j != string::npos) ? path.substr(j+1, i-j-1) : path;
	}

	static wstring getFilePath(const wstring& path) {
		wstring::size_type i = path.rfind(PATH_SEPARATOR);
		return (i != wstring::npos) ? path.substr(0, i + 1) : path;
	}
	static wstring getFileName(const wstring& path) {
		wstring::size_type i = path.rfind(PATH_SEPARATOR);
		return (i != wstring::npos) ? path.substr(i + 1) : path;
	}
	static wstring getFileExt(const wstring& path) {
		wstring::size_type i = path.rfind('.');
		return (i != wstring::npos) ? path.substr(i) : Util::emptyStringW;
	}
	static wstring getLastDir(const wstring& path) {
		wstring::size_type i = path.rfind(PATH_SEPARATOR);
		if(i == wstring::npos)
			return Util::emptyStringW;
		wstring::size_type j = path.rfind(PATH_SEPARATOR, i-1);
		return (j != wstring::npos) ? path.substr(j+1, i-j-1) : path;
	}

	static void decodeUrl(const string& aUrl, string& aServer, short& aPort, string& aFile);
	static string validateFileName(string aFile);
	
	static string formatBytes(const string& aString) {
		return formatBytes(toInt64(aString));
	}

	static string toDOS(const string& tmp);

	static string getShortTimeString();

	static string getTimeString() {
		char buf[64];
		time_t _tt;
		time(&_tt);
		tm* _tm = localtime(&_tt);
		if(_tm == NULL) {
			strcpy(buf, "xx:xx:xx");
		} else {
			strftime(buf, 64, "%X", _tm);
		}
		return buf;
	}

	static string toAdcFile(const string& file) {
		string ret;
		ret.reserve(file.length() + 1);
		ret += '/';
		ret += file;
		for(string::size_type i = 0; i < ret.length(); ++i) {
			if(ret[i] == '\\') {
				ret[i] = '/';
			}
		}
		return ret;
	}
	static string toNmdcFile(const string& file) {
		if(file.empty())
			return Util::emptyString;
		
		string ret(file.substr(1));
		for(string::size_type i = 0; i < ret.length(); ++i) {
			if(ret[i] == '/') {
				ret[i] = '\\';
			}
		}
		return ret;
	}
	
	static string formatBytes(int64_t aBytes);

	static string formatExactSize(int64_t aBytes);

	static string formatSeconds(int64_t aSec) {
		char buf[64];
#ifdef _WIN32
		sprintf(buf, "%01I64d:%02d:%02d", aSec / (60*60), (int)((aSec / 60) % 60), (int)(aSec % 60));
#else
		sprintf(buf, "%01lld:%02d:%02d", aSec / (60*60), (int)((aSec / 60) % 60), (int)(aSec % 60));
#endif		
		return buf;
	}

	static string formatParams(const string& msg, StringMap& params);
	static string formatTime(const string &msg, const time_t t);

	static int64_t toInt64(const string& aString) {
#ifdef _WIN32
		return _atoi64(aString.c_str());
#else
		return atoll(aString.c_str());
#endif
	}

	static int toInt(const string& aString) {
		return atoi(aString.c_str());
	}
	static u_int32_t toUInt32(const char* c) {
		return (u_int32_t)atoi(c);
	}

	static double toDouble(const string& aString) {
		// Work-around for atof and locales...
		string::size_type i = aString.rfind(',');
		if(i != string::npos) {
			string tmp(aString);
			tmp[i] = '.';
			return atof(tmp.c_str());
		}
		return atof(aString.c_str());
	}

	static float toFloat(const string& aString) {
		return (float)toDouble(aString.c_str());
	}

	static string toString(int64_t val) {
		char buf[32];
#ifdef _WIN32
		return _i64toa(val, buf, 10);
#else
		sprintf(buf, "%lld", val);
		return buf;
#endif
	}

	static string toString(u_int32_t val) {
		char buf[16];
		sprintf(buf, "%lu", (unsigned long)val);
		return buf;
	}
	/*
	static string toString(size_t val) {
		// TODO A better conversion the day we hit 64 bits
		return toString((u_int32_t)val);
	}
	*/
	static string toString(int32_t val) {
		char buf[16];
		sprintf(buf, "%d", val);
		return buf;
	}
	static string toString(int16_t val) {
		return toString((int32_t) val);
	}
	/*
	static string toString(u_int16_t val) {
		return toString((u_int32_t)val);
	}
	*/
	/*
	static string toString(int val) {
		char buf[16];
		sprintf(buf, "%d", val);
		return buf;
	}
	*/
	static string toString(double val) {
		char buf[16];
		sprintf(buf, "%0.2f", val);
		return buf;
	}
	static string toHexEscape(char val) {
		char buf[sizeof(int)*2+1+1];
		sprintf(buf, "%%%X", val&0x0FF);
		return buf;
	}
	static char fromHexEscape(const string aString) {
		int res = 0;
		sscanf(aString.c_str(), "%X", &res);
		return static_cast<char>(res);
	}

	static string encodeURI(const string& /*aString*/, bool reverse = false);
	static string getLocalIp();
	static bool isPrivateIp(string const& ip);
	/**
	 * Case insensitive substring search.
	 * @return First position found or string::npos
	 */
	static string::size_type findSubString(const string& aString, const string& aSubString, string::size_type start = 0) throw();

	/* Utf-8 versions of strnicmp and stricmp, unicode char code order (!) */
	static int stricmp(const char* a, const char* b);
	static int stricmp(const wchar_t* a, const wchar_t* b) {
#ifdef _WIN32
		return ::_wcsicmp(a, b);
#else
		return wcscasecmp(a, b);
#endif
		// return ::stricmp(a, b);
		
	}
	static int strnicmp(const char* a, const char* b, size_t n);
	static int strnicmp(const wchar_t* a, const wchar_t* b, size_t n) {
#ifdef _WIN32
		return ::_wcsnicmp(a, b, n);
#else
		return ::wcsncasecmp(a, b, n);
#endif
	}

	//static int stricmp(const string& a, const string& b) { return stricmp(a.c_str(), b.c_str()); };
	//static int strnicmp(const string& a, const string& b, int n) { return strnicmp(a.c_str(), b.c_str(), n); };
	static int stricmp(const string& a, const string& b) { return stricmp(Text::utf8ToWide(a), Text::utf8ToWide(b)); };
	static int strnicmp(const string& a, const string& b, size_t n) { return strnicmp(Text::utf8ToWide(a), Text::utf8ToWide(b), n); };
	static int stricmp(const wstring& a, const wstring& b) { return stricmp(a.c_str(), b.c_str()); };
	static int strnicmp(const wstring& a, const wstring& b, size_t n) { return strnicmp(a.c_str(), b.c_str(), n); };
	
	static string validateMessage(string tmp, bool reverse, bool checkNewLines = true);

	static string getOsVersion();

	static string getIpCountry (string IP);

	static int getOsMinor();
	static int getOsMajor(); 

	static bool getAway() { return away; };
	static void setAway(bool aAway) {
		away = aAway;
		if (away)
			awayTime = time(NULL);
	};
	static string getAwayMessage();

	static void setAwayMessage(const string& aMsg) { awayMsg = aMsg; };

	static u_int32_t rand();
	static u_int32_t rand(u_int32_t high) { return rand() % high; };
	static u_int32_t rand(u_int32_t low, u_int32_t high) { return rand(high-low) + low; };
	static double randd() { return ((double)rand()) / ((double)0xffffffff); };

private:
	static string appPath;
	static string dataPath;
	static bool away;
	static string awayMsg;
	static time_t awayTime;

	typedef map<u_int32_t, u_int16_t> CountryList;
	typedef CountryList::iterator CountryIter;

	static CountryList countries;

};

/** Case insensitive hash function for strings */
struct noCaseStringHash {
#if _MSC_VER < 1300 
	enum {bucket_size = 4}; 
	enum {min_buckets = 8}; 
#else 
	static const size_t bucket_size = 4; 
	static const size_t min_buckets = 8; 
#endif // _MSC_VER == 1200

	size_t operator()(const string* s) const {
		return operator()(*s);
	}

	size_t operator()(const string& s) const {
		size_t x = 0;
		const char* end = s.data() + s.size();
		for(const char* str = s.data(); str < end; ) {
			wchar_t c = 0;
			int n = Text::utf8ToWc(str, c);
			if(n == -1) {
				str++;
			} else {
				x = x*31 + (size_t)Text::toLower(c);
				str += n;
			}
		}
		return x;
	}

	size_t operator()(const wstring* s) const {
		return operator()(*s);
	}
	size_t operator()(const wstring& s) const {
		size_t x = 0;
		const wchar_t* y = s.data();
		wstring::size_type j = s.size();
		for(wstring::size_type i = 0; i < j; ++i) {
			x = x*31 + (size_t)Text::toLower(y[i]);
		}
		return x;
	}
};

/** Case insensitive string comparison */
struct noCaseStringEq {
	bool operator()(const string* a, const string* b) const {
		return a == b || Util::stricmp(*a, *b) == 0;
	}
	bool operator()(const string& a, const string& b) const {
		return Util::stricmp(a, b) == 0;
	}
	bool operator()(const wstring* a, const wstring* b) const {
		return a == b || Util::stricmp(*a, *b) == 0;
	}
	bool operator()(const wstring& a, const wstring& b) const {
		return Util::stricmp(a, b) == 0;
	}
};

/** Case insensitive string ordering */
struct noCaseStringLess {
	bool operator()(const string* a, const string* b) const {
		return Util::stricmp(*a, *b) < 0;
	}
	bool operator()(const string& a, const string& b) const {
		return Util::stricmp(a, b) < 0;
	}
	bool operator()(const wstring* a, const wstring* b) const {
		return Util::stricmp(*a, *b) < 0;
	}
	bool operator()(const wstring& a, const wstring& b) const {
		return Util::stricmp(a, b) < 0;
	}
};


#endif // !defined(AFX_UTIL_H__1758F242_8D16_4C50_B40D_E59B3DD63913__INCLUDED_)

/**
 * @file
 * $Id: Util.h,v 1.2 2004/11/12 16:29:31 phase Exp $
 */
