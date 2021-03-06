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

#if !defined(AFX_BITINPUTSTREAM_H__EAF695A9_6D5C_4791_88A2_3FA0D47697AF__INCLUDED_)
#define AFX_BITINPUTSTREAM_H__EAF695A9_6D5C_4791_88A2_3FA0D47697AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Exception.h"

STANDARD_EXCEPTION(BitStreamException);

/**
 * A clumsy bit streamer, assumes that there's enough data to complete the operations.
 * No, doesn't operate on streams...=)
 */
class BitInputStream  
{
public:
	BitInputStream(const u_int8_t* aStream, size_t aStart, size_t aEnd) : endPos(aEnd*8), bitPos(aStart*8), is(aStream) { };
	~BitInputStream() { };
	
	bool get() throw(BitStreamException) {
		if(bitPos > endPos) {
			throw BitStreamException(STRING(SEEK_BEYOND_END));
		}
		bool ret = (((u_int8_t)is[bitPos>>3]) >> (bitPos&0x07)) & 0x01;
		bitPos++;
		return ret;
	}
	
	void skipToByte() {
		if(bitPos%8 != 0)
			bitPos = (bitPos & (~7)) + 8;
	}
	
	void skip(int n) {
		bitPos += n * 8;
		return ;
	}
private:
	size_t bitPos;
	size_t endPos;
	const u_int8_t* is;
};

#endif // !defined(AFX_BITINPUTSTREAM_H__EAF695A9_6D5C_4791_88A2_3FA0D47697AF__INCLUDED_)

/**
 * @file
 * $Id: BitInputStream.h,v 1.2 2004/11/12 16:29:30 phase Exp $
 */
