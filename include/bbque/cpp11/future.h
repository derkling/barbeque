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

#ifndef BBQUE_FUTURE_H_
#define BBQUE_FUTURE_H_

#include "bbque/config.h"

#ifdef CONFIG_TARGET_SUPPORT_CPP11
// Use the standard C++11 library implementation
# include <future>
#else
// Use the BBQ backport C++11 library implementation
#include "bbque/cpp11/gnu/future.h"
#endif // CONFIG_TARGET_SUPPORT_CPP11

#endif /* end of include guard: BBQUE_FUTURE_H_ */
