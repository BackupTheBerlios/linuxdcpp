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

#if !defined(AFX_FILE_H__CB551CD7_189C_4175_922E_8B00B4C8D6F1__INCLUDED_)
#define AFX_FILE_H__CB551CD7_189C_4175_922E_8B00B4C8D6F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SettingsManager.h"

#include "Exception.h"
#include "Util.h"
#include "Text.h"

#ifndef _WIN32
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#endif

STANDARD_EXCEPTION(FileException);

#ifdef _WIN32
#include "../zlib/zlib.h"
#else

#include <zlib.h>

#ifdef HAVE_SYS_SENDFILE_H
#include <sys/sendfile.h>
#endif

#endif

/**
 * A naive output stream. We don't use the stl ones to avoid compiling STLPort,
 * besides this is a lot more lightweight (and less flexible)...
 */
class OutputStream {
public:
	virtual ~OutputStream() { }
	
	/**
	 * @return The actual number of bytes written. len bytes will always be
	 *         consumed, but fewer or more bytes may actually be written,
	 *         for example if the stream is being compressed.
	 */
	virtual size_t write(const void* buf, size_t len) throw(Exception) = 0;
	/**
	 * This must be called before destroying the object to make sure all data
	 * is properly written (we don't want destructors that throw exceptions
	 * and the last flush might actually throw). Note that some implementations
	 * might not need it...
	 */
	virtual size_t flush() throw(Exception) = 0;

	size_t write(const string& str) throw(Exception) { return write(str.c_str(), str.size()); };
};

class InputStream {
public:
	virtual ~InputStream() { }
	/**
	 * Call this function until it returns 0 to get all bytes.
	 * @return The number of bytes read. len reflects the number of bytes
	 *		   actually read from the stream source in this call.
	 */
	virtual size_t read(void* buf, size_t& len) throw(Exception) = 0;
};

class IOStream : public InputStream, public OutputStream {
};

class File : public IOStream {
public:
	enum {
		OPEN = 0x01,
		CREATE = 0x02,
		TRUNCATE = 0x04
	};

#ifdef _WIN32
	enum {
		READ = GENERIC_READ,
		WRITE = GENERIC_WRITE,
		RW = READ | WRITE
	};

