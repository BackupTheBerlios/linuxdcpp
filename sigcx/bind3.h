/* This is for emacs: -*-Mode: C++;-*- */
/* Copyright 2002, Andreas Rottmann */
/* This is a generated file, do not edit.  Generated from template.macros.m4 */

#ifndef SIGC_BIND3_H
#define SIGC_BIND3_H

#include <sigc++/bind.h>

namespace SigCX
{

using namespace SigC;

template <class C1,class C2,class C3>
struct AdaptorBindData3_
  {
    typedef AdaptorBindData3_ Self;
    AdaptorBindSlotNode adaptor;
    C1 c1_;
        C2 c2_;
        C3 c3_;
    AdaptorBindData3_(FuncPtr p, const Node& s ,FuncPtr d,
      C1 c1,C2 c2,C3 c3)
    : adaptor(p, s, d), c1_(c1),c2_(c2),c3_(c3)
      {}

    static void dtor(void* data)
      {
        Self& node = *reinterpret_cast<Self*>(data);
        node.c1_.~C1();
        node.c2_.~C2();
        node.c3_.~C3();
      }
  }; 



template <class R,class C1,class C2,class C3>
struct AdaptorBindSlot0_3_
  {
    typedef typename Trait<R>::type RType;
    typedef typename SigC::Slot3<R,C1,C2,C3>::Proxy Proxy;
    static RType proxy(void *data) 
      { 
        typedef AdaptorBindData3_<C1,C2,C3> Data;
        Data& node=*reinterpret_cast<Data*>(data);
        SlotNode* slot=static_cast<SlotNode*>(node.adaptor.slot_.impl());
        return ((Proxy)(slot->proxy_))
          (node.c1_,node.c2_,node.c3_,slot);
      }

  };

/// @ingroup bind
template <class A1,class A2,class A3,class R,class C1,class C2,class C3>
SigC::Slot0<R>
  bind(const SigC::Slot3<R,C1,C2,C3>& s,
       A1 a1,A2 a2,A3 a3)
  { 
    typedef AdaptorBindData3_<C1,C2,C3> Data;
    typedef AdaptorBindSlot0_3_<R,C1,C2,C3> Adaptor;
    return reinterpret_cast<SlotNode*>(
       new Data((FuncPtr)(&Adaptor::proxy),s,
                (FuncPtr)(&Data::dtor),a1,a2,a3));
  }


template <class R,class P1,class C1,class C2,class C3>
struct AdaptorBindSlot1_3_
  {
    typedef typename Trait<R>::type RType;
    typedef typename SigC::Slot4<R,P1,C1,C2,C3>::Proxy Proxy;
    static RType proxy(typename SigC::Trait<P1>::ref p1,void *data) 
      { 
        typedef AdaptorBindData3_<C1,C2,C3> Data;
        Data& node=*reinterpret_cast<Data*>(data);
        SlotNode* slot=static_cast<SlotNode*>(node.adaptor.slot_.impl());
        return ((Proxy)(slot->proxy_))
          (p1,node.c1_,node.c2_,node.c3_,slot);
      }

  };

/// @ingroup bind
template <class A1,class A2,class A3,class R,class P1,class C1,class C2,class C3>
SigC::Slot1<R,P1>
  bind(const SigC::Slot4<R,P1,C1,C2,C3>& s,
       A1 a1,A2 a2,A3 a3)
  { 
    typedef AdaptorBindData3_<C1,C2,C3> Data;
    typedef AdaptorBindSlot1_3_<R,P1,C1,C2,C3> Adaptor;
    return reinterpret_cast<SlotNode*>(
       new Data((FuncPtr)(&Adaptor::proxy),s,
                (FuncPtr)(&Data::dtor),a1,a2,a3));
  }


template <class R,class P1,class P2,class C1,class C2,class C3>
struct AdaptorBindSlot2_3_
  {
    typedef typename Trait<R>::type RType;
    typedef typename SigC::Slot5<R,P1,P2,C1,C2,C3>::Proxy Proxy;
    static RType proxy(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,void *data) 
      { 
        typedef AdaptorBindData3_<C1,C2,C3> Data;
        Data& node=*reinterpret_cast<Data*>(data);
        SlotNode* slot=static_cast<SlotNode*>(node.adaptor.slot_.impl());
        return ((Proxy)(slot->proxy_))
          (p1,p2,node.c1_,node.c2_,node.c3_,slot);
      }

  };

/// @ingroup bind
template <class A1,class A2,class A3,class R,class P1,class P2,class C1,class C2,class C3>
SigC::Slot2<R,P1,P2>
  bind(const SigC::Slot5<R,P1,P2,C1,C2,C3>& s,
       A1 a1,A2 a2,A3 a3)
  { 
    typedef AdaptorBindData3_<C1,C2,C3> Data;
    typedef AdaptorBindSlot2_3_<R,P1,P2,C1,C2,C3> Adaptor;
    return reinterpret_cast<SlotNode*>(
       new Data((FuncPtr)(&Adaptor::proxy),s,
                (FuncPtr)(&Data::dtor),a1,a2,a3));
  }


template <class R,class P1,class P2,class P3,class C1,class C2,class C3>
struct AdaptorBindSlot3_3_
  {
    typedef typename Trait<R>::type RType;
    typedef typename SigC::Slot6<R,P1,P2,P3,C1,C2,C3>::Proxy Proxy;
    static RType proxy(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,void *data) 
      { 
        typedef AdaptorBindData3_<C1,C2,C3> Data;
        Data& node=*reinterpret_cast<Data*>(data);
        SlotNode* slot=static_cast<SlotNode*>(node.adaptor.slot_.impl());
        return ((Proxy)(slot->proxy_))
          (p1,p2,p3,node.c1_,node.c2_,node.c3_,slot);
      }

  };

/// @ingroup bind
template <class A1,class A2,class A3,class R,class P1,class P2,class P3,class C1,class C2,class C3>
SigC::Slot3<R,P1,P2,P3>
  bind(const SigC::Slot6<R,P1,P2,P3,C1,C2,C3>& s,
       A1 a1,A2 a2,A3 a3)
  { 
    typedef AdaptorBindData3_<C1,C2,C3> Data;
    typedef AdaptorBindSlot3_3_<R,P1,P2,P3,C1,C2,C3> Adaptor;
    return reinterpret_cast<SlotNode*>(
       new Data((FuncPtr)(&Adaptor::proxy),s,
                (FuncPtr)(&Data::dtor),a1,a2,a3));
  }


template <class R,class P1,class P2,class P3,class P4,class C1,class C2,class C3>
struct AdaptorBindSlot4_3_
  {
    typedef typename Trait<R>::type RType;
    typedef typename SigC::Slot7<R,P1,P2,P3,P4,C1,C2,C3>::Proxy Proxy;
    static RType proxy(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,void *data) 
      { 
        typedef AdaptorBindData3_<C1,C2,C3> Data;
        Data& node=*reinterpret_cast<Data*>(data);
        SlotNode* slot=static_cast<SlotNode*>(node.adaptor.slot_.impl());
        return ((Proxy)(slot->proxy_))
          (p1,p2,p3,p4,node.c1_,node.c2_,node.c3_,slot);
      }

  };

/// @ingroup bind
template <class A1,class A2,class A3,class R,class P1,class P2,class P3,class P4,class C1,class C2,class C3>
SigC::Slot4<R,P1,P2,P3,P4>
  bind(const SigC::Slot7<R,P1,P2,P3,P4,C1,C2,C3>& s,
       A1 a1,A2 a2,A3 a3)
  { 
    typedef AdaptorBindData3_<C1,C2,C3> Data;
    typedef AdaptorBindSlot4_3_<R,P1,P2,P3,P4,C1,C2,C3> Adaptor;
    return reinterpret_cast<SlotNode*>(
       new Data((FuncPtr)(&Adaptor::proxy),s,
                (FuncPtr)(&Data::dtor),a1,a2,a3));
  }


template <class R,class P1,class P2,class P3,class P4,class P5,class C1,class C2,class C3>
struct AdaptorBindSlot5_3_
  {
    typedef typename Trait<R>::type RType;
    typedef typename SigC::Slot8<R,P1,P2,P3,P4,P5,C1,C2,C3>::Proxy Proxy;
    static RType proxy(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,void *data) 
      { 
        typedef AdaptorBindData3_<C1,C2,C3> Data;
        Data& node=*reinterpret_cast<Data*>(data);
        SlotNode* slot=static_cast<SlotNode*>(node.adaptor.slot_.impl());
        return ((Proxy)(slot->proxy_))
          (p1,p2,p3,p4,p5,node.c1_,node.c2_,node.c3_,slot);
      }

  };

/// @ingroup bind
template <class A1,class A2,class A3,class R,class P1,class P2,class P3,class P4,class P5,class C1,class C2,class C3>
SigC::Slot5<R,P1,P2,P3,P4,P5>
  bind(const SigC::Slot8<R,P1,P2,P3,P4,P5,C1,C2,C3>& s,
       A1 a1,A2 a2,A3 a3)
  { 
    typedef AdaptorBindData3_<C1,C2,C3> Data;
    typedef AdaptorBindSlot5_3_<R,P1,P2,P3,P4,P5,C1,C2,C3> Adaptor;
    return reinterpret_cast<SlotNode*>(
       new Data((FuncPtr)(&Adaptor::proxy),s,
                (FuncPtr)(&Data::dtor),a1,a2,a3));
  }


template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class C1,class C2,class C3>
struct AdaptorBindSlot6_3_
  {
    typedef typename Trait<R>::type RType;
    typedef typename SigC::Slot9<R,P1,P2,P3,P4,P5,P6,C1,C2,C3>::Proxy Proxy;
    static RType proxy(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,void *data) 
      { 
        typedef AdaptorBindData3_<C1,C2,C3> Data;
        Data& node=*reinterpret_cast<Data*>(data);
        SlotNode* slot=static_cast<SlotNode*>(node.adaptor.slot_.impl());
        return ((Proxy)(slot->proxy_))
          (p1,p2,p3,p4,p5,p6,node.c1_,node.c2_,node.c3_,slot);
      }

  };

/// @ingroup bind
template <class A1,class A2,class A3,class R,class P1,class P2,class P3,class P4,class P5,class P6,class C1,class C2,class C3>
SigC::Slot6<R,P1,P2,P3,P4,P5,P6>
  bind(const SigC::Slot9<R,P1,P2,P3,P4,P5,P6,C1,C2,C3>& s,
       A1 a1,A2 a2,A3 a3)
  { 
    typedef AdaptorBindData3_<C1,C2,C3> Data;
    typedef AdaptorBindSlot6_3_<R,P1,P2,P3,P4,P5,P6,C1,C2,C3> Adaptor;
    return reinterpret_cast<SlotNode*>(
       new Data((FuncPtr)(&Adaptor::proxy),s,
                (FuncPtr)(&Data::dtor),a1,a2,a3));
  }


template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class C1,class C2,class C3>
struct AdaptorBindSlot7_3_
  {
    typedef typename Trait<R>::type RType;
    typedef typename SigC::Slot10<R,P1,P2,P3,P4,P5,P6,P7,C1,C2,C3>::Proxy Proxy;
    static RType proxy(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,typename SigC::Trait<P7>::ref p7,void *data) 
      { 
        typedef AdaptorBindData3_<C1,C2,C3> Data;
        Data& node=*reinterpret_cast<Data*>(data);
        SlotNode* slot=static_cast<SlotNode*>(node.adaptor.slot_.impl());
        return ((Proxy)(slot->proxy_))
          (p1,p2,p3,p4,p5,p6,p7,node.c1_,node.c2_,node.c3_,slot);
      }

  };

/// @ingroup bind
template <class A1,class A2,class A3,class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class C1,class C2,class C3>
SigC::Slot7<R,P1,P2,P3,P4,P5,P6,P7>
  bind(const SigC::Slot10<R,P1,P2,P3,P4,P5,P6,P7,C1,C2,C3>& s,
       A1 a1,A2 a2,A3 a3)
  { 
    typedef AdaptorBindData3_<C1,C2,C3> Data;
    typedef AdaptorBindSlot7_3_<R,P1,P2,P3,P4,P5,P6,P7,C1,C2,C3> Adaptor;
    return reinterpret_cast<SlotNode*>(
       new Data((FuncPtr)(&Adaptor::proxy),s,
                (FuncPtr)(&Data::dtor),a1,a2,a3));
  }



} // namespace SigCX
 
#endif
