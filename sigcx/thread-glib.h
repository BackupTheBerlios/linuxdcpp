// -*- c++ -*-
/* 
 * Copyright 2002 Andreas Rottmann <rottmann@users.sourceforge.net>
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
#ifndef __INC_SIGCX_THREAD_GLIB_H
#define __INC_SIGCX_THREAD_GLIB_H

#include <glib.h>

#include <sigc++/object.h>
#include <sigc++/slot.h>
#include <sigcx/thread.h>

namespace SigCX
{
namespace Threads
{

class ThreadImpl : public SigC::Object
{
  private:
    static Private<ThreadImpl *> cur_thread_;
    static void *call_slot(void *);

    GThread *thread_;
    SigC::Slot0<void> slot_;
    
    ThreadImpl(GThread *thread) : thread_(thread) { }
  public:
    static void ensure_init();

    ThreadImpl(const SigC::Slot0<void>& slot, bool joinable);
    ThreadImpl(const SigC::Slot0<void>& slot,
               unsigned long stacksize,
               bool joinable,
               bool bound,
               Thread::Priority priority);
    ~ThreadImpl();

    bool join();
    
    static ThreadImpl *self();
    static void yield();
};

class MutexImpl
{
    friend class ConditionImpl;
  private:
    GMutex *mutex_;
  public:
    MutexImpl()  { ThreadImpl::ensure_init(); mutex_ = g_mutex_new(); }
    ~MutexImpl() { g_mutex_free(mutex_); }

    void lock()    { g_mutex_lock(mutex_); }
    bool trylock() { return g_mutex_trylock(mutex_); }
    void unlock()  { g_mutex_unlock(mutex_); }
};

struct ConditionImpl
{
  private:
    GCond *cond_;
  public:
    ConditionImpl()  { ThreadImpl::ensure_init(); cond_ = g_cond_new(); }
    ~ConditionImpl() { g_cond_free(cond_); }

    void signal() { g_cond_signal(cond_); }
    void broadcast() { g_cond_broadcast(cond_); }
    void wait(MutexImpl *m) { g_cond_wait(cond_, m->mutex_); }
    bool wait(MutexImpl *m, const TimeVal& tmout);
};

class PrivateImpl
{
  private:
    struct PrivData
    {
        GPrivate *key;
        void (*dtor)(void *);
    };
    static GMemChunk *privdata_memchunk_;
    static GList *free_keys_;
    static GStaticMutex mutex_;

    static void free_statics();
    
    GPrivate *key_;
    void (*dtor_)(void *);
  public:
    PrivateImpl();
    
    void* get() { return g_private_get(key_); }
    void set(void *value) { g_private_set(key_, value); }
    void create(void (*dtor)(void*));
    void destroy();
};

} /* namespace Threads */
} /* namespace SigC */

#endif /* __INC_SIGCX_THREAD_GLIB_H */
