dnl 
dnl  SigC++ CORBA Signal Templates
dnl 
dnl  Copyright (C) 2000 Andreas Rottmann <rottmann@users.sourceforge.net>
dnl 
dnl  This library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Library General Public
dnl  License as published by the Free Software Foundation; either
dnl  version 2 of the License, or (at your option) any later version.
dnl 
dnl  This library is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Library General Public License for more details.
dnl 
dnl  You should have received a copy of the GNU Library General Public
dnl  License along with this library; if not, write to the Free
dnl  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl 
// -*- c++ -*-
dnl Ignore the next line
/* This is a generated file, do not edit.  Generated from __file__ */
include(template.macros.m4)
#ifndef __header__
#define __header__

#include <sigc++/corba_base.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

dnl
dnl CORBA_SIGNAL([P1, P2, ...])
dnl
define([CORBA_SIGNAL],
[/****************************************************************
*****  Signal NUM($1)
****************************************************************/
LINE(]__line__[)dnl

template <LIST(class R,1,ARG_CLASS($1),[$1])>
  class [CORBASignal]NUM($1) : public Object
  {
     typedef [Signal]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1])> SignalType;
   public:
     [CORBASignal]NUM($1)() {
        init(0, 0);
        sigc_signal_->connect(convert<LIST(R,1,ARG_TYPE($1),[$1])>(
            slot(this, &[CORBASignal]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1])>::corba_emit)));
     }
     [CORBASignal]NUM($1)(SignalType& signal) {
        init(&signal, 0);
        sigc_signal_->connect(convert<LIST(R,1,ARG_TYPE($1),[$1])>(
            slot(this, &[CORBASignal]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1])>::corba_emit)));
     }
     [CORBASignal]NUM($1)(corbaSigC::Signal_ptr signal) {
       init(0, signal);
       corba_signal_->connect(
         corbaSigC::Slot_impl::create(convert(sigc_signal_->slot())));
     }
     [CORBASignal]NUM($1)(SignalType& sigc_sig,
                          corbaSigC::Signal_ptr corba_sig) {
        init(&sigc_sig, corba_sig);
        sigc_signal_->connect(convert<LIST(R,1,ARG_TYPE($1),[$1])>(
                slot(this, &[CORBASignal]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1])>::corba_emit)));
     }
     [CORBASignal]NUM($1)(corbaSigC::Signal_ptr corba_sig,
                          SignalType& sigc_sig) {
       init(&sigc_sig, corba_sig);
       corba_signal_->connect(
         corbaSigC::Slot_impl::create(convert(sigc_signal_->slot())));
     }
     ~[CORBASignal]NUM($1)() {
       if (own_sigc_signal_)
         delete sigc_signal_;
       if (own_corba_signal_)
         CORBA::release(corba_signal_);
     }
     operator SignalType&() {
       return(sigc_signal_);
     }
     operator corbaSigC::Signal_ptr() {
       return(corba_signal_);
     }
     Connection connect(const [Slot]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1])>& s) {
       return(sigc_signal_->connect(s));
     }
     R emit(LIST(ARG_BOTH($1),[$1])) {
       return(sigc_signal_->emit(LIST(ARG_NAME($1),[$1])));
     }
   private:
     CORBA::Any *corba_emit(const ParamList& params) {
       return(corba_signal_->emit(params));
     }
     void init(SignalType *sigc_sig, corbaSigC::Signal_ptr corba_sig) {
       if (sigc_sig)
         sigc_signal_ = sigc_sig, own_sigc_signal_ = 0;
       else
         sigc_signal_ = new SignalType(), own_sigc_signal_ = 1;
       if (corba_sig)
        corba_signal_ = corba_sig, own_corba_signal_ = 0;
       else
       {
         corba_signal_ = corbaSigC::Signal_impl::create();
         own_corba_signal_ = 1;
       }
     }

     corbaSigC::Signal_ptr corba_signal_;
     SignalType *sigc_signal_;
     unsigned own_sigc_signal_ : 1;
     unsigned own_corba_signal_ : 1;
  };
])dnl

CORBA_SIGNAL(ARGS(P,0))
CORBA_SIGNAL(ARGS(P,1))
CORBA_SIGNAL(ARGS(P,2))
CORBA_SIGNAL(ARGS(P,3))
CORBA_SIGNAL(ARGS(P,4))
CORBA_SIGNAL(ARGS(P,5))
CORBA_SIGNAL(ARGS(P,6))
CORBA_SIGNAL(ARGS(P,7))

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
