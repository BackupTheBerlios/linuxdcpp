// -*- c++ -*-
/* 
 * Copyright 1999 Karl Nelson <kenelson@ece.ucdavis.edu>
 * Modified by Andreas Rottmann <rottmann@users.sourceforge.net>,
 * Copyright 2001.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __LIB_SIGCX_THREAD_H
#define __LIB_SIGCX_THREAD_H

#include <stdexcept>

#include <sigc++/slot.h>
#include <sigcx/timeval.h>

/** \defgroup sigcx_thread Threading */

namespace SigCX
{

namespace Threads
{

/** \addtogroup sigcx */
/*@{*/

/** \addtogroup sigcx_thread */
/*@{*/

class ThreadImpl;

/** Thread class.
 *
 * Class representing a thread (lightweight process).
 */
class Thread
{
  private:
    ThreadImpl *impl_;
    Thread(ThreadImpl *impl);
  public:
    /** Thread priorities. */
    enum Priority
    {
      Low,
      Normal,
      High,
      Urgent
    };
    
    /** Thread exit exception.
     * Throw this in the main function of a thread if you want to exit. */
    class Exit : public std::exception
    {
    };

    /** Constructor. 
     * Create a new thread, using \a slot as main function.
     * \param slot Invoked as main function of the thread.
     * \param joinable Wether the thread should be joinable or detached. */
    Thread(const SigC::Slot0<void>& slot, bool joinable = false);
    
    /** Constructor.
     * Create a new thread, using \a slot as main function.
     * In general, avoid using this constructor, when you can use 
     * Thread::Thread(joinable).
     * \param slot Invoked as main function of the thread.
     * \param stacksize Initial stacksize for the thread (system default 
     *   if \c stacksize == 0).
     * \param joinable Wether the thread should be joinable or detached.
     * \param bound If \c true, this thread will be scheduled in the 
     *   system scope, otherwise the implementation is free to do 
     *   scheduling in the process scope. The first variant is more expensive
     *   resource-wise, but generally faster. On some systems (e.g. Linux) 
     *   all threads are bound. 
     * \param priority One of \c Low, \c Normal, \c High,  \c Urgent. 
     *   It is not guaranteed, that threads
     *   with different priorities really behave accordingly. On some systems
     *   (e.g. Linux) only root can increase priorities. On other systems 
     *   (e.g. Solaris) there doesn't seem to be different scheduling for 
     *   different priorities. All in all try to avoid being dependent on
     *   priorities. Use Normal here as a default. */
    Thread(const SigC::Slot0<void>& slot,
           unsigned long stacksize,
           bool joinable,
           bool bound,
           Priority priority = Normal);
    
    /** Copy constructor. */
    Thread(const Thread& th);
    
    /** Destructor. */
    ~Thread();
    
    /** Assignment operator. */
    Thread& operator=(const Thread& th);
    
    /** Join thread.
     * Waits until the thread finishes., i.e. Thread::main(arg)
     * returns. All resources of thread are released. The thread must have
     * been created with \c joinable == \c true in the
     * constructor. 
     * \return \c true on success. */
    bool join();

    /** Check for Thread equality.
     * \param th Thread to check for equality.
     * \return \c true if \c *this and \a th are equal. */
    bool operator==(const Thread& th) const {
      return impl_ == th.impl_;
    }
    
    /** Check for Thread unequality.
     * \param th Thread to check for unequality.
     * \return \c true if \c *this and \a th are not equal. */
    bool operator!=(const Thread& th) const {
      return impl_ != th.impl_;
    }
    
    /** Get current thread. 
     * \return A Thread object representing the thread of the caller. */
    static Thread self();
    static void yield();
};

class MutexImpl;

/** Mutual Exclusion class.
 *
 * A class representing a mutual exclusion.
 */
class Mutex
{
    friend class Condition;
  private:
    MutexImpl *impl_;
  public:
    /** Constructor.
     * The mutex is initially unlocked. */
    Mutex();
    /* Destructor. */
    ~Mutex();
    
