/* This is for emacs: -*-Mode: C++;-*- */
/* Copyright 2002, Andreas Rottmann */
/* This is a generated file, do not edit.  Generated from template.macros.m4 */

#ifndef SIGC_TUNNEL_H
#define SIGC_TUNNEL_H

#include <sigc++/slot.h>
#include <sigc++/object_slot.h>

#include <sigcx/bind3.h>




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






/****************************************************************
*****  Tunnel 0
****************************************************************/


/** Tunnel callback for 0 arguments. */
template <class R>
class TunnelCallback0 : public Tunnel::Callback
{
  public:
    TunnelCallback0(const SigC::Slot0<R>& slot)
: slot_(slot) {}
    virtual void invoke() { rv_ = slot_(); }
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
	R tmp = slot_(); 
	delete this;
        return tmp;
      }
    }
    static R pack_n_tunnel(SigC::Slot0<R> s,Tunnel *tunnel,bool sync) {
    
      TunnelCallback0<R> *cb = pack(s);
      return cb->tunnel(tunnel, sync);
    }
  protected:
    SigC::Slot0<R> slot_;
    typename TunnelTrait<R>::type rv_;
    
};


/** Pack  arguments in a callback.
 * \param s A slot.
 * 
 * \return A pointer to a callback corresponding to \a s with all 
 *   the parameters specified bound to it.
 */
template <class R>
   TunnelCallback0<R> *pack(const SigC::Slot0<R>& s)
  {       return new TunnelCallback0<R>(s);
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <>
class TunnelCallback0<void> : public Tunnel::Callback
{
  public:
    TunnelCallback0(SigC::Slot0<void> slot)
: slot_(slot) { }
    virtual void invoke() { slot_(); }
    void tunnel(Tunnel *tunnel, bool sync = false) {
      if (tunnel) {
        tunnel->send(this, sync);
        if (sync)
          delete this;
      }
      else {
        slot_();
	delete this;
      }
    }
    static void pack_n_tunnel(SigC::Slot0<void> s,Tunnel *tunnel,bool sync) {

      TunnelCallback0<void> *cb = pack(s);
      cb->tunnel(tunnel, sync);
   }
  protected:
    SigC::Slot0<void> slot_;
    
};
#endif

/** Create a slot using a tunnel.
 * \param tunnel Tunnel to use.
 * \param s Slot to invoke on the other tunnel side.
 * \param sync Wether to invoke the callback synchronously.
 * \return The tunneled version of slot \a s. */
template <class R>
SigC::Slot0<R>
  open_tunnel(Tunnel *tunnel, const SigC::Slot0<R>& s, bool sync = false)
  { return SigCX::bind(SigC::slot(&TunnelCallback0<R>::pack_n_tunnel), s, tunnel, sync);
  }


template <class R>
R tunnel(SigC::Slot0<R> s,Tunnel *tunnel,bool sync = false) 
{
  return tunnel ? TunnelCallback0<R>::pack_n_tunnel(s,tunnel,sync) : s();
}


/****************************************************************
*****  Tunnel 1
****************************************************************/


/** Tunnel callback for 1 arguments. */
template <class R,class P1>
class TunnelCallback1 : public Tunnel::Callback
{
  public:
    TunnelCallback1(const SigC::Slot1<R,P1>& slot,P1 p1)
: slot_(slot),p1_(p1) {}
    virtual void invoke() { rv_ = slot_(p1_); }
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
	R tmp = slot_(p1_); 
	delete this;
        return tmp;
      }
    }
    static R pack_n_tunnel(P1 p1,SigC::Slot1<R,P1> s,Tunnel *tunnel,bool sync) {
    TunnelCallback1<R,P1> *cb = pack<R,P1>(s,p1);
      return cb->tunnel(tunnel, sync);
    }
  protected:
    SigC::Slot1<R,P1> slot_;
    typename TunnelTrait<R>::type rv_;
    typename TunnelTrait<P1>::type p1_;
};


/** Pack 1 arguments in a callback.
 * \param s A slot.
 * \param p1 Slot argument 1. 
 * \return A pointer to a callback corresponding to \a s with all 
 *   the parameters specified bound to it.
 */
