/**
 *       @file  bbque_exc.h
 *      @brief  The base class for EXC being managed by the RTLIB
 *
 * This is a base class suitable for the implementation of EXC that should be
 * managed by the Barbeque RTRM.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
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

#ifndef BBQUE_EXC_H_
#define BBQUE_EXC_H_

#include <bbque/rtlib.h>
#include <bbque/utils/timer.h>

#include <map>
#include <string>
#include <thread>

class BbqueEXC {

public:

	BbqueEXC(std::string const & name,
			std::string const & recipe,
			RTLIB_Services_t *rtlib,
			bool enabled = true);

	~BbqueEXC();

	RTLIB_ExitCode_t Enable();

	RTLIB_ExitCode_t Disable();

	RTLIB_ExitCode_t Start();

	RTLIB_ExitCode_t Terminate();

	RTLIB_ExitCode_t WaitCompletion();

	inline bool isRegistered() const {
		return registered;
	}

	inline bool Done() const {
		return done;
	}

	virtual RTLIB_ExitCode_t onConfigure(uint8_t awm_id);

	virtual RTLIB_ExitCode_t onSuspend();

	virtual RTLIB_ExitCode_t onRun();

	virtual RTLIB_ExitCode_t onMonitor();

protected:

	virtual void ControlLoop();

private:

	std::string const exc_name;

	std::string const rpc_name;

	RTLIB_Services_t * const rtlib;

	RTLIB_ExecutionContextHandler_t exc_hdl;

	RTLIB_WorkingModeParams_t wmp;

	std::thread ctrl_trd;

	std::mutex ctrl_mtx;

	std::condition_variable ctrl_cv;

	bool registered;

	bool started;

	bool enabled;

	bool done;

	bool WaitEnable();

	RTLIB_ExitCode_t Reconfigure(RTLIB_ExitCode_t result);

	RTLIB_ExitCode_t GetWorkingMode();

	RTLIB_ExitCode_t Run();

	RTLIB_ExitCode_t Monitor();

};

#endif // BBQUE_EXC_H_