	File(const string& aFileName, int access, int mode) throw(FileException) {
		dcassert(access == WRITE || access == READ || access == (READ | WRITE));

		int m = 0;
		if(mode & OPEN) {
			if(mode & CREATE) {
				m = (mode & TRUNCATE) ? CREATE_ALWAYS : OPEN_ALWAYS;
			} else {
				m = (mode & TRUNCATE) ? TRUNCATE_EXISTING : OPEN_EXISTING;
			}
		} else {
			if(mode & CREATE) {
				m = (mode & TRUNCATE) ? CREATE_ALWAYS : CREATE_NEW;
			} else {
				dcassert(0);
			}
		}

		h = ::CreateFile(Text::utf8ToWide(aFileName).c_str(), access, FILE_SHARE_READ, NULL, m, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		
		if(h == INVALID_HANDLE_VALUE) {
			throw FileException(Util::translateError(GetLastError()));
		}

	}

	u_int32_t getLastModified() {
		FILETIME f = {0};
		::GetFileTime(h, NULL, NULL, &f);
		return convertTime(&f);
	}

	static u_int32_t convertTime(FILETIME* f) {
		SYSTEMTIME s = { 1970, 1, 0, 1, 0, 0, 0, 0 };
		FILETIME f2 = {0};
		if(::SystemTimeToFileTime(&s, &f2)) {
			u_int64_t* a = (u_int64_t*)f;
			u_int64_t* b = (u_int64_t*)&f2;
			*a -= *b;
			*a /= (1000LL*1000LL*1000LL/100LL);		// 100ns > s
			return (u_int32_t)*a;
		}
		return 0;
	}

	bool isOpen() { return h != INVALID_HANDLE_VALUE; };

	virtual void close() throw() {
		if(isOpen()) {
			CloseHandle(h);
			h = INVALID_HANDLE_VALUE;
		}
	}
	
	virtual int64_t getSize() throw() {
		DWORD x;
		DWORD l = ::GetFileSize(h, &x);
		
		if( (l == INVALID_FILE_SIZE) && (GetLastError() != NO_ERROR))
			return -1;
		
		return (int64_t)l | ((int64_t)x)<<32;
	}

	virtual void setSize(int64_t newSize) throw(FileException) {
		int64_t pos = getPos();
		setPos(newSize);
		setEOF();
		setPos(pos);
	}

	virtual int64_t getPos() throw() {
		LONG x = 0;
		DWORD l = ::SetFilePointer(h, 0, &x, FILE_CURRENT);
		
		return (int64_t)l | ((int64_t)x)<<32;
	}		

	virtual void setPos(int64_t pos) throw() {
		LONG x = (LONG) (pos>>32);
		::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_BEGIN);
	}		
	virtual void setEndPos(int64_t pos) throw() {
		LONG x = (LONG) (pos>>32);
		::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_END);
	}		

	virtual void movePos(int64_t pos) throw() {
		LONG x = (LONG) (pos>>32);
		::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_CURRENT);
	}
	
	virtual size_t read(void* buf, size_t& len) throw(Exception) {
		DWORD x;
		if(!::ReadFile(h, buf, (DWORD)len, &x, NULL)) {
			throw(FileException(Util::translateError(GetLastError())));
		}
		len = x;
		return x;
	}

	virtual size_t write(const void* buf, size_t len) throw(Exception) {
		DWORD x;
		if(!::WriteFile(h, buf, (DWORD)len, &x, NULL)) {
			throw FileException(Util::translateError(GetLastError()));
		}
		dcassert(x == len);
		return x;
	}
	virtual void setEOF() throw(FileException) {
		dcassert(isOpen());
		if(!SetEndOfFile(h)) {
			throw FileException(Util::translateError(GetLastError()));
		}
	}

	virtual size_t flush() throw(Exception) {
		if(!FlushFileBuffers(h))
			throw FileException(Util::translateError(GetLastError()));
		return 0;
	}

	static void deleteFile(const string& aFileName) throw() { ::DeleteFile(Text::toT(aFileName).c_str()); }
	static void renameFile(const string& source, const string& target) throw(FileException) { 
		if(!::MoveFile(Text::toT(source).c_str(), Text::toT(target).c_str())) {
			// Can't move, try copy/delete...
			if(!CopyFile(Text::toT(source).c_str(), Text::toT(target).c_str(), FALSE)) {
				throw FileException(Util::translateError(GetLastError()));
			}
			deleteFile(source);
		}
	}
	static void copyFile(const string& src, const string& target) throw(FileException) {
		if(!::CopyFile(Text::toT(src).c_str(), Text::toT(target).c_str(), FALSE)) {
			throw FileException(Util::translateError(GetLastError()));
		}
	}

	static int64_t getSize(const string& aFileName) throw() {
		WIN32_FIND_DATA fd;
		HANDLE hFind;
		
		hFind = FindFirstFile(Text::toT(aFileName).c_str(), &fd);
		
		if (hFind == INVALID_HANDLE_VALUE) {
			return -1;
		} else {
			FindClose(hFind);
			return ((int64_t)fd.nFileSizeHigh << 32 | (int64_t)fd.nFileSizeLow);
		}
	}

	static void ensureDirectory(const string& aFile) {
		// Skip the first dir...
		tstring file;
		Text::toT(aFile, file);
		wstring::size_type start = file.find_first_of(L"\\/");
		if(start == string::npos)
			return;
		start++;
		while( (start = file.find_first_of(L"\\/", start)) != string::npos) {
			CreateDirectory(file.substr(0, start+1).c_str(), NULL);
			start++;
		}
	}
	
