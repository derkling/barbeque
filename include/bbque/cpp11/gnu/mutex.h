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

#ifndef BBQUE_GNU_MUTEX_H_
#define BBQUE_GNU_MUTEX_H_

#include "bbque/cpp11/chrono.h"

#include <tuple>
#include <exception>
#include <type_traits>
#include <functional>
#include <system_error>
#include <bits/functexcept.h>
#include <bits/gthr.h>
#include <bits/move.h> // for std::swap

namespace std {

  /// mutex
  class mutex
  {
    typedef pthread_mutex_t			__native_type;
    __native_type  _M_mutex;

  public:
    typedef __native_type* 			native_handle_type;

    constexpr mutex() : _M_mutex(PTHREAD_MUTEX_INITIALIZER) { }

    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;

    void
    lock()
    {
      int __e = pthread_mutex_lock(&_M_mutex);

      // EINVAL, EAGAIN, EBUSY, EINVAL, EDEADLK(may)
      if (__e)
	__throw_system_error(__e);
    }

    bool
    try_lock()
    {
      // XXX EINVAL, EAGAIN, EBUSY
      return ! pthread_mutex_trylock(&_M_mutex);
    }

    void
    unlock()
    {
      // XXX EINVAL, EAGAIN, EPERM
      pthread_mutex_unlock(&_M_mutex);
    }

    native_handle_type
    native_handle()
    { return &_M_mutex; }
  };

  // FIXME: gthreads doesn't define __gthread_recursive_mutex_destroy
  // so we need to obtain a __gthread_mutex_t to destroy
  class __destroy_recursive_mutex
  {
    template<typename _Mx, typename _Rm>
      static void
      _S_destroy_win32(_Mx* __mx, _Rm const* __rmx)
      {
        __mx->counter = __rmx->counter;
        __mx->sema = __rmx->sema;
        pthread_mutex_destroy(__mx);
      }

  public:
    // matches a gthr-win32.h recursive mutex
    template<typename _Rm>
      static typename enable_if<sizeof(&_Rm::sema), void>::type
      _S_destroy(_Rm* __mx)
      {
        pthread_mutex_t __tmp;
        _S_destroy_win32(&__tmp, __mx);
      }

    // matches a recursive mutex with a member 'actual'
    template<typename _Rm>
      static typename enable_if<sizeof(&_Rm::actual), void>::type
      _S_destroy(_Rm* __mx)
      { pthread_mutex_destroy(&__mx->actual); }

    // matches when there's only one mutex type
    template<typename _Rm>
      static
      typename enable_if<is_same<_Rm, pthread_mutex_t>::value, void>::type
      _S_destroy(_Rm* __mx)
      { pthread_mutex_destroy(__mx); }
  };

  /// recursive_mutex
  class recursive_mutex
  {
    typedef pthread_mutex_t		__native_type;
    __native_type  _M_mutex;

  public:
    typedef __native_type* 			native_handle_type;

#if defined(__BIONIC__)
    recursive_mutex() : _M_mutex(PTHREAD_RECURSIVE_MUTEX_INITIALIZER) { }
#elif defined(__GTHREAD_RECURSIVE_MUTEX_INIT)
    recursive_mutex() : _M_mutex(__GTHREAD_RECURSIVE_MUTEX_INIT) { }
# else
    recursive_mutex()
    {
      // XXX EAGAIN, ENOMEM, EPERM, EBUSY(may), EINVAL(may)
      __GTHREAD_RECURSIVE_MUTEX_INIT_FUNCTION(&_M_mutex);
    }

    ~recursive_mutex()
    { __destroy_recursive_mutex::_S_destroy(&_M_mutex); }
#endif


    recursive_mutex(const recursive_mutex&) = delete;
    recursive_mutex& operator=(const recursive_mutex&) = delete;

    void
    lock()
    {
      int __e = pthread_mutex_lock(&_M_mutex);

      // EINVAL, EAGAIN, EBUSY, EINVAL, EDEADLK(may)
      if (__e)
	__throw_system_error(__e);
    }

