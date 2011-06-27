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

#include <map>
#include <memory>
#include <string>

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

	typedef struct RegisteredExecutionContext {
		/** The Execution Context data */
		RTLIB_ExecutionContextParams exc_params;
		/** The name of this Execuion Context */
		std::string name;
		/** The RTLIB assigned ID for this Execution Context */
		uint8_t exc_id;
#define EXC_FLAGS_AWM_VALID   0x01 ///< The EXC has been assigned a valid AWM
#define EXC_FLAGS_AWM_WAITING 0x02 ///< The EXC is waiting for a valid AWM
		/** A set of flags to define the state of this EXC */
		uint8_t flags;
		/** The ID of the assigned AWM (if valid) */
		uint8_t awm_id;
		/** The mutex protecting access to this structure */
		std::mutex mtx;
		/** The conditional variable to be notified on changes for this EXC */
		std::condition_variable cv;
	} RegisteredExecutionContext_t;

	typedef std::shared_ptr<RegisteredExecutionContext_t> pregExCtx_t;

	//--- AWM Wait
	inline bool isAwmWaiting(pregExCtx_t prec) const {
		return (prec->flags & EXC_FLAGS_AWM_WAITING);
	}
	inline void setAwmWaiting(pregExCtx_t prec) const {
		prec->flags |= EXC_FLAGS_AWM_WAITING;
	}
	inline void clearAwmWaiting(pregExCtx_t prec) const {
		prec->flags &= ~EXC_FLAGS_AWM_WAITING;
	}

	/**
	 * @brief Build a new RTLib
	 */
	BbqueRPC(void);

/******************************************************************************
 * Channel Dependant interface
 ******************************************************************************/

	virtual RTLIB_ExitCode _Init(
			const char *name) = 0;

	virtual RTLIB_ExitCode _Register(pregExCtx_t pregExCtx) = 0;

	virtual RTLIB_ExitCode _Unregister(pregExCtx_t pregExCtx) = 0;

	virtual RTLIB_ExitCode _Start(pregExCtx_t pregExCtx) = 0;

	virtual RTLIB_ExitCode _Stop(pregExCtx_t pregExCtx) = 0;

	virtual RTLIB_ExitCode _Set(
		const RTLIB_ExecutionContextHandler ech,
			RTLIB_Constraint* constraints,
			uint8_t count) = 0;

	virtual RTLIB_ExitCode _Clear(
			const RTLIB_ExecutionContextHandler ech) = 0;

	virtual RTLIB_ExitCode _ScheduleRequest(pregExCtx_t prec) = 0;
			pregExCtx_t prec,

	virtual void _Exit() = 0;


private:

	bool initialized;

	/**
	 * @brief The map of Execution Context registered by the application
	 *
	 * This maps Execution Context ID (exc_id) into pointers to
	 * RegisteredExecutionContext structures.
	 */
	typedef std::map<uint8_t, pregExCtx_t> excMap_t;

	excMap_t exc_map;

	typedef std::pair<uint8_t, pregExCtx_t> excMapEntry_t;




	/**
	 * @brief Get the next available (and unique) Execution Context ID
	 */
	uint8_t GetNextExcID();

/******************************************************************************
 * Application Callbacks Proxies
 ******************************************************************************/

	RTLIB_ExitCode StopExecution(
			RTLIB_ExecutionContextHandler ech,
			struct timespec timeout);

/******************************************************************************
 * Utility functions
 ******************************************************************************/

	pregExCtx_t getRegistered(
			const RTLIB_ExecutionContextHandler ech);

};

} // namespace rtlib

} // namespace bbque

#endif /* end of include guard: BBQUE_RPC_H_ */

