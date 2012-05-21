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
#include "bbque/cpp11/thread.h"

#include <cerrno>

#if !defined(CONFIG_TARGET_SUPPORT_CPP11)

namespace std
{
  namespace
  {
    extern "C" void*
    execute_native_thread_routine(void* __p)
    {
      thread::_Impl_base* __t = static_cast<thread::_Impl_base*>(__p);
      thread::__shared_base_type __local;
      __local.swap(__t->_M_this_ptr);

      __try
	{
	  __t->_M_run();
	}
      __catch(...)
	{
	  std::terminate();
	}

      return 0;
    }
  }

  void
  thread::join()
  {
    int __e = EINVAL;

    if (_M_id != id())
      __e = pthread_join(_M_id._M_thread, NULL);

    if (__e)
      __throw_system_error(__e);

    _M_id = id();
  }

  void
  thread::detach()
  {
    int __e = EINVAL;

    if (_M_id != id())
      __e = pthread_detach(_M_id._M_thread);

    if (__e)
      __throw_system_error(__e);

    _M_id = id();
  }

  void
  thread::_M_start_thread(__shared_base_type __b)
  {
    if (!__gthread_active_p())
      __throw_system_error(int(errc::operation_not_permitted));

    __b->_M_this_ptr = __b;
    int __e = pthread_create(&_M_id._M_thread, NULL,
			       &execute_native_thread_routine, __b.get());
    if (__e)
    {
      __b->_M_this_ptr.reset();
      __throw_system_error(__e);
    }
  }
} // namespace std

#endif // !CONFIG_TARGET_SUPPORT_CPP11
