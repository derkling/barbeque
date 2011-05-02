/**
 *       @file  bbque_rpc.h
 *      @brief  The Barbeque RPC interface to support comunicaiton among
 *      applications and the RTRM.
 *
 * This RPC mechanism is channel agnostic and defines a set of procedure that
 * applications could call to send requests to the Barbeque RTRM.  The actual
 * implementation of the communication channel is provided by class derived by
 * this one. This class provides also a factory method which allows to obtain
 * an instance to the concrete communication channel that can be selected at
 * compile time.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/11/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#ifndef BBQUE_RPC_H_
#define BBQUE_RPC_H_

#include "bbque/rtlib.h"

namespace bbque { namespace rtlib {

class BbqueRPC {

public:

	/**
	 * @brief Get a reference to the (singleton) RPC service
	 *
	 * This class is a factory of different RPC communication channels. The
	 * actual instance returned is defined at compile time by selecting the
	 * proper specialization class.
	 *
	 * @return A reference to an actual BbqueRPC implementing the compile-time
	 * selected communication channel.
	 */
	static BbqueRPC * GetInstance();

	/**
	 * @brief Release the RPC channel
	 */
	virtual ~BbqueRPC(void);

/******************************************************************************
 * Channel Independant interface
 ******************************************************************************/

	RTLIB_ExitCode Init(const char *name);

	RTLIB_ExecutionContextHandler Register(
			const char* name,
			const RTLIB_ExecutionContextParams* params);

	void Unregister(
			const RTLIB_ExecutionContextHandler ech);

	RTLIB_ExitCode Start(
			const RTLIB_ExecutionContextHandler ech);

	RTLIB_ExitCode Stop(
			const RTLIB_ExecutionContextHandler ech);

	bool Sync(
			const RTLIB_ExecutionContextHandler ech,
			const char *name,
			RTLIB_SyncType type);

	RTLIB_ExitCode Set(
			const RTLIB_ExecutionContextHandler ech,
			RTLIB_Constraint* constraints,
			uint8_t count);

	RTLIB_ExitCode Clear(
			const RTLIB_ExecutionContextHandler ech);

	RTLIB_ExitCode GetWorkingMode(
			RTLIB_ExecutionContextHandler ech,
			RTLIB_WorkingModeParams *wm);

protected:

	/**
	 * @brief Build a new RTLib
	 */
	BbqueRPC(void);

/******************************************************************************
 * Channel Dependant interface
 ******************************************************************************/

	virtual RTLIB_ExitCode _Init(
			const char *name) = 0;

	virtual RTLIB_ExecutionContextHandler _Register(
			const char* name,
			const RTLIB_ExecutionContextParams* params) = 0;

	virtual void _Unregister(
			const RTLIB_ExecutionContextHandler ech) = 0;

	virtual RTLIB_ExitCode _Start(
			const RTLIB_ExecutionContextHandler ech) = 0;

	virtual RTLIB_ExitCode _Stop(
			const RTLIB_ExecutionContextHandler ech) = 0;

	virtual RTLIB_ExitCode _Set(
		const RTLIB_ExecutionContextHandler ech,
			RTLIB_Constraint* constraints,
			uint8_t count) = 0;

	virtual RTLIB_ExitCode _Clear(
			const RTLIB_ExecutionContextHandler ech) = 0;

	virtual RTLIB_ExitCode _GetWorkingMode(
			RTLIB_ExecutionContextHandler ech,
			RTLIB_WorkingModeParams *wm) = 0;

	virtual void _Exit() = 0;

private:


/******************************************************************************
 * Application Callbacks Proxies
 ******************************************************************************/

	RTLIB_ExitCode StopExecution(
			RTLIB_ExecutionContextHandler ech,
			struct timespec timeout);

};

} // namespace rtlib

} // namespace bbque

#endif /* end of include guard: BBQUE_RPC_H_ */

