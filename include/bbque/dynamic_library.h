/**
 *       @file  dynamic_library.h
 *      @brief  A generic class to manage shared libraries
 *
 * This class provides a generic interface to manage shared libraries.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/27/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef BBQUE_DYNAMIC_LIBRARY_H_
#define BBQUE_DYNAMIC_LIBRARY_H_

#include <string>

namespace bbque { namespace plugins {

class DynamicLibrary {

public:

	static DynamicLibrary * Load(const std::string & path,
								std::string &errorString);
	~DynamicLibrary();

	void * GetSymbol(const std::string & name);

private:

	DynamicLibrary();

	DynamicLibrary(void * handle);

	DynamicLibrary(const DynamicLibrary &);

private:

	void * handle;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_DYNAMIC_LIBRARY_H_

