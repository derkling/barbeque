/**
 *       @file  Exception.h
 *      @brief  A generic execption.
 *
 * This provides support for the generation of a generic exception defining
 * where into the code it has been generated.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  01/13/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef BBQUE_EXCEPTION_H_
#define BBQUE_EXCEPTION_H_

#include <iostream>
#include <sstream>
#include <memory>
#include <stdexcept>

namespace bbque {

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
	 * @brief   
	 * @param   
	 * @return  
	 */
	Exception(const std::string file = "", uint32_t line = 0) :
		std::runtime_error(""),
		file_name(file),
		line_number(line),
		str_stream(std::shared_ptr<std::stringstream> (
			new std::stringstream()))
	{
	}

	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	~Exception() throw()
	{

	}

	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	template <typename T>
	Exception & operator << (const T & t)
	{
		(*str_stream) << t;
		return *this;
	}

	/**
	 * @brief   
	 * @param   
	 * @return  
	 */
	virtual const char * what() const throw()
	{
		str = str_stream->str();
		return str.c_str();
	}


}; // Class Exception

} // namespace bbque

#endif // BBQUE_EXCEPTION_H_