template <class R,class P1>
   TunnelCallback1<R,P1> *pack(const SigC::Slot1<R,P1>& s,P1 p1)
  {       return new TunnelCallback1<R,P1>(s,p1);
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1>
class TunnelCallback1<void,P1> : public Tunnel::Callback
{
  public:
    TunnelCallback1(SigC::Slot1<void,P1> slot,P1 p1)
: slot_(slot),p1_(p1) { }
    virtual void invoke() { slot_(p1_); }
    void tunnel(Tunnel *tunnel, bool sync = false) {
      if (tunnel) {
        tunnel->send(this, sync);
        if (sync)
          delete this;
      }
      else {
        slot_(p1_);
	delete this;
      }
    }
    static void pack_n_tunnel(P1 p1,SigC::Slot1<void,P1> s,Tunnel *tunnel,bool sync) {
TunnelCallback1<void,P1> *cb = pack<void,P1>(s,p1);
      cb->tunnel(tunnel, sync);
   }
  protected:
    SigC::Slot1<void,P1> slot_;
    typename TunnelTrait<P1>::type p1_;
};
#endif

/** Create a slot using a tunnel.
 * \param tunnel Tunnel to use.
 * \param s Slot to invoke on the other tunnel side.
 * \param sync Wether to invoke the callback synchronously.
 * \return The tunneled version of slot \a s. */
template <class R,class P1>
SigC::Slot1<R,P1>
  open_tunnel(Tunnel *tunnel, const SigC::Slot1<R,P1>& s, bool sync = false)
  { return SigCX::bind(SigC::slot(&TunnelCallback1<R,P1>::pack_n_tunnel), s, tunnel, sync);
  }


template <class R,class P1>
R tunnel(SigC::Slot1<R,P1> s,P1 p1,Tunnel *tunnel,bool sync = false) 
{
  return tunnel ? TunnelCallback1<R,P1>::pack_n_tunnel(p1,s,tunnel,sync) : s(p1);
}


/****************************************************************
*****  Tunnel 2
****************************************************************/


/** Tunnel callback for 2 arguments. */
template <class R,class P1,class P2>
class TunnelCallback2 : public Tunnel::Callback
{
  public:
    TunnelCallback2(const SigC::Slot2<R,P1,P2>& slot,P1 p1,P2 p2)
: slot_(slot),p1_(p1),p2_(p2) {}
    virtual void invoke() { rv_ = slot_(p1_,p2_); }
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
	R tmp = slot_(p1_,p2_); 
	delete this;
        return tmp;
      }
    }
    static R pack_n_tunnel(P1 p1,P2 p2,SigC::Slot2<R,P1,P2> s,Tunnel *tunnel,bool sync) {
    TunnelCallback2<R,P1,P2> *cb = pack<R,P1,P2>(s,p1,p2);
      return cb->tunnel(tunnel, sync);
    }
  protected:
    SigC::Slot2<R,P1,P2> slot_;
    typename TunnelTrait<R>::type rv_;
    typename TunnelTrait<P1>::type p1_; typename TunnelTrait<P2>::type p2_;
};


/** Pack 2 arguments in a callback.
 * \param s A slot.
 * \param p1 Slot argument 1. \param p2 Slot argument 2. 
 * \return A pointer to a callback corresponding to \a s with all 
 *   the parameters specified bound to it.
 */