#else // _WIN32
	
	enum {
		READ = 0x01,
		WRITE = 0x02,
		RW = READ | WRITE,
	};
	File(const string& aFileName, int access, int mode) throw(FileException) {
		dcassert(access == WRITE || access == READ || access == (READ | WRITE));
		
		int m = 0;
		if(access == READ)
			m |= O_RDONLY;
		else if(access == WRITE)
			m |= O_WRONLY;
		else
			m |= O_RDWR;
		
		if(mode & CREATE) {
			m |= O_CREAT;
		}
		if(mode & TRUNCATE) {
			m |= O_TRUNC;
		}
		h = open(aFileName.c_str(), m, S_IRUSR | S_IWUSR);
		if(h == -1)
			throw FileException("Could not open file");
	}	

	u_int32_t getLastModified() {
		struct stat s;
		if (::fstat(h, &s) == -1)
			return 0;

		return (u_int32_t)s.st_mtime;
	}

	bool isOpen() { return h != -1; };

	virtual void close() throw(FileException) {
		if(h != -1) {
			::close(h);
			h = -1;
		}
	}

	virtual int64_t getSize() throw(FileException) {
		struct stat s;
		if(::fstat(h, &s) == -1)
			return -1;
		
		return (int64_t)s.st_size;
	}

	virtual int64_t getPos() throw(FileException) {
		return (int64_t) lseek(h, 0, SEEK_CUR);
	}

	virtual void setPos(int64_t pos) throw(FileException) { lseek(h, (off_t)pos, SEEK_SET); };
	virtual void setEndPos(int64_t pos) throw(FileException) { lseek(h, (off_t)pos, SEEK_END); };
	virtual void movePos(int64_t pos) throw(FileException) { lseek(h, (off_t)pos, SEEK_CUR); };

	virtual size_t read(void* buf, size_t& len) throw(FileException) {
		ssize_t x = ::read(h, buf, len);
		if(x == -1)
			throw("Read error");
		len = x;
		return (size_t)x;
	}
	
	virtual size_t write(const void* buf, size_t len) throw(FileException) {
		ssize_t x = ::write(h, buf, len);
		if(x == -1)
			throw FileException("Write error");
		if(x < (ssize_t)len)
			throw FileException("Disk full(?)");
		return x;
	}

	virtual void setEOF() throw(FileException) {
		if(ftruncate(h, (off_t)getPos()) == -1)
			throw FileException(Util::translateError(errno));
	}
	virtual void setSize(int64_t newSize) throw(FileException) {
		if(ftruncate(h, (off_t)newSize) == -1)
			throw FileException(Util::translateError(errno));
	}

	virtual size_t flush() throw(Exception) {
		if(fsync(h) == -1)
			throw FileException(Util::translateError(errno));
		return 0;
	}

	static void deleteFile(const string& aFileName) throw() { ::unlink(aFileName.c_str()); };
	static void renameFile(const string& source, const string& target) throw() { ::rename(source.c_str(), target.c_str()); };
	static void copyFile(const string& source, const string& target) throw() { 
		string err;
		int in, out;
		int64_t count;
		off_t offset;
		count = getSize(source);
		if (count == -1)
			throw FileException(Util::translateError(errno) + ": " + source);
		in = ::open(source.c_str(), O_RDONLY);
		if (in == -1)
			throw FileException(Util::translateError(errno) + ": " + source);
		out = ::open(target.c_str(), O_WRONLY);
		if (out == -1) {
			err = Util::translateError(errno) + ": " + target;
			::close(in);
			throw FileException(err);
		}
		offset = 0;
		while (offset < count) {
			ssize_t ret;
#ifdef HAVE_SYS_SENDFILE_H
			// sendfile copies files in kernel space
			ret = ::sendfile(in, out, &offset, count - offset);
			if (ret == -1)
				// FIXME if errno == EINVAL or ENOSYS then maybe fall back to read/write?
				break;
#else
			uint8_t buf[0x10000];	// FIXME: don't know what's a good buffer size
			ret = ::read(in, buf, count > sizeof(buf) ? sizeof(buf) : count);
			if (ret == -1)
				break;
			ret = ::write(out, buf, ret);
			if (ret == -1)
				break;
			offset += ret;
#endif
		}

		// aborted due to error?
		if (offset < count) {
			err = Util::translateError(errno);
			::close(out);
			::close(in);
			deleteFile(target.c_str());
			throw FileException(err);
		}
		
	}

	static int64_t getSize(const string& aFileName) {
		struct stat s;
		if(stat(aFileName.c_str(), &s) == -1)
			return -1;
		
		return s.st_size;
	}

	static void ensureDirectory(const string& aFile) {
		string acp = Text::utf8ToAcp(aFile);
		string::size_type start = 0;
		while( (start = aFile.find_first_of(L'/', start)) != string::npos) {
			mkdir(aFile.substr(0, start+1).c_str(), 0755);
			start++;
		}
	}

	
