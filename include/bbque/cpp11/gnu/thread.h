/*
 * Copyright (C) 2012  Politecnico di Milano
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This source is based on code from the GNU ISO C++ Library.
 * Copyright (C) 2008, 2009 Free Software Foundation, Inc.
 */

#ifndef BBQUE_GNU_THREAD_H_
#define BBQUE_GNU_THREAD_H_

#include "bbque/cpp11/chrono.h"
#include "bbque/cpp11/mutex.h"
#include "bbque/cpp11/condition_variable.h"

#include <functional>
#include <memory>
#include <cstddef>
#include <bits/functexcept.h>
#include <bits/gthr.h>

namespace std {

class thread {
public:
	typedef pthread_t native_handle_type;
	struct _Impl_base;
	typedef shared_ptr<_Impl_base> __shared_base_type;

	class id {
		native_handle_type _M_thread;

	public:
		id() : _M_thread() { }

		explicit
		id(native_handle_type __id) : _M_thread(__id) { }

	private:
		friend class thread;

		friend bool
			operator==(thread::id __x, thread::id __y) {
				return pthread_equal(__x._M_thread, __y._M_thread);
			}

		friend bool
			operator<(thread::id __x, thread::id __y) {
				return __x._M_thread < __y._M_thread;
			}

		template<class _CharT, class _Traits>
			friend basic_ostream<_CharT, _Traits>&
			operator<<(basic_ostream<_CharT, _Traits>&& __out, thread::id __id);
	};

	// Simple base type that the templatized, derived class containing
	// an arbitrary functor can be converted to and called.
	struct _Impl_base {
		__shared_base_type _M_this_ptr;

		//virtual ~_Impl_base() = default;
		virtual ~_Impl_base() {};

		virtual void _M_run() = 0;
	};

	template<typename _Callable>
	struct _Impl : public _Impl_base {
		_Callable _M_func;

		_Impl(_Callable&& __f) : _M_func(std::forward<_Callable>(__f))
		{ }

		void _M_run() { _M_func(); }
	};

private:
	id _M_id;

public:
	thread() = default;
	thread(const thread&) = delete;

	thread(thread&& __t) {
		swap(__t);
	}

	template<typename _Callable>
	explicit thread(_Callable __f) {
		_M_start_thread(_M_make_routine<_Callable>(__f));
	}

	template<typename _Callable, typename... _Args>
	thread(_Callable&& __f, _Args&&... __args) {
		_M_start_thread(_M_make_routine(std::bind(__f, __args...)));
	}

	~thread() {
		if (joinable())
			std::terminate();
	}

	thread& operator=(const thread&) = delete;

	thread& operator=(thread&& __t) {
		if (joinable())
			std::terminate();
		swap(__t);
		return *this;
	}

	void swap(thread& __t) {
		std::swap(_M_id, __t._M_id);
	}

	bool joinable() const {
		return !(_M_id == id());
	}

	void join();

	void detach();

	thread::id get_id() const {
		return _M_id;
	}

	/** @pre thread is joinable
	*/
	native_handle_type native_handle() {
		return _M_id._M_thread;
	}

	// Returns a value that hints at the number of hardware thread contexts.
	static unsigned int hardware_concurrency() {
		return 0;
	}

private:
	void _M_start_thread(__shared_base_type);

	template<typename _Callable>
	shared_ptr<_Impl<_Callable>>
	_M_make_routine(_Callable&& __f) {
		// Create and allocate full data structure, not base.
		return make_shared<_Impl<_Callable>>(std::forward<_Callable>(__f));
	}

};

inline void swap(thread& __x, thread& __y) {
	__x.swap(__y);
}

inline void swap(thread&& __x, thread& __y) {
	__x.swap(__y);
}

inline void swap(thread& __x, thread&& __y) {
	__x.swap(__y);
}

inline bool operator!=(thread::id __x, thread::id __y) {
	return !(__x == __y);
}

inline bool operator<=(thread::id __x, thread::id __y) {
	return !(__y < __x);
}

inline bool operator>(thread::id __x, thread::id __y) {
	return __y < __x;
}

inline bool operator>=(thread::id __x, thread::id __y) {
	return !(__x < __y);
}

template<class _CharT, class _Traits>
inline basic_ostream<_CharT, _Traits>&
operator<<(basic_ostream<_CharT, _Traits>&& __out, thread::id __id) {
	if (__id == thread::id())
		return __out << "thread::id of a non-executing thread";
	else
		return __out << __id._M_thread;
}

/** @namespace std::this_thread
 *  @brief ISO C++ 0x entities sub namespace for thread.
 *  30.2.2 Namespace this_thread.
 */
namespace this_thread {

	/// get_id
	inline thread::id get_id() {
		return thread::id(pthread_self());
	}

#ifdef _GLIBCXX_USE_SCHED_YIELD
	/// yield
	inline void yield() {
		pthread_yield();
	}
#endif

	/// sleep_until
	template<typename _Clock, typename _Duration>
	inline void sleep_until(const chrono::time_point<_Clock, _Duration>& __atime) {
		sleep_for(__atime - _Clock::now());
	}

	/// sleep_for
	template<typename _Rep, typename _Period>
	inline void sleep_for(const chrono::duration<_Rep, _Period>& __rtime) {
		chrono::seconds __s =
			chrono::duration_cast<chrono::seconds>(__rtime);

		chrono::nanoseconds __ns =
			chrono::duration_cast<chrono::nanoseconds>(__rtime - __s);

		struct timespec __ts = {
			static_cast<std::time_t>(__s.count()),
			static_cast<long>(__ns.count())
		};

		::nanosleep(&__ts, 0);
	}

} // namespace this_thread

} // namespace std

#endif /* end of include guard: BBQUE_GNU_THREAD_H_ */