template <class R,class P1,class P2>
   TunnelCallback2<R,P1,P2> *pack(const SigC::Slot2<R,P1,P2>& s,P1 p1,P2 p2)
  {       return new TunnelCallback2<R,P1,P2>(s,p1,p2);
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2>
class TunnelCallback2<void,P1,P2> : public Tunnel::Callback
{
  public:
    TunnelCallback2(SigC::Slot2<void,P1,P2> slot,P1 p1,P2 p2)
: slot_(slot),p1_(p1),p2_(p2) { }
    virtual void invoke() { slot_(p1_,p2_); }
    void tunnel(Tunnel *tunnel, bool sync = false) {
      if (tunnel) {
        tunnel->send(this, sync);
        if (sync)
          delete this;
      }
      else {
        slot_(p1_,p2_);
	delete this;
      }
    }
    static void pack_n_tunnel(P1 p1,P2 p2,SigC::Slot2<void,P1,P2> s,Tunnel *tunnel,bool sync) {
TunnelCallback2<void,P1,P2> *cb = pack<void,P1,P2>(s,p1,p2);
      cb->tunnel(tunnel, sync);
   }
  protected:
    SigC::Slot2<void,P1,P2> slot_;
    typename TunnelTrait<P1>::type p1_; typename TunnelTrait<P2>::type p2_;
};
#endif

/** Create a slot using a tunnel.
 * \param tunnel Tunnel to use.
 * \param s Slot to invoke on the other tunnel side.
 * \param sync Wether to invoke the callback synchronously.
 * \return The tunneled version of slot \a s. */
template <class R,class P1,class P2>
SigC::Slot2<R,P1,P2>
  open_tunnel(Tunnel *tunnel, const SigC::Slot2<R,P1,P2>& s, bool sync = false)
  { return SigCX::bind(SigC::slot(&TunnelCallback2<R,P1,P2>::pack_n_tunnel), s, tunnel, sync);
  }


template <class R,class P1,class P2>
R tunnel(SigC::Slot2<R,P1,P2> s,P1 p1,P2 p2,Tunnel *tunnel,bool sync = false) 
{
  return tunnel ? TunnelCallback2<R,P1,P2>::pack_n_tunnel(p1,p2,s,tunnel,sync) : s(p1,p2);
}


/****************************************************************
*****  Tunnel 3
****************************************************************/


/** Tunnel callback for 3 arguments. */
template <class R,class P1,class P2,class P3>
class TunnelCallback3 : public Tunnel::Callback
{
  public:
    TunnelCallback3(const SigC::Slot3<R,P1,P2,P3>& slot,P1 p1,P2 p2,P3 p3)
: slot_(slot),p1_(p1),p2_(p2),p3_(p3) {}
    virtual void invoke() { rv_ = slot_(p1_,p2_,p3_); }
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
	R tmp = slot_(p1_,p2_,p3_); 
	delete this;
        return tmp;
      }
    }
    static R pack_n_tunnel(P1 p1,P2 p2,P3 p3,SigC::Slot3<R,P1,P2,P3> s,Tunnel *tunnel,bool sync) {
    TunnelCallback3<R,P1,P2,P3> *cb = pack<R,P1,P2,P3>(s,p1,p2,p3);
      return cb->tunnel(tunnel, sync);
    }
  protected:
    SigC::Slot3<R,P1,P2,P3> slot_;
    typename TunnelTrait<R>::type rv_;
    typename TunnelTrait<P1>::type p1_; typename TunnelTrait<P2>::type p2_; typename TunnelTrait<P3>::type p3_;
};


/** Pack 3 arguments in a callback.
 * \param s A slot.
 * \param p1 Slot argument 1. \param p2 Slot argument 2. \param p3 Slot argument 3. 
 * \return A pointer to a callback corresponding to \a s with all 
 *   the parameters specified bound to it.
 */
