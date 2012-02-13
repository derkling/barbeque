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

#ifndef BBQUE_APPLICATION_RTRM_H_
#define BBQUE_APPLICATION_RTRM_H_

#include <vector>

#include "bbque/rtlib/bbque_exc.h"

#include "op_manager.h"
#include "generic_window.h"

using bbque::rtlib::BbqueEXC;

typedef std::vector<GenericWindowIF*> GoalsList;

/**
 * @brief Application-Specific Runtime Resource Manager
 * @ingroup rtlib_sec04_rtrm
 *
 * @details
 * This class provides the ApplicationRTRM, used to communicate between the
 * application and Barbeque
 */
class ApplicationRTRM {

public:
	/**
	 * @brief Default constructor of the class
	 */
	ApplicationRTRM() {

	}

	/**
	 * @brief Constructor of the class
	 *
	 * @param Pointer to a BbqueEXC. It is be used to communicate with
	 * Barbeque
	 * @param opManager Pointer to the OP_Manager of an Execution Context.
	 * It is used to to have a knowledge of operating points
	 */
	ApplicationRTRM(BbqueEXC *bbqexc, OP_Manager &opManager)
			: bbqexc(bbqexc), opManager(opManager) {

	}

	/**
	 * @brief Constructor of the class
	 *
	 * @param bbqexc Pointer to a BbqueEXC.It is be used to communicate with
	 * Barbeque
	 * @param opManager Reference to the OP_Manager of an Execution Context.
	 * It is used to to have a knowledge of operating points
	 * @param goalsList Reference to a list of goals to register for the use
	 * with ApplicationRTRM
	 */
	ApplicationRTRM(BbqueEXC *bbqexc, OP_Manager &opManager,
			GoalsList &goalsList)
			: bbqexc(bbqexc),
			  opManager(opManager),
			  goalsList(goalsList) {

	}

	/**
	 * @brief Sets the GoalsList to use inside ApplicationRTRM
	 * @param goalsList reference to a list of goals
	 */
	void setGoals(GoalsList &goalsList);

	/**
	 * @brief Notifies Barbeque of a need of more resources when the
	 * registered goals are not achieved and OP_Manager is already in the
	 * higher operating point.
	 *
	 * @todo Consider having a check also on awm_id
	 */
	void increaseResources();

private:
	/**
	 * Pointer to a BbqueEXC. It is be used to communicate with Barbeque
	 */
	BbqueEXC *bbqexc;

	/**
	 * OP_Manager of an Execution Context. It is used to have a knowledge of
	 * operating points
	 */
	OP_Manager opManager;

	/**
	 * List of goals to register for the use with ApplicationRTRM
	 */
	GoalsList goalsList;
};

/*******************************************************************************
 *    Doxygen Module Documentation
 ******************************************************************************/

/**
 * @defgroup rtlib_sec04_rtrm Application-Specific RTRM
 * @ingroup rtlib
 *
 * ADD MORE DETAILS HERE (Monitors)
 *
 */

#endif /* BBQUE_APPLICATION_RTRM_H_ */
