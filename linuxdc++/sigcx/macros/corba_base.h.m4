dnl 
dnl  SigC++ CORBA Signal Templates
dnl 
dnl  Copyright (C) 2000 Andreas Rottmann <rottmann@users.sourceforge.net>
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
// -*- c++ -*-
dnl Ignore the next line
/* This is a generated file, do not edit.  Generated from __file__ */
include(template.macros.m4)
#ifndef __header__
#define __header__

#include "sigc++config.h"

#if defined(SIGC_ORB_MICO)
#  include "sigc++/corba/mico/impl.h"
#elif defined(SIGC_ORB_ILU) 
#  include "sigc++/corba/ilu/impl.h"
#else
#  error "No orb specified. Use -DSIGCXX_ORB_ILU or -DSIGCXX_ORB_MICO."
#endif

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

typdef corbaSigC::Server CORBAServer;

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
