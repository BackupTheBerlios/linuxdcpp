// -*- c++ -*-
//   Copyright 2000, Karl Einar Nelson
/* This is a generated file, do not edit.  Generated from template.macros.m4 */

#ifndef SIGC_SIGNAL_H
#define SIGC_SIGNAL_H
#include <sigc++/slot.h>
#include <sigc++/connection.h>
#include <sigc++/marshal.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

/** @defgroup Signals
 * Use connect() with SigC::slot to connect a method or function with a Signal.
 *
 * @code
 * signal_clicked.connect( SigC::slot(*this, &MyWindow::on_clicked) );
 * @endcode
 *
 * When the signal is emitted your method will be called.
 *
 * connect() returns a Connection, which you can later use to disconnect your method.
 *
 * When Signals are copied they share the underlying information,
 * so you can have a protected/private SigC::Signal member and a public accessor method.
 */

class SignalConnectionNode;
class SignalExec_;

class LIBSIGC_API SignalNode : public SlotNode
  {
    public:
      int exec_count_; // atomic
      SignalConnectionNode *begin_,*end_;

      SignalNode();
      ~SignalNode();

      // must be inline to avoid emission slowdowns.
      void exec_reference()
        { 
          reference();
          exec_count_ += 1;
        }

      // must be inline to avoid emission slowdowns.
      void exec_unreference()
        {
          exec_count_ -= 1;

          if (defered_ && !exec_count_)
            cleanup();

          unreference();
        }

      SlotNode* create_slot(FuncPtr proxy); // nothrow
      ConnectionNode* push_front(const SlotBase& s);
      ConnectionNode* push_back(const SlotBase& s);

      virtual void remove(SignalConnectionNode* c);
      bool empty();
      void clear();
      void cleanup(); // nothrow
  };

class LIBSIGC_API SignalBase
  {
      friend class SignalConnectionNode;
    private:
      SignalBase& operator= (const SignalBase&); // no copy

    protected:
      typedef SignalExec_ Exec;

      mutable SignalNode *impl_;

      SlotNode* create_slot(FuncPtr c) const
        { return impl()->create_slot(c); }

      ConnectionNode* push_front(const SlotBase& s)
        { return impl()->push_front(s); }

      ConnectionNode* push_back(const SlotBase& s)
        { return impl()->push_back(s); }

      SignalBase();
      SignalBase(const SignalBase& s);
      SignalBase(SignalNode* s);
      ~SignalBase();

    public:
      bool empty() const
        { return !impl_ || impl()->empty(); }

      void clear()
        {
          if(impl_)
            impl()->clear();
        }

      SignalNode* impl() const;
  };

class LIBSIGC_API SignalConnectionNode : public ConnectionNode
  {
    public:
      virtual void notify(bool from_child);

      virtual ~SignalConnectionNode();
      SignalConnectionNode(SlotNode*);
  
      SignalNode *parent_;
      SignalConnectionNode *next_,*prev_;
      
      SlotNode* dest() { return (SlotNode*)(slot().impl()); }
  };

// Exeception-safe class for tracking signals.
class LIBSIGC_API SignalExec_
  {
  public:
    SignalNode* signal_;

    SignalExec_(SignalNode* signal) :signal_(signal)
      { signal_->exec_reference(); }

    ~SignalExec_()
      { signal_->exec_unreference(); }
  };
    

/********************************************************/




/// @ingroup Signals
template <class R,class Marsh=Marshal<R> >
class Signal0 : public SignalBase
  {
    public:
      typedef Slot0<R> InSlotType;
      typedef Slot0<typename Marsh::OutType> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit()
        { return emit_(impl_); }

      /// See emit()
      OutType operator()()
        { return emit_(impl_); }
 
      Signal0() 
        : SignalBase() 
        {}

      Signal0(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal0() {}
  };


// emit
template <class R,class Marsh>
typename Signal0<R,Marsh>::OutType
Signal0<R,Marsh>::emit_(void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot0<R>::Proxy)(s->proxy_))(s)))
          return rc.value();
      }
    return rc.value();
  }



