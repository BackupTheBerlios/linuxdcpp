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
#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <fcntl.h>

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sigc++/slot.h>
#include <sigc++/class_slot.h>
#include <sigc++/bind.h>
#include <sigc++/retype_return.h>

#include "sigcx/thread_tunnel.h"

namespace SigCX
{

using namespace SigC;
using namespace Threads;

void ThreadTunnel::handle_input()
{
  Packet packet;

  Guard guard(mutex_);

  if (drained_)
    return;
  
  while (true)
  {
    if (pending_ <= 0)
      break;

    receive_packet(packet);
    pending_--;
    
    try
    {
      UnGuard unguard(mutex_);
      packet.callback->invoke();
    }
    catch (...)
    {
      // the callback raised an exception, so indicate we're done and
      // pass it on
#if SIGCX_THREADS
      if (!packet.sync)
        delete packet.callback;
      else
        cb_finished_.signal();
#endif    
      throw; // pass the exception on
    }
    // the callback finished without exception
#if SIGCX_THREADS
    if (!packet.sync)
      delete packet.callback;
    else
      cb_finished_.signal();
#endif
  }
}

ThreadTunnel::ThreadTunnel(Dispatcher& disp, Mode mode) 
    : disp_(disp), mode_(mode), in_sync_cb_(false), 
      drained_(false), pending_(0)
{
  if (pipe(pipe_) != 0)
    throw FatalError(errno);
  
  disp_.reference();
  disp_hid_ = disp_.add_input_handler(
          slot_class(*this, &ThreadTunnel::handle_input), 
          pipe_[0]);
  
#if SIGCX_THREADS
  if (mode == NewThread)
    thread_ = new Thread(
            retype_return<void>(bind(slot(disp, &Dispatcher::run), true)));
  else if (mode == CurrentThread)
    thread_ = new Thread(Thread::self());
  else
    thread_ = 0;
#else
  if (mode != SingleThread)
    throw FatalError("cannot create multithreaded tunnel "
                     "without thread support");
  thread_ = 0;
#endif
}

ThreadTunnel::~ThreadTunnel()
{
  Guard guard(mutex_);
  
  disp_.remove(disp_hid_);
  disp_.unreference();

  close(pipe_[0]);
  close(pipe_[1]);
  
  if (thread_)
    delete thread_;
}

void ThreadTunnel::send(Callback *cb, bool sync)
{
  Guard guard(mutex_);

#if SIGCX_THREADS
  if ((sync && thread_ == 0) || 
      (thread_ && *thread_ == Threads::Thread::self()))
  {
    // We have the dispatcher in the same thread
    UnGuard unguard(mutex_);
    cb->invoke();
  }
  else
  {
    if (sync)
      in_sync_cb_ = true;
    
    Packet packet;
#ifdef DEBUG
    memset(&packet, 0, sizeof(Packet)); // silence valgrind
#endif

    packet.callback = cb;
    packet.sync = sync;

    send_packet(packet);
    pending_++;
    if (sync)
      cb_finished_.wait(mutex_);
    in_sync_cb_ = false;
  }
#else
  if (sync)
    cb->invoke();
  else
  {
    Packet packet;

    packet.callback = cb;
    packet.sync = sync;

    send_packet(packet);
  }
#endif
}

bool ThreadTunnel::in_sync_callback()
{
  Guard guard(mutex_);
  
  return in_sync_cb_;
}

void ThreadTunnel::drain()
{
  Guard guard(mutex_);

  if (drained_)
    return;
  
  char buffer[sizeof(Packet)];
  int oflags = fcntl(pipe_[0], F_GETFL, 0);
  fcntl(pipe_[0], F_SETFL, oflags | O_NONBLOCK);
  
  while (read(pipe_[0], buffer, sizeof(buffer)) > 0)
    ;
  // we must signal, since the sending end might have done
  // send_packet() already and being waiting for the signal.
  if (in_sync_cb_)
    cb_finished_.signal();
  
  drained_ = true;
}

void ThreadTunnel::receive_packet(Packet& packet)
{
  int n;

  for (unsigned read_bytes = 0; read_bytes < sizeof(Packet); read_bytes += n)
  {
    if ((n = read(pipe_[0], ((char *)&packet) + read_bytes,
                  sizeof(Packet) - read_bytes)) == -1)
      throw FatalError(errno);
  }
}

void ThreadTunnel::send_packet(const Packet& packet)
{
  int n;
  
  for (unsigned written_bytes = 0; written_bytes < sizeof(Packet);
       written_bytes += n)
  {
    if ((n = write(pipe_[1], ((char *)&packet) + written_bytes,
                   sizeof(Packet) - written_bytes)) == -1)
      throw FatalError(errno);
  }
}

}
