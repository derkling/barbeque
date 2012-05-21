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

#ifndef BBQUE_GNU_CONDITION_VARIABLE_H_
#define BBQUE_GNU_CONDITION_VARIABLE_H_

#include "bbque/cpp11/chrono.h"
#include "bbque/cpp11/mutex.h"

namespace std {

  /**
   * @defgroup condition_variables Condition Variables
   * @ingroup concurrency
   *
   * Classes for condition_variable support.
   * @{
   */

  /// cv_status
  enum class cv_status { no_timeout, timeout };

  /// condition_variable
  class condition_variable
  {
    typedef chrono::system_clock	__clock_t;
    typedef pthread_cond_t		__native_type;
    __native_type			_M_cond;

  public:
    typedef __native_type* 		native_handle_type;

    condition_variable() throw ();
    ~condition_variable() throw ();

    condition_variable(const condition_variable&) = delete;
    condition_variable& operator=(const condition_variable&) = delete;

    void
    notify_one();

    void
    notify_all();

    void
    wait(unique_lock<mutex>& __lock);

    template<typename _Predicate>
      void
      wait(unique_lock<mutex>& __lock, _Predicate __p)
      {
	while (!__p())
	  wait(__lock);
      }

    template<typename _Duration>
      cv_status
      wait_until(unique_lock<mutex>& __lock,
		 const chrono::time_point<__clock_t, _Duration>& __atime)
      { return __wait_until_impl(__lock, __atime); }

    template<typename _Clock, typename _Duration>
      cv_status
      wait_until(unique_lock<mutex>& __lock,
		 const chrono::time_point<_Clock, _Duration>& __atime)
      {
	// DR 887 - Sync unknown clock to known clock.
	const typename _Clock::time_point __c_entry = _Clock::now();
	const __clock_t::time_point __s_entry = __clock_t::now();
	const chrono::nanoseconds __delta = __atime - __c_entry;
	const __clock_t::time_point __s_atime = __s_entry + __delta;

	return __wait_until_impl(__lock, __s_atime);
      }

    template<typename _Clock, typename _Duration, typename _Predicate>
      bool
      wait_until(unique_lock<mutex>& __lock,
		 const chrono::time_point<_Clock, _Duration>& __atime,
		 _Predicate __p)
      {
	while (!__p())
	  if (wait_until(__lock, __atime) == cv_status::timeout)
	    return __p();
	return true;
      }

    template<typename _Rep, typename _Period>
      cv_status
      wait_for(unique_lock<mutex>& __lock,
	       const chrono::duration<_Rep, _Period>& __rtime)
      { return wait_until(__lock, __clock_t::now() + __rtime); }

    template<typename _Rep, typename _Period, typename _Predicate>
      bool
      wait_for(unique_lock<mutex>& __lock,
	       const chrono::duration<_Rep, _Period>& __rtime,
	       _Predicate __p)
      { return wait_until(__lock, __clock_t::now() + __rtime, std::move(__p)); }

    native_handle_type
    native_handle()
    { return &_M_cond; }

  private:
    template<typename _Clock, typename _Duration>
      cv_status
      __wait_until_impl(unique_lock<mutex>& __lock,
			const chrono::time_point<_Clock, _Duration>& __atime)
      {
	chrono::time_point<__clock_t, chrono::seconds> __s =
	  chrono::time_point_cast<chrono::seconds>(__atime);

	chrono::nanoseconds __ns =
	  chrono::duration_cast<chrono::nanoseconds>(__atime - __s);

	struct timespec __ts =
	  {
	    static_cast<std::time_t>(__s.time_since_epoch().count()),
	    static_cast<long>(__ns.count())
	  };

	pthread_cond_timedwait(&_M_cond, __lock.mutex()->native_handle(),
				 &__ts);

	return (_Clock::now() < __atime
		? cv_status::no_timeout : cv_status::timeout);
      }
  };

  /// condition_variable_any
  // Like above, but mutex is not required to have try_lock.
  class condition_variable_any
  {
    typedef chrono::system_clock	__clock_t;
    condition_variable			_M_cond;
    mutex				_M_mutex;

  public:
    typedef condition_variable::native_handle_type	native_handle_type;

    condition_variable_any() throw ();
    ~condition_variable_any() throw ();

    condition_variable_any(const condition_variable_any&) = delete;
    condition_variable_any& operator=(const condition_variable_any&) = delete;

    void
    notify_one()
    {
      lock_guard<mutex> __lock(_M_mutex);
      _M_cond.notify_one();
    }

    void
    notify_all()
    {
      lock_guard<mutex> __lock(_M_mutex);
      _M_cond.notify_all();
    }

    template<typename _Lock>
      void
      wait(_Lock& __lock)
      {
	// scoped unlock - unlocks in ctor, re-locks in dtor
	struct _Unlock {
	  explicit _Unlock(_Lock& __lk) : _M_lock(__lk) { __lk.unlock(); }
	  ~_Unlock() noexcept(false)
	  {
	    if (uncaught_exception())
	      __try { _M_lock.lock(); } __catch(...) { }
	    else
	      _M_lock.lock();
	  }
	  _Lock& _M_lock;
	};

	unique_lock<mutex> __my_lock(_M_mutex);
	_Unlock __unlock(__lock);
	// _M_mutex must be unlocked before re-locking __lock so move
	// ownership of _M_mutex lock to an object with shorter lifetime.
	unique_lock<mutex> __my_lock2(std::move(__my_lock));
	_M_cond.wait(__my_lock2);
      }

    template<typename _Lock, typename _Predicate>
      void
      wait(_Lock& __lock, _Predicate __p)
      {
	while (!__p())
	  wait(__lock);
      }

    template<typename _Lock, typename _Clock, typename _Duration>
      cv_status
      wait_until(_Lock& __lock,
		 const chrono::time_point<_Clock, _Duration>& __atime)
      {
        unique_lock<mutex> __my_lock(_M_mutex);
        __lock.unlock();
        cv_status __status = _M_cond.wait_until(__my_lock, __atime);
        __lock.lock();
        return __status;
      }

    template<typename _Lock, typename _Clock,
	     typename _Duration, typename _Predicate>
      bool
      wait_until(_Lock& __lock,
		 const chrono::time_point<_Clock, _Duration>& __atime,
		 _Predicate __p)
      {
	while (!__p())
	  if (wait_until(__lock, __atime) == cv_status::timeout)
	    return __p();
	return true;
      }

    template<typename _Lock, typename _Rep, typename _Period>
      cv_status
      wait_for(_Lock& __lock, const chrono::duration<_Rep, _Period>& __rtime)
      { return wait_until(__lock, __clock_t::now() + __rtime); }

    template<typename _Lock, typename _Rep,
	     typename _Period, typename _Predicate>
      bool
      wait_for(_Lock& __lock,
	       const chrono::duration<_Rep, _Period>& __rtime, _Predicate __p)
      { return wait_until(__lock, __clock_t::now() + __rtime, std::move(__p)); }

    native_handle_type
    native_handle()
    { return _M_cond.native_handle(); }
  };

} // namespace std

#endif /* end of include guard: BBQUE_GNU_CONDITION_VARIABLE_H_ */