/// @ingroup Signals
template <class R,class P1,class Marsh=Marshal<R> >
class Signal1 : public SignalBase
  {
    public:
      typedef Slot1<R,P1> InSlotType;
      typedef Slot1<typename Marsh::OutType,P1> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1)
        { return emit_(p1,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1)
        { return emit_(p1,impl_); }
 
      Signal1() 
        : SignalBase() 
        {}

      Signal1(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal1() {}
  };


// emit
template <class R,class P1,class Marsh>
typename Signal1<R,P1,Marsh>::OutType
Signal1<R,P1,Marsh>::emit_(typename Trait<P1>::ref p1,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot1<R,P1>::Proxy)(s->proxy_))(p1,s)))
          return rc.value();
      }
    return rc.value();
  }



/// @ingroup Signals
template <class R,class P1,class P2,class Marsh=Marshal<R> >
class Signal2 : public SignalBase
  {
    public:
      typedef Slot2<R,P1,P2> InSlotType;
      typedef Slot2<typename Marsh::OutType,P1,P2> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2)
        { return emit_(p1,p2,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2)
        { return emit_(p1,p2,impl_); }
 
      Signal2() 
        : SignalBase() 
        {}

      Signal2(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal2() {}
  };


// emit
template <class R,class P1,class P2,class Marsh>
typename Signal2<R,P1,P2,Marsh>::OutType
Signal2<R,P1,P2,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot2<R,P1,P2>::Proxy)(s->proxy_))(p1,p2,s)))
          return rc.value();
      }
    return rc.value();
  }



/// @ingroup Signals
template <class R,class P1,class P2,class P3,class Marsh=Marshal<R> >
class Signal3 : public SignalBase
  {
    public:
      typedef Slot3<R,P1,P2,P3> InSlotType;
      typedef Slot3<typename Marsh::OutType,P1,P2,P3> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3)
        { return emit_(p1,p2,p3,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3)
        { return emit_(p1,p2,p3,impl_); }
 
      Signal3() 
        : SignalBase() 
        {}

      Signal3(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal3() {}
  };


// emit
template <class R,class P1,class P2,class P3,class Marsh>
typename Signal3<R,P1,P2,P3,Marsh>::OutType
Signal3<R,P1,P2,P3,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot3<R,P1,P2,P3>::Proxy)(s->proxy_))(p1,p2,p3,s)))
          return rc.value();
      }
    return rc.value();
  }



/// @ingroup Signals
template <class R,class P1,class P2,class P3,class P4,class Marsh=Marshal<R> >
class Signal4 : public SignalBase
  {
    public:
      typedef Slot4<R,P1,P2,P3,P4> InSlotType;
      typedef Slot4<typename Marsh::OutType,P1,P2,P3,P4> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4)
        { return emit_(p1,p2,p3,p4,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4)
        { return emit_(p1,p2,p3,p4,impl_); }
 
      Signal4() 
        : SignalBase() 
        {}

      Signal4(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal4() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class Marsh>
typename Signal4<R,P1,P2,P3,P4,Marsh>::OutType
Signal4<R,P1,P2,P3,P4,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot4<R,P1,P2,P3,P4>::Proxy)(s->proxy_))(p1,p2,p3,p4,s)))
          return rc.value();
      }
    return rc.value();
  }



/// @ingroup Signals
template <class R,class P1,class P2,class P3,class P4,class P5,class Marsh=Marshal<R> >
class Signal5 : public SignalBase
  {
    public:
      typedef Slot5<R,P1,P2,P3,P4,P5> InSlotType;
      typedef Slot5<typename Marsh::OutType,P1,P2,P3,P4,P5> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5)
        { return emit_(p1,p2,p3,p4,p5,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5)
        { return emit_(p1,p2,p3,p4,p5,impl_); }
 
      Signal5() 
        : SignalBase() 
        {}

      Signal5(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal5() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class P5,class Marsh>
typename Signal5<R,P1,P2,P3,P4,P5,Marsh>::OutType
Signal5<R,P1,P2,P3,P4,P5,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot5<R,P1,P2,P3,P4,P5>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,s)))
          return rc.value();
      }
    return rc.value();
  }



