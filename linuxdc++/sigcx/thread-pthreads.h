/* This is for emacs: -*-Mode: C++;-*- */
#if !defined(__INC_THREAD_PTHREADS_H)
#define __INC_THREAD_PTHREADS_H

#include <pthread.h>

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
    static pthread_once_t self_key_once_;
    static pthread_key_t self_key_;
    static void self_key_alloc() {
      pthread_key_create(&self_key_, NULL);
    }

    static void *call_slot(void *);

    pthread_t thread_;
    SigC::Slot0<void> slot_;
    
    ThreadImpl(pthread_t thread) : thread_(thread) { }
  public:
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
    pthread_mutex_t mutex_;
  public:
    MutexImpl() {  pthread_mutex_init(&mutex_, NULL); }
    ~MutexImpl() { pthread_mutex_destroy(&mutex_); }
    
    void lock() { pthread_mutex_lock(&mutex_); }
    bool trylock() { return pthread_mutex_trylock(&mutex_) == 0; }
    void unlock() { pthread_mutex_unlock(&mutex_); }
};

class ConditionImpl
{
    pthread_cond_t cond_;
  public:
    ConditionImpl() { pthread_cond_init(&cond_, NULL); }
    ~ConditionImpl() { pthread_cond_destroy(&cond_); }

    void signal() { pthread_cond_signal(&cond_); }
    void broadcast() { pthread_cond_broadcast(&cond_); }
    void wait(MutexImpl *mutex) { pthread_cond_wait(&cond_, &mutex->mutex_); }
    bool wait(MutexImpl *mutex, const TimeVal& tmout);
};

class PrivateImpl
{
    pthread_key_t key_;
  public:
    PrivateImpl() { }
    ~PrivateImpl() { }
    
    void create(void (*dtor)(void *)) { pthread_key_create(&key_, dtor); }
    void destroy() { pthread_key_delete(key_); }

    void *get() { return pthread_getspecific(key_); }
    void set(void *val) { pthread_setspecific(key_, val); }
};

}

}

#endif
