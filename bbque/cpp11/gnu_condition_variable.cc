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

#include "bbque/config.h"
#include "bbque/cpp11/condition_variable.h"

#if !defined(CONFIG_TARGET_SUPPORT_CPP11)

namespace std
{
  condition_variable::condition_variable() throw ()
  {
#if defined(__BIONIC__)
    __native_type __tmp = PTHREAD_COND_INITIALIZER;
    _M_cond = __tmp;
#elif defined (__GTHREAD_COND_INIT)
    __native_type __tmp = __GTHREAD_COND_INIT;
    _M_cond = __tmp;
#else
    int __e = pthread_cond_init(&_M_cond, NULL);

    if (__e)
      __throw_system_error(__e);
#endif
  }

  condition_variable::~condition_variable() throw ()
  {
    // XXX no thread blocked
    /* int __e = */ pthread_cond_destroy(&_M_cond);
    // if __e == EBUSY then blocked
  }

  void
  condition_variable::wait(unique_lock<mutex>& __lock)
  {
    int __e = pthread_cond_wait(&_M_cond, __lock.mutex()->native_handle());

    if (__e)
      __throw_system_error(__e);
  }

  void
  condition_variable::notify_one()
  {
    int __e = pthread_cond_signal(&_M_cond);

    // XXX not in spec
    // EINVAL
    if (__e)
      __throw_system_error(__e);
  }

  void
  condition_variable::notify_all()
  {
    int __e = pthread_cond_broadcast(&_M_cond);

    // XXX not in spec
    // EINVAL
    if (__e)
      __throw_system_error(__e);
  }

  condition_variable_any::condition_variable_any() throw ()
  { }

  condition_variable_any::~condition_variable_any() throw ()
  { }

} // namespace std

#endif // !CONFIG_TARGET_SUPPORT_CPP11