/// @ingroup Signals
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class Marsh=Marshal<R> >
class Signal6 : public SignalBase
  {
    public:
      typedef Slot6<R,P1,P2,P3,P4,P5,P6> InSlotType;
      typedef Slot6<typename Marsh::OutType,P1,P2,P3,P4,P5,P6> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
        { return emit_(p1,p2,p3,p4,p5,p6,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
        { return emit_(p1,p2,p3,p4,p5,p6,impl_); }
 
      Signal6() 
        : SignalBase() 
        {}

      Signal6(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal6() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class Marsh>
typename Signal6<R,P1,P2,P3,P4,P5,P6,Marsh>::OutType
Signal6<R,P1,P2,P3,P4,P5,P6,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot6<R,P1,P2,P3,P4,P5,P6>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,s)))
          return rc.value();
      }
    return rc.value();
  }



/// @ingroup Signals
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class Marsh=Marshal<R> >
class Signal7 : public SignalBase
  {
    public:
      typedef Slot7<R,P1,P2,P3,P4,P5,P6,P7> InSlotType;
      typedef Slot7<typename Marsh::OutType,P1,P2,P3,P4,P5,P6,P7> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7)
        { return emit_(p1,p2,p3,p4,p5,p6,p7,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7)
        { return emit_(p1,p2,p3,p4,p5,p6,p7,impl_); }
 
      Signal7() 
        : SignalBase() 
        {}

      Signal7(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal7() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class Marsh>
typename Signal7<R,P1,P2,P3,P4,P5,P6,P7,Marsh>::OutType
Signal7<R,P1,P2,P3,P4,P5,P6,P7,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot7<R,P1,P2,P3,P4,P5,P6,P7>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,p7,s)))
          return rc.value();
      }
    return rc.value();
  }



/// @ingroup Signals
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class Marsh=Marshal<R> >
class Signal8 : public SignalBase
  {
    public:
      typedef Slot8<R,P1,P2,P3,P4,P5,P6,P7,P8> InSlotType;
      typedef Slot8<typename Marsh::OutType,P1,P2,P3,P4,P5,P6,P7,P8> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8)
        { return emit_(p1,p2,p3,p4,p5,p6,p7,p8,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8)
        { return emit_(p1,p2,p3,p4,p5,p6,p7,p8,impl_); }
 
      Signal8() 
        : SignalBase() 
        {}

      Signal8(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal8() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class Marsh>
typename Signal8<R,P1,P2,P3,P4,P5,P6,P7,P8,Marsh>::OutType
Signal8<R,P1,P2,P3,P4,P5,P6,P7,P8,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot8<R,P1,P2,P3,P4,P5,P6,P7,P8>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,p7,p8,s)))
          return rc.value();
      }
    return rc.value();
  }



/// @ingroup Signals
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class Marsh=Marshal<R> >
class Signal9 : public SignalBase
  {
    public:
      typedef Slot9<R,P1,P2,P3,P4,P5,P6,P7,P8,P9> InSlotType;
      typedef Slot9<typename Marsh::OutType,P1,P2,P3,P4,P5,P6,P7,P8,P9> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9)
        { return emit_(p1,p2,p3,p4,p5,p6,p7,p8,p9,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9)
        { return emit_(p1,p2,p3,p4,p5,p6,p7,p8,p9,impl_); }
 
      Signal9() 
        : SignalBase() 
        {}

      Signal9(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal9() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class Marsh>
typename Signal9<R,P1,P2,P3,P4,P5,P6,P7,P8,P9,Marsh>::OutType
Signal9<R,P1,P2,P3,P4,P5,P6,P7,P8,P9,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot9<R,P1,P2,P3,P4,P5,P6,P7,P8,P9>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,p7,p8,p9,s)))
          return rc.value();
      }
    return rc.value();
  }



/// @ingroup Signals
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class Marsh=Marshal<R> >
class Signal10 : public SignalBase
  {
    public:
      typedef Slot10<R,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10> InSlotType;
      typedef Slot10<typename Marsh::OutType,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10)
        { return emit_(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10)
        { return emit_(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,impl_); }
 
      Signal10() 
        : SignalBase() 
        {}

      Signal10(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal10() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class Marsh>
typename Signal10<R,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,Marsh>::OutType
Signal10<R,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot10<R,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,s)))
          return rc.value();
      }
    return rc.value();
  }



