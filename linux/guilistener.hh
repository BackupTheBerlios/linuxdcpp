/* 
* Copyright (C) 2001-2003 Jens Oknelid, paskharen@spray.se
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

#ifndef WULFOR_GUI_LISTENER
#define WULFOR_GUI_LISTENER

#include <vector>
#include <iostream>

#include <gtkmm.h>

#include <sigcx/thread_tunnel.h>
#include <sigcx/gtk_dispatch.h>

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"
#include "../client/HubManager.h"
#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/QueueManagerListener.h"
#include "../client/ConnectionManagerListener.h"
#include "../client/Client.h"
#include "../client/SearchManagerListener.h"
#include "../client/TimerManager.h"
#include "../client/CriticalSection.h"
#include "../client/Speaker.h"

/*
	This is a somewhat ugly way of allowing for storing multiple types
	of GuiListeners in the same vector in the GuiProxy
*/
class GuiListenerBase {
	public:
		virtual ~GuiListenerBase() {};
		virtual void blah() {}
};


/*
	This class is the one that actually listens to the backend. Since
	there are lots of different speakers it needs to be a template class,
	so that we can create different objects for different kinds of speakers.
	
	Listener should be the class that will hear the messages, like MainWindow.
	This needs to inherit from Type and from SigC::Object.
	
	Type is the type of listener, TimerManagerListener for example.
*/
template<class Listener, class Type>
class GuiListener:
	public Type,
	public GuiListenerBase
{
	public:

		GuiListener (Listener *myListener, 
			Speaker<Type> *mySpeaker, SigCX::ThreadTunnel *tunnel)
		{
			this->mySpeaker = mySpeaker;
			this->myListener = myListener;
			this->tunnel = tunnel;
			mySpeaker->addListener(this);
		}
		
		~GuiListener() {
			mySpeaker->removeListener(this);
		}

		//ConnectionManagerListener
		void on(ConnectionManagerListener::Added t, ConnectionQueueItem *a) throw()
		{
			realOn (t, a);
		}
		void on(ConnectionManagerListener::Connected t, ConnectionQueueItem *a) throw()
		{
			realOn (t, a);
		}
		void on(ConnectionManagerListener::Removed t, ConnectionQueueItem *a) throw()
		{
			realOn (t, a);
		}
		void on(ConnectionManagerListener::Failed t, ConnectionQueueItem *a, const string& b) throw()
		{
			realOnConst (t, a, b);
		}
		void on(ConnectionManagerListener::StatusChanged t, ConnectionQueueItem *a) throw()
		{
			realOn (t, a);
		}
		
		//HubManagerListener
		void on(HubManagerListener::DownloadStarting t, const string& a) throw()
			{realOnConst(t, a);}
		void on(HubManagerListener::DownloadFailed t, const string& a) throw() 
			{realOnConst(t, a);}
		void on(HubManagerListener::DownloadFinished t, const string& a) throw()
			{realOnConst(t, a);}
		void on(HubManagerListener::FavoriteAdded t, const FavoriteHubEntry* a) 
			throw()
			{realOn(t, a);}
		void on(HubManagerListener::FavoriteRemoved t, 
			const FavoriteHubEntry* a) throw()
			{realOn(t, a);}
		void on(HubManagerListener::UserAdded t, const User::Ptr& a) throw()
			{realOnConst(t, a);}
		void on(HubManagerListener::UserRemoved t, const User::Ptr& a) throw()
			{realOnConst(t, a);}

		//DownloadManagerListener
		void on(DownloadManagerListener::Starting t, Download* a) throw()
			{realOn(t, a);}
		void on(DownloadManagerListener::Tick t, const Download::List& a)
			throw()
			{realOnConst(t, a);}
		void on(DownloadManagerListener::Complete t, Download* a) throw() 
			{realOn(t, a);}
		void on(DownloadManagerListener::Failed t, Download* a, 
			const std::string& b) throw()
			{realOnConst(t, a, b);}

		//UploadManagerListener
		void on(UploadManagerListener::Starting t, Upload* a) throw()
			{realOn(t, a);}
		void on(UploadManagerListener::Tick t, const Upload::List& a) throw() 
			{realOnConst(t, a);}
		void on(UploadManagerListener::Complete t, Upload* a) throw()
			{realOn(t, a);}
		void on(UploadManagerListener::Failed t, Upload* a, 
			const std::string& b) throw()
			{realOnConst(t, a, b);}

		//QueueManagerListener
		void on(QueueManagerListener::Added t, QueueItem* a) throw()
			{realOn(t, a);}
		void on(QueueManagerListener::Finished t, QueueItem* a) throw()
			{realOn(t, a);}
		void on(QueueManagerListener::Removed t, QueueItem* a) throw()
			{realOn(t, a);}
		void on(QueueManagerListener::Moved t, QueueItem* a) throw()
			{realOn(t, a);}
		void on(QueueManagerListener::SourcesUpdated t, QueueItem* a) throw()
			{realOn(t, a);}
		void on(QueueManagerListener::StatusUpdated t, QueueItem* a) throw()
			{realOn(t, a);}
		void on(QueueManagerListener::SearchStringUpdated t, QueueItem* a)
			throw()
			{realOn(t, a);}

		//ClientListener
		void on(ClientListener::Connecting t, Client* a) throw()
			{realOn(t, a);}
		void on(ClientListener::Connected t, Client* a) throw()
			{realOn(t, a);}
		void on(ClientListener::BadPassword t, Client* a) throw()
			{realOn(t, a);}
		void on(ClientListener::UserUpdated t, Client* a, const User::Ptr& b)
			throw()
			{realOnConst(t, a, b);}
		void on(ClientListener::UsersUpdated t, Client* a, const User::List& b)
			throw()
			{realOnConst(t, a, b);}
		void on(ClientListener::UserRemoved t, Client* a, const User::Ptr& b)
			throw() 
			{realOnConst(t, a, b);}
		void on(ClientListener::Redirect t, Client* a, const string& b) throw() 
			{realOnConst(t, a, b);}
		void on(ClientListener::Failed t, Client* a, const string& b) throw() 
			{realOnConst(t, a, b);}
		void on(ClientListener::GetPassword t, Client* a) throw()
			{realOn(t, a);}
		void on(ClientListener::HubUpdated t, Client* a) throw()
			{realOn(t, a);}
		void on(ClientListener::Message t, Client* a, const string& b) throw() 
			{realOnConst(t, a, b);}
		/*
			this is not supported since SigCX only supports 3 arg slots
			in the thread_tunnel. Should probably compile a special
			version of SigCX at some point...
		*/
		void on(ClientListener::PrivateMessage t, Client* a, const User::Ptr& b,
			const string& c) throw()
			{realOnConst(t, a, b, c);}
		//see above
		void on(ClientListener::UserCommand t, Client* a, int b, int c, 
			const string& d, const string& e) throw()
			{realOnConst(t, a, b, c, d, e);}
		void on(ClientListener::HubFull t, Client* a) throw()
			{realOn(t, a);}
		void on(ClientListener::NickTaken t, Client* a) throw()
			{realOn(t, a);}
		void on(ClientListener::SearchFlood t, Client* a, const string& b)
			throw()
			{realOnConst(t, a, b);}
		//see above
		void on(ClientListener::NmdcSearch t, Client* a, const string& b, int c,
			int64_t d, int e, const string& f) throw()
			{realOnConst(t, a, b, c, d, e, f);}

		//SearchManagerListener
		void on(SearchManagerListener::SR t, SearchResult *a) throw()
			{realOn(t, a);}
		
		//TimerManagerListener
		void on(TimerManagerListener::Second t, u_int32_t a) throw() 
			{realOn(t, a);}
		void on(TimerManagerListener::Minute t, u_int32_t a) throw()
			{realOn(t, a);}

	private:
		template<class T0>
		void realOn(const T0 &type) throw() {
			Lock l(listenerCS);
			void (Type::* func)(T0) = &Type::on;

			SigC::Slot1<void, T0> callback = open_tunnel(
				tunnel, SigC::slot(*myListener, &Listener::on), false);
			callback(type);
		}

		template<class T0, class T1>
		void realOn(T0 type, T1 p1) throw() {
			Lock l(listenerCS);
			void (Type::* func)(T0, T1) = &Type::on;
			
			SigC::Slot2<void, T0, T1> callback = open_tunnel(
				tunnel, SigC::slot(*myListener, func), false);
			callback(type, p1);
		}

		template<class T0, class T1, class T2>
		void realOn(T0 type, T1 p1, T2 p2) throw() {
			Lock l(listenerCS);
			void (Type::* func)(T0, T1, T2) = &Type::on;

			SigC::Slot3<void, T0, T1, T2> callback = open_tunnel(
				tunnel, SigC::slot(*myListener, func), false);
			callback(type, p1, p2);
		}

		template<typename T0, class T1, class T2, class T3>
		void realOn(T0 type, const T1& p1, const T2& p2, const T3& p3) throw() {
			Lock l(listenerCS);
			void (Type::* func)(T0, T1, T2, T3) = &Type::on;
			
			SigC::Slot4<void, T0, T1, T2, T3> callback = open_tunnel(
				tunnel, SigC::slot(*myListener, func), false);
			callback(type, p1, p2, p3);
		}

		template<typename T0, class T1, class T2, class T3, class T4>
		void realOn(T0 type, const T1& p1, 
			const T2& p2, const T3& p3, const T4& p4) throw()
		{
			Lock l(listenerCS);
			void (Type::* func)(T0, T1, T2, T3, T4) = &Type::on;

			SigC::Slot5<void, T0, T1, T2, T3, T4> callback = open_tunnel(
				tunnel, SigC::slot(*myListener, func), false);
			callback(type, p1, p2, p3, p4);
		}

		template<typename T0, class T1, class T2, class T3, class T4, class T5>
		void realOn(T0 type, const T1& p1, const T2& p2, 
			const T3& p3, const T4& p4, const T5& p5) throw()
		{
			Lock l(listenerCS);
			void (Type::* func)(T0, T1, T2, T3, T4, T5) = &Type::on;

			SigC::Slot6<void, T0, T1, T2, T3, T4, T5> callback = open_tunnel(
				tunnel, SigC::slot(*myListener, &Listener::on), false);
			callback(type, p1, p2, p3, p4, p5);
		}

		template<typename T0, 
			class T1, class T2, class T3, class T4, class T5, class T6>
		void realOn(T0 type, const T1& p1, const T2& p2, 
			const T3& p3, const T4& p4, const T5& p5, const T6& p6) throw()
		{
			Lock l(listenerCS);
			void (Type::* func)(T0, T1, T3, T4, T5, T6) = &Type::on;

			SigC::Slot7<void, T0, T1, T2, T3, T4, T5, T6> callback =
				open_tunnel(tunnel, SigC::slot(*myListener, func), false);
			callback(type, p1, p2, p3, p4, p5, p6);
		}

		template<class T0, class T1>
		void realOnConst(T0 type, const T1 &p1) throw() {
			Lock l(listenerCS);
			void (Type::* func)(T0, const T1&) = &Type::on;
			
			SigC::Slot2<void, T0, const T1&> callback = open_tunnel(
				tunnel, SigC::slot(*myListener, func), false);
			callback(type, p1);
		}

		template<class T0, class T1, class T2>
		void realOnConst(T0 type, T1 p1, const T2 &p2) throw() {
			Lock l(listenerCS);
			void (Type::* func)(T0, T1, const T2&) = &Type::on;

			SigC::Slot3<void, T0, T1, const T2&> callback = open_tunnel(
				tunnel, SigC::slot(*myListener, func), false);
			callback(type, p1, p2);
		}

		template<class T0, class T1, class T2, class T3>
		void realOnConst(T0 type, T1 p1, const T2& p2, const T3& p3)
			throw()
		{
			Lock l(listenerCS);
			void (Type::* func)
				(T0, T1, const T2&, const T3&) = &Type::on;

			SigC::Slot4<void, T0, T1, const T2&, const T3&> callback =
				open_tunnel(tunnel, SigC::slot(*myListener, func), false);
			callback(type, p1, p2, p3);
		}

		template<typename T0, 
			class T1, class T2, class T3, class T4, class T5>
		void realOnConst(T0 type, T1 p1, T2 p2, T3 p3, 
			const T4 &p4, const T5& p5) throw()
		{
			Lock l(listenerCS);
			void (Type::* func)(T0, T1, T2, T3, const T4&, const T5&) 
				= &Type::on;

			SigC::Slot6<void, T0, T1, T2, T3, const T4&, const T5&> callback =
				open_tunnel(tunnel, SigC::slot(*myListener, func), false);
			callback(type, p1, p2, p3, p4, p5);
		}

		template<typename T0, 
			class T1, class T2, class T3, class T4, class T5, class T6>
		void realOnConst(T0 type, T1 p1, const T2& p2, T3 p3, T4 p4, T5 p5,
			const T6& p6) throw()
		{
			Lock l(listenerCS);
			void (Type::* func)(T0, T1, const T2&, T3, T4, T5, const T6&) 
				= &Type::on;

			SigC::Slot7<void, T0, T1, const T2&, T3, T4, T5, const T6&> callback
				= open_tunnel(tunnel, SigC::slot(*myListener, func), false);
			callback(type, p1, p2, p3, p4, p5, p6);
		}

		SigCX::ThreadTunnel *tunnel;
		Speaker<Type> *mySpeaker;
		Listener *myListener;
		
		CriticalSection listenerCS;
};

#else
template<class Listener, class Type>
class GuiListener;
#endif
