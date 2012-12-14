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

#ifndef BBQUE_CHRONO_H_
#define BBQUE_CHRONO_H_

#include "bbque/config.h"

#ifdef CONFIG_TARGET_SUPPORT_CPP11
// Use the standard C++11 library implementation
# include <chrono>
#else
// Use the BBQ backport C++11 library implementation
#include "bbque/cpp11/gnu/chrono.h"
#endif // CONFIG_TARGET_SUPPORT_CPP11

#if GCC_TAG >= 47 || defined(ANDROID)
# define steady_clock steady_clock
#else
# define steady_clock monotonic_clock
#endif

#endif /* end of include guard: BBQUE_CHRONO_H_ */
