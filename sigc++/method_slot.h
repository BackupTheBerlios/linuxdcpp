// -*- c++ -*-
//   Copyright 2000, Karl Einar Nelson
/* This is a generated file, do not edit.  Generated from template.macros.m4 */


#ifndef   SIGC_METHOD_SLOT
#define   SIGC_METHOD_SLOT
#include <sigc++/slot.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

/**************************************************************/
// These are internal classes used to represent function varients of slots

class Object;
// (internal) 
struct LIBSIGC_API MethodSlotNode : public SlotNode
  {
#ifdef _MSC_VER
private:
	/** the sole purpose of this declaration is to introduce a new type that is
	guaranteed not to be related to any other type. (Ab)using class SigC::Object
	for this lead to some faulty conversions taking place with MSVC6. */
	class GenericObject;
    typedef void* (GenericObject::*Method)(void*);
public:
#else
    typedef void* (Object::*Method)(void*);
#endif
    Method     method_;
    
    template <class T2>
    MethodSlotNode(FuncPtr proxy,T2 method)
      : SlotNode(proxy)
      { init(reinterpret_cast<Method&>(method)); }
    void init(Method method);
    virtual ~MethodSlotNode();
  };

struct LIBSIGC_API ConstMethodSlotNode : public SlotNode
  {
#ifdef _MSC_VER
private:
	/** the sole purpose of this declaration is to introduce a new type that is
	guaranteed not to be related to any other type. (Ab)using class SigC::Object
	for this lead to some faulty conversions taking place with MSVC6. */
	class GenericObject;
    typedef void* (GenericObject::*Method)(void*) const;
public:
#else
    typedef void* (Object::*Method)(void*) const;
#endif
    Method     method_;
    
    template <class T2>
    ConstMethodSlotNode(FuncPtr proxy,T2 method)
      : SlotNode(proxy)
      { init(reinterpret_cast<Method&>(method)); }
    void init(Method method);
    virtual ~ConstMethodSlotNode();
  };



// These do not derive from MethodSlot, they merely are extended
// ctor wrappers.  They introduce how to deal with the proxy.
template <class R,class Obj>
struct MethodSlot0_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,void * s) 
      { 
        typedef RType (Obj::*Method)();
        MethodSlotNode* os = (MethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))();
      }
  };

template <class R,class Obj>
struct ConstMethodSlot0_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,void * s) 
      { 
        typedef RType (Obj::*Method)() const;
        ConstMethodSlotNode* os = (ConstMethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))();
      }
  };

template <class R,class Obj>
Slot1<R,Obj&>
  slot(R (Obj::*method)())
  { 
    typedef MethodSlot0_<R,Obj> SType;
    return new MethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }

template <class R,class Obj>
Slot1<R,const Obj&>
  slot(R (Obj::*method)() const)
  {
    typedef ConstMethodSlot0_<R,Obj> SType;
    return new ConstMethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }


template <class R,class Obj,class P1>
struct MethodSlot1_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,void * s) 
      { 
        typedef RType (Obj::*Method)(P1);
        MethodSlotNode* os = (MethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1);
      }
  };

template <class R,class Obj,class P1>
struct ConstMethodSlot1_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,void * s) 
      { 
        typedef RType (Obj::*Method)(P1) const;
        ConstMethodSlotNode* os = (ConstMethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1);
      }
  };

template <class R,class Obj,class P1>
Slot2<R,Obj&,P1>
  slot(R (Obj::*method)(P1))
  { 
    typedef MethodSlot1_<R,Obj,P1> SType;
    return new MethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }

template <class R,class Obj,class P1>
Slot2<R,const Obj&,P1>
  slot(R (Obj::*method)(P1) const)
  {
    typedef ConstMethodSlot1_<R,Obj,P1> SType;
    return new ConstMethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }


template <class R,class Obj,class P1,class P2>
struct MethodSlot2_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2);
        MethodSlotNode* os = (MethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2);
      }
  };

template <class R,class Obj,class P1,class P2>
struct ConstMethodSlot2_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2) const;
        ConstMethodSlotNode* os = (ConstMethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2);
      }
  };

