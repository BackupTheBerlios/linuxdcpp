/* This is for emacs: -*-Mode: C++;-*- */
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
#if !defined(__LIB_SIGCX_DISPATCH_H)
#define __LIB_SIGCX_DISPATCH_H

#include <sys/types.h>

#include <list>
#include <map>

#include <sigc++/object.h>
#include <sigc++/slot.h>
#include <sigcx/thread.h>

/** \defgroup sigcx SigC++ Extras */
namespace SigCX
{

/** \addtogroup sigcx */
/*@{*/

/** Dispatcher class.
 *
 * This abstract class defines a generic interface to an event
 * dispatcher.
 */
class Dispatcher : public SigC::Object
{
  public:
    /** Event types. */
    enum Event 
    { 
      Timer,	///< Timer event.
      Read, 	///< Data ready for reading.
      Write, 	///< IO channel ready for writing.
      Except, 	///< IO channel exception.
      Signal, 	///< Signal.
      All, 	///< All events.
      Remove, 	///< Handler remove event.
      Moved 	///< Handler move event.
    };
  
    /** Event handler. */
    typedef SigC::Slot0<void> Handler;
    
    /** Event handler ID. */
    typedef unsigned long HandlerID;
    static const unsigned long InvalidHandlerID = 0;

    /** Constructor. */
    Dispatcher() : handler_id_(0) { }
    
    /** Destructor. */
    virtual ~Dispatcher();

    /** Add input handler. 
     * The handler \a h is invoked when data is ready for reading from \a fd.
     * \param h The input handler.
     * \param fd File descriptor. */
    virtual HandlerID add_input_handler(const Handler& h, int fd) = 0;

    /** Add output handler. 
     * The handler \a h is invoked when \a fd is ready for writing.
     * \param h The input handler.
     * \param fd File descriptor. */
    virtual HandlerID add_output_handler(const Handler& h, int fd) = 0;

    /** Add exception handler.
     * The handler \a h is invoked when an exception occurs on \a fd.
     * \param h The input handler.
     * \param fd File descriptor. */
    virtual HandlerID add_exception_handler(const Handler& h, int fd) = 0;

    /** Add timeout handler.
     * The handler \a h is invoked when the time specified by 
     *   \a tv has passed.
     * \param h The input handler.
     * \param tv TimeVal timeout. */
    virtual HandlerID add_timeout_handler(const Handler& h, const TimeVal& tv) = 0;

    /** Add timeout handler.
     * The handler \a h is invoked when the time specified by 
     *   \a tv has passed.
     * \param h The input handler.
     * \param tmout timeout in milliseconds. */
    HandlerID add_timeout_handler_msec(const Handler& h, 
                                             unsigned long tmout) {
      return add_timeout_handler(h, TimeVal(tmout / 1000, 
                                            (tmout % 1000) * 1000));
    }
    
    /** Add signal handler.
     * The handler \a h is invoked when the signal \a sig is received 
     * by the program. */
    virtual HandlerID add_signal_handler(const Handler& h, int sig) = 0;
    
    /** Remove a handler.
     * \param id The ID of the handler. */
    virtual void remove(HandlerID id) = 0;

    /** Run the dispatcher.
     * Run the dispatcher event loop, receiving events and calling 
     * the registered callbacks.
     * \param infinite If false, run only one iteration, else run until 
     *   exit() is called on this dispatcher instance. 
     * \return \c true if exit() was called.
     */
    virtual bool run(bool infinite = true) = 0;
    /** Cause exit of event loop. */
    virtual void exit() = 0;
    /** Move all callbacks to another dispatcher.
     * \param d The dispatcher to move the callbacks to. */
    virtual void move(Dispatcher& d) = 0;
    /** Get idle status. 
     * \return \c true if the dispatcher is idle. */
    virtual bool idle() const = 0;
  protected:
    HandlerID get_new_handler_id() { return ++handler_id_; }
    unsigned long handler_id_;
};

/** Signal dispatcher.
 * 
 * This class only implements signal events since this is generally
 * not implemented in standard event loops (like GLib or GTK).
 */
class SignalDispatcher : public Dispatcher
{
  public:
    SignalDispatcher();
    virtual ~SignalDispatcher();
    virtual HandlerID add_signal_handler(const Handler& h, int sig);
    virtual void remove(HandlerID id);
  private:
    struct SignalEvent
    {
        SignalDispatcher *disp;
        Handler cb;
        void (*prev_handler)(int);
        int sig;
        
        SignalEvent(SignalDispatcher *_disp, const Handler& _cb, int _sig, 
                    void (*_prev_handler)(int));
    };
    typedef std::map<HandlerID, SignalEvent> HandlerMap;
    
    static HandlerMap events_;
    static Threads::Mutex mutex_;
    static std::map<int, int> count_map_;

    static void signal_handler(int sig);
};

/** StandardDispatcher class.
 *
 * This class implements a event dispatcher on top of the standard
 * UNIX select() function.
*/
class StandardDispatcher : public SignalDispatcher
{
  public:
    /** Constructor. */
    StandardDispatcher();

    /** Destructor. */
    virtual ~StandardDispatcher();

    virtual HandlerID add_input_handler(const Handler& h, int fd);
    virtual HandlerID add_output_handler(const Handler& h, int fd);
    virtual HandlerID add_exception_handler(const Handler& h, int fd);
    virtual HandlerID add_timeout_handler(const Handler& h, const TimeVal& tv);
    virtual void remove(HandlerID id);
    virtual bool run(bool infinite = true);
    virtual void exit();
    virtual void move(Dispatcher& d);
    virtual bool idle() const;
  private:
    struct FileEvent
    {
	Handler cb;
	Event ev;
        int fd;
        
	FileEvent() {}
	FileEvent(const Handler& _cb, Event _ev, int _fd)
            : cb(_cb), ev(_ev), fd(_fd)
          {}
    };
    struct TimerEvent
    {
	Handler cb;
        TimeVal expiration;
        
	TimerEvent() {}
	TimerEvent(const Handler& _cb, const TimeVal& tv);
        
        bool operator<(const TimerEvent& t) const { 
          return expiration < t.expiration; 
        }
    };
    typedef std::multimap<TimerEvent, HandlerID> TMEventMap;
    typedef std::map<HandlerID, TMEventMap::iterator> TMHandlerMap;
    typedef std::map<HandlerID, FileEvent> FDHandlerMap;
    Threads::Mutex mutex_;
    
    
    TMHandlerMap tm_handlers_;
    TMEventMap tm_events_;
    FDHandlerMap fd_handlers_;
    fd_set rd_fds_, wr_fds_, ex_fds_;
    bool do_exit_;
};

/*@}*/

}

#endif
