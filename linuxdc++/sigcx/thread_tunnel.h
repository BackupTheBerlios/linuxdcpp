/* This is for emacs: -*-Mode: C++;-*- */
/*
  SigC++ cross-thread signalling support.
   
  Copyright 2000 Andreas Rottmannn <rottmann@users.sourceforge.net>
                                                         
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
 
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
 
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef __LIB_SIGCX_THREAD_TUNNEL_H
#define __LIB_SIGCX_THREAD_TUNNEL_H

#include <stdexcept>

#include <sigcx/sigcxconfig.h>

#include <sigcx/util.h>
#include <sigcx/thread.h>
#include <sigcx/tunnel.h>
#include <sigcx/dispatch.h>

namespace SigCX
{

/** \addtogroup sigcx */
/*@{*/

/** A inter-thread tunnel.
 *
 * This tunnel is implemented using a pipe to transfer callbacks from
 * one thread to another. It also has a single-threaded mode in which
 * it uses the pipe anyway. This can be used to code nearly non-blocking 
 * UIs without the use of threads.
 */
class ThreadTunnel : public Tunnel
{
  public:
    /** Execution mode of the ThreadTunnel. */
    enum Mode
    {
      NewThread,     /**< Run dispatcher in newly created thread. */
      CurrentThread, /**< The dispatcher will run in current thread. 
                      * No thread will be created, and the
                      * callbacks are deferred until the next
                      * iteration of the dispatchers main loop. */
      SingleThread    /**< This is similiar to CurrentThread, but for
                       * use in a single threaded program. */
    };
    
    /** Constructor.
     * \param disp Dispatcher to use for the destination thread 
     *   (working thread, the thread where the callbacks are executed).
     * \param mode Working mode of the dispatcher. */
    ThreadTunnel(Dispatcher& disp, Mode mode = NewThread);
    virtual ~ThreadTunnel();
    
    /** Get the destination thread dispatcher.
     * \return The destination thread dispatcher. */
    Dispatcher& dispatcher() { return disp_; }

    // From Tunnel
    virtual void send(Callback *cb, bool sync = false);
    virtual bool in_sync_callback();
    
    /** Exception class.
     *
     * This is thrown whenever a fatal error occurs.
     */
    class FatalError : public std::runtime_error
    {
      public:
        /** Constructor.
         * \param e An \c errno value. */
        FatalError(int e) : runtime_error(errno_string(e)) { }
        /** Constructor.
         * \param what An error message. */
        FatalError(const std::string& what) : runtime_error(what) { }
    };
    virtual void drain();
  private:
    void handle_input();

    struct Packet
    {
        Callback *callback;
        bool sync;
    };

    void send_packet(const Packet& packet);
    void receive_packet(Packet& packet);

    Dispatcher& disp_;
    int pipe_[2];
    Mode mode_;
    unsigned in_sync_cb_ : 1;
    unsigned drained_ : 1;
    int pending_;
    Dispatcher::HandlerID disp_hid_;
    
    Threads::Mutex mutex_;
    Threads::Condition cb_finished_;
    Threads::Thread *thread_;
};

/*@}*/

} // namespace SigCX
 
#endif