/// @ingroup Signals
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class P11,class Marsh=Marshal<R> >
class Signal11 : public SignalBase
  {
    public:
      typedef Slot11<R,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11> InSlotType;
      typedef Slot11<typename Marsh::OutType,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,typename Trait<P11>::ref p11,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,typename Trait<P11>::ref p11)
        { return emit_(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,typename Trait<P11>::ref p11)
        { return emit_(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,impl_); }
 
      Signal11() 
        : SignalBase() 
        {}

      Signal11(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal11() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class P11,class Marsh>
typename Signal11<R,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,Marsh>::OutType
Signal11<R,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,typename Trait<P11>::ref p11,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot11<R,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,s)))
          return rc.value();
      }
    return rc.value();
  }



#ifdef SIGC_CXX_PARTIAL_SPEC

/// @ingroup Signals
template <class Marsh>
class Signal0<void,Marsh> : public SignalBase
  {
    public:
      typedef Slot0<void> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit()
        {  emit_(impl_); }

      /// See emit()
      OutType operator()()
        {  emit_(impl_); }
 
      Signal0() 
        : SignalBase() 
        {}

      Signal0(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal0() {}
  };


// emit
template <class Marsh>
void Signal0<void,Marsh>::emit_(void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot0<void>::Proxy)(s->proxy_))(s);
      }
    return;
  }



/// @ingroup Signals
template <class P1,class Marsh>
class Signal1<void,P1,Marsh> : public SignalBase
  {
    public:
      typedef Slot1<void,P1> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1)
        {  emit_(p1,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1)
        {  emit_(p1,impl_); }
 
      Signal1() 
        : SignalBase() 
        {}

      Signal1(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal1() {}
  };


// emit
template <class P1,class Marsh>
void Signal1<void,P1,Marsh>::emit_(typename Trait<P1>::ref p1,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot1<void,P1>::Proxy)(s->proxy_))(p1,s);
      }
    return;
  }



/// @ingroup Signals
template <class P1,class P2,class Marsh>
class Signal2<void,P1,P2,Marsh> : public SignalBase
  {
    public:
      typedef Slot2<void,P1,P2> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2)
        {  emit_(p1,p2,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2)
        {  emit_(p1,p2,impl_); }
 
      Signal2() 
        : SignalBase() 
        {}

      Signal2(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal2() {}
  };


// emit
template <class P1,class P2,class Marsh>
void Signal2<void,P1,P2,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot2<void,P1,P2>::Proxy)(s->proxy_))(p1,p2,s);
      }
    return;
  }



/// @ingroup Signals
template <class P1,class P2,class P3,class Marsh>
class Signal3<void,P1,P2,P3,Marsh> : public SignalBase
  {
    public:
      typedef Slot3<void,P1,P2,P3> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3)
        {  emit_(p1,p2,p3,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3)
        {  emit_(p1,p2,p3,impl_); }
 
      Signal3() 
        : SignalBase() 
        {}

      Signal3(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal3() {}
  };


// emit
template <class P1,class P2,class P3,class Marsh>
void Signal3<void,P1,P2,P3,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot3<void,P1,P2,P3>::Proxy)(s->proxy_))(p1,p2,p3,s);
      }
    return;
  }



/// @ingroup Signals
template <class P1,class P2,class P3,class P4,class Marsh>
class Signal4<void,P1,P2,P3,P4,Marsh> : public SignalBase
  {
    public:
      typedef Slot4<void,P1,P2,P3,P4> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4)
        {  emit_(p1,p2,p3,p4,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4)
        {  emit_(p1,p2,p3,p4,impl_); }
 
      Signal4() 
        : SignalBase() 
        {}

      Signal4(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal4() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class Marsh>
void Signal4<void,P1,P2,P3,P4,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot4<void,P1,P2,P3,P4>::Proxy)(s->proxy_))(p1,p2,p3,p4,s);
      }
    return;
  }



/// @ingroup Signals
template <class P1,class P2,class P3,class P4,class P5,class Marsh>
class Signal5<void,P1,P2,P3,P4,P5,Marsh> : public SignalBase
  {
    public:
      typedef Slot5<void,P1,P2,P3,P4,P5> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5)
        {  emit_(p1,p2,p3,p4,p5,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5)
        {  emit_(p1,p2,p3,p4,p5,impl_); }
 
      Signal5() 
        : SignalBase() 
        {}

      Signal5(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal5() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class P5,class Marsh>
void Signal5<void,P1,P2,P3,P4,P5,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot5<void,P1,P2,P3,P4,P5>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,s);
      }
    return;
  }



