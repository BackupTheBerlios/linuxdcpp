#include "thread-none.h"

namespace SigCX
{

namespace Threads
{

ThreadImpl *ThreadImpl::self_ = 0;

ThreadImpl *ThreadImpl::self()
{
  if (!self_)
    self_ = manage(new ThreadImpl);
  
  return self_;
}

void ThreadImpl::yield()
{
}

}

}
