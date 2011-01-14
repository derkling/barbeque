/**
 *       @file  logger.cpp
 *      @brief  
 *
 * Detailed description starts here.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/14/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include "bbque/logger.h"

#include <memory>

#include "bbque/logger_log4cpp.h"


namespace bbque {

Logger::Logger(std::string const & name) :
	log_name(name)
{

}

std::unique_ptr<Logger> Logger::GetInstance(std::string const & name)
{

#ifdef BBQUE_HAVE_LOG4CPP
	std::unique_ptr<Logger> l(new Log4CppLogger(name));
#else
#error TODO: Missing ConsoleLogger implementation
	std::unique_ptr<Logger> l(new ConsoleLogger(name));
#endif

	return l;
}

} // namespace bbque

