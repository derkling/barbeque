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
#include "bbque/cpp11/chrono.h"

// conditional inclusion of sys/time.h for gettimeofday
#if !defined(_GLIBCXX_USE_CLOCK_MONOTONIC) && \
    !defined(_GLIBCXX_USE_CLOCK_REALTIME) && \
     defined(_GLIBCXX_USE_GETTIMEOFDAY)
#include <sys/time.h>
#endif

#if !defined(CONFIG_TARGET_SUPPORT_CPP11)

namespace std
{
  namespace chrono
  {
    system_clock::time_point
    system_clock::now() throw ()
    {
#if defined(_GLIBCXX_USE_CLOCK_REALTIME) || defined(__BIONIC__)
      timespec tp;
      // -EINVAL, -EFAULT
      clock_gettime(CLOCK_REALTIME, &tp);
      return time_point(duration(chrono::seconds(tp.tv_sec)
				 + chrono::nanoseconds(tp.tv_nsec)));
#elif defined(_GLIBCXX_USE_GETTIMEOFDAY)
      timeval tv;
      // EINVAL, EFAULT
      gettimeofday(&tv, 0);
      return time_point(duration(chrono::seconds(tv.tv_sec)
				 + chrono::microseconds(tv.tv_usec)));
#else
      std::time_t __sec = std::time(0);
      return system_clock::from_time_t(__sec);
#endif
    }

#if defined(_GLIBCXX_USE_CLOCK_MONOTONIC) || defined(__BIONIC__)
    const bool steady_clock::is_steady;

    steady_clock::time_point
    steady_clock::now()
    {
      timespec tp;
      // -EINVAL, -EFAULT
      clock_gettime(CLOCK_MONOTONIC, &tp);
      return time_point(duration(chrono::seconds(tp.tv_sec)
				 + chrono::nanoseconds(tp.tv_nsec)));
    }
#endif
  } // namespace chrono
} // namespace std

#endif // !CONFIG_TARGET_SUPPORT_CPP11