template <class R,class Obj,class P1,class P2>
Slot3<R,Obj&,P1,P2>
  slot(R (Obj::*method)(P1,P2))
  { 
    typedef MethodSlot2_<R,Obj,P1,P2> SType;
    return new MethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }

template <class R,class Obj,class P1,class P2>
Slot3<R,const Obj&,P1,P2>
  slot(R (Obj::*method)(P1,P2) const)
  {
    typedef ConstMethodSlot2_<R,Obj,P1,P2> SType;
    return new ConstMethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }


template <class R,class Obj,class P1,class P2,class P3>
struct MethodSlot3_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3);
        MethodSlotNode* os = (MethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3);
      }
  };

template <class R,class Obj,class P1,class P2,class P3>
struct ConstMethodSlot3_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3) const;
        ConstMethodSlotNode* os = (ConstMethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3);
      }
  };

template <class R,class Obj,class P1,class P2,class P3>
Slot4<R,Obj&,P1,P2,P3>
  slot(R (Obj::*method)(P1,P2,P3))
  { 
    typedef MethodSlot3_<R,Obj,P1,P2,P3> SType;
    return new MethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }

template <class R,class Obj,class P1,class P2,class P3>
Slot4<R,const Obj&,P1,P2,P3>
  slot(R (Obj::*method)(P1,P2,P3) const)
  {
    typedef ConstMethodSlot3_<R,Obj,P1,P2,P3> SType;
    return new ConstMethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }


template <class R,class Obj,class P1,class P2,class P3,class P4>
struct MethodSlot4_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4);
        MethodSlotNode* os = (MethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4>
struct ConstMethodSlot4_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4) const;
        ConstMethodSlotNode* os = (ConstMethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4>
Slot5<R,Obj&,P1,P2,P3,P4>
  slot(R (Obj::*method)(P1,P2,P3,P4))
  { 
    typedef MethodSlot4_<R,Obj,P1,P2,P3,P4> SType;
    return new MethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }

template <class R,class Obj,class P1,class P2,class P3,class P4>
Slot5<R,const Obj&,P1,P2,P3,P4>
  slot(R (Obj::*method)(P1,P2,P3,P4) const)
  {
    typedef ConstMethodSlot4_<R,Obj,P1,P2,P3,P4> SType;
    return new ConstMethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }


template <class R,class Obj,class P1,class P2,class P3,class P4,class P5>
struct MethodSlot5_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4,P5);
        MethodSlotNode* os = (MethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4,p5);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5>
struct ConstMethodSlot5_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4,P5) const;
        ConstMethodSlotNode* os = (ConstMethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4,p5);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5>
Slot6<R,Obj&,P1,P2,P3,P4,P5>
  slot(R (Obj::*method)(P1,P2,P3,P4,P5))
  { 
    typedef MethodSlot5_<R,Obj,P1,P2,P3,P4,P5> SType;
    return new MethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5>
Slot6<R,const Obj&,P1,P2,P3,P4,P5>
  slot(R (Obj::*method)(P1,P2,P3,P4,P5) const)
  {
    typedef ConstMethodSlot5_<R,Obj,P1,P2,P3,P4,P5> SType;
    return new ConstMethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }


template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6>
struct MethodSlot6_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4,P5,P6);
        MethodSlotNode* os = (MethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4,p5,p6);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6>
struct ConstMethodSlot6_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4,P5,P6) const;
        ConstMethodSlotNode* os = (ConstMethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4,p5,p6);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6>
Slot7<R,Obj&,P1,P2,P3,P4,P5,P6>
  slot(R (Obj::*method)(P1,P2,P3,P4,P5,P6))
  { 
    typedef MethodSlot6_<R,Obj,P1,P2,P3,P4,P5,P6> SType;
    return new MethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6>
Slot7<R,const Obj&,P1,P2,P3,P4,P5,P6>
  slot(R (Obj::*method)(P1,P2,P3,P4,P5,P6) const)
  {
    typedef ConstMethodSlot6_<R,Obj,P1,P2,P3,P4,P5,P6> SType;
    return new ConstMethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }


template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
struct MethodSlot7_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4,P5,P6,P7);
        MethodSlotNode* os = (MethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4,p5,p6,p7);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