template <class R,class P1,class P2,class P3>
   TunnelCallback3<R,P1,P2,P3> *pack(const SigC::Slot3<R,P1,P2,P3>& s,P1 p1,P2 p2,P3 p3)
  {       return new TunnelCallback3<R,P1,P2,P3>(s,p1,p2,p3);
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3>
class TunnelCallback3<void,P1,P2,P3> : public Tunnel::Callback
{
  public:
    TunnelCallback3(SigC::Slot3<void,P1,P2,P3> slot,P1 p1,P2 p2,P3 p3)
: slot_(slot),p1_(p1),p2_(p2),p3_(p3) { }
    virtual void invoke() { slot_(p1_,p2_,p3_); }
    void tunnel(Tunnel *tunnel, bool sync = false) {
      if (tunnel) {
        tunnel->send(this, sync);
        if (sync)
          delete this;
      }
      else {
        slot_(p1_,p2_,p3_);
	delete this;
      }
    }
    static void pack_n_tunnel(P1 p1,P2 p2,P3 p3,SigC::Slot3<void,P1,P2,P3> s,Tunnel *tunnel,bool sync) {
TunnelCallback3<void,P1,P2,P3> *cb = pack<void,P1,P2,P3>(s,p1,p2,p3);
      cb->tunnel(tunnel, sync);
   }
  protected:
    SigC::Slot3<void,P1,P2,P3> slot_;
    typename TunnelTrait<P1>::type p1_; typename TunnelTrait<P2>::type p2_; typename TunnelTrait<P3>::type p3_;
};
#endif

/** Create a slot using a tunnel.
 * \param tunnel Tunnel to use.
 * \param s Slot to invoke on the other tunnel side.
 * \param sync Wether to invoke the callback synchronously.
 * \return The tunneled version of slot \a s. */
template <class R,class P1,class P2,class P3>
SigC::Slot3<R,P1,P2,P3>
  open_tunnel(Tunnel *tunnel, const SigC::Slot3<R,P1,P2,P3>& s, bool sync = false)
  { return SigCX::bind(SigC::slot(&TunnelCallback3<R,P1,P2,P3>::pack_n_tunnel), s, tunnel, sync);
  }


template <class R,class P1,class P2,class P3>
R tunnel(SigC::Slot3<R,P1,P2,P3> s,P1 p1,P2 p2,P3 p3,Tunnel *tunnel,bool sync = false) 
{
  return tunnel ? TunnelCallback3<R,P1,P2,P3>::pack_n_tunnel(p1,p2,p3,s,tunnel,sync) : s(p1,p2,p3);
}


/****************************************************************
*****  Tunnel 4
****************************************************************/


/** Tunnel callback for 4 arguments. */
template <class R,class P1,class P2,class P3,class P4>
class TunnelCallback4 : public Tunnel::Callback
{
  public:
    TunnelCallback4(const SigC::Slot4<R,P1,P2,P3,P4>& slot,P1 p1,P2 p2,P3 p3,P4 p4)
: slot_(slot),p1_(p1),p2_(p2),p3_(p3),p4_(p4) {}
    virtual void invoke() { rv_ = slot_(p1_,p2_,p3_,p4_); }
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
	R tmp = slot_(p1_,p2_,p3_,p4_); 
	delete this;
        return tmp;
      }
    }
    static R pack_n_tunnel(P1 p1,P2 p2,P3 p3,P4 p4,SigC::Slot4<R,P1,P2,P3,P4> s,Tunnel *tunnel,bool sync) {
    TunnelCallback4<R,P1,P2,P3,P4> *cb = pack<R,P1,P2,P3,P4>(s,p1,p2,p3,p4);
      return cb->tunnel(tunnel, sync);
    }
  protected:
    SigC::Slot4<R,P1,P2,P3,P4> slot_;
    typename TunnelTrait<R>::type rv_;
    typename TunnelTrait<P1>::type p1_; typename TunnelTrait<P2>::type p2_; typename TunnelTrait<P3>::type p3_; typename TunnelTrait<P4>::type p4_;
};


/** Pack 4 arguments in a callback.
 * \param s A slot.
 * \param p1 Slot argument 1. \param p2 Slot argument 2. \param p3 Slot argument 3. \param p4 Slot argument 4. 
 * \return A pointer to a callback corresponding to \a s with all 
 *   the parameters specified bound to it.
 */
