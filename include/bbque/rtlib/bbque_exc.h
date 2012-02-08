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

#ifndef BBQUE_EXC_H_
#define BBQUE_EXC_H_

#include <bbque/rtlib.h>
#include <bbque/utils/timer.h>

#include <map>
#include <string>
#include <thread>

namespace bbque { namespace rtlib {

/**
 * @brief The AEM base-class
 * @ingroup rtlib_sec02_aem
 *
 * This is a base class suitable for the implementation of an EXC that should
 * be managed by the Barbeque RTRM.
 */
class BbqueEXC {

public:

	BbqueEXC(std::string const & name,
			std::string const & recipe,
			RTLIB_Services_t *rtlib);

	virtual ~BbqueEXC();

	RTLIB_ExitCode_t Enable();

	RTLIB_ExitCode_t Disable();

	RTLIB_ExitCode_t SetConstraints(
		RTLIB_Constraint_t *constraints,
		uint8_t count);

	RTLIB_ExitCode_t ClearConstraints();

	RTLIB_ExitCode_t SetGoalGap(uint8_t percent);

	RTLIB_ExitCode_t Start();

	RTLIB_ExitCode_t Terminate();

	RTLIB_ExitCode_t WaitCompletion();

	inline bool isRegistered() const {
		return registered;
	}

	inline bool Done() const {
		return done;
	}

protected:

	std::string const exc_name;

	std::string const rpc_name;

	virtual RTLIB_ExitCode_t onSetup();

	virtual RTLIB_ExitCode_t onConfigure(uint8_t awm_id);

	virtual RTLIB_ExitCode_t onSuspend();

	virtual RTLIB_ExitCode_t onResume();

	virtual RTLIB_ExitCode_t onRun();

	virtual RTLIB_ExitCode_t onMonitor();

	inline uint32_t Cycles() const {
		return cycles_count;
	}

	RTLIB_WorkingModeParams_t const & WorkingModeParams() const {
		return wmp;
	}

private:

	RTLIB_Services_t * const rtlib;

	RTLIB_ExecutionContextHandler_t exc_hdl;

	std::thread ctrl_trd;

	std::mutex ctrl_mtx;

	std::condition_variable ctrl_cv;

	RTLIB_WorkingModeParams_t wmp;

	/**
	 * @brief The number of onRun executions
	 * 
	 * This counter is incremented at each onRun execution thus allowing to
	 * keep track of the amount of workload processed.
	 * Considering a 30fps video decoding, where an onRun is called for each
	 * frame, a uint32 should allow for +4.5 years continous playback ;-)
	 */
	uint32_t cycles_count;

	bool registered;

	bool started;

	bool enabled;

	bool suspended;

	bool done;

	RTLIB_ExitCode_t _Enable();

	bool WaitEnable();

	RTLIB_ExitCode_t Setup();

	RTLIB_ExitCode_t Configure(uint8_t awm_id, RTLIB_ExitCode_t event);

	RTLIB_ExitCode_t Suspend();

	RTLIB_ExitCode_t Reconfigure(RTLIB_ExitCode_t result);

	RTLIB_ExitCode_t GetWorkingMode();

	RTLIB_ExitCode_t Run();

	RTLIB_ExitCode_t Monitor();

	void ControlLoop();
};

} // namespace rtlib

} // namespace bbque

/*******************************************************************************
 *    Doxygen Module Documentation
 ******************************************************************************/

/**
 * @defgroup rtlib_sec02_aem RTLib Abstract Execution Model (AEM) API
 * @ingroup rtlib
 *
 * ADD MORE DETAILS HERE
 *
 */

/**
 * @defgroup rtlib_sec02_aem_app Application Callbacks
 * @ingroup rtlib_sec02_aem
 *
 * ADD MORE DETAILS HERE (Application Callbacks)
 *
 */

/**
 * @defgroup rtlib_sec02_aem_exc Execution Context Management
 * @ingroup rtlib_sec02_aem
 *
 * ADD MORE DETAILS HERE (Execution Context Management)
 *
 */

/**
 * @defgroup rtlib_sec02_aem_constr Constraints Management
 * @ingroup rtlib_sec02_aem
 *
 * ADD MORE DETAILS HERE (Constraints Management)
 *
 */

/**
 * @defgroup rtlib_sec02_aem_utils Utility
 * @ingroup rtlib_sec02_aem
 *
 * ADD MORE DETAILS HERE (Utility)
 *
 */

#endif // BBQUE_EXC_H_
