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

#ifndef WULFOR_FUNC_HH
#define WULFOR_FUNC_HH

class FuncBase {
	public:
		virtual void call() {};
};

template<class c>
class Func0: public FuncBase {
	public:
		Func0(c *obj, void (c::*func)()) {
			this->obj = obj;
			this->func = func;
		}

		void call() {
			(*obj.*func)();
		}

	private:
		c *obj;
		void (c::*func)();
};

template<class c, class p1>
class Func1: public FuncBase {
	public:
		Func1(c *obj, void (c::*func)(p1), p1 param1) {
			this->obj = obj;
			this->func = func;
			this->param1 = param1;
		}

		void call() {
			(*obj.*func)(param1);
		}

	private:
		c *obj;
		void (c::*func)(p1);
		p1 param1;
};

template<class c, class p1, class p2>
class Func2: public FuncBase {
	public:
		Func2(c *obj, void (c::*func)(p1, p2), p1 param1, p2 param2) {
			this->obj = obj;
			this->func = func;
			this->param1 = param1;
			this->param2 = param2;
		}

		void call() {
			(*obj.*func)(param1, param2);
		}

	private:
		c *obj;
		void (c::*func)(p1, p2);
		p1 param1;
		p2 param2;
};

template<class c, class p1, class p2, class p3>
class Func3: public FuncBase {
	public:
		Func3(c *obj, void (c::*func)(p1, p2, p3), p1 param1, p2 param2, p3 param3) {
			this->obj = obj;
			this->func = func;
			this->param1 = param1;
			this->param2 = param2;
			this->param3 = param3;
		}

		void call() {
			(*obj.*func)(param1, param2, param3);
		}

	private:
		c *obj;
		void (c::*func)(p1, p2, p3);
		p1 param1;
		p2 param2;
		p3 param3;
};

template<class c, class p1, class p2, class p3, class p4>
class Func4: public FuncBase {
	public:
		Func4(c *obj, void (c::*func)(p1, p2, p3, p4), 
			p1 param1, p2 param2, p3 param3, p4 param4)
		{
			this->obj = obj;
			this->func = func;
			this->param1 = param1;
			this->param2 = param2;
			this->param3 = param3;
			this->param4 = param4;
		}

		void call() {
			(*obj.*func)(param1, param2, param3, param4);
		}

	private:
		c *obj;
		void (c::*func)(p1, p2, p3, p4);
		p1 param1;
		p2 param2;
		p3 param3;
		p4 param4;
};

template<class c, class p1, class p2, class p3, class p4, class p5>
class Func5: public FuncBase {
	public:
		Func5(c *obj, void (c::*func)(p1, p2, p3, p4, p5), 
			p1 param1, p2 param2, p3 param3, p4 param4, p5 param5)
		{
			this->obj = obj;
			this->func = func;
			this->param1 = param1;
			this->param2 = param2;
			this->param3 = param3;
			this->param4 = param4;
			this->param5 = param5;
		}

		void call() {
			(*obj.*func)(param1, param2, param3, param4, param5);
		}

	private:
		c *obj;
		void (c::*func)(p1, p2, p3, p4, p5);
		p1 param1;
		p2 param2;
		p3 param3;
		p4 param4;
		p5 param5;
};

template<class c, class p1, class p2, class p3, class p4, class p5, class p6>
class Func6: public FuncBase {
	public:
		Func6(c *obj, void (c::*func)(p1, p2, p3, p4, p5, p6), 
			p1 param1, p2 param2, p3 param3, p4 param4, p5 param5, p6 param6)
		{
			this->obj = obj;
			this->func = func;
			this->param1 = param1;
			this->param2 = param2;
			this->param3 = param3;
			this->param4 = param4;
			this->param5 = param5;
			this->param6 = param6;
		}

		void call() {
			(*obj.*func)(param1, param2, param3, param4, param5, param6);
		}

	private:
		c *obj;
		void (c::*func)(p1, p2, p3, p4, p5, p6);
		p1 param1;
		p2 param2;
		p3 param3;
		p4 param4;
		p5 param5;
		p6 param6;
};

#else
template<class c>
class Func0;
template<class c, class p1>
class Func1;
template<class c, class p1, class p2>
class Func2;
template<class c, class p1, class p2, class p3>
class Func3;
template<class c, class p1, class p2, class p3, class p4, class p5, class p6>
class Func6;
#endif