struct ConstMethodSlot7_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4,P5,P6,P7) const;
        ConstMethodSlotNode* os = (ConstMethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4,p5,p6,p7);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
Slot8<R,Obj&,P1,P2,P3,P4,P5,P6,P7>
  slot(R (Obj::*method)(P1,P2,P3,P4,P5,P6,P7))
  { 
    typedef MethodSlot7_<R,Obj,P1,P2,P3,P4,P5,P6,P7> SType;
    return new MethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
Slot8<R,const Obj&,P1,P2,P3,P4,P5,P6,P7>
  slot(R (Obj::*method)(P1,P2,P3,P4,P5,P6,P7) const)
  {
    typedef ConstMethodSlot7_<R,Obj,P1,P2,P3,P4,P5,P6,P7> SType;
    return new ConstMethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }


template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8>
struct MethodSlot8_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4,P5,P6,P7,P8);
        MethodSlotNode* os = (MethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4,p5,p6,p7,p8);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8>
struct ConstMethodSlot8_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4,P5,P6,P7,P8) const;
        ConstMethodSlotNode* os = (ConstMethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4,p5,p6,p7,p8);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8>
Slot9<R,Obj&,P1,P2,P3,P4,P5,P6,P7,P8>
  slot(R (Obj::*method)(P1,P2,P3,P4,P5,P6,P7,P8))
  { 
    typedef MethodSlot8_<R,Obj,P1,P2,P3,P4,P5,P6,P7,P8> SType;
    return new MethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8>
Slot9<R,const Obj&,P1,P2,P3,P4,P5,P6,P7,P8>
  slot(R (Obj::*method)(P1,P2,P3,P4,P5,P6,P7,P8) const)
  {
    typedef ConstMethodSlot8_<R,Obj,P1,P2,P3,P4,P5,P6,P7,P8> SType;
    return new ConstMethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }


template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
struct MethodSlot9_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4,P5,P6,P7,P8,P9);
        MethodSlotNode* os = (MethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4,p5,p6,p7,p8,p9);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
struct ConstMethodSlot9_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4,P5,P6,P7,P8,P9) const;
        ConstMethodSlotNode* os = (ConstMethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4,p5,p6,p7,p8,p9);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
Slot10<R,Obj&,P1,P2,P3,P4,P5,P6,P7,P8,P9>
  slot(R (Obj::*method)(P1,P2,P3,P4,P5,P6,P7,P8,P9))
  { 
    typedef MethodSlot9_<R,Obj,P1,P2,P3,P4,P5,P6,P7,P8,P9> SType;
    return new MethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
Slot10<R,const Obj&,P1,P2,P3,P4,P5,P6,P7,P8,P9>
  slot(R (Obj::*method)(P1,P2,P3,P4,P5,P6,P7,P8,P9) const)
  {
    typedef ConstMethodSlot9_<R,Obj,P1,P2,P3,P4,P5,P6,P7,P8,P9> SType;
    return new ConstMethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }


template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
struct MethodSlot10_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10);
        MethodSlotNode* os = (MethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
struct ConstMethodSlot10_ 
  {
    typedef typename Trait<R>::type RType;
    static RType proxy(Obj& obj,typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,void * s) 
      { 
        typedef RType (Obj::*Method)(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10) const;
        ConstMethodSlotNode* os = (ConstMethodSlotNode*)s;
        return ((Obj*)(&obj)
           ->*(reinterpret_cast<Method&>(os->method_)))(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10);
      }
  };

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
Slot11<R,Obj&,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10>
  slot(R (Obj::*method)(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10))
  { 
    typedef MethodSlot10_<R,Obj,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10> SType;
    return new MethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }

template <class R,class Obj,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
Slot11<R,const Obj&,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10>
  slot(R (Obj::*method)(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10) const)
  {
    typedef ConstMethodSlot10_<R,Obj,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10> SType;
    return new ConstMethodSlotNode((FuncPtr)(&SType::proxy),
                            method);
  }



#ifdef SIGC_CXX_NAMESPACES
}
#endif


#endif // SIGC_SLOT
