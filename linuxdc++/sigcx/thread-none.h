/* This is for emacs: -*-Mode: C++;-*- */
#if !defined(__INC_THREAD_NONE_H)
#define __INC_THREAD_NONE_H

#include <sigc++/object.h>
#include <sigc++/slot.h>
#include <sigcx/thread.h>

namespace SigCX
{

namespace Threads
{

class ThreadImpl : public SigC::Object
{
  public:
    ThreadImpl(const SigC::Slot0<void>& slot, bool joinable) { 
      SigC::Slot0<void> real_slot = slot;
      real_slot();
    }
    ThreadImpl(const SigC::Slot0<void>& slot,
               unsigned long stacksize,
               bool joinable,
               bool bound,
               Thread::Priority priority) {
      SigC::Slot0<void> real_slot = slot;
      real_slot();
    }
    ~ThreadImpl() { }

    bool join() { return false; }
    
    static ThreadImpl *self();
  private:
    ThreadImpl() { }
    static ThreadImpl *self_;
    static void yield();
};

class MutexImpl
{
    friend class ConditionImpl;
  public:
    MutexImpl() { }
    ~MutexImpl() { }
    
    void lock() { }
    bool trylock() { return false; }
    void unlock() { }
};

class ConditionImpl
{
  public:
    ConditionImpl() {  }
    ~ConditionImpl() { }

    void signal() { }
    void broadcast() { }
    void wait(MutexImpl *mutex) { }
    bool wait(MutexImpl *mutex, const TimeVal& tmout) { return false; }
};

class PrivateImpl
{
  private:
    void *val_;
    void (*dtor_)(void *);
  public:
    PrivateImpl() { }
    ~PrivateImpl() { }
    
    void create(void (*dtor)(void *)) { 
      dtor_ = dtor; 
      val_ = 0;
    }
    void destroy() { 
      dtor_(val_);
    }

    void *get() { 
      return val_;
    }
    void set(void *val) { 
      val_ = val;
    }
};

}

}

#endif
