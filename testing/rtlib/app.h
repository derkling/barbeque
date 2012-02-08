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

#ifndef BBQUE_APP_H_
#define BBQUE_APP_H_

#include "bbque/rtlib.h"

#include <string>
#include <map>

/**
 * @brief  A dummy application which interact with the barbeque RTRM
 *
 * This is a really simple application which could be used as a template to
 * develop real applications interacting with the Barbeque RTRM
 */
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
