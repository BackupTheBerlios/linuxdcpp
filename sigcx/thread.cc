#include <sigcx/thread.h>
#include <sigcx/thread-pthreads.h>

namespace SigCX
{

namespace Threads
{

using namespace SigC;

Thread::Thread(ThreadImpl *impl)
{
  impl_ = impl;
  impl_->reference();
}

Thread::Thread(const Slot0<void>& slot, bool joinable)
{
  impl_ = manage(new ThreadImpl(slot, joinable));
  impl_->reference();
}

Thread::Thread(const SigC::Slot0<void>& slot,
               unsigned long stacksize,
               bool joinable,
               bool bound,
               Priority priority)
{
  impl_ = manage(new ThreadImpl(slot, stacksize, joinable, bound, priority));
  impl_->reference();
}

Thread::Thread(const Thread& th)
 {
   impl_ = th.impl_;
   impl_->reference();
 }

Thread::~Thread()
{
  impl_->unreference();
}

Thread& Thread::operator=(const Thread& th) 
{
  impl_ = th.impl_;
  impl_->reference();
  return *this;
}

bool Thread::join()
{
  return impl_->join();
}

Thread Thread::self()
{
  return Thread(ThreadImpl::self());
}

void Thread::yield()
{
	ThreadImpl::yield();
}

Mutex::Mutex()
{
  impl_ = new MutexImpl;
}

Mutex::~Mutex()
{
  delete impl_;
}

void Mutex::lock()
{
  impl_->lock();
}

bool Mutex::trylock()
{
  return impl_->trylock();
}

void Mutex::unlock()
{
  impl_->unlock();
}

void Semaphore::up()
{
  access_.lock();
  value_++;
  access_.unlock();
  sig_.signal();
}

void Semaphore::down()
{
  access_.lock();
  while (value_ < 1)
    sig_.wait(access_);
  
  value_--;
  access_.unlock();
}

Condition::Condition()
{
  impl_ = new ConditionImpl;
}

Condition::~Condition()
{
  delete impl_;
}

void Condition::signal()
{
  impl_->signal();
}

void Condition::broadcast()
{
  impl_->broadcast();
}

void Condition::wait(Mutex& m)
{
  impl_->wait(m.impl_);
}

bool Condition::wait(Mutex& m, const TimeVal& tmout)
{
  return impl_->wait(m.impl_, tmout);
}

Private_::Private_()
{
  impl_ = new PrivateImpl;
}

Private_::~Private_()
{
  delete impl_;
}

void Private_::create(void (*dtor)(void *))
{
  impl_->create(dtor);
}

void *Private_::get()
{
  return impl_->get();
}

void Private_::set(void *val)
{
  impl_->set(val);
}

void Private_::destroy()
{
  impl_->destroy();
}

}

}
