
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
#include "bbque/cpp11/future.h"

#if !defined(CONFIG_TARGET_SUPPORT_CPP11)

namespace
{
  struct future_error_category : public std::error_category
  {
    future_error_category() {}

    virtual const char*
    name() const
    { return "future"; }

    virtual std::string message(int __ec) const
    {
      std::string __msg;
      switch (std::future_errc(__ec))
      {
      case std::future_errc::broken_promise:
          __msg = "Broken promise";
          break;
      case std::future_errc::future_already_retrieved:
          __msg = "Future already retrieved";
          break;
      case std::future_errc::promise_already_satisfied:
          __msg = "Promise already satisfied";
          break;
      default:
          __msg = "Unknown error";
          break;
      }
      return __msg;
    }
  };

  const future_error_category&
  __future_category_instance()
  {
    static const future_error_category __fec;
    return __fec;
  }
}

namespace std
{
  const error_category& future_category()
  { return __future_category_instance(); }

  future_error::~future_error() throw() { }

  const char*
  future_error::what() const throw() { return _M_code.message().c_str(); }

  __future_base::_Result_base::_Result_base() = default;

  __future_base::_Result_base::~_Result_base() = default;

  __future_base::_State_base::~_State_base() = default;
}

#endif // !CONFIG_TARGET_SUPPORT_CPP11