#endif // _WIN32

	virtual ~File() throw(FileException) {
		File::close();
	}

	string read(size_t len) throw(FileException) {
		string s(len, 0);
		size_t x = read(&s[0], len);
		if(x != len)
			s.resize(x);
		return s;
	}

	string read() throw(FileException) {
		setPos(0);
		int64_t sz = getSize();
		if(sz == -1)
			return Util::emptyString;
		return read((u_int32_t)sz);
	}

	void write(const string& aString) throw(FileException) { write((void*)aString.data(), aString.size()); };

protected:
#ifdef _WIN32
	HANDLE h;
#else
	int h;
#endif
private:
	File(const File&);
	File& operator=(const File&);
};

template<bool managed>
class LimitedInputStream : public InputStream {
public:
	LimitedInputStream(InputStream* is, int64_t aMaxBytes) : s(is), maxBytes(aMaxBytes) {
	}
	virtual ~LimitedInputStream() { if(managed) delete s; }

	size_t read(void* buf, size_t& len) throw(FileException) {
		dcassert(maxBytes >= 0);
		len = (size_t)min(maxBytes, (int64_t)len);
		if(len == 0)
			return 0;
		size_t x = s->read(buf, len);
		maxBytes -= x;
		return x;
	}

private:
	InputStream* s;
	int64_t maxBytes;
};

template<bool managed>
class BufferedOutputStream : public OutputStream {
public:
	using OutputStream::write;

	BufferedOutputStream(OutputStream* aStream, size_t aBufSize = SETTING(BUFFER_SIZE) * 1024) : s(aStream), pos(0), bufSize(aBufSize), buf(new u_int8_t[bufSize]) { }
	virtual ~BufferedOutputStream() { 
		try {
			// We must do this in order not to lose bytes when a download
			// is disconnected prematurely
			flush();
		} catch(const Exception&) {
		}
		if(managed) delete s; delete buf; 
	}

	virtual size_t flush() throw(Exception) {
		if(pos > 0)
			s->write(buf, pos);
		pos = 0;
		s->flush();
		return 0;
	}

	virtual size_t write(const void* wbuf, size_t len) throw(Exception) {
		u_int8_t* b = (u_int8_t*)wbuf;
		size_t l2 = len;
		while(len > 0) {
			if(pos == 0 && len >= bufSize) {
				s->write(b, len);
				break;
			} else {
				size_t n = min(bufSize - pos, len);
				memcpy(buf + pos, b, n);
				b += n;
				pos += n;
				len -= n;
				if(pos == bufSize) {
					s->write(buf, bufSize);
					pos = 0;
				}
			}
		}
		return l2;
	}
private:
	OutputStream* s;
	size_t pos;
	size_t bufSize;
	u_int8_t* buf;
};

class StringOutputStream : public OutputStream {
public:
	virtual size_t flush() throw(Exception) { return 0; }
	virtual size_t write(const void* buf, size_t len) throw(Exception) {
		str.append((char*)buf, len);
		return len;
	}
	const string& getString() { return str; }
private:
	string str;
};
#endif // !defined(AFX_FILE_H__CB551CD7_189C_4175_922E_8B00B4C8D6F1__INCLUDED_)

/**
 * @file
 * $Id: File.h,v 1.2 2004/11/12 16:29:31 phase Exp $
 */

