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
#include <gtk/gtkmain.h>

#include <sigcx/gtk_dispatch.h>

namespace SigCX
{

using namespace std;
using namespace Threads;

void GtkDispatcher::input_callback(gpointer _event, gint fd,
                                   GdkInputCondition o)
{
  FileEvent *event = (FileEvent *)_event;

  event->cb();
}

gint GtkDispatcher::timer_callback(gpointer _event)
{
  TimerEvent *event = (TimerEvent *)_event;
  GtkDispatcher *disp = event->disp;

  {
    Guard guard(disp->mutex_);
    
    map<HandlerID, TimerEvent>::iterator i;
    for (i = disp->tevents_.begin(); i != disp->tevents_.end(); ++i) 
    {
      if (&i->second == event) 
      {
        disp->tevents_.erase(i);
        break;
      }
    }
  }
  event->cb();

  return FALSE;
}

GtkDispatcher::GtkDispatcher()
{
}

GtkDispatcher::~GtkDispatcher()
{
}

Dispatcher::HandlerID 
GtkDispatcher::add_input_handler(const Handler& cb, int fd)
{
  Guard guard(mutex_);
  
  HandlerID id = get_new_handler_id();
  FMap::iterator it = 
    fevents_.insert(make_pair(id, FileEvent(this, 0, cb, Read))).first;
  it->second.tag = gdk_input_add(fd, GDK_INPUT_READ, input_callback, 
                                 (gpointer)&it->second);

  return id;
}

Dispatcher::HandlerID 
GtkDispatcher::add_output_handler(const Handler& cb, int fd)
{
  Guard guard(mutex_);
  
  HandlerID id = get_new_handler_id();
  FMap::iterator it = fevents_.insert(
          make_pair(id, FileEvent(this, 0, cb, Write))).first;
  it->second.tag = gdk_input_add(fd, GDK_INPUT_WRITE, 
                                 input_callback, (gpointer)&it->second);

  return id;
}

Dispatcher::HandlerID 
GtkDispatcher::add_exception_handler(const Handler& cb, int fd)
{
  Guard guard(mutex_);

  HandlerID id = get_new_handler_id();
  FMap::iterator it = 
    fevents_.insert(make_pair(id, FileEvent(this, 0, cb, Except))).first;
  it->second.tag = gdk_input_add(fd, GDK_INPUT_EXCEPTION, 
                                 input_callback, (gpointer)&it->second);

  return id;
}

Dispatcher::HandlerID 
GtkDispatcher::add_timeout_handler(const Handler& cb, const TimeVal& tmout)
{
  guint32 interval = tmout.tv_sec * 1000 + tmout.tv_usec / 1000;

  Guard guard(mutex_);
  HandlerID id = get_new_handler_id();
  TMap::iterator it = 
    tevents_.insert(make_pair(id, TimerEvent(this, 0, cb))).first;
  it->second.tag = 
    gtk_timeout_add(interval, timer_callback, (gpointer)&it->second);

  return id;
}

void GtkDispatcher::remove(HandlerID id)
{
  Guard guard(mutex_);
  
  map<HandlerID, TimerEvent>::iterator tm_it = tevents_.find(id);
  if (tm_it != tevents_.end())
  {
    gtk_timeout_remove(tm_it->second.tag);
    tevents_.erase(tm_it);
  }

  FMap::iterator f_it = fevents_.find(id);
  if (f_it != fevents_.end())
  {
    gdk_input_remove(f_it->second.tag);
    fevents_.erase(f_it);
  }
}

bool GtkDispatcher::run(bool infinite)
{
  if (infinite)
  {
    gtk_main();
    return true;
  }
  else
    return gtk_main_iteration();
}

void GtkDispatcher::move(Dispatcher&)
{
  g_assert_not_reached();
}

bool GtkDispatcher::idle() const
{
  Guard guard(const_cast<GtkDispatcher *>(this)->mutex_);
  
  return (fevents_.size() + tevents_.size() == 0);
}

void GtkDispatcher::exit()
{
  gtk_main_quit();
}

}
