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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "SFVReader.h"

#include "StringTokenizer.h"

bool SFVReader::tryFile(const string& sfvFile, const string& fileName) throw(FileException) {
	
	string sfv = File(sfvFile, File::READ, File::OPEN).read();
	
	string::size_type i = 0;
	while( (i = Util::findSubString(sfv, fileName, i)) != string::npos) {
		// Either we're at the beginning of the file or the line...otherwise skip...
		if( (i == 0) || (sfv[i-1] == '\n') ) {
			string::size_type j = i + fileName.length() + 1;
			if(j < sfv.length() - 8) {
				sscanf(sfv.c_str() + j, "%x", &crc32);
				crcFound = true;
				return true;
			}
		}
		i += fileName.length();
	}
	
	return false;
}

void SFVReader::load(const string& fileName) throw() {
#ifdef _WIN32
	string path = Util::getFilePath(fileName);
	string fname = Util::getFileName(fileName);

	WIN32_FIND_DATA fd;
	HANDLE hf = FindFirstFile(Text::toT(path + "*.sfv").c_str(), &fd);
	if(hf == INVALID_HANDLE_VALUE) {
		return;
	}

	do {
		try {
			if(tryFile(path + Text::fromT(fd.cFileName), fname)) {
				FindClose(hf);
				return;
			}
		} catch(const FileException&) {
			// Ignore...
		}
	} while(FindNextFile(hf, &fd));

	FindClose(hf);
	
#endif
}

/**
 * @file
 * $Id: SFVReader.cpp,v 1.2 2004/11/12 16:29:31 phase Exp $
 */
