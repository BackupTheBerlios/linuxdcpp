/* 
* Copyright (C) 2004 Jens Oknelid, paskharen@spray.se
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef WULFOR_CALLBACK_HH
#define WULFOR_CALLBACK_HH

#include <gtk/gtk.h>
#include <string>

template <class C>
class Callback1 {
	public:
		Callback1(C *obj, void (C::*func)(gpointer)):
			_obj(obj),
			_func(func)
		{
			
		}
		
		void connect(GObject *obj, std::string signal, gpointer data) {
			this->data = data;
		    g_signal_connect(obj, signal.c_str(), 
				G_CALLBACK(callFunc), (gpointer)this);
		}

		void connect_after(GObject *obj, std::string signal, gpointer data) {
			this->data = data;
		    g_signal_connect_after(obj, signal.c_str(), 
				G_CALLBACK(callFunc), (gpointer)this);
		}

	private:
		static void callFunc(gpointer data) {
			Callback1<C> *c = (Callback1<C> *)data;
			c->call();
		}
		
		void call() {
			(*_obj.*_func)(data);
		}
		
		C *_obj;
		void (C::*_func)(gpointer);
		gpointer data;
};

template <class C, typename P1>
class Callback2 {
	public:
		Callback2(C *obj, void (C::*func)(P1, gpointer)):
			_obj(obj),
			_func(func)
		{
			
		}
		
		void connect(GObject *obj, std::string signal, gpointer data) {
			this->data = data;
		    g_signal_connect(obj, signal.c_str(), 
				G_CALLBACK(callFunc), (gpointer)this);
		}

		void connect_after(GObject *obj, std::string signal, gpointer data) {
			this->data = data;
		    g_signal_connect_after(obj, signal.c_str(), 
				G_CALLBACK(callFunc), (gpointer)this);
		}

	private:
		static void callFunc(P1 param1, gpointer data) {
			Callback2<C, P1> *c = (Callback2<C, P1> *)data;
			c->call(param1);
		}
		
		void call(P1 param1) {
			(*_obj.*_func)(param1, data);
		}
		
		C *_obj;
		void (C::*_func)(P1, gpointer);
		gpointer data;
};

template <class C, typename P1, typename P2>
class Callback3 {
	public:
		Callback3(C *obj, void (C::*func)(P1, P2, gpointer)):
			_obj(obj),
			_func(func)
		{
			
		}
		
		void connect(GObject *obj, std::string signal, gpointer data) {
			this->data = data;
		    g_signal_connect(obj, signal.c_str(), 
				G_CALLBACK(callFunc), (gpointer)this);
		}

		void connect_after(GObject *obj, std::string signal, gpointer data) {
			this->data = data;
		    g_signal_connect_after(obj, signal.c_str(), 
				G_CALLBACK(callFunc), (gpointer)this);
		}

	private:
		static void callFunc(P1 param1, P2 param2, gpointer data) {
			Callback3<C, P1, P2> *c = (Callback3<C, P1, P2> *)data;
			c->call(param1, param2);
		}
		
		void call(P1 param1, P2 param2) {
			(*_obj.*_func)(param1, param2, data);
		}
		
		C *_obj;
		void (C::*_func)(P1, P2, gpointer);
		gpointer data;
};

#else
template <class C>
class Callback1;
template <class C, typename P1>
class Callback2;
template <class C, typename P1, typename P2>
class Callback3;
#endif
