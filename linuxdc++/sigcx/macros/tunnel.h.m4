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
#ifndef __header__
#define __header__

#include <sigc++/slot.h>
#include <sigc++/object_slot.h>

#include <sigcx/bind3.h>

define([ARG1], [$1])
define([PACK_DOC_PARAMS],[dnl
ifelse(NUM($1),0,[],
       [\param LOWER(ARG1($1)) Slot argument $2. PACK_DOC_PARAMS([shift($1)], incr($2))])])

namespace SigCX
{

/** \addtogroup sigcx */
/*@{*/

/** A tunnel.
 *
 * A tunnel is a device that accepts callbacks to be executed, either 
 * synchronous or asynchronous.
 */
class Tunnel
{
  public:
    /** Tunnel callback. */
    class Callback
    {
      public:
	/** Destructor. */
        virtual ~Callback() { }
	/** Invoke callback. */
        virtual void invoke() = 0;
    };
    
    /** Destructor. */
    virtual ~Tunnel() { }
    
    /** Send a callback to the other side.
     * \param cb Callback to be executed on the other side of the tunnel.
     * \param sync If true, this thread is suspended until the callback has
     *   finished execution. */
    virtual void send(Callback *cb, bool sync = false) = 0;
    /** Check if tunnel is executing a synchronous callback.
     * \return \c true if a synchronous callback is in execution. */
    virtual bool in_sync_callback() = 0;

