dnl 
dnl  CORBA Convert Slot Templates
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

/*
  corbaSigC::convert
  -------------
  convert() alters a Slot by assigning a conversion function 
  which can completely alter the parameter types of a slot. 

*/
#include <sigc++/adaptor.h>
#include <sigc++/convert.h>

#include <sigc++/corba_base.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

using namespace corbaSigC;

template <typename T>
struct CORBATraits
{
  typedef T in_type;
  typedef T& out_type;
}; 

#ifdef SIGC_CXX_PARTIAL_SPEC
template <>
struct CORBATraits<int>
{
  typedef CORBA::Long in_type;
  typedef CORBA::Long out_type;
};

template <>
struct CORBATraits<const char *>
{
  typedef const char *in_type;
};

#endif

define([MARSHAL_VAL],[[params]BRACE(decr(regexp([$1], [\([0-9]+\)], [\1]))) <<= CORBATraits<$1>::in_type(LOWER($1));])
define([MARSHAL],[dnl
ifelse(NUM($*), 0, , NUM($*), 1, [MARSHAL_VAL($1)], 
       [MARSHAL_VAL($1, $2) MARSHAL(shift($*))])])

define([UNMARSHAL_VAL],[[$1]BRACE(decr(regexp([$2], [\([0-9]+\)], [\1]))) >>= $2;])
define([UNMARSHAL],[dnl
ifelse(NUM($*), 0, , NUM($*), 1, [UNMARSHAL_VAL([params], $1)], 
       [UNMARSHAL_VAL([params], $1) UNMARSHAL(shift($*))])])

dnl
dnl ADAPTOR_FUNCTION(R,[P1..PN])
dnl
define([ADAPTOR_FUNCTION],[dnl
/****************************************************************
***** Adaptor Convert Slot NUM($2)
****************************************************************/
ADAPTOR_FUNCTION_IMPL($1,[$2])

#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
ADAPTOR_FUNCTION_IMPL(void,[$2])
#endif
#endif

template <LIST(class $1,1,ARG_CLASS($2),[$2])>
__SLOT__($1, [$2])
  convert(const __SLOT__(CORBA::Any*,const ParamList&) &s)
  {return convert(s, &[_corba_marshal_convert_]NUM($2)<$1, [$2]>);
  }

template <LIST(class R1,1,ARG_CLASS($2),[$2])>
Slot1<CORBA::Any*,const ParamList&>
  convert(const __SLOT__(R1, [$2]) &s)
  {return convert(s, &[_corba_unmarshal_convert_]NUM($2)<R1, [$2]>);
  }
])dnl

dnl
dnl ADAPTOR_FUNCTION_IMPL(R,[P1..PN])
dnl
define([ADAPTOR_FUNCTION_IMPL],[dnl
ifelse($1,void,[dnl
template <LIST(ARG_CLASS($2),[$2])>
CORBA::Any *[_corba_unmarshal_convert_]NUM($2)([Callback]NUM($2)PROT(<LIST($1,1,ARG_TYPE($2), [$2])>)* d, const ParamList& params)
],[dnl
LINE(]__line__[)dnl
template <class $1, LIST(ARG_CLASS($2),[$2])>
CORBA::Any *[_corba_unmarshal_convert_]NUM($2)([Callback]NUM($2)PROT(<LIST($1,1,ARG_TYPE($2), [$2])>)* d, const ParamList& params)
])dnl
  {
    ARG_CBDECL($2)

    if (params.length() < NUM($2))
      throw CORBA::BAD_PARAM();
    UNMARSHAL(ARG_CBNAME($2))
ifelse($1,void,[dnl
    d->call(ARG_CBNAME($2));
    return(new CORBA::Any());
],[dnl
    [$1] retval = d->call(ARG_CBNAME($2));
    CORBA::Any *any_retval = new CORBA::Any();
    *any_retval <<= CORBATraits<$1>::in_type(retval);
    return(any_retval);
])dnl
  }


ifelse($1,void,[dnl
template <LIST(ARG_CLASS($2),[$2])>
void [_corba_marshal_convert_]NUM($2)([Callback1]PROT(<CORBA::Any*, const ParamList&>)* d, LIST(ARG_BOTH($2), [$2]))
],[dnl
template <LIST(class $1,1,ARG_CLASS($2),[$2])>
[$1] [_corba_marshal_convert_]NUM($2)([Callback1]PROT(<CORBA::Any*, const ParamList&>)* d, LIST(ARG_BOTH($2), [$2]))
])
  {
    ParamList params;
    params.length(NUM($2));
    MARSHAL($2)
    CORBA::Any *any_retval = d->call(params);
ifelse($1,void, [dnl
    if (any_retval)
      delete any_retval;
],[dnl
    [$1] retval;
    if (any_retval)
    {
      *any_retval >>= CORBATraits<$1>::out_type(retval);
      delete any_retval;
    }
    return(retval);
])dnl
  }
])dnl

ADAPTOR_FUNCTION(R,ARGS(P,1))
ADAPTOR_FUNCTION(R,ARGS(P,2))
ADAPTOR_FUNCTION(R,ARGS(P,3))
ADAPTOR_FUNCTION(R,ARGS(P,4))
ADAPTOR_FUNCTION(R,ARGS(P,5))
ADAPTOR_FUNCTION(R,ARGS(P,6))
ADAPTOR_FUNCTION(R,ARGS(P,7))

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