    bool
    try_lock()
    {
      // XXX EINVAL, EAGAIN, EBUSY
      return !pthread_mutex_trylock(&_M_mutex);
    }

    void
    unlock()
    {
      // XXX EINVAL, EAGAIN, EBUSY
      pthread_mutex_unlock(&_M_mutex);
    }

    native_handle_type
    native_handle()
    { return &_M_mutex; }
  };

  /// timed_mutex
  // NOTE Not supported by Android::Bionic
  // due to missing pthread_mutex_timedlock

  /// Do not acquire ownership of the mutex.
  struct defer_lock_t { };

  /// Try to acquire ownership of the mutex without blocking.
  struct try_to_lock_t { };

  /// Assume the calling thread has already obtained mutex ownership
  /// and manage it.
  struct adopt_lock_t { };

  constexpr defer_lock_t	defer_lock { };
  constexpr try_to_lock_t	try_to_lock { };
  constexpr adopt_lock_t	adopt_lock { };

  /// @brief  Scoped lock idiom.
  // Acquire the mutex here with a constructor call, then release with
  // the destructor call in accordance with RAII style.
  template<typename _Mutex>
    class lock_guard
    {
    public:
      typedef _Mutex mutex_type;

      explicit lock_guard(mutex_type& __m) : _M_device(__m)
      { _M_device.lock(); }

      lock_guard(mutex_type& __m, adopt_lock_t) : _M_device(__m)
      { } // calling thread owns mutex

      ~lock_guard()
      { _M_device.unlock(); }

      lock_guard(const lock_guard&) = delete;
      lock_guard& operator=(const lock_guard&) = delete;

    private:
      mutex_type&  _M_device;
    };

  /// unique_lock
  template<typename _Mutex>
    class unique_lock
    {
    public:
      typedef _Mutex mutex_type;

      unique_lock()
      : _M_device(0), _M_owns(false)
      { }

      explicit unique_lock(mutex_type& __m)
      : _M_device(&__m), _M_owns(false)
      {
	lock();
	_M_owns = true;
      }

      unique_lock(mutex_type& __m, defer_lock_t)
      : _M_device(&__m), _M_owns(false)
      { }

      unique_lock(mutex_type& __m, try_to_lock_t)
      : _M_device(&__m), _M_owns(_M_device->try_lock())
      { }

      unique_lock(mutex_type& __m, adopt_lock_t)
      : _M_device(&__m), _M_owns(true)
      {
	// XXX calling thread owns mutex
      }

      template<typename _Clock, typename _Duration>
	unique_lock(mutex_type& __m,
		    const chrono::time_point<_Clock, _Duration>& __atime)
	: _M_device(&__m), _M_owns(_M_device->try_lock_until(__atime))
	{ }

      template<typename _Rep, typename _Period>
	unique_lock(mutex_type& __m,
		    const chrono::duration<_Rep, _Period>& __rtime)
	: _M_device(&__m), _M_owns(_M_device->try_lock_for(__rtime))
	{ }

      ~unique_lock()
      {
	if (_M_owns)
	  unlock();
      }

      unique_lock(const unique_lock&) = delete;
      unique_lock& operator=(const unique_lock&) = delete;

      unique_lock(unique_lock&& __u)
      : _M_device(__u._M_device), _M_owns(__u._M_owns)
      {
	__u._M_device = 0;
	__u._M_owns = false;
      }

      unique_lock& operator=(unique_lock&& __u)
      {
	if(_M_owns)
	  unlock();

	unique_lock(std::move(__u)).swap(*this);

	__u._M_device = 0;
	__u._M_owns = false;

	return *this;
      }

      void
      lock()
      {
	if (!_M_device)
	  __throw_system_error(int(errc::operation_not_permitted));
	else if (_M_owns)
	  __throw_system_error(int(errc::resource_deadlock_would_occur));
	else
	  {
	    _M_device->lock();
	    _M_owns = true;
	  }
      }