/// @ingroup Signals
template <class P1,class P2,class P3,class P4,class P5,class P6,class Marsh>
class Signal6<void,P1,P2,P3,P4,P5,P6,Marsh> : public SignalBase
  {
    public:
      typedef Slot6<void,P1,P2,P3,P4,P5,P6> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
        {  emit_(p1,p2,p3,p4,p5,p6,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
        {  emit_(p1,p2,p3,p4,p5,p6,impl_); }
 
      Signal6() 
        : SignalBase() 
        {}

      Signal6(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal6() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class P5,class P6,class Marsh>
void Signal6<void,P1,P2,P3,P4,P5,P6,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot6<void,P1,P2,P3,P4,P5,P6>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,s);
      }
    return;
  }



/// @ingroup Signals
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class Marsh>
class Signal7<void,P1,P2,P3,P4,P5,P6,P7,Marsh> : public SignalBase
  {
    public:
      typedef Slot7<void,P1,P2,P3,P4,P5,P6,P7> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7)
        {  emit_(p1,p2,p3,p4,p5,p6,p7,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7)
        {  emit_(p1,p2,p3,p4,p5,p6,p7,impl_); }
 
      Signal7() 
        : SignalBase() 
        {}

      Signal7(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal7() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class Marsh>
void Signal7<void,P1,P2,P3,P4,P5,P6,P7,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot7<void,P1,P2,P3,P4,P5,P6,P7>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,p7,s);
      }
    return;
  }



/// @ingroup Signals
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class Marsh>
class Signal8<void,P1,P2,P3,P4,P5,P6,P7,P8,Marsh> : public SignalBase
  {
    public:
      typedef Slot8<void,P1,P2,P3,P4,P5,P6,P7,P8> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8)
        {  emit_(p1,p2,p3,p4,p5,p6,p7,p8,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8)
        {  emit_(p1,p2,p3,p4,p5,p6,p7,p8,impl_); }
 
      Signal8() 
        : SignalBase() 
        {}

      Signal8(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal8() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class Marsh>
void Signal8<void,P1,P2,P3,P4,P5,P6,P7,P8,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot8<void,P1,P2,P3,P4,P5,P6,P7,P8>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,p7,p8,s);
      }
    return;
  }



/// @ingroup Signals
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class Marsh>
class Signal9<void,P1,P2,P3,P4,P5,P6,P7,P8,P9,Marsh> : public SignalBase
  {
    public:
      typedef Slot9<void,P1,P2,P3,P4,P5,P6,P7,P8,P9> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9)
        {  emit_(p1,p2,p3,p4,p5,p6,p7,p8,p9,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9)
        {  emit_(p1,p2,p3,p4,p5,p6,p7,p8,p9,impl_); }
 
      Signal9() 
        : SignalBase() 
        {}

      Signal9(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal9() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class Marsh>
void Signal9<void,P1,P2,P3,P4,P5,P6,P7,P8,P9,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot9<void,P1,P2,P3,P4,P5,P6,P7,P8,P9>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,p7,p8,p9,s);
      }
    return;
  }



/// @ingroup Signals
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class Marsh>
class Signal10<void,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,Marsh> : public SignalBase
  {
    public:
      typedef Slot10<void,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10)
        {  emit_(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10)
        {  emit_(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,impl_); }
 
      Signal10() 
        : SignalBase() 
        {}

      Signal10(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal10() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class Marsh>
void Signal10<void,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot10<void,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,s);
      }
    return;
  }



/// @ingroup Signals
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class P11,class Marsh>
class Signal11<void,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,Marsh> : public SignalBase
  {
    public:
      typedef Slot11<void,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,typename Trait<P11>::ref p11,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,typename Trait<P11>::ref p11)
        {  emit_(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,typename Trait<P11>::ref p11)
        {  emit_(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,impl_); }
 
      Signal11() 
        : SignalBase() 
        {}

      Signal11(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal11() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class P11,class Marsh>
void Signal11<void,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,Marsh>::emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7,typename Trait<P8>::ref p8,typename Trait<P9>::ref p9,typename Trait<P10>::ref p10,typename Trait<P11>::ref p11,void* data)
  {
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot11<void,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,s);
      }
    return;
  }


#endif


#ifdef SIGC_CXX_NAMESPACES
}
#endif

#endif // SIGC_SIGNAL_H
