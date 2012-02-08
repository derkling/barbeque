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
 */

#ifndef BBQUE_EXCEPTION_H_
#define BBQUE_EXCEPTION_H_

#include <iostream>
#include <sstream>
#include <memory>
#include <stdexcept>

namespace bbque {

/**
 * @brief A basic exception
 *
 * This provides support for the generation of a generic exception defining
 * where into the code it has been generated.
 */
class Exception : public std::runtime_error {

public:

	/**
	 * 
	 */
	std::string  file_name;

	/**
	 * 
	 */
	uint32_t line_number;

private:

	/**
	 * 
	 */
	mutable std::shared_ptr<std::stringstream> str_stream;

	/**
	 * 
	 */
	mutable std::string str;

public:

	/**
	 *
	 */
	Exception(const std::string file = "", uint32_t line = 0) :
		std::runtime_error(""),
		file_name(file),
		line_number(line),
		str_stream(std::shared_ptr<std::stringstream> (
			new std::stringstream())) {
	}

	/**
	 *
	 */
	~Exception() throw() {

	}

	/**
	 *
	 */
	template <typename T>
	Exception & operator << (const T & t) {
		(*str_stream) << t;
		return *this;
	}

	/**
	 *
	 */
	virtual const char * what() const throw() {
		str = str_stream->str();
		return str.c_str();
	}


}; // Class Exception

} // namespace bbque

#endif // BBQUE_EXCEPTION_H_
