/*  
  Copyright 2002, Andreas Rottmann

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/
#include "thread-glib.h"

namespace SigCX
{

namespace Threads
{

using namespace SigC;

//======================================================================
// ThreadImpl
//======================================================================

Private<ThreadImpl *> ThreadImpl::cur_thread_;

void *ThreadImpl::call_slot(void *arg)
{
  ThreadImpl *obj = (ThreadImpl *)arg;

  // We execute in new thread
  cur_thread_ = obj;
  
  try
  {
    obj->slot_();
  }
  catch (Thread::Exit)
  {
    // Just exit from the thread.  The Thread::Exit exception
    // is our sane C++ replacement of g_thread_exit().
  }
  return 0;
}

ThreadImpl::ThreadImpl(const Slot0<void>& slot, bool joinable) : slot_(slot)
{
  ensure_init();
  
  // FIXME: we are not particularly safe, since slot_ & slot both
  // reference the same SlotNode
  
  GError *error = 0;
  
  thread_ = g_thread_create(&call_slot, this, joinable, &error);
  if (error)
    throw std::runtime_error(error->message);
}

ThreadImpl::ThreadImpl(const Slot0<void>& slot,                
                       unsigned long stacksize,
                       bool joinable,
                       bool bound,
                       Thread::Priority priority)
{
  ensure_init();
  
  GThreadPriority gpriority = G_THREAD_PRIORITY_NORMAL;
  GError *error = 0;
  
  switch (priority)
  {
    case Thread::Low: 		gpriority = G_THREAD_PRIORITY_LOW; break;
    case Thread::Normal: 	gpriority = G_THREAD_PRIORITY_NORMAL; break;
    case Thread::High: 		gpriority = G_THREAD_PRIORITY_HIGH; break;
    case Thread::Urgent: 	gpriority = G_THREAD_PRIORITY_URGENT; break;
  }
  
  thread_ = g_thread_create_full(
          &call_slot, this, stacksize, joinable, bound, gpriority, &error);
  if (error)
    throw std::runtime_error(error->message);
}

ThreadImpl::~ThreadImpl()
{
}

inline void ThreadImpl::ensure_init() {
  if (!g_thread_supported())
    g_thread_init(NULL);
}

bool ThreadImpl::join()
{
  return g_thread_join(thread_);
}

ThreadImpl *ThreadImpl::self()
{
  if (cur_thread_ == 0)
  {
    // We execute in a thread that was not created by ourselves
    // (e.g. main thread)
    cur_thread_ = new ThreadImpl(g_thread_self());
  }
  return cur_thread_;
}

void ThreadImpl::yield()
{
	if (cur_thread_)
		g_thread_yield();
}


//======================================================================
// ConditionImpl
//======================================================================

bool ConditionImpl::wait(MutexImpl *m, const TimeVal& tmout)
{
  TimeVal now;
  GTimeVal abstime;
  
  now.get_current_time();
  TimeVal abs_tv = now + tmout;
  abstime.tv_sec = abs_tv.tv_sec;
  abstime.tv_sec = abs_tv.tv_usec;

  return g_cond_timed_wait(cond_, m->mutex_, &abstime);
}

GList *PrivateImpl::free_keys_ = NULL;
GMemChunk *PrivateImpl::privdata_memchunk_ = NULL;
GStaticMutex PrivateImpl::mutex_ = G_STATIC_MUTEX_INIT;

PrivateImpl::PrivateImpl()  : key_(0), dtor_(0) 
{ 
  ThreadImpl::ensure_init(); 

  g_static_mutex_lock(&mutex_);
  if (!privdata_memchunk_)
  {
    privdata_memchunk_ = g_mem_chunk_new(
            "SigCX Threads::Private data memchunk",
            sizeof(PrivData),
            32,
            G_ALLOC_AND_FREE);

    g_atexit(PrivateImpl::free_statics);
  }
  g_static_mutex_unlock(&mutex_);
}

void PrivateImpl::create(void (*dtor)(void *))
{
  g_static_mutex_lock(&mutex_);
  
  if (key_)
    if (dtor_ == dtor)
    {
      dtor(g_private_get(key_));
      g_private_set(key_, 0);
    }
    else
      destroy();
  
  if (!key_)
  {
    GList *free_key = 0;
    
    for (GList *node = free_keys_; node; node = g_list_next(node))
      if (((PrivData *)node->data)->dtor == dtor)
      {
        free_key = node;
        break;
      }
    
    if (free_key)
    {
      key_ = ((PrivData *)free_key->data)->key;
      dtor_ = ((PrivData *)free_key->data)->dtor;
      g_chunk_free(free_key->data, privdata_memchunk_);
      free_keys_ = g_list_remove_link(free_keys_, free_key);
    }
    else
    {
      key_ = g_private_new(dtor);
      dtor_ = dtor;
    }
  }
  g_static_mutex_unlock(&mutex_);
}

void PrivateImpl::destroy()
{
  g_static_mutex_lock(&mutex_);
  
  PrivData *data = g_chunk_new(PrivData, privdata_memchunk_);
  data->key = key_;
  data->dtor = dtor_;
  free_keys_ = g_list_append(free_keys_, data);
  key_ = 0;

  g_static_mutex_unlock(&mutex_);
}

void PrivateImpl::free_statics()
{
  g_static_mutex_lock(&mutex_);
  g_mem_chunk_destroy(privdata_memchunk_);
  g_list_free(free_keys_);
  privdata_memchunk_ = NULL;
  free_keys_ = NULL;
  g_static_mutex_unlock(&mutex_);
}

}

}
