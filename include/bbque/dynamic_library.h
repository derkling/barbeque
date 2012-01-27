/**
 *       @file  dynamic_library.h
 *      @brief  A generic class to manage shared libraries
 *
 * This class provides a generic interface to manage shared libraries.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
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

	/**
	 * @class DynamicLibrary
	 * @brief A convenience class to manage shared library
	 * This class provides a set of basic services to load a shared library
	 * and release them when not more needed.
	 */
class DynamicLibrary {

public:

	/**
	 * @brief   Load the specified library
	 * @param   path the filesyste path of the shared object library to load
	 * @param	errorString a string to contain eventually load error messages
	 * @return  a pointer to the loaded library, NULL on errors
	 */
	static DynamicLibrary * Load(const std::string & path,
								std::string &errorString);
	/**
	 * @brief   Release this dynamic library loader
	 */
	~DynamicLibrary();

	/**
	 * @brief   Get a pointer to the specified library symbol
	 * @param   name the symbol name to lookup
	 * @return  a valid pointer to the requested symbol, NULL on errrors
	 */
	void * GetSymbol(const std::string & name);

private:

	/**
	 * @brief   Build a new instance of the dynamic library loader
	 */
	DynamicLibrary();

	/**
	 * @brief   Clone the dynamic library loader
	 * @param   handle the loader to clone
	 */
	DynamicLibrary(void * handle);

private:

	/**
	 * A pointer to the dynamic library represented by this object
	 */
	void * handle;

};

} // namespace plugins

} // namespace bbque

#endif // BBQUE_DYNAMIC_LIBRARY_H_

