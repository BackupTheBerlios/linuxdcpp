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
#include <stdexcept>
#include <iostream>

#include <signal.h>

#include "sigcx/dispatch.h"

namespace SigCX
{

using namespace std;
using namespace Threads;

Dispatcher::~Dispatcher()
{
}

SignalDispatcher::SignalEvent::SignalEvent(SignalDispatcher *_disp, 
                                           const Handler& _cb, 
                                           int _sig,
                                           void (*_prev_handler)(int))
{
  disp = _disp;
  cb = _cb;
  sig = _sig;
  prev_handler = _prev_handler;
}


SignalDispatcher::HandlerMap SignalDispatcher::events_;
Threads::Mutex SignalDispatcher::mutex_;
std::map<int, int> SignalDispatcher::count_map_;

SignalDispatcher::SignalDispatcher()
{
}

SignalDispatcher::~SignalDispatcher()
{
  Guard guard(mutex_);

  // We go through the signal handlers, signal by signal, and
  // establish the previous signal handlers for thos signals where no
  // handlers are left.
  for (std::map<int, int>::iterator it = count_map_.begin(); 
       it != count_map_.end();  ++it)
  {
    int sig = it->first;
    int count = it->second;
    int rmcount = 0;
    void (*prev_handler)(int) = SIG_DFL;
    SignalEvent *another_ev = 0;
  
    for (HandlerMap::iterator ev_it = events_.begin(); 
         ev_it != events_.end(); )
    {
      SignalEvent& ev = ev_it->second;
      if (ev.sig == sig)
      {
        if (ev.disp == this)
        {
          if (ev.prev_handler != &signal_handler)
            prev_handler = ev.prev_handler;
          HandlerMap::iterator del_it = ev_it++;
          events_.erase(del_it);
          rmcount++;
        }
        else
        {
          another_ev = &ev;
          ++ev_it;
        }
      }
      else
        ev_it++;
    }
    if (rmcount >= count)
    {
      count_map_[sig] = 0;
      signal(sig, prev_handler);
    }
    else if (prev_handler != SIG_ERR)
    {
      another_ev->prev_handler = prev_handler; // we just store the
                                               // original handler in
                                               // another, remaining event
      count_map_[sig] = count - rmcount;
    }
  }
}

Dispatcher::HandlerID 
SignalDispatcher::add_signal_handler(const Handler& h, int sig)
{
  void (*handler)(int);
  
  if ((handler = signal(sig, &signal_handler)) == SIG_ERR)
    return InvalidHandlerID;
  
  Guard guard(mutex_);
  
  HandlerID id = get_new_handler_id();
  events_.insert(make_pair(id, SignalEvent(this, h, sig, handler)));
  std::map<int, int>::iterator it = count_map_.find(sig);
  if (it != count_map_.end())
    it->second++;
  else
    count_map_[sig] = 1;

  return id;
}

void SignalDispatcher::remove(HandlerID id)
{
  Guard guard(mutex_);

  map<HandlerID, SignalEvent>::iterator it = events_.find(id);
  if (it != events_.end())
    events_.erase(it);
}

void SignalDispatcher::signal_handler(int sig)
{
  Guard guard(mutex_);
  
  for (HandlerMap::iterator it = events_.begin(); it != events_.end(); ++it)
  {
    SignalEvent& ev = it->second;
    if (ev.sig == sig)
    {
      UnGuard unguard(mutex_);
      ev.cb();
    }
  }
}

StandardDispatcher::StandardDispatcher()
{
  Guard guard(mutex_);
  FD_ZERO(&rd_fds_);
  FD_ZERO(&wr_fds_);
  FD_ZERO(&ex_fds_);
}

StandardDispatcher::TimerEvent::TimerEvent(const Handler& _cb,
                                           const TimeVal& tmout)
    : cb(_cb)
{
  expiration.get_current_time();

  expiration += tmout;
}

StandardDispatcher::~StandardDispatcher()
{
}

Dispatcher::HandlerID
StandardDispatcher::add_input_handler(const Handler& cb, int fd)
{
  Guard guard(mutex_);

  HandlerID id = get_new_handler_id();
  FD_SET(fd, &rd_fds_);
  fd_handlers_.insert(make_pair(id, FileEvent(cb, Read, fd)));
  
  return id;
}

Dispatcher::HandlerID
StandardDispatcher::add_output_handler(const Handler& cb, int fd)
{
  Guard guard(mutex_);

  HandlerID id = get_new_handler_id();
  FD_SET(fd, &wr_fds_);
  fd_handlers_.insert(make_pair(id, FileEvent(cb, Write, fd)));
  
  return id;
}

Dispatcher::HandlerID 
StandardDispatcher::add_exception_handler(const Handler& cb, int fd)
{
  Guard guard(mutex_);
  HandlerID id = get_new_handler_id();
  FD_SET(fd, &ex_fds_);
  fd_handlers_.insert(make_pair(id, FileEvent(cb, Except, fd)));
  
  return id;
}

Dispatcher::HandlerID 
StandardDispatcher::add_timeout_handler(const Handler& cb, 
                                        const TimeVal& tmout)
{
  Guard guard(mutex_);
  
  HandlerID id = get_new_handler_id();
  TMEventMap::iterator it 
    = tm_events_.insert(make_pair(TimerEvent(cb, tmout), id));
  tm_handlers_.insert(make_pair(id, it));
  
  return id;
}

void StandardDispatcher::remove(HandlerID id)
{
  Guard guard(mutex_);
  
  TMHandlerMap::iterator tm_it = tm_handlers_.find(id);
  if (tm_it != tm_handlers_.end())
  {
    TMEventMap::iterator ev_it = tm_it->second;
    tm_handlers_.erase(tm_it);
    tm_events_.erase(ev_it);
    return;
  }
  
  FDHandlerMap::iterator fd_it = fd_handlers_.find(id);
  if (fd_it != fd_handlers_.end())
  {
    FileEvent& fevent = fd_it->second;
    
    if (fevent.ev == Read && FD_ISSET(fevent.fd, &rd_fds_))
      FD_CLR(fevent.fd, &rd_fds_);
    if (fevent.ev == Write && FD_ISSET(fevent.fd, &wr_fds_))
      FD_CLR(fevent.fd, &wr_fds_);
    if (fevent.ev == Except && FD_ISSET(fevent.fd, &ex_fds_))
      FD_CLR(fevent.fd, &ex_fds_);
    
    fd_handlers_.erase(fd_it);
    return;
  }
  
  SignalDispatcher::remove(id);
}


bool StandardDispatcher::run(bool infinite)
{
  TimeVal now, timeout;
  Event event;
  
  Guard guard(mutex_);
  
  do
  {
    // We set it here, it may be unset in a handler by calling exit
    do_exit_ = false;

    fd_set rd = rd_fds_;
    fd_set wr = wr_fds_;
    fd_set ex = ex_fds_;
    
    // We service timeouts in two passes: first collect all expired
    // timeouts, then invoke them. If we did this in one pass, the
    // callbacks could starve the FD handlers by adding new timeouts
    
    now.get_current_time();
    typedef std::list<TMEventMap::iterator> TimeoutList;
    TimeoutList expired_tmouts;

    // Pass 1
    for (TMEventMap::iterator it = tm_events_.begin(); 
         it != tm_events_.end(); ++it)
    {
      TimeVal remain = it->first.expiration - now;
      if (remain <= TimeVal(0))
        expired_tmouts.push_back(it);
      else
        break;
    }

    // Pass 2
    for (TimeoutList::iterator it = expired_tmouts.begin(); 
         it != expired_tmouts.end(); ++it)
    {
      TMEventMap::iterator ev_it = *it;
      try
      {
        UnGuard unguard(mutex_);
        const_cast<Handler&>(ev_it->first.cb)();
      }
      catch (...)
      {
        // we do the removal also on an exception 
        tm_handlers_.erase(ev_it->second);
        tm_events_.erase(ev_it);
        throw;
      }
      tm_handlers_.erase(ev_it->second);
      tm_events_.erase(ev_it);
      if (do_exit_)
        break;
    }
    if (do_exit_)
      break;
    
    if (tm_events_.size() == 0)
    {
      UnGuard unguard (mutex_);
      select(FD_SETSIZE, &rd, &wr, &ex, 0);
    }
    else
    {
      now.get_current_time();
      timeout = tm_events_.begin()->first.expiration - now;

      UnGuard unguard(mutex_);
      
      if (timeout < TimeVal(0))
        timeout = TimeVal(0);
      
      struct timeval tv;
      tv.tv_sec = timeout.tv_sec;
      tv.tv_usec = timeout.tv_usec;
      
      select(FD_SETSIZE, &rd, &wr, &ex, &tv);
    }

    // check after select, we might have caught a signal
    if (do_exit_)
      break;

    map<HandlerID, FileEvent>::iterator j;
    for (j = fd_handlers_.begin(); j != fd_handlers_.end(); j++)
    {
      FileEvent& fevent = j->second;
      
      if (fevent.ev == Read && FD_ISSET(fevent.fd, &rd))
        event = Read;
      else if (fevent.ev == Write && FD_ISSET(fevent.fd, &wr))
        event = Write;
      else if (fevent.ev == Except && FD_ISSET(fevent.fd, &ex))
        event = Except;
      else 
        event = All;
      
      if (event != All)
      {
        UnGuard unguard(mutex_);
        fevent.cb();
      }
      if (do_exit_)
        break;
    }
  } while (infinite && !do_exit_);
  
  return do_exit_;
}

void StandardDispatcher::move(Dispatcher&)
{
  throw std::runtime_error("not implemented");
}

bool StandardDispatcher::idle() const
{
  Guard(const_cast<StandardDispatcher *>(this)->mutex_);
  return (fd_handlers_.size() + tm_handlers_.size() == 0);
}

void StandardDispatcher::exit()
{
  Guard guard(mutex_);
  do_exit_ = true;
}

}
