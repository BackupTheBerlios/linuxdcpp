dnl 
dnl  SigC++ Cross-Thread Signal Templates
dnl 
dnl  Copyright (C) 2002 Andreas Rottmann <rottmann@users.sourceforge.net>
dnl 
dnl  This library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Library General Public
dnl  License as published by the Free Software Foundation; either
dnl  version 2 of the License, or (at your option) any later version.
dnl 
dnl  This library is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Library General Public License for more details.
dnl 
dnl  You should have received a copy of the GNU Library General Public
dnl  License along with this library; if not, write to the Free
dnl  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl 
/* This is for emacs: -*-Mode: C++;-*- */
/* Copyright 2002, Andreas Rottmann */
include(template.macros.m4)
minclude(bind.h.m4)
#ifndef __header__
#define __header__

#include <sigc++/bind.h>

namespace SigCX
{

using namespace SigC;

ADAPTOR_BIND_DATA(ARGS(C,3))

ADAPTOR_BIND_SLOT(ARGS(P,0),ARGS(C,3),ARGS(A,3))
ADAPTOR_BIND_SLOT(ARGS(P,1),ARGS(C,3),ARGS(A,3))
ADAPTOR_BIND_SLOT(ARGS(P,2),ARGS(C,3),ARGS(A,3))
ADAPTOR_BIND_SLOT(ARGS(P,3),ARGS(C,3),ARGS(A,3))
ADAPTOR_BIND_SLOT(ARGS(P,4),ARGS(C,3),ARGS(A,3))
ADAPTOR_BIND_SLOT(ARGS(P,5),ARGS(C,3),ARGS(A,3))
ADAPTOR_BIND_SLOT(ARGS(P,6),ARGS(C,3),ARGS(A,3))
ADAPTOR_BIND_SLOT(ARGS(P,7),ARGS(C,3),ARGS(A,3))

} // namespace SigCX
 
#endif
