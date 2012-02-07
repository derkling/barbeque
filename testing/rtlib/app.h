/**
 *       @file  rtlib/app.h
 *      @brief  A dummy application which interact with the barbeque RTRM
 *
 * This is a really simple application which could be used as a template to
 * develop real applications interacting with the Barbeque RTRM
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  04/28/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#ifndef BBQUE_APP_H_
#define BBQUE_APP_H_

#include "bbque/rtlib.h"

#include <string>
#include <map>

class BbqueApp {

public:

	BbqueApp(std::string const & name);

	int RegisterEXC(std::string const & name, uint8_t recipe_id);

	RTLIB_ExitCode_t Enable(uint8_t first, uint8_t last);

	void SwitchConfiguration(std::string const & name,
			RTLIB_WorkingModeParams_t & wmp);

	void BlockExecution(std::string const & name);
	
	RTLIB_ExitCode_t CheckForReconfiguration(std::string const & name,
			RTLIB_ExitCode_t result,
			RTLIB_WorkingModeParams_t & wmp);

	int GetWorkingMode(std::string const & name);

	RTLIB_ExitCode_t Disable(uint8_t first, uint8_t last);

	static RTLIB_ExitCode_t Stop(
			RTLIB_ExecutionContextHandler_t ech,
			struct timespec timeout);

	void UnregisterAll();

private:

	RTLIB_Services_t *rtlib;

	typedef std::pair<std::string, RTLIB_ExecutionContextHandler_t> excMapEntry_t;

	typedef std::map<std::string, RTLIB_ExecutionContextHandler_t> excMap_t;

	excMap_t exc_map;
};

#endif // BBQUE_APP_H_