template <class R,class P1,class P2,class P3,class P4>
   TunnelCallback4<R,P1,P2,P3,P4> *pack(const SigC::Slot4<R,P1,P2,P3,P4>& s,P1 p1,P2 p2,P3 p3,P4 p4)
  {       return new TunnelCallback4<R,P1,P2,P3,P4>(s,p1,p2,p3,p4);
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4>
class TunnelCallback4<void,P1,P2,P3,P4> : public Tunnel::Callback
{
  public:
    TunnelCallback4(SigC::Slot4<void,P1,P2,P3,P4> slot,P1 p1,P2 p2,P3 p3,P4 p4)
: slot_(slot),p1_(p1),p2_(p2),p3_(p3),p4_(p4) { }
    virtual void invoke() { slot_(p1_,p2_,p3_,p4_); }
    void tunnel(Tunnel *tunnel, bool sync = false) {
      if (tunnel) {
        tunnel->send(this, sync);
        if (sync)
          delete this;
      }
      else {
        slot_(p1_,p2_,p3_,p4_);
	delete this;
      }
    }
    static void pack_n_tunnel(P1 p1,P2 p2,P3 p3,P4 p4,SigC::Slot4<void,P1,P2,P3,P4> s,Tunnel *tunnel,bool sync) {
TunnelCallback4<void,P1,P2,P3,P4> *cb = pack<void,P1,P2,P3,P4>(s,p1,p2,p3,p4);
      cb->tunnel(tunnel, sync);
   }
  protected:
    SigC::Slot4<void,P1,P2,P3,P4> slot_;
    typename TunnelTrait<P1>::type p1_; typename TunnelTrait<P2>::type p2_; typename TunnelTrait<P3>::type p3_; typename TunnelTrait<P4>::type p4_;
};
#endif

/** Create a slot using a tunnel.
 * \param tunnel Tunnel to use.
 * \param s Slot to invoke on the other tunnel side.
 * \param sync Wether to invoke the callback synchronously.
 * \return The tunneled version of slot \a s. */
template <class R,class P1,class P2,class P3,class P4>
SigC::Slot4<R,P1,P2,P3,P4>
  open_tunnel(Tunnel *tunnel, const SigC::Slot4<R,P1,P2,P3,P4>& s, bool sync = false)
  { return SigCX::bind(SigC::slot(&TunnelCallback4<R,P1,P2,P3,P4>::pack_n_tunnel), s, tunnel, sync);
  }


template <class R,class P1,class P2,class P3,class P4>
R tunnel(SigC::Slot4<R,P1,P2,P3,P4> s,P1 p1,P2 p2,P3 p3,P4 p4,Tunnel *tunnel,bool sync = false) 
{
  return tunnel ? TunnelCallback4<R,P1,P2,P3,P4>::pack_n_tunnel(p1,p2,p3,p4,s,tunnel,sync) : s(p1,p2,p3,p4);
}


/****************************************************************
*****  Tunnel 5
****************************************************************/


/** Tunnel callback for 5 arguments. */
template <class R,class P1,class P2,class P3,class P4,class P5>
class TunnelCallback5 : public Tunnel::Callback
{
  public:
    TunnelCallback5(const SigC::Slot5<R,P1,P2,P3,P4,P5>& slot,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
: slot_(slot),p1_(p1),p2_(p2),p3_(p3),p4_(p4),p5_(p5) {}
    virtual void invoke() { rv_ = slot_(p1_,p2_,p3_,p4_,p5_); }
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
	R tmp = slot_(p1_,p2_,p3_,p4_,p5_); 
	delete this;
        return tmp;
      }
    }
    static R pack_n_tunnel(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,SigC::Slot5<R,P1,P2,P3,P4,P5> s,Tunnel *tunnel,bool sync) {
    TunnelCallback5<R,P1,P2,P3,P4,P5> *cb = pack<R,P1,P2,P3,P4,P5>(s,p1,p2,p3,p4,p5);
      return cb->tunnel(tunnel, sync);
    }
  protected:
    SigC::Slot5<R,P1,P2,P3,P4,P5> slot_;
    typename TunnelTrait<R>::type rv_;
    typename TunnelTrait<P1>::type p1_; typename TunnelTrait<P2>::type p2_; typename TunnelTrait<P3>::type p3_; typename TunnelTrait<P4>::type p4_; typename TunnelTrait<P5>::type p5_;
};


/** Pack 5 arguments in a callback.
 * \param s A slot.
 * \param p1 Slot argument 1. \param p2 Slot argument 2. \param p3 Slot argument 3. \param p4 Slot argument 4. \param p5 Slot argument 5. 
 * \return A pointer to a callback corresponding to \a s with all 
 *   the parameters specified bound to it.
 */
