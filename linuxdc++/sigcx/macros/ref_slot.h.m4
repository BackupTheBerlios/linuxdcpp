// -*- c++ -*-
dnl  slot.h.m4 - slot class for sigc++
dnl 
//   Copyright 2000, Karl Einar Nelson
//   Copyright 2002, Andreas Rottmann
dnl
dnl  This library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Lesser General Public
dnl  License as published by the Free Software Foundation; either
dnl  version 2 of the License, or (at your option) any later version.
dnl
dnl  This library is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Lesser General Public License for more details.
dnl
dnl  You should have received a copy of the GNU Lesser General Public
dnl  License along with this library; if not, write to the Free Software
dnl  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
dnl
include(template.macros.m4)
#ifndef   __header__
#define   __header__

#include <stdexcept>

#include <sigc++/slot.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

define([__FUNC_SLOT__],[[FuncSlot]eval(NUM($*)-1)_<LIST($*)>])dnl

dnl
dnl  SLOT([P1...PN])
dnl
dnl
define([SLOT],[dnl
template <LIST(class R,ARG_CLASS($1))>
class Slot[]NUM($1)<LIST(R&, ARG_TYPE($1))> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)(ARG_TYPE($1));
      typedef RType (*Proxy)(LIST(ARG_REF($1),void*));
      RType operator ()(ARG_REF($1))
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (LIST(ARG_NAME($1),node_));
        }
  
      Slot[]NUM($1)& operator= (const Slot[]NUM($1) &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot[]NUM($1)() 
        : SlotBase() {} 
      Slot[]NUM($1)(const Slot[]NUM($1)& s) 
        : SlotBase(s) 
        {}
      Slot[]NUM($1)(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot[]NUM($1)(Callback callback)
        : SlotBase()
        { 
          typedef __FUNC_SLOT__(R&,$1) Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot[]NUM($1)() {}
  };
])

// these represent the callable structure of a slot with various types.
// they are fundimentally just wrappers.  They have no differences other
// that the types they cast data to.
SLOT(ARGS(P,0))
SLOT(ARGS(P,1))
SLOT(ARGS(P,2))
SLOT(ARGS(P,3))
SLOT(ARGS(P,4))
SLOT(ARGS(P,5))
SLOT(ARGS(P,6))
SLOT(ARGS(P,7))
SLOT(ARGS(P,8))
SLOT(ARGS(P,9))
SLOT(ARGS(P,10))
SLOT(ARGS(P,11))

#ifdef SIGC_CXX_NAMESPACES
}
#endif

#endif // SIGC_SLOT
