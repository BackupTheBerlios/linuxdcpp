// -*- c++ -*-
//   Copyright 2000, Karl Einar Nelson
//   Copyright 2002, Andreas Rottmann
/* This is a generated file, do not edit.  Generated from template.macros.m4 */

#ifndef   SIGC_REF_SLOT_H
#define   SIGC_REF_SLOT_H

#include <stdexcept>

#include <sigc++/slot.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif




// these represent the callable structure of a slot with various types.
// they are fundimentally just wrappers.  They have no differences other
// that the types they cast data to.
template <class R>
class Slot0<R&> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)();
      typedef RType (*Proxy)(void*);
      RType operator ()()
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (node_);
        }
  
      Slot0& operator= (const Slot0 &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot0() 
        : SlotBase() {} 
      Slot0(const Slot0& s) 
        : SlotBase(s) 
        {}
      Slot0(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot0(Callback callback)
        : SlotBase()
        { 
          typedef FuncSlot0_<R&> Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot0() {}
  };

template <class R,class P1>
class Slot1<R&,P1> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)(P1);
      typedef RType (*Proxy)(typename SigC::Trait<P1>::ref p1,void*);
      RType operator ()(typename SigC::Trait<P1>::ref p1)
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (p1,node_);
        }
  
      Slot1& operator= (const Slot1 &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot1() 
        : SlotBase() {} 
      Slot1(const Slot1& s) 
        : SlotBase(s) 
        {}
      Slot1(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot1(Callback callback)
        : SlotBase()
        { 
          typedef FuncSlot1_<R&,P1> Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot1() {}
  };

template <class R,class P1,class P2>
class Slot2<R&,P1,P2> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)(P1,P2);
      typedef RType (*Proxy)(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,void*);
      RType operator ()(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2)
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (p1,p2,node_);
        }
  
      Slot2& operator= (const Slot2 &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot2() 
        : SlotBase() {} 
      Slot2(const Slot2& s) 
        : SlotBase(s) 
        {}
      Slot2(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot2(Callback callback)
        : SlotBase()
        { 
          typedef FuncSlot2_<R&,P1,P2> Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot2() {}
  };

template <class R,class P1,class P2,class P3>
class Slot3<R&,P1,P2,P3> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)(P1,P2,P3);
      typedef RType (*Proxy)(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,void*);
      RType operator ()(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3)
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (p1,p2,p3,node_);
        }
  
      Slot3& operator= (const Slot3 &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot3() 
        : SlotBase() {} 
      Slot3(const Slot3& s) 
        : SlotBase(s) 
        {}
      Slot3(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot3(Callback callback)
        : SlotBase()
        { 
          typedef FuncSlot3_<R&,P1,P2,P3> Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot3() {}
  };

template <class R,class P1,class P2,class P3,class P4>
class Slot4<R&,P1,P2,P3,P4> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)(P1,P2,P3,P4);
      typedef RType (*Proxy)(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,void*);
      RType operator ()(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4)
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (p1,p2,p3,p4,node_);
        }
  
      Slot4& operator= (const Slot4 &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot4() 
        : SlotBase() {} 
      Slot4(const Slot4& s) 
        : SlotBase(s) 
        {}
      Slot4(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot4(Callback callback)
        : SlotBase()
        { 
          typedef FuncSlot4_<R&,P1,P2,P3,P4> Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot4() {}
  };

template <class R,class P1,class P2,class P3,class P4,class P5>
class Slot5<R&,P1,P2,P3,P4,P5> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)(P1,P2,P3,P4,P5);
      typedef RType (*Proxy)(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,void*);
      RType operator ()(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5)
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (p1,p2,p3,p4,p5,node_);
        }
  
      Slot5& operator= (const Slot5 &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot5() 
        : SlotBase() {} 
      Slot5(const Slot5& s) 
        : SlotBase(s) 
        {}
      Slot5(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot5(Callback callback)
        : SlotBase()
        { 
          typedef FuncSlot5_<R&,P1,P2,P3,P4,P5> Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot5() {}
  };

template <class R,class P1,class P2,class P3,class P4,class P5,class P6>
class Slot6<R&,P1,P2,P3,P4,P5,P6> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)(P1,P2,P3,P4,P5,P6);
      typedef RType (*Proxy)(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,void*);
      RType operator ()(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6)
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (p1,p2,p3,p4,p5,p6,node_);
        }
  
      Slot6& operator= (const Slot6 &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot6() 
        : SlotBase() {} 
      Slot6(const Slot6& s) 
        : SlotBase(s) 
        {}
      Slot6(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot6(Callback callback)
        : SlotBase()
        { 
          typedef FuncSlot6_<R&,P1,P2,P3,P4,P5,P6> Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot6() {}
  };

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
class Slot7<R&,P1,P2,P3,P4,P5,P6,P7> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)(P1,P2,P3,P4,P5,P6,P7);
      typedef RType (*Proxy)(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,typename SigC::Trait<P7>::ref p7,void*);
      RType operator ()(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,typename SigC::Trait<P7>::ref p7)
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (p1,p2,p3,p4,p5,p6,p7,node_);
        }
  
      Slot7& operator= (const Slot7 &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot7() 
        : SlotBase() {} 
      Slot7(const Slot7& s) 
        : SlotBase(s) 
        {}
      Slot7(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot7(Callback callback)
        : SlotBase()
        { 
          typedef FuncSlot7_<R&,P1,P2,P3,P4,P5,P6,P7> Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot7() {}
  };

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8>
class Slot8<R&,P1,P2,P3,P4,P5,P6,P7,P8> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)(P1,P2,P3,P4,P5,P6,P7,P8);
      typedef RType (*Proxy)(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,typename SigC::Trait<P7>::ref p7,typename SigC::Trait<P8>::ref p8,void*);
      RType operator ()(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,typename SigC::Trait<P7>::ref p7,typename SigC::Trait<P8>::ref p8)
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (p1,p2,p3,p4,p5,p6,p7,p8,node_);
        }
  
      Slot8& operator= (const Slot8 &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot8() 
        : SlotBase() {} 
      Slot8(const Slot8& s) 
        : SlotBase(s) 
        {}
      Slot8(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot8(Callback callback)
        : SlotBase()
        { 
          typedef FuncSlot8_<R&,P1,P2,P3,P4,P5,P6,P7,P8> Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot8() {}
  };

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
class Slot9<R&,P1,P2,P3,P4,P5,P6,P7,P8,P9> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)(P1,P2,P3,P4,P5,P6,P7,P8,P9);
      typedef RType (*Proxy)(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,typename SigC::Trait<P7>::ref p7,typename SigC::Trait<P8>::ref p8,typename SigC::Trait<P9>::ref p9,void*);
      RType operator ()(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,typename SigC::Trait<P7>::ref p7,typename SigC::Trait<P8>::ref p8,typename SigC::Trait<P9>::ref p9)
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (p1,p2,p3,p4,p5,p6,p7,p8,p9,node_);
        }
  
      Slot9& operator= (const Slot9 &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot9() 
        : SlotBase() {} 
      Slot9(const Slot9& s) 
        : SlotBase(s) 
        {}
      Slot9(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot9(Callback callback)
        : SlotBase()
        { 
          typedef FuncSlot9_<R&,P1,P2,P3,P4,P5,P6,P7,P8,P9> Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot9() {}
  };

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
class Slot10<R&,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10);
      typedef RType (*Proxy)(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,typename SigC::Trait<P7>::ref p7,typename SigC::Trait<P8>::ref p8,typename SigC::Trait<P9>::ref p9,typename SigC::Trait<P10>::ref p10,void*);
      RType operator ()(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,typename SigC::Trait<P7>::ref p7,typename SigC::Trait<P8>::ref p8,typename SigC::Trait<P9>::ref p9,typename SigC::Trait<P10>::ref p10)
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,node_);
        }
  
      Slot10& operator= (const Slot10 &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot10() 
        : SlotBase() {} 
      Slot10(const Slot10& s) 
        : SlotBase(s) 
        {}
      Slot10(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot10(Callback callback)
        : SlotBase()
        { 
          typedef FuncSlot10_<R&,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10> Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot10() {}
  };

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class P11>
class Slot11<R&,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11> : public SigC::SlotBase
  {
    public:
      typedef typename Trait<R&>::type RType;
      typedef R& (*Callback)(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11);
      typedef RType (*Proxy)(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,typename SigC::Trait<P7>::ref p7,typename SigC::Trait<P8>::ref p8,typename SigC::Trait<P9>::ref p9,typename SigC::Trait<P10>::ref p10,typename SigC::Trait<P11>::ref p11,void*);
      RType operator ()(typename SigC::Trait<P1>::ref p1,typename SigC::Trait<P2>::ref p2,typename SigC::Trait<P3>::ref p3,typename SigC::Trait<P4>::ref p4,typename SigC::Trait<P5>::ref p5,typename SigC::Trait<P6>::ref p6,typename SigC::Trait<P7>::ref p7,typename SigC::Trait<P8>::ref p8,typename SigC::Trait<P9>::ref p9,typename SigC::Trait<P10>::ref p10,typename SigC::Trait<P11>::ref p11)
        {
          if (!node_) throw std::runtime_error("invalid slot");
          if (node_->notified_)
            { clear(); throw std::runtime_error("invalid slot"); }
          return ((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
            (p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,node_);
        }
  
      Slot11& operator= (const Slot11 &s)
        {
          SlotBase::operator=(s);
          return *this;
        }
  
      Slot11() 
        : SlotBase() {} 
      Slot11(const Slot11& s) 
        : SlotBase(s) 
        {}
      Slot11(SlotNode* node)
        : SlotBase()
        { assign(node); }
      Slot11(Callback callback)
        : SlotBase()
        { 
          typedef FuncSlot11_<R&,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11> Proxy_;
          assign(new FuncSlotNode((FuncPtr)&Proxy_::proxy,(FuncPtr)callback));
        }
      ~Slot11() {}
  };


#ifdef SIGC_CXX_NAMESPACES
}
#endif

#endif // SIGC_SLOT
