#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <errno.h>

#if defined(HAVE_SCHED_YIELD_H)
#  include <sched.h>
#endif

#include "thread-pthreads.h"

namespace SigCX
{

namespace Threads
{

using namespace SigC;

//======================================================================
// ThreadImpl
//======================================================================
pthread_once_t ThreadImpl::self_key_once_ = PTHREAD_ONCE_INIT;
pthread_key_t ThreadImpl::self_key_;

void *ThreadImpl::call_slot(void *arg)
{
  ThreadImpl *obj = (ThreadImpl *)arg;

  // We execute in new thread
  pthread_once(&self_key_once_, &self_key_alloc);
  pthread_setspecific(self_key_, obj);
  
  try
  {
    obj->slot_();
  }
  catch (Thread::Exit)
  {
    // Just exit from the thread.  The Thread::Exit exception
    // is our sane C++ replacement of pthread_exit().
  }
  return 0;
}

ThreadImpl::ThreadImpl(const Slot0<void>& slot, bool joinable) : slot_(slot)
{
  // FIXME: we are not particularly safe, since slot_ & slot both
  // reference the same SlotNode

  pthread_attr_t attr;
  
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, joinable ? PTHREAD_CREATE_JOINABLE : 
                              PTHREAD_CREATE_DETACHED);
  
  pthread_create(&thread_, &attr, &ThreadImpl::call_slot, this);
}

ThreadImpl::ThreadImpl(const SigC::Slot0<void>& slot,
                       unsigned long stacksize,
                       bool joinable,
                       bool bound,
                       Thread::Priority priority)
{
  // FIXME: see other ctor
  pthread_attr_t attr;
  
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, stacksize);
  pthread_attr_setscope(&attr, bound ? PTHREAD_SCOPE_SYSTEM : 
                        PTHREAD_SCOPE_PROCESS);
  pthread_attr_setdetachstate(&attr, joinable ? PTHREAD_CREATE_JOINABLE : 
                              PTHREAD_CREATE_DETACHED);
  
  pthread_create(&thread_, &attr, &ThreadImpl::call_slot, this);
}

ThreadImpl::~ThreadImpl()
{
}

bool ThreadImpl::join()
{
  int err = 0;
  
  do
  {
    err = pthread_join(thread_, NULL);
  } while (err == EINTR);
  
  return err == 0;
}

ThreadImpl *ThreadImpl::self()
{
  pthread_once(&self_key_once_, &self_key_alloc);
  ThreadImpl *myself = (ThreadImpl *)pthread_getspecific(self_key_);
  
  if (myself == 0)
  {
    // We execute in a thread that was not created by ourselves
    // (e.g. main thread)
    
    // The next line introduces a memleak (we leak when the thread
    // exits), but this doesn't matter that much, since the line will
    // probably only executed once, in the main thread, if executed at
    // all.
    myself = new ThreadImpl(pthread_self());
    pthread_setspecific(self_key_, myself);
  }
  return myself;
}

void ThreadImpl::yield()
{
#if defined(HAVE_SCHED_YIELD)
  sched_yield();
#elif defined(HAVE_PTHREAD_YIELD)
  pthread_yield();
#endif
}

//======================================================================
// ConditionImpl
//======================================================================

bool ConditionImpl::wait(MutexImpl *mutex, const TimeVal& tmout)
{
  TimeVal now;
  struct timespec abstime;
  
  now.get_current_time();
  TimeVal abs_tv = now + tmout;
  abstime.tv_sec = abs_tv.tv_sec;
  abstime.tv_sec = abs_tv.tv_usec;
  
  int err = 0;
  do
  {
    err = pthread_cond_timedwait(&cond_, &mutex->mutex_, &abstime);
  } while (err == EINTR);
  
  return err != ETIMEDOUT;
}

}

}
