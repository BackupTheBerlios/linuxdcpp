dnl 
dnl  SigC++ Cross-Thread Signal Templates
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

#include <sigc++/slot.h>
#include <sigc++/object_slot.h>
#include <sigc++/bind.h>
#include <sigc++/corba_base.h>

namespace SigC
{

namespace CORBA
{

using namespace corbaSigC;

template <typename T>
struct Traits
{
  typedef T in_type;
  typedef T& out_type;
}; 

#ifdef SIGC_CXX_PARTIAL_SPEC
template <>
struct Traits<int>
{
  typedef CORBA::Long in_type;
  typedef CORBA::Long out_type;
};

template <>
struct Traits<const char *>
{
  typedef const char *in_type;
};

#endif

define([MARSHAL_VAL],[[params]BRACE(decr(regexp([$1], [\([0-9]+\)], [\1]))) <<= Traits<$1>::in_type(LOWER($1));])
define([MARSHAL],[dnl
ifelse(NUM($*), 0, , NUM($*), 1, [MARSHAL_VAL($1)], 
       [MARSHAL_VAL($1, $2) MARSHAL(shift($*))])])

define([UNMARSHAL_VAL],[[$1]BRACE(decr(regexp([$2], [\([0-9]+\)], [\1]))) >>= $2;])
define([UNMARSHAL],[dnl
ifelse(NUM($*), 0, , NUM($*), 1, [UNMARSHAL_VAL([params], $1)], 
       [UNMARSHAL_VAL([params], $1) UNMARSHAL(shift($*))])])

dnl
dnl TUNNEL(R,[P1..PN])
dnl
define([ADAPTOR_FUNCTION],[dnl
/****************************************************************
***** Tunnel NUM($2)
****************************************************************/
ADAPTOR_FUNCTION_IMPL($1,[$2])

#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
ADAPTOR_FUNCTION_IMPL(void,[$2])
#endif
#endif

template <LIST(class R,1,ARG_CLASS($2),[$2])>
__SLOT__($1, [$2])
  open_tunnel(Tunnel *tunnel, corbaSigC::Slot_ptr, const ParamList&)& s)
  {return bind(s, &[_corba_marshal_convert_]NUM($2)<$1, [$2]>);
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

namespace SigC
{

namespace CORBA
{

class Tunnel : virtual public SigC::Tunnel
{
  public:
    Tunnel(Dispatcher *disp, bool multithreaded = true);
    ~Tunnel();

    virtual void send(Callback *cb, bool sync = false) = 0;
};

dnl
dnl TUNNEL([P1, P2, ...])
dnl
define([TUNNEL],
[/****************************************************************
*****  Tunnel NUM($1)
****************************************************************/
LINE(]__line__[)dnl

template <LIST(class R, 1, ARG_CLASS($1),[$1])>
class [TunnelCallback]NUM($1) : public Tunnel
{
  public:
    [TunnelCallback]NUM($1)(LIST(corbaSigC::Slot_ptr slot, 1, ARG_BOTH($1), [$1])) {
      
    R return_value(bool free = true) {
      R tmp = rv_; if (free) delete this; return(tmp);
    }
    virtual void invoke() { rv_ = slot_.call(ARG_MEMBER($1)); }
    R emit(Tunnel *tunnel, bool sync = false) {
      tunnel->send(this, sync);
      R tmp;
      if (sync)
        tmp = rv_;
      return(tmp);
    }
  private:
    __SLOT__(R, [$1]) slot_;
    R rv_;
    MEMBER_DECLS($1)
};

#ifdef SIGC_CXX_PARTIAL_SPEC
template <ARG_CLASS($1)>
class [TunnelCallback]NUM($1)<LIST(void, 1, ARG_TYPE($1), [$1])> : public Tunnel::Callback
{
  public:
    [TunnelCallback]NUM($1)(LIST(PROT(__SLOT__(void, [$1]) slot), 1, ARG_BOTH($1), [$1]))
ifelse(NUM($1), 0, [: slot_(slot) {}], [: MEMBER_INIT(slot, [$1]) { }])
    void return_value(bool do_free = true) { if (do_free) delete this; }
    virtual void invoke() { slot_.call(ARG_MEMBER($1)); }
    void emit(Tunnel *tunnel, bool sync = false) {
      tunnel->send(this, sync);
      if (sync)
        delete this;
    }
  private:
    __SLOT__(void, [$1]) slot_;
    MEMBER_DECLS($1)
};
#endif

template <LIST(class R, 1, ARG_CLASS($1),[$1])>
   [TunnelCallback]NUM($1)<LIST(R, 1, ARG_TYPE($1), [$1])> *pack(LIST(PROT(__SLOT__(R, [$1])) s, 1, ARG_BOTH($1), [$1]))
  {       return(new [TunnelCallback]NUM($1)<LIST(R, 1, ARG_TYPE($1), [$1])>(LIST(s, 1, ARG_NAME($1), [$1])));
  }

template <LIST(class R, 1, ARG_CLASS($1),[$1])>
__SLOT__(R, [$1])
  open_tunnel(Tunnel *tunnel, PROT(__SLOT__(R, [$1])) s)
  { [TunnelCallback]NUM($1)<LIST(R, 1, ARG_TYPE($1), [$1])> *cb =
      [TunnelCallback]NUM($1)<LIST(R, 1, ARG_TYPE($1), [$1])>(LIST(s, 1, ARG_NAME($1), [$1]));
    return(bind(slot(cb, [&TunnelCallback]NUM($1)<LIST(R, 1, ARG_TYPE($1), [$1])>::emit), tunnel, sync));
  }

])dnl

TUNNEL(ARGS(P,0))
TUNNEL(ARGS(P,1))
TUNNEL(ARGS(P,2))
TUNNEL(ARGS(P,3))
TUNNEL(ARGS(P,4))
TUNNEL(ARGS(P,5))
TUNNEL(ARGS(P,6))
TUNNEL(ARGS(P,7))

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif
 
#endif
