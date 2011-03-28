/**
 *       @file  working_mode_conf.h
 *      @brief  "Updating" interface for WorkingMode runtime status
 *
 * This defines the interface for updating WorkingMode at runtime by other
 * RTRM components, i.e. the Optimizer.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  01/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_WORKING_MODE_CONF_IF_H
#define BBQUE_WORKING_MODE_CONF_IF_H

#include "bbque/app/working_mode_status.h"

namespace bbque { namespace app {


/**
 * @class WorkingModeConfIF
 * @brief Working mode interface for updating runtime status
 */
class WorkingModeConfIF: public WorkingModeStatusIF {

public:

	/**
	 * @brief Set the QoS value associated to the working mode
	 * @param value The QoS value of the working mode
	 */
	virtual void SetValue(uint8_t value) = 0;

	/**
	 * @brief Create a new object TransitionOverheads managing the overhead
	 * information in switching to 'dest_wm' working mode
	 * @param dest_wm Destination working mode
	 * @param time Time spent in switching to destination working mode
	 * @return A reference to the TransitionOverheads just created
	 */
	virtual ExitCode_t AddOverheadInfo(std::string const & dest_awm,
			double time) = 0;

};

} // namespace app

} // namespace bbque

#endif // BBQUE_WORKING_MODE_CONF_IF_H