      bool
      try_lock()
      {
	if (!_M_device)
	  __throw_system_error(int(errc::operation_not_permitted));
	else if (_M_owns)
	  __throw_system_error(int(errc::resource_deadlock_would_occur));
	else
	  {
	    _M_owns = _M_device->try_lock();
	    return _M_owns;
	  }
      }

      template<typename _Clock, typename _Duration>
	bool
	try_lock_until(const chrono::time_point<_Clock, _Duration>& __atime)
	{
	  if (!_M_device)
	    __throw_system_error(int(errc::operation_not_permitted));
	  else if (_M_owns)
	    __throw_system_error(int(errc::resource_deadlock_would_occur));
	  else
	    {
	      _M_owns = _M_device->try_lock_until(__atime);
	      return _M_owns;
	    }
	}

      template<typename _Rep, typename _Period>
	bool
	try_lock_for(const chrono::duration<_Rep, _Period>& __rtime)
	{
	  if (!_M_device)
	    __throw_system_error(int(errc::operation_not_permitted));
	  else if (_M_owns)
	    __throw_system_error(int(errc::resource_deadlock_would_occur));
	  else
	    {
	      _M_owns = _M_device->try_lock_for(__rtime);
	      return _M_owns;
	    }
	 }

      void
      unlock()
      {
	if (!_M_owns)
	  __throw_system_error(int(errc::operation_not_permitted));
	else if (_M_device)
	  {
	    _M_device->unlock();
	    _M_owns = false;
	  }
      }

      void
      swap(unique_lock& __u)
      {
	std::swap(_M_device, __u._M_device);
	std::swap(_M_owns, __u._M_owns);
      }

      mutex_type*
      release()
      {
	mutex_type* __ret = _M_device;
	_M_device = 0;
	_M_owns = false;
	return __ret;
      }

      bool
      owns_lock() const
      { return _M_owns; }

      explicit operator bool() const
      { return owns_lock(); }

      mutex_type*
      mutex() const
      { return _M_device; }

    private:
      mutex_type*	_M_device;
      bool		_M_owns; // XXX use atomic_bool
    };

  template<typename _Mutex>
    inline void
    swap(unique_lock<_Mutex>& __x, unique_lock<_Mutex>& __y)
    { __x.swap(__y); }

  template<int _Idx>
    struct __unlock_impl
    {
      template<typename... _Lock>
	static void
	__do_unlock(tuple<_Lock&...>& __locks)
	{
	  std::get<_Idx>(__locks).unlock();
	  __unlock_impl<_Idx - 1>::__do_unlock(__locks);
	}
    };

  template<>
    struct __unlock_impl<-1>
    {
      template<typename... _Lock>
	static void
	__do_unlock(tuple<_Lock&...>&)
	{ }
    };

  template<typename _Lock>
    unique_lock<_Lock>
    __try_to_lock(_Lock& __l)
    { return unique_lock<_Lock>(__l, try_to_lock); }

  template<int _Idx, bool _Continue = true>
    struct __try_lock_impl
    {
      template<typename... _Lock>
	static void
	__do_try_lock(tuple<_Lock&...>& __locks, int& __idx)
	{
          __idx = _Idx;
          auto __lock = __try_to_lock(std::get<_Idx>(__locks));
          if (__lock.owns_lock())
            {
              __try_lock_impl<_Idx + 1, _Idx + 2 < sizeof...(_Lock)>::
                __do_try_lock(__locks, __idx);
              if (__idx == -1)
                __lock.release();
            }
	}
    };

  template<int _Idx>
    struct __try_lock_impl<_Idx, false>
    {
      template<typename... _Lock>
	static void
	__do_try_lock(tuple<_Lock&...>& __locks, int& __idx)
	{
          __idx = _Idx;
          auto __lock = __try_to_lock(std::get<_Idx>(__locks));
          if (__lock.owns_lock())
            {
              __idx = -1;
              __lock.release();
            }
	}
    };