    /** Lock the mutex.
     * This blocks until the mutex is unlocked, if it is already locked. */
    void lock();
    /** Try to lock the mutex.
     * \return true, if the operation was successfull, false, if the mutex 
     *   was already locked. */
    bool trylock();
    /** Unlock the mutex. */
    void unlock();
};

/** MLock class.
 *
 * A lazy way to unlock at end of scope.
 */
class MLock
{
  private:
    Mutex &mutex_;
  public:
    /** Constructor.
     * \param mutex A mutex to be locked. */
    MLock(Mutex& mutex) : mutex_(mutex) { mutex_.lock();}
    /** Destructor.
     * The mutex passed to the constructor is unlocked. */
    ~MLock()                            { mutex_.unlock();}
};

class ConditionImpl;

/** Condition class.
 *
 * A class representing a condition that can be waited upon.
 */
class Condition
{
  private:
    ConditionImpl *impl_;
  public:
    /** Constructor. */
    Condition();
    /** Destructor. */
    ~Condition();

    /** Signal condition.
     * Restarts exactly one thread hung on condition. */
    void signal();
    
    /** Broadcast condition.
     * Restarts all threads waiting on condition. */
    void broadcast();
    
    /** Wait for condition.
     * Unlocks a mutex while waiting on a condition, then reaquires lock.
     * \param m Mutex to act upon. */
    void wait(Mutex &m);
    
    /** Timed wait.
     * Unlocks a mutex while waiting on a condition, then reaquires lock
     * with a fixed maximum duration.
     * \param m Mutex to act upon.
     * \param tmout The maximum time interval to wait for the condition. 
     * \return \c true if woken up in time. */
    bool wait(Mutex &m, const TimeVal& tmout);

    /** Timed wait.
     * Unlocks a mutex while waiting on a condition, then reaquires lock
     * with a fixed maximum duration.
     * \param m Mutex to act upon.
     * \param tmout The maximum time interval in microseconds
     *   to wait for the condition.
     * \return \c true if woken up in time. */
    bool wait(Mutex& m, unsigned long tmout) {
      return wait(m, TimeVal(tmout / 1000000, tmout % 1000000));
    }
};

/** Integer Semaphore.
 *
 * A Semaphore is a counter that has can be decremented and
 * incremented. The increment never blocks, the decrement blocks when
 * trying to decrement below zero. 
 */
class Semaphore
{
  private:
    int value_;
    Condition sig_;
    Mutex access_;
  public:
    /** Constructor. 
     * \param value initial value of the semaphore. */
    Semaphore(int value = 1) : value_(value) { }
    /** Destructor. */
    ~Semaphore() { }

    /** Increment. */
    void up();
    /** Decrement. */
    void down();
};

class PrivateImpl;

class Private_
{
  private:
    PrivateImpl *impl_;
  public:
    Private_();
    ~Private_();
    
    void create(void (*dtor)(void *));
    void destroy();
    
    void *get();
    void set(void *);
};

/** Thread-private data.
 *
 * Thread-private data is like a thread split static.
 */
template <class T>
class Private : protected Private_
{
  private:
    static void dtor(void* v) {
      T* obj = (T*)v;
      delete obj;
    }
    T initval_; // FIXME: we need to lock this probably
  public:
    /** Constructor. */
    Private(const T& t = T()) : initval_(t) { create(&dtor); }
    /** Destructor. */
    ~Private() { destroy(); }
    
    /** Assignment operator. */
    T& operator =(const T& t) { return (((T&)*this) = t);}
    
    /** Get data. */
    operator T&() {
      T *value = (T*)get();
      if (!value)
        set((void*)(value = new T(initval_)));  
      return *value;
    }
};

class Guard
{
  private:
    Mutex& mutex_;
  public:
    Guard(Mutex& mutex) : mutex_(mutex) { mutex_.lock(); }
    ~Guard() { mutex_.unlock(); }
};

class UnGuard
{
  private:
    Mutex& mutex_;
  public:
    UnGuard(Mutex& mutex) : mutex_(mutex) { mutex_.unlock(); }
    ~UnGuard() { mutex_.lock(); }
};

/*@}*/
/*@}*/

} /* namespace Threads */
} /* namespace SigC */

#endif /* __INC_SIGCX_THREAD_H */