    /** Drain the tunnel.
     * Any callbacks in the tunnel are cancelled. This is intended to
     * be called at the destination end of the tunnel. */
    virtual void drain() = 0;
};

/****************************************************************
*****  Tunnel Traits
****************************************************************/

template <class T>
struct TunnelTrait
{
    typedef T type;
};

// we have to copy at least const refs. FIXME: dunno about normal refs... 
template <class T>
struct TunnelTrait<const T&>
{
    typedef T type; 
};

define([FORMAT_ARG_TTCBDECL],[typename TunnelTrait<[$1]>::type LOWER([$1])_;])

dnl
dnl ARG_TTCBDECL([P1,P2]) -> typename TunnelTrait<P1>::type p1_, typename TunnelTrait<P2>::type p2_
dnl
define([ARG_TTCBDECL],[PROT(ARG_LOOP([FORMAT_ARG_TTCBDECL],[ ],$*))])

dnl
dnl TUNNEL([P1, P2, ...])
dnl
define([TUNNEL],
[/****************************************************************
*****  Tunnel NUM($1)
****************************************************************/
LINE(]__line__[)dnl


/** Tunnel callback for NUM($1) arguments. */
template <LIST(class R,ARG_CLASS($1))>
class [TunnelCallback]NUM($1) : public Tunnel::Callback
{
  public:
    [TunnelCallback]NUM($1)(LIST(PROT(const __SLOT__(R, [$1])& slot), ARG_BOTH($1)))
: LIST(slot_(slot), ARG_CBINIT($1)) {}
    virtual void invoke() { rv_ = slot_(ARG_CBNAME($1)); }
    /** Tunnel the invocation of this callback.
     * \param tunnel The tunnel to use. This may be 0, if the callback 
         should be invoked without tunneling.
     * \param sync If true, wait for callback to terminate.
     * \return Return value of the callback slot, if \a sync is \c true, 
         otherwise undefined. */
    R tunnel(Tunnel *tunnel, bool sync = false) {
      if (tunnel) {
        tunnel->send(this, sync);
        R tmp = R();
        if (sync) {
          tmp = rv_;
	  delete this;
        }
        return tmp;
      }
      else {
	R tmp = slot_(ARG_CBNAME($1)); 
	delete this;
        return tmp;
      }
    }
    static R pack_n_tunnel(LIST(ARG_BOTH($1), PROT(__SLOT__(R, [$1]) s), Tunnel *tunnel, bool sync)) {
    ifelse(NUM($1),0,[
      [TunnelCallback]NUM($1)<LIST(R, ARG_TYPE($1))> *cb = pack(LIST(s, ARG_NAME($1)));],[[TunnelCallback]NUM($1)<LIST(R, ARG_TYPE($1))> *cb = pack<LIST(R, LIST(ARG_TYPE($1)))>(LIST(s, ARG_NAME($1)));])
      return cb->tunnel(tunnel, sync);
    }
  protected:
    __SLOT__(R, [$1]) slot_;
    typename TunnelTrait<R>::type rv_;
    ARG_TTCBDECL($1)
};


/** Pack ifelse(NUM($1),0,[],NUM($1)) arguments in a callback.
 * \param s A slot.
 * PACK_DOC_PARAMS([$1],1)
 * \return A pointer to a callback corresponding to \a s with all 
 *   the parameters specified bound to it.
 */
template <LIST(class R, ARG_CLASS($1))>
   [TunnelCallback]NUM($1)<LIST(R, ARG_TYPE($1))> *pack(LIST(PROT(const __SLOT__(R,[$1])& s), ARG_BOTH($1)))
  {       return new [TunnelCallback]NUM($1)<LIST(R, ARG_TYPE($1))>(LIST(s, ARG_NAME($1)));
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <ARG_CLASS($1)>
class [TunnelCallback]NUM($1)<LIST(void, ARG_TYPE($1))> : public Tunnel::Callback
{
  public:
    [TunnelCallback]NUM($1)(LIST(PROT(__SLOT__(void, [$1]) slot), ARG_BOTH($1)))
: LIST(slot_(slot), ARG_CBINIT($1)) { }
    virtual void invoke() { slot_(ARG_CBNAME($1)); }
    void tunnel(Tunnel *tunnel, bool sync = false) {
      if (tunnel) {
        tunnel->send(this, sync);
        if (sync)
          delete this;
      }
      else {
        slot_(ARG_CBNAME($1));
	delete this;
      }
    }
    static void pack_n_tunnel(LIST(ARG_BOTH($1), PROT(__SLOT__(void, [$1]) s), Tunnel *tunnel, bool sync)) {
ifelse(NUM($1),0,[
      [TunnelCallback]NUM($1)<LIST(void, ARG_TYPE($1))> *cb = pack(LIST(s, ARG_NAME($1)));],[[TunnelCallback]NUM($1)<LIST(void, ARG_TYPE($1))> *cb = pack<LIST(void, LIST(ARG_TYPE($1)))>(LIST(s, ARG_NAME($1)));])
      cb->tunnel(tunnel, sync);
   }
  protected:
    __SLOT__(void, [$1]) slot_;
    ARG_TTCBDECL($1)
};
#endif

/** Create a slot using a tunnel.
 * \param tunnel Tunnel to use.
 * \param s Slot to invoke on the other tunnel side.
 * \param sync Wether to invoke the callback synchronously.
 * \return The tunneled version of slot \a s. */
template <LIST(class R, ARG_CLASS($1))>
__SLOT__(R, ARG_TYPE($1))
  open_tunnel(Tunnel *tunnel, const __SLOT__(R,[$1])& s, bool sync = false)
  { return SigCX::bind(SigC::slot(&[TunnelCallback]NUM($1)<LIST(R, ARG_TYPE($1))>::pack_n_tunnel), s, tunnel, sync);
  }


template <LIST(class R, ARG_CLASS($1))>
R tunnel(LIST(PROT(__SLOT__(R, [$1]) s), ARG_BOTH($1), Tunnel *tunnel, bool sync = false)) 
{
  return tunnel ? [TunnelCallback]NUM($1)<LIST(R, ARG_TYPE($1))>::pack_n_tunnel(LIST(ARG_NAME($1), s, tunnel, sync)) : s(ARG_NAME($1));
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

/*@}*/

} // namespace SigCX
 
#endif