template <class R,class P1,class P2,class P3,class P4,class P5>
   TunnelCallback5<R,P1,P2,P3,P4,P5> *pack(const SigC::Slot5<R,P1,P2,P3,P4,P5>& s,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
  {       return new TunnelCallback5<R,P1,P2,P3,P4,P5>(s,p1,p2,p3,p4,p5);
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5>
class TunnelCallback5<void,P1,P2,P3,P4,P5> : public Tunnel::Callback
{
  public:
    TunnelCallback5(SigC::Slot5<void,P1,P2,P3,P4,P5> slot,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
: slot_(slot),p1_(p1),p2_(p2),p3_(p3),p4_(p4),p5_(p5) { }
    virtual void invoke() { slot_(p1_,p2_,p3_,p4_,p5_); }
    void tunnel(Tunnel *tunnel, bool sync = false) {
      if (tunnel) {
        tunnel->send(this, sync);
        if (sync)
          delete this;
      }
      else {
        slot_(p1_,p2_,p3_,p4_,p5_);
	delete this;
      }
    }
    static void pack_n_tunnel(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,SigC::Slot5<void,P1,P2,P3,P4,P5> s,Tunnel *tunnel,bool sync) {
TunnelCallback5<void,P1,P2,P3,P4,P5> *cb = pack<void,P1,P2,P3,P4,P5>(s,p1,p2,p3,p4,p5);
      cb->tunnel(tunnel, sync);
   }
  protected:
    SigC::Slot5<void,P1,P2,P3,P4,P5> slot_;
    typename TunnelTrait<P1>::type p1_; typename TunnelTrait<P2>::type p2_; typename TunnelTrait<P3>::type p3_; typename TunnelTrait<P4>::type p4_; typename TunnelTrait<P5>::type p5_;
};
#endif

/** Create a slot using a tunnel.
 * \param tunnel Tunnel to use.
 * \param s Slot to invoke on the other tunnel side.
 * \param sync Wether to invoke the callback synchronously.
 * \return The tunneled version of slot \a s. */
template <class R,class P1,class P2,class P3,class P4,class P5>
SigC::Slot5<R,P1,P2,P3,P4,P5>
  open_tunnel(Tunnel *tunnel, const SigC::Slot5<R,P1,P2,P3,P4,P5>& s, bool sync = false)
  { return SigCX::bind(SigC::slot(&TunnelCallback5<R,P1,P2,P3,P4,P5>::pack_n_tunnel), s, tunnel, sync);
  }


template <class R,class P1,class P2,class P3,class P4,class P5>
R tunnel(SigC::Slot5<R,P1,P2,P3,P4,P5> s,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,Tunnel *tunnel,bool sync = false) 
{
  return tunnel ? TunnelCallback5<R,P1,P2,P3,P4,P5>::pack_n_tunnel(p1,p2,p3,p4,p5,s,tunnel,sync) : s(p1,p2,p3,p4,p5);
}


/****************************************************************
*****  Tunnel 6
****************************************************************/


/** Tunnel callback for 6 arguments. */
template <class R,class P1,class P2,class P3,class P4,class P5,class P6>
class TunnelCallback6 : public Tunnel::Callback
{
  public:
    TunnelCallback6(const SigC::Slot6<R,P1,P2,P3,P4,P5,P6>& slot,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
: slot_(slot),p1_(p1),p2_(p2),p3_(p3),p4_(p4),p5_(p5),p6_(p6) {}
    virtual void invoke() { rv_ = slot_(p1_,p2_,p3_,p4_,p5_,p6_); }
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
	R tmp = slot_(p1_,p2_,p3_,p4_,p5_,p6_); 
	delete this;
        return tmp;
      }
    }
    static R pack_n_tunnel(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,SigC::Slot6<R,P1,P2,P3,P4,P5,P6> s,Tunnel *tunnel,bool sync) {
    TunnelCallback6<R,P1,P2,P3,P4,P5,P6> *cb = pack<R,P1,P2,P3,P4,P5,P6>(s,p1,p2,p3,p4,p5,p6);
      return cb->tunnel(tunnel, sync);
    }
  protected:
    SigC::Slot6<R,P1,P2,P3,P4,P5,P6> slot_;
    typename TunnelTrait<R>::type rv_;
    typename TunnelTrait<P1>::type p1_; typename TunnelTrait<P2>::type p2_; typename TunnelTrait<P3>::type p3_; typename TunnelTrait<P4>::type p4_; typename TunnelTrait<P5>::type p5_; typename TunnelTrait<P6>::type p6_;
};


/** Pack 6 arguments in a callback.
 * \param s A slot.
 * \param p1 Slot argument 1. \param p2 Slot argument 2. \param p3 Slot argument 3. \param p4 Slot argument 4. \param p5 Slot argument 5. \param p6 Slot argument 6. 
 * \return A pointer to a callback corresponding to \a s with all 
 *   the parameters specified bound to it.
 */
template <class R,class P1,class P2,class P3,class P4,class P5,class P6>
   TunnelCallback6<R,P1,P2,P3,P4,P5,P6> *pack(const SigC::Slot6<R,P1,P2,P3,P4,P5,P6>& s,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
  {       return new TunnelCallback6<R,P1,P2,P3,P4,P5,P6>(s,p1,p2,p3,p4,p5,p6);
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class P6>
class TunnelCallback6<void,P1,P2,P3,P4,P5,P6> : public Tunnel::Callback
{
  public:
    TunnelCallback6(SigC::Slot6<void,P1,P2,P3,P4,P5,P6> slot,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
: slot_(slot),p1_(p1),p2_(p2),p3_(p3),p4_(p4),p5_(p5),p6_(p6) { }
    virtual void invoke() { slot_(p1_,p2_,p3_,p4_,p5_,p6_); }
    void tunnel(Tunnel *tunnel, bool sync = false) {
      if (tunnel) {
        tunnel->send(this, sync);
        if (sync)
          delete this;
      }
      else {
        slot_(p1_,p2_,p3_,p4_,p5_,p6_);
	delete this;
      }
    }
    static void pack_n_tunnel(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,SigC::Slot6<void,P1,P2,P3,P4,P5,P6> s,Tunnel *tunnel,bool sync) {
TunnelCallback6<void,P1,P2,P3,P4,P5,P6> *cb = pack<void,P1,P2,P3,P4,P5,P6>(s,p1,p2,p3,p4,p5,p6);
      cb->tunnel(tunnel, sync);
   }
  protected:
    SigC::Slot6<void,P1,P2,P3,P4,P5,P6> slot_;
    typename TunnelTrait<P1>::type p1_; typename TunnelTrait<P2>::type p2_; typename TunnelTrait<P3>::type p3_; typename TunnelTrait<P4>::type p4_; typename TunnelTrait<P5>::type p5_; typename TunnelTrait<P6>::type p6_;
};
#endif

/** Create a slot using a tunnel.
 * \param tunnel Tunnel to use.
 * \param s Slot to invoke on the other tunnel side.
 * \param sync Wether to invoke the callback synchronously.
 * \return The tunneled version of slot \a s. */
template <class R,class P1,class P2,class P3,class P4,class P5,class P6>
SigC::Slot6<R,P1,P2,P3,P4,P5,P6>
  open_tunnel(Tunnel *tunnel, const SigC::Slot6<R,P1,P2,P3,P4,P5,P6>& s, bool sync = false)
  { return SigCX::bind(SigC::slot(&TunnelCallback6<R,P1,P2,P3,P4,P5,P6>::pack_n_tunnel), s, tunnel, sync);
  }


template <class R,class P1,class P2,class P3,class P4,class P5,class P6>
R tunnel(SigC::Slot6<R,P1,P2,P3,P4,P5,P6> s,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,Tunnel *tunnel,bool sync = false) 
{
  return tunnel ? TunnelCallback6<R,P1,P2,P3,P4,P5,P6>::pack_n_tunnel(p1,p2,p3,p4,p5,p6,s,tunnel,sync) : s(p1,p2,p3,p4,p5,p6);
}


/****************************************************************
*****  Tunnel 7
****************************************************************/


/** Tunnel callback for 7 arguments. */
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
class TunnelCallback7 : public Tunnel::Callback
{
  public:
    TunnelCallback7(const SigC::Slot7<R,P1,P2,P3,P4,P5,P6,P7>& slot,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
: slot_(slot),p1_(p1),p2_(p2),p3_(p3),p4_(p4),p5_(p5),p6_(p6),p7_(p7) {}
    virtual void invoke() { rv_ = slot_(p1_,p2_,p3_,p4_,p5_,p6_,p7_); }
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
	R tmp = slot_(p1_,p2_,p3_,p4_,p5_,p6_,p7_); 
	delete this;
        return tmp;
      }
    }
    static R pack_n_tunnel(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,SigC::Slot7<R,P1,P2,P3,P4,P5,P6,P7> s,Tunnel *tunnel,bool sync) {
    TunnelCallback7<R,P1,P2,P3,P4,P5,P6,P7> *cb = pack<R,P1,P2,P3,P4,P5,P6,P7>(s,p1,p2,p3,p4,p5,p6,p7);
      return cb->tunnel(tunnel, sync);
    }
  protected:
    SigC::Slot7<R,P1,P2,P3,P4,P5,P6,P7> slot_;
    typename TunnelTrait<R>::type rv_;
    typename TunnelTrait<P1>::type p1_; typename TunnelTrait<P2>::type p2_; typename TunnelTrait<P3>::type p3_; typename TunnelTrait<P4>::type p4_; typename TunnelTrait<P5>::type p5_; typename TunnelTrait<P6>::type p6_; typename TunnelTrait<P7>::type p7_;
};


/** Pack 7 arguments in a callback.
 * \param s A slot.
 * \param p1 Slot argument 1. \param p2 Slot argument 2. \param p3 Slot argument 3. \param p4 Slot argument 4. \param p5 Slot argument 5. \param p6 Slot argument 6. \param p7 Slot argument 7. 
 * \return A pointer to a callback corresponding to \a s with all 
 *   the parameters specified bound to it.
 */
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
   TunnelCallback7<R,P1,P2,P3,P4,P5,P6,P7> *pack(const SigC::Slot7<R,P1,P2,P3,P4,P5,P6,P7>& s,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
  {       return new TunnelCallback7<R,P1,P2,P3,P4,P5,P6,P7>(s,p1,p2,p3,p4,p5,p6,p7);
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7>
class TunnelCallback7<void,P1,P2,P3,P4,P5,P6,P7> : public Tunnel::Callback
{
  public:
    TunnelCallback7(SigC::Slot7<void,P1,P2,P3,P4,P5,P6,P7> slot,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
: slot_(slot),p1_(p1),p2_(p2),p3_(p3),p4_(p4),p5_(p5),p6_(p6),p7_(p7) { }
    virtual void invoke() { slot_(p1_,p2_,p3_,p4_,p5_,p6_,p7_); }
    void tunnel(Tunnel *tunnel, bool sync = false) {
      if (tunnel) {
        tunnel->send(this, sync);
        if (sync)
          delete this;
      }
      else {
        slot_(p1_,p2_,p3_,p4_,p5_,p6_,p7_);
	delete this;
      }
    }
    static void pack_n_tunnel(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,SigC::Slot7<void,P1,P2,P3,P4,P5,P6,P7> s,Tunnel *tunnel,bool sync) {
TunnelCallback7<void,P1,P2,P3,P4,P5,P6,P7> *cb = pack<void,P1,P2,P3,P4,P5,P6,P7>(s,p1,p2,p3,p4,p5,p6,p7);
      cb->tunnel(tunnel, sync);
   }
  protected:
    SigC::Slot7<void,P1,P2,P3,P4,P5,P6,P7> slot_;
    typename TunnelTrait<P1>::type p1_; typename TunnelTrait<P2>::type p2_; typename TunnelTrait<P3>::type p3_; typename TunnelTrait<P4>::type p4_; typename TunnelTrait<P5>::type p5_; typename TunnelTrait<P6>::type p6_; typename TunnelTrait<P7>::type p7_;
};
#endif

/** Create a slot using a tunnel.
 * \param tunnel Tunnel to use.
 * \param s Slot to invoke on the other tunnel side.
 * \param sync Wether to invoke the callback synchronously.
 * \return The tunneled version of slot \a s. */
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
SigC::Slot7<R,P1,P2,P3,P4,P5,P6,P7>
  open_tunnel(Tunnel *tunnel, const SigC::Slot7<R,P1,P2,P3,P4,P5,P6,P7>& s, bool sync = false)
  { return SigCX::bind(SigC::slot(&TunnelCallback7<R,P1,P2,P3,P4,P5,P6,P7>::pack_n_tunnel), s, tunnel, sync);
  }


template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
R tunnel(SigC::Slot7<R,P1,P2,P3,P4,P5,P6,P7> s,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,Tunnel *tunnel,bool sync = false) 
{
  return tunnel ? TunnelCallback7<R,P1,P2,P3,P4,P5,P6,P7>::pack_n_tunnel(p1,p2,p3,p4,p5,p6,p7,s,tunnel,sync) : s(p1,p2,p3,p4,p5,p6,p7);
}



/*@}*/

} // namespace SigCX
 
#endif