  /** @brief Generic try_lock.
   *  @param __l1 Meets Mutex requirements (try_lock() may throw).
   *  @param __l2 Meets Mutex requirements (try_lock() may throw).
   *  @param __l3 Meets Mutex requirements (try_lock() may throw).
   *  @return Returns -1 if all try_lock() calls return true. Otherwise returns
   *          a 0-based index corresponding to the argument that returned false.
   *  @post Either all arguments are locked, or none will be.
   *
   *  Sequentially calls try_lock() on each argument.
   */
  template<typename _Lock1, typename _Lock2, typename... _Lock3>
    int
    try_lock(_Lock1& __l1, _Lock2& __l2, _Lock3&... __l3)
    {
      int __idx;
      auto __locks = std::tie(__l1, __l2, __l3...);
      __try
      { __try_lock_impl<0>::__do_try_lock(__locks, __idx); }
      __catch(...)
      { }
      return __idx;
    }

  /** @brief Generic lock.
   *  @param __l1 Meets Mutex requirements (try_lock() may throw).
   *  @param __l2 Meets Mutex requirements (try_lock() may throw).
   *  @param __l3 Meets Mutex requirements (try_lock() may throw).
   *  @throw An exception thrown by an argument's lock() or try_lock() member.
   *  @post All arguments are locked.
   *
   *  All arguments are locked via a sequence of calls to lock(), try_lock()
   *  and unlock().  If the call exits via an exception any locks that were
   *  obtained will be released.
   */
  template<typename _L1, typename _L2, typename ..._L3>
    void
    lock(_L1& __l1, _L2& __l2, _L3&... __l3)
    {
      while (true)
        {
          unique_lock<_L1> __first(__l1);
          int __idx;
          auto __locks = std::tie(__l2, __l3...);
          __try_lock_impl<0, sizeof...(_L3)>::__do_try_lock(__locks, __idx);
          if (__idx == -1)
            {
              __first.release();
              return;
            }
        }
    }

  /// once_flag
  struct once_flag
  {
  private:
    typedef pthread_once_t __native_type;
    __native_type  _M_once;

  public:
    constexpr once_flag() : _M_once(PTHREAD_ONCE_INIT) { }

    once_flag(const once_flag&) = delete;
    once_flag& operator=(const once_flag&) = delete;

    template<typename _Callable, typename... _Args>
      friend void
      call_once(once_flag& __once, _Callable&& __f, _Args&&... __args);
  };

#if defined(_GLIBCXX_HAVE_TLS) || defined(__BIONIC__)
  extern __thread void* __once_callable;
  extern __thread void (*__once_call)();

  template<typename _Callable>
    inline void
    __once_call_impl()
    {
      (*(_Callable*)__once_callable)();
    }
#else
  extern function<void()> __once_functor;

  extern void
  __set_once_functor_lock_ptr(unique_lock<mutex>*);

  extern mutex&
  __get_once_mutex();
#endif

  extern "C" void __once_proxy();

  /// call_once
  template<typename _Callable, typename... _Args>
    void
    call_once(once_flag& __once, _Callable&& __f, _Args&&... __args)
    {
#if defined(_GLIBCXX_HAVE_TLS) || defined(__BIONIC__)
      auto __bound_functor = std::bind<void>(std::forward<_Callable>(__f),
          std::forward<_Args>(__args)...);
      __once_callable = &__bound_functor;
      __once_call = &__once_call_impl<decltype(__bound_functor)>;
#else
      unique_lock<mutex> __functor_lock(__get_once_mutex());
      __once_functor = std::bind<void>(std::forward<_Callable>(__f),
          std::forward<_Args>(__args)...);
      __set_once_functor_lock_ptr(&__functor_lock);
#endif

      int __e = pthread_once(&(__once._M_once), &__once_proxy);

#if !defined(_GLIBCXX_HAVE_TLS) && !defined(__BIONIC__)
      if (__functor_lock)
        __set_once_functor_lock_ptr(0);
#endif

      if (__e)
	__throw_system_error(__e);
    }

} // namespace std

#endif /* end of include guard: BBQUE_GNU_MUTEX_H_ */