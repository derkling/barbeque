/**
 *       @file  rtlib.h
 *      @brief  The Barbeque RTRM Run-Time Library
 *
 * Definition of the services exported by the Barbeque Run-Time library
 * (RTLib). This library is provided to be linked by application that needs to
 * interface with the Barbeque run-time resource manager. The library not only
 * defines the set of services accessable by applications, but allows also to
 * mask the platform specific communication channel between the applications
 * and the run-time manager.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  01/13/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */


#ifndef BBQUE_RTLIB_H_
#define BBQUE_RTLIB_H_

#include <cstdint>
#include <ctime>

#ifdef __cplusplus
extern "C" {
#endif

#define RTLIB_VERSION_MAJOR 1
#define RTLIB_VERSION_MINOR 0

/**
 * The maximum length for an "application" name
 */
#define RTLIB_APP_NAME_LENGTH 32

/**
 * The maximum length for an "execution context" name
 */
#define RTLIB_EXC_NAME_LENGTH 32

/**
 * The maximum length for a recipe name
 */
#define RTLIB_RECIPE_NAME_LENGTH 64

/**
 * @brief The recipe of an "execution context".
 *
 * Each "execution context" that an application could register to the Barbeque
 * run-time resource manager, is identified and described by a corresponding
 * "recipe". A recipe collects a detailed description of resource amount
 * required by each supported working mode.
 * @note This name must never exceed the size specified by
 * RTLIB_RECIPE_NAME_LENGTH
 */
typedef char* RTLIB_Recipe;

// Forward declaration
typedef struct RTLIB_Services RTLIB_Services;

/**
 * @brief The library entry point.
 *
 * This function must be the first one called by an application to properly
 * initialized the RTLib.  Library initialization is required to propery setup
 * the communication channel with the Barbeque RTRM and prepare the ground for
 * execution contexts registration.
 */
RTLIB_Services *RTLIB_Init(const char* name);

/**
 * @brief The Execution Context (EC) programming language.
 */
typedef enum RTLIB_ProgrammingLanguage {
	/** Undefined application language */
	RTLIB_LANG_UNDEF = 0,
	/** C coded EC */
	RTLIB_LANG_C,
	/** CPP coded EC */
	RTLIB_LANG_CPP,
	/** OpenCL coded EC */
	RTLIB_LANG_OPENCL,
	/** P2012 Native Programming Layer coded EC */
	RTLIB_LANG_NPL,
	/** P2012 Parallel Programmaing Patterns coded EC */
	RTLIB_LANG_PPP
} RTLIB_ProgrammingLanguage;

/**
 * @brief The Barbeque API version number.
 */
typedef struct RTLIB_APIVersion {
	/** Major API version number */
	int32_t major;
	/** Minor API version number */
	int32_t minor;
} RTLIB_PluginAPIVersion;


typedef enum RTLIB_ExitCode {
	/** Success (no errors) */
	RTLIB_OK = 0,
	/** Uspecified (generic) error */
	RTLIB_ERROR,
	/** No new working mode error */
	RTLIB_NO_WORKING_MODE,
	/** Failed to setup the channel to connect the Barbeque RTRM */
	RTLIB_BBQUE_CHANNEL_SETUP_FAILED,
	/** Failed to release the channel to connect the Barbeque RTRM */
	RTLIB_BBQUE_CHANNEL_TEARDOWN_FAILED,
	/** Failed to write to the Barbeque RTRM communication channel */
	RTLIB_BBQUE_CHANNEL_WRITE_FAILED,
	/** Failed to read form the Barbeque RTRM communication channel */
	RTLIB_BBQUE_CHANNEL_READ_FAILED,
	/** Timeout on read form the Barbeque RTRM communication channel */
	RTLIB_BBQUE_CHANNEL_READ_TIMEOUT,
	/** The bbque and application RPC protocol versions does not match */
	RTLIB_BBQUE_CHANNEL_PROTOCOL_MISMATCH,
	/** The (expected) communicaiton channel is not available */
	RTLIB_BBQUE_CHANNEL_UNAVAILABLE,
	/** The Barbeque RTRM is not available */
	RTLIB_BBQUE_UNREACHABLE,
	/** The Execution Context Duplicated */
	RTLIB_EXC_DUPLICATE,
	/** The Execution Context has not been registered */
	RTLIB_EXC_NOT_REGISTERED,
	/** The Execution Context Registration Failed */
	RTLIB_EXC_REGISTRATION_FAILED,
	/** The Execution Context Registration Failed */
	RTLIB_EXC_MISSING_RECIPE,
	/** The Execution Context Unregistration Failed */
	RTLIB_EXC_UNREGISTRATION_FAILED,
	/** The Execution Context Start Failed */
	RTLIB_EXC_START_FAILED,
	/** The Execution Context Stop Failed */
	RTLIB_EXC_STOP_FAILED,
	/** Failed to get a working mode */
	RTLIB_EXC_GWM_FAILED,

//---- Reconfiguration actions required for an EXC
// NOTE these values should match (in number and order) those defined by the
//	    ApplicationStatusIF::SyncState_t

	/** Start running on the assigned AWM */
	RTLIB_EXC_GWM_START,
	/** Reconfiguration into a different AWM */
	RTLIB_EXC_GWM_RECONF,
	/** Migration and reconfiguration into a different AWM */
	RTLIB_EXC_GWM_MIGREC,
	/** Migration (still running on the same AWM) */
	RTLIB_EXC_GWM_MIGRATE,
	/** EXC suspended (resources not available) */
	RTLIB_EXC_GWM_BLOCKED,

//---- Internal values not exposed to applications

	/** The EXC is in sync mode */
	RTLIB_EXC_SYNC_MODE,
	/** A step of the synchronization protocol has failed */
	RTLIB_EXC_SYNCP_FAILED

// NOTE The last entry should not overflow a uint8_t, otherwise some of the RPC
// channel messages should be updated

} RTLIB_ExitCode;


/**
 * @brief The information passed to an application to set its new Working Mode.
 */
typedef struct RTLIB_WorkingModeParams {
	/** The ID of the working mode */
	uint8_t awm_id;
	/** The set of platform supported services */
	const struct RTLIB_Services* services;
} RTLIB_WorkingModeParams;


// Forward declaration
struct RTLIB_ExecutionContextParams;

/**
 * @brief An "execution context" handler.
 *
 * This handler could be used to uniquely identify a previously registered
 * execution context. Such an handler is passed back to the application by the
 * resource manager each time it needs to communicate some information.
 * The application could eventually "associate" more data to this handler by
 * embedding the handler within a custom data structure and using the
 * \a container_of macro to cast an handler to the containing structure.
 */
typedef struct RTLIB_ExecutionContextParams* RTLIB_ExecutionContextHandler;

/**
 * @brief An application provided function to stop the execution of an EC.
 *
 * This function allow the run-time manager to (gracefully) stop an
 * application thus releasing resources it is using.  Each application is
 * required to registers such a functions with the RTLib at library
 * initialization time. Once this function returns, or the specified timeout
 * expires [us], the Barbeque RTRM is authorized to forcely stop the
 * application, i.e. kill and release all the resources.
 *
 * @note after such a call the application is not forced to completely exit,
 * instead it could wait for resources becoming available and assigned and
 * then restert the processing from the point it was suspended.
 * If properly handled, this mechanism should allows to easily support more
 * advanced suspension/hibernation features.
 */
typedef RTLIB_ExitCode (*RTLIB_StopExecution)(
		RTLIB_ExecutionContextHandler ech,
		struct timespec timeout);

/**
 * @brief The parameters to register an execution context.
 *
 * Contains all the information that an application must provide to the RTLib
 * upon library initialization (e.g. version, interfacing functions, and
 * programming language).
 */
typedef struct RTLIB_ExecutionContextParams {
        /** The "execution context" implemented API version */
        RTLIB_APIVersion version;
        /** The application code language */
        RTLIB_ProgrammingLanguage language;
		/** The identifier of the "execution context" recipe */
		RTLIB_Recipe recipe;
        /** The execution object destruction function */
        RTLIB_StopExecution StopExecution;
} RTLIB_ExecutionContextParams;

/**
 * @brief A pointer to an "execution context" registration function.
 *
 * A function implemented by the RTLib which allows an application to register
 * an "execution context" (EC) to the Barbeque run-time manager, by provinding
 * all the parameters required by the RTLIB_RegisterParams struct.
 *
 * @param name the EC name
 * @param params the EC registration parameters
 *
 * @return an handler to the registered execution context, or NULL on errors.
 *
 * @note This schema allows a single application to register different
 * "execution contexts". Each "execution context", from the prespective of the
 * Barbeque run-time manager is a independent entity which require access to
 * computation fabric resources based on its proper "working modes".
 */
typedef RTLIB_ExecutionContextHandler (*RTLIB_Register)(
		const char* name,
		const RTLIB_ExecutionContextParams *params);

/**
 * @brief A pointer to an "execution context" start function.
 *
 * A function implemented by the RTLib which allows an application to mark an
 * "execution context" (EC), which has been previouslty registered to the
 * Barbeque run-time manager, to start executing
 *
 * @param ech the handlers if the EC to start.
 *
 * @return RTLIB_OK on requrest success, an error exit code otherwise.
 *
 * @note This call ask the Barbeque RTRM to schedule resources for this EC as
 * soon as possible. Indeed, once this call returns, the application must wait
 * for the RTRM to assign him an initiali working mode.  This is done by
 * calling the RTLIB_SetWorkingMode, a callback function defined by the
 * application at EC registration time.
 */
typedef RTLIB_ExitCode (*RTLIB_Start)(
		const RTLIB_ExecutionContextHandler ech);

/**
 * @brief A pointer to an "execution context" stop function.
 *
 * A function implemented by the RTLib which allows an application to stop an
 * "execution context" (EC), which has been previouslty started, by provinding
 * an handler to such EC.
 *
 * @param ech a vector of handlers representing the EC to stop.
 *
 * @return RTLIB_OK on requrest success, an error exit code otherwise.
 *
 * @note This call ask the Barbeque RTRM to release schedule resources for
 * this EC as soon as possible.
 */
typedef RTLIB_ExitCode (*RTLIB_Stop)(
		const RTLIB_ExecutionContextHandler ech);

/**
 * @brief A pointer to an "execution context" un-registration function.
 *
 * A function implemented by the RTLib which allows an application to
 * unregister a previously defined "execution context" (EC) by the Barbeque
 * run-time manager.  This call will release all the resources currently
 * allocated to the specified EC.
 *
 * @param ech a vector of handlers representing the ECs to undergister.
 * @param count the number of ECs to unregister.
 */
typedef void (*RTLIB_Unregister)(
		const RTLIB_ExecutionContextHandler ech);

typedef enum RTLIB_SyncType {
	/** @brief A stateless synchronization point.
	 * When an application is on a STATELESS synchronization point,
	 * the current working mode could be switched without saving any current
	 * status, thus this is usually associated to a lower switching overhead */
	RTLIB_SYNC_STATELESS = 0,
	/** @brief A statefull synchronization point.
	 * When an application is on a STATEFULL synchronization point, switching
	 * the current working mode requires to save some status. This could
	 * incurr on an higer switching overhead */
	RTLIB_SYNC_STATEFULL
} RTLIB_SyncType;

/**
 * @brief A pointer to a "synchronization point" notification function.
 *
 * An execution context (EC) is enchouraged to support the Barbeque run-time
 * manager by notifying the reaching of a synchronization point. A
 * synchronization point is a point during the workload processing, when the
 * EC is on a consistent state, i.e. a changin of working mode has the lower
 * impact on both performances and workload trashing. Some example of
 * synchronization point are: the end of a frame deconding for a video
 * decoding application, the end of a packet processing for networking
 * applications.
 * The execution context should notify these events to the RTLib which in
 * turns could exploit them to better arrange working modes reconfiguration
 * among all the active applicatons.
 *
 * @param ech the handler of the EC which is at the sync point
 * @param name the id of the notified syncpoint
 * @type the type of the reached syncpoint.
 *
 * @return true if the application could continue its execution, false
 * otherwise (i.e. the applications should suspend waiting for a
 * working mode reconfiguration)
 */
typedef bool (*RTLIB_Sync)(
		const RTLIB_ExecutionContextHandler ech,
		const char *name,
		RTLIB_SyncType type);

/**
 * @brief The possible boundary asserted by a resource constraint.
 */
typedef enum RTLIB_ConstraintType {
	/** Targets AWMs lower or equal to the specified one */
	LOW_BOUND = 0,
	/** Targets AWMs higer or equal to the specified one */
	UPPER_BOUND,
	/** Targets the specified AWM */
	EXACT_VALUE
} RTLIB_ConstraintType;

/**
 * @brief A constraint asserted on recipe specified working modes
 *
 * Applications have an associated set of working modes, each one defining a
 * certain amount of resources usage.  Applications could assert some
 * constraints at run-time, to invalidate a subset of its own working modes.
 */
typedef struct RTLIB_Constraint {
	/** The identified of an Application Working Mode (AWM */
	uint8_t awm;
	/** The required operation: true to add the specified constraint, false to
	 * remove it */
	bool add;
	/** The constraint boundary */
	RTLIB_ConstraintType type;
} RTLIB_Constraint;

/**
 * @brief A pointer to a function to assert a resource constraint.
 *
 * An execution context (EC) could assert a constraint to the Barbeque RTRM, so
 * that it could do the best to schedule for this EC a working mode which is
 * compliant with the requitement. An application could assert a set of
 * constraints at run-time to invalidate certain working modes.
 *
 * @param constraints a vector of constraints to be asserted
 * @oaran count the number of constraint to be asserted
 *
 * @note the Barbeque RTRM will do the best to satisfay a constraint
 * requirements, however, it is worth to notice that, since we are in a mixed
 * workload scenario, it could be not possible to achieve such a result.
 * Thus, an application asserting a constraint <em>must</em> wait for a
 * confermative responce from Barbeque before accessing the required
 * resources, this is particularely critical in the case the EC want to
 * increase the used resource (i.e. setting a lower bound higer than the
 * warking mode currently in use).
 */
typedef RTLIB_ExitCode (*RTLIB_SetConstraints)(
		RTLIB_ExecutionContextHandler ech,
		RTLIB_Constraint *constraints,
		uint8_t count);

/**
 * @brief A pointer to a function to remove previously asserted contraints.
 */
typedef RTLIB_ExitCode (*RTLIB_ClearConstraints)(
		RTLIB_ExecutionContextHandler ech);

/**
 * @brief Get the authorized working moded.
 *
 * This function allow the Barbeque RTRM to assigne an Operating Point to the
 * application. Each application should call this method when an Operating
 * Point change is expected, e.g. after a sync which returned false, or when a
 * working mode should be initially assigned.
 *
 * @param ech the handler of the EC to configure
 * @param wm a pointer to the selected working mode
 *
 * @return true if a working mode has been assigned, false otherwise
 *
 * @note this method is blocking, the application could be "suspended" on this
 * call till a new working mode has been assigned to it.
 */
typedef RTLIB_ExitCode (*RTLIB_GetWorkingMode)(
		RTLIB_ExecutionContextHandler ech,
		RTLIB_WorkingModeParams *wm);

/**
 * @brief Information passed to the application at RTLib initialization time.
 *
 * This struct aggregate all the services that the Barbeque provides to
 * application (e.g., version, registeration of "execution contexts" and
 * service functions). This struct is passed to each application at library
 * initialization time.
 */
typedef struct RTLIB_Services {
        /** Current version of the plugins API */
        RTLIB_APIVersion version;
		/** Execution contexts registration
		 * Applications use this function at RTLib library initialization
		 * time to register each "execution context" they want. */
        RTLIB_Register RegisterExecutionContext;
		/** Execution contexts scheduing
		 * Applications use this function to ask resources for a specified ECs
		 * (ar all the registered ones).*/
        RTLIB_Start StartExecutionContext;
		/** Synchronization point notification
		 * An execution context must use this function to notify the RTLib each
		 * time they reach a synchronization point */
		RTLIB_Sync NotifySync;
		/** Return the assigned Working Mode.
		 * Applications call this method, during the initialization or after a
		 * false returning sync, to get a reference to the new assigned
		 * working mode. */
		RTLIB_GetWorkingMode GetWorkingMode;
		/** Constraints assertion on a recipe working modes An execution
		 * context could set a boundary on a set of working modes to consider
		 * at run-time for resource scheduling. The Barbeque RTRM resource
		 * scheduling of working modes should satisfying these requirement. */
		RTLIB_SetConstraints SetConstraints;
		/** Constraints removal on recipe working modes */
		RTLIB_ClearConstraints ClearConstraints;
		/** Execution contexts release
		 * Applications use this function to release resources for a specified
		 * ECs (ar all the scheduled ones).*/
        RTLIB_Stop StopExecutionContext;
		/** Execution contexts un-registration
		 * Applications use this function to un-register an "execution
		 * context" */
        RTLIB_Unregister UnregisterExecutionContext;
} RTLIB_Services;


#ifdef  __cplusplus
}
#endif

#endif // BBQUE_RTLIB_H_

