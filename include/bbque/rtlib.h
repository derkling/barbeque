/**
 *       @file  rtlib.h
 *      @brief  The Run-Time Library (RTLib) to access the Barbeque RTRM
 *      framework
 *
 * The Barbeque RTRM is a modular and efficient resource allocator for
 * many-core clustered accelerator engines, which provide support for:
 * <ul>
 *  <li>mixed workload management</li>
 *  <li>dynamic resource partitioning</li>
 *  <li>resources abstraction</li>
 *  <li>multi-objectve optimization policy</li>
 *  <li>low-overheads run-time management</li>
 * </ul>
 *
 * Modern many-core computing platform are targeted to the execution of a
 * <i>mixed-workload</i> where multiple applications, with different
 * requirements and criticality levels, run concurrently thus competing on
 * accessing a reduce set of shared resource. Provided that each of these
 * application is associated with a proper description, which should specify
 * the mapping between an expected Quality-of-Service (QoS) level and the
 * correposnding resources demand, the Barbeque RTRM allows to arrange for an
 * "optimal" resources allocation to all the active applications.
 *
 * The main goal of the <i>dynamic resource partitioning</i> support is to
 * grant resources to critical workloads while dynamically yield these
 * resources to best-effort workloads when (and only while) they are not
 * required by critical ones, thus optimize resource usage and fairness.
 *
 * In order to improve applications portability among different acceleration
 * platforms, the Barbeque RTRM provides support for <i>resource
 * abstraction</i> which allows to exploit a decoupled perspective of the
 * resources between the users and the underlying hardware.  The user
 * applications will see virtual resources, e.g., the number of processing
 * elements available, but they will not be aware of which of the physical
 * resources are effectively available. At run-time the RTRM will perform
 * the virtual-to-physical mapping according to the current <i>multi-objective
 * optimization function</i> (low power, high performance, etc..) and run-time
 * phenomena (process variation, temporal and spatial temperature gradients,
 * hardware failures and, above all, workload variation).
 *
 * Finally, introducing a low overhead on managing resource at run-time, while
 * still granting a valuable solution for the aformentioned issues is one more
 * goal of the Barbeque RTRM framework. This goals also as required the
 * development if this RTLib run-time library which applications should
 * exploit in order to take advantages from the Barbeque RTRM.
 *
 * This document provides the definition of the interface and related services
 * exported by the Barbeque Run-Time library (RTLib). This library is provided
 * to be linked by application that needs to interact with the Barbeque (BBQ)
 * Run-Time Resource Manager (RTRM). The library not only defines the set of
 * services accessable by applications, but allows also to mask the platform
 * specific communication channel between controlled applications and the
 * run-time manager. Moreover, the library provides some of the functional
 * supports required for a proper integration of an application with the RTRM,
 * thus offloading the application developer as much as possible from the
 * Run-Time management required boilerplate integration code. Indeed, form an
 * application developer perspective this library enforeces just the
 * conformance to a properly define "Application Lifecycle", by specifying a
 * set of calls and the order on which these should be performed.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
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

#include <stdint.h>
#include <time.h>
#include "assert.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief The RTLib main version
 *
 * This version is checked at library initialization time with respect to the
 * verions of the Barbeque RTRM instance running on the target system.
 * A Major version metch is required in order to properly setup the
 * communication with the RTRM, otherwise the library initilization will
 * fails.
 */
#define RTLIB_VERSION_MAJOR 1

/**
 * @brief The RTLib revision version
 *
 * The mainor version is increased at each internal library updated which do
 * not preclude backward compatibilities.
 */
#define RTLIB_VERSION_MINOR 2

/**
 * @brief The maximum length for an "application" name
 *
 * The application name is used mainly for debugging and tracing purposes in
 * order to uniquely identify an application within the Barbeque RTRM.
 */
#define RTLIB_APP_NAME_LENGTH 32

/**
 * The maximum length for an "execution context" name
 *
 * Each Execution Context (EXC) an application register is associated to a
 * name which is mainly used for debugging and tracing purposes.
 */
#define RTLIB_EXC_NAME_LENGTH 32

/**
 * @brief The maximum length for a recipe name
 *
 * Each EXC is associated to a recipe file describing the application working
 * modes, their expected Quaility-of-Service and the correponding resourcec
 * demand. The recipe name is used to identify and load the corresponding
 * recipe from within a system folder where all recipes should be deployed
 * when the application is installed.
 *
 * Recipes filename are expected to end with the ".recipe" extension. However,
 * the name indicated by an application should <i>not</i> include the
 * extension and it could have the maximum lenght defined by this constant.
 */
#define RTLIB_RECIPE_NAME_LENGTH 64

// Forward declarations
typedef struct RTLIB_Services RTLIB_Services_t;
typedef struct RTLIB_APIVersion RTLIB_APIVersion_t;
typedef struct RTLIB_WorkingModeParams  RTLIB_WorkingModeParams_t;
typedef struct RTLIB_ExecutionContextParams RTLIB_ExecutionContextParams_t;
typedef struct RTLIB_Constraint RTLIB_Constraint_t;


/*******************************************************************************
 *    RTLib Types
 ******************************************************************************/

/**
 * @brief The Execution Context (EXC) programming language.
 */
typedef enum RTLIB_ProgrammingLanguage {
	/** Undefined application language */
	RTLIB_LANG_UNDEF = 0,
	/** C coded EXC */
	RTLIB_LANG_C,
	/** CPP coded EXC */
	RTLIB_LANG_CPP,
	/** OpenCL coded EXC */
	RTLIB_LANG_OPENCL,
	/** P2012 Native Programming Model coded EXC */
	RTLIB_LANG_NPL
} RTLIB_ProgrammingLanguage_t;

/**
 * @brief Return code generated by RTLib services
 */
typedef enum RTLIB_ExitCode {

	/** Success (no errors) */
	RTLIB_OK = 0,
	/** Uspecified (generic) error */
	RTLIB_ERROR,
	/** RTLib Version does not match that of the Barbeque RTRM */
	RTLIB_VERSION_MISMATCH,
	/** No new working mode error */
	RTLIB_NO_WORKING_MODE,

//---- Barbeque Communicaiton errors

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
	/** The (expected) response has gone in TIMEOUT */
	RTLIB_BBQUE_CHANNEL_TIMEOUT,
	/** The Barbeque RTRM is not available */
	RTLIB_BBQUE_UNREACHABLE,

//---- EXC Management errors

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
	/** The Execution Context has not been started yet */
	RTLIB_EXC_NOT_STARTED,
	/** The Execution Context Start Failed */
	RTLIB_EXC_ENABLE_FAILED,
	/** The Execution Context is not currentyl enabled */
	RTLIB_EXC_NOT_ENABLED,
	/** The Execution Context Stop Failed */
	RTLIB_EXC_DISABLE_FAILED,
	/** Failed to get a working mode */
	RTLIB_EXC_GWM_FAILED,

//---- Reconfiguration actions required for an EXC
// NOTE these values should match (in number and order) those defined by the
//      ApplicationStatusIF::SyncState_t

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
	RTLIB_EXC_SYNCP_FAILED,
	/** No more workload to process */
	RTLIB_EXC_WORKLOAD_NONE,

// NOTE The last entry should not overflow a uint8_t, otherwise some of the RPC
// channel messages should be updated

	RTLIB_EXIT_CODE_COUNT

} RTLIB_ExitCode_t;

/**
 * @brief The operation requested on a resource constraint.
 */
typedef enum RTLIB_ConstraintOperation {
	/** Remove the specified consraint */
	CONSTRAINT_REMOVE = 0,
	/** Add the specified consraint */
	CONSTRAINT_ADD
} RTLIB_ConstraintOperation_t;

/**
 * @brief The possible boundary asserted by a resource constraint.
 */
typedef enum RTLIB_ConstraintType {
	/** Targets AWMs lower or equal to the specified one */
	LOWER_BOUND = 0,
	/** Targets AWMs higer or equal to the specified one */
	UPPER_BOUND,
	/** Targets the specified AWM */
	EXACT_VALUE
} RTLIB_ConstraintType_t;

/**
 * @brief The kind of a synchronization point.
 *
 * The Barbeque RTRM is able to manage applicaitons which could have multiple
 * Synchronization Points (SP) at different time-frame granulatiry level. Mainly it
 * allows to distinguis between two kind of synchronization points: stateless
 * and statefull.
 *
 * <ul>
 *   <li>
 * A <i>stateless synchronization point (SLSP)</i> happens when the application could
 * be reconfigured with almost zero overhead. This happens when the
 * application has not a currently valida state saved on Barbeque managed
 * resources.
 *   </li>
 *   <li>
 * A <i>statefull synchronization point(SFSP)</i> happens when the application reach
 * a point in which can switch to a different working mode, but this requires
 * some preparational activity in order to save the current state to avoid
 * losing some of the work being already completed. Status saving will
 * probably introduce overheads which could impact on the platform
 * synchronization latency.
 *   </li>
 * </ul>
 *
 * For example, considering a video deconding application, a SFSP is reached
 * every time a frame is decoded, while we are on a SFSP at each keyframe.
 */
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
} RTLIB_SyncType_t;

/**
 * @brief The recipe of an Execution Context (EXC).
 *
 * Each EXC that an application could register to the Barbeque run-time
 * resource manager, is identified and described by a corresponding "recipe".
 * A recipe collects a detailed description of resource amount required by
 * each working mode of the application.
 *
 * @note This name must never exceed the size specified by
 * RTLIB_RECIPE_NAME_LENGTH
 */
typedef const char* RTLIB_Recipe_t;

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
typedef RTLIB_ExecutionContextParams_t* RTLIB_ExecutionContextHandler_t;


/*******************************************************************************
 *    RTLib Data Structures
 ******************************************************************************/

/**
 * @brief The Barbeque API version number.
 */
struct RTLIB_APIVersion {
	/** Major API version number */
	int32_t major;
	/** Minor API version number */
	int32_t minor;
};

/**
 * @brief The information passed to an application to set its new Working Mode.
 */
struct RTLIB_WorkingModeParams {
	/** The ID of the working mode */
	uint8_t awm_id;
	/** The set of platform supported services */
	const RTLIB_Services_t* services;
};

/**
 * @brief The parameters to register an execution context.
 *
 * Contains all the information that an application must provide to the RTLib
 * upon library initialization (e.g. version, interfacing functions, and
 * programming language).
 */
struct RTLIB_ExecutionContextParams {
        /** The "execution context" implemented API version */
        RTLIB_APIVersion_t version;
        /** The application code language */
        RTLIB_ProgrammingLanguage_t language;
		/** The identifier of the "execution context" recipe */
		RTLIB_Recipe_t recipe;
};

/**
 * @brief A constraint asserted on recipe specified working modes
 *
 * Applications have an associated set of working modes, each one defining a
 * certain amount of resources usage.  Applications could assert some
 * constraints at run-time, to invalidate a subset of its own working modes.
 */
struct RTLIB_Constraint {
	/** The identified of an Application Working Mode (AWM) */
	uint8_t awm;
	/** The required operation on the previous AWM */
	RTLIB_ConstraintOperation_t operation;
	/** The constraint boundary */
	RTLIB_ConstraintType_t type;
};


/*******************************************************************************
 *    RTLib "plain" API (RSD)
 ******************************************************************************/

/**
 * @brief The RTLib library entry point.
 *
 * This function must be the first one called by an application to properly
 * initialized the RTLib.  Library initialization is required to propery setup
 * the communication channel with the Barbeque RTRM and prepare the ground for
 * EXC management.
 *
 * @param name the name of the calling application, this name will be used
 * by both the Barbeque RTRM and the RTLib for tracing purposes. The name
 * should not exceed the value defined by RTLIB_APP_NAME_LENGTH.
 *
 * @param services a pointer to a pointer to an RTLib services struct. Such
 * struct will provide the handlers for all the library provided services.
 * Thus, this is an output parameter which is not NULL only if the library
 * initialization success, i.e. this call return RTLIB_OK.
 *
 * @return RTLIB_OK on registration success, RTLIB_VERSION_MISMATCH if the
 * library verison does not match that of the running Barbeque instance,
 * RTLIB_CHANNEL_SETUP_FAILED if the library failed to setup a communication
 * channel with the Barbeque RTRM instance.
 */
RTLIB_ExitCode_t RTLIB_Init(const char* name, RTLIB_Services_t **services);

/**
 * @brief Application provided callback to stop the specified EXC.
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
typedef RTLIB_ExitCode_t (*RTLIB_Stop_t)(
		RTLIB_ExecutionContextHandler_t ech,
		struct timespec timeout);

/**
 * @brief Pointer to an EXC registration function
 *
 * A function implemented by the RTLib which allows an application to register
 * an "execution context" (EXC) to the Barbeque run-time manager, by provinding
 * all the parameters required by the RTLIB_RegisterParams struct.
 *
 * @param name the EXC name
 * @param params the EXC registration parameters
 *
 * @return an handler to the registered execution context, or NULL on errors.
 *
 * @note This schema allows a single application to register different
 * "execution contexts". Each "execution context", from the prespective of the
 * Barbeque run-time manager is a independent entity which require access to
 * computation fabric resources based on its proper "working modes".
 */
typedef RTLIB_ExecutionContextHandler_t (*RTLIB_Register_t)(
		const char* name,
		const RTLIB_ExecutionContextParams_t *params);

/**
 * @brief Pointer to an EXC enabling function
 *
 * This function is provided by the RTLib to enable the scheduling of the
 * specified EXC. The specified execution context must have been previouslty
 * registered.  A call to this method will switch the EXC state to READY, thus
 * making it eligible for resources allocation.
 *
 * @param ech the handlers of the EXC to enable.
 *
 * @return RTLIB_OK on enabling success, an error exit code otherwise.
 *
 * @note This call enable the EXC but does not require the Barbeque RTRM to
 * schedule resources for this EXC as soon as possible.  Indeed, after this
 * call returns, the application must issue a GetWorkingMode() to actually get
 * an AWM assigned.
 */
typedef RTLIB_ExitCode_t (*RTLIB_Enable_t)(
		const RTLIB_ExecutionContextHandler_t ech);

/**
 * @brief Pointer to an EXC disabling function.
 *
 * This function is provided by the RTLib to disable the scheduling of the
 * specified EXC. The specified execution context must have been previouslty
 * registered.  A call to this method will switch the EXC state to DISABLED,
 * thus making it not more eligible for resources allocation and forcing a
 * release of all the resources eventaully assigned to the specified EXC.
 *
 * @param ech the handler of the EXC to disable
 *
 * @return RTLIB_OK on disabling success, an error exit code otherwise.
 *
 * @note This call ask the Barbeque RTRM to release schedule resources for
 * this EXC as soon as possible.
 */
typedef RTLIB_ExitCode_t (*RTLIB_Disable_t)(
		const RTLIB_ExecutionContextHandler_t ech);

/**
 * @brief Pointer to an EXC un-registration function.
 *
 * A function implemented by the RTLib which allows an application to
 * unregister a previously defined "execution context" (EXC).  This call will
 * release all the resources currently allocated to the specified EXC.
 *
 * @param ech the handler of the EXCs to undergister.
 */
typedef void (*RTLIB_Unregister_t)(
		const RTLIB_ExecutionContextHandler_t ech);

/**
 * @brief Pointer to an EXC constraint assert function.
 *
 * An execution context (EXC) could assert a constraint to the Barbeque RTRM, so
 * that it could do the best to schedule for this EXC a working mode which is
 * compliant with the requitement. An application could assert a set of
 * constraints at run-time to invalidate certain working modes.
 *
 * @param constraints a vector of constraints to be asserted
 * @param count the number of constraint to be asserted
 *
 * @note the Barbeque RTRM will do the best to satisfay a constraint
 * requirements, however, it is worth to notice that, since we are in a mixed
 * workload scenario, it could be not possible to achieve such a result.
 * Thus, an application asserting a constraint <em>must</em> wait for a
 * confermative responce from Barbeque before accessing the required
 * resources, this is particularely critical in the case the EXC want to
 * increase the used resource (i.e. setting a lower bound higer than the
 * warking mode currently in use).
 */
typedef RTLIB_ExitCode_t (*RTLIB_SetConstraints_t)(
		RTLIB_ExecutionContextHandler_t ech,
		RTLIB_Constraint_t *constraints,
		uint8_t count);

/**
 * @brief Pointer to an EXC contraint release function.
 */
typedef RTLIB_ExitCode_t (*RTLIB_ClearConstraints_t)(
		RTLIB_ExecutionContextHandler_t ech);

/**
 * @brief Synchronization point notification and AWM authorization.
 *
 * An execution context (EXC) is enchouraged to support the Barbeque RTRM by
 * notifying the reaching of a synchronization point. A synchronization point
 * is a point during the workload processing, when the EXC is on a consistent
 * state, i.e. a changing of working mode has the lower impact on both
 * performances and workload trashing. Some example of synchronization point
 * could be: the end of a frame deconding for a video decoding application,
 * the end of a packet processing for networking applications.  The execution
 * context is required to notify these events to the RTLib which in turns
 * could exploit them to better arrange working modes reconfiguration among
 * all the active applicatons.
 *
 * Moreover, in a Run-Time Resource Managed system the Barbeque RTRM has the
 * role to assigne an Operating Point to each active application. Thus, each
 * application is required to call this method every time a Working Mode
 * change could be expected, i.e. at each synchronization point or when a
 * working mode should be initially assigned after the EXC has been enabled.
 *
 * Thus, at each synchronization point the application is required to call
 * this method and decide what to do based on the returned code.  If a working
 * mode change is required by the Barbeque RTRM, this method returns an error
 * which could be used to identify the kind of reconfiguration required.
 * Please not that, in each of this last cases, the application is expected to
 * perform the required re-configuration and then to call this method again to
 * get the authorization on using it. Indeed, this second call is used the the
 * RTLib to know that a reconfiguration has been completed thus notifying the
 * Barbeque RTRM, but also to collect useful statistics on reconfigurations
 * overhead in an application transparent way.
 *
 * @param ech the handler of the EXC to configure
 * @param wm a pointer to the selected working mode
 * @param st the kind of this synchronization point
 *
 * @return RTLIB_OK if a working mode has been assigned, RTLIB_EXC_GWM_FAILED
 * if a working mode could not been assinged. A value between
 * RTLIB_EXC_GWM_START and RTLIB_EXC_GWM_BLOCKED if a reconfiguraiton is
 * required. Please not that, in each of this last cases, the application is
 * expected to perform the required re-configuration and then to call this
 * method again to get the authorization on using it. Indeed, this second call
 * is used the the RTLib to know that a reconfiguration has been completed
 * thus notifying the Barbeque RTRM, but also to collect useful statistics on
 * reconfigurations overhead in an application transparent way.
 *
 * @note this method is blocking, the application could be "suspended" on this
 * call until a new working mode has been assigned to it.
 */
typedef RTLIB_ExitCode_t (*RTLIB_GetWorkingMode_t)(
		RTLIB_ExecutionContextHandler_t ech,
		RTLIB_WorkingModeParams_t *wm,
		RTLIB_SyncType_t st);


/*******************************************************************************
 *    Performance Monitoring Support
 ******************************************************************************/

/**
 * @brief Setup the RTLib notification support
 *
 * Before using notification handler, an application must call this method to
 * allow the run-time library to properly initialize the performance
 * monitoring support. This method must be called from the main thread that
 * want to be monitored and profiled.
 *
 * @param ech the handler of the EXC to configure
 */
typedef void (*RTLIB_Notify_Setup)(
		RTLIB_ExecutionContextHandler_t ech);

/**
 * @brief Notify the RTLib an EXC has been initialized
 *
 * Once an EXC has been properly configured and initialized, an application is
 * encouraged to notify the run-time library by calling this method.  This
 * could be used by the RTLib implementation to properly enable resources
 * required to monitor application perforamnces.
 *
 * @param ech the handler of the EXC to configure
 */
typedef void (*RTLIB_Notify_Init)(
		RTLIB_ExecutionContextHandler_t ech);

/**
 * @brief Notify the RTLib an EXC has completed
 *
 * Once an EXC has completed its workload, an application is encouraged to
 * notify the run-time library by calling this method.  This could be used by
 * the RTLib implementation to properly release all the resources used to
 * monitor application perforamnces.
 *
 * @param ech the handler of the EXC to configure
  */
typedef void (*RTLIB_Notify_Exit)(
		RTLIB_ExecutionContextHandler_t ech);

/**
 * @brief Notify the RTLib a "reconfiguration" is starting
 *
 * Once a working mode reconfiguration is srarting, an application is
 * encouraged to notify the run-time library by calling this method. This
 * could be used by the RTLib implementation to collect suitable information
 * and statistics on reconfigurations.
 *
 * @param ech the handler of the EXC to configure
 */
typedef void (*RTLIB_Notify_PreConfigure)(
		RTLIB_ExecutionContextHandler_t ech);

/**
 * @brief Notify the RTLib a "reconfiguration" has completed
 *
 * Once a working mode reconfiguration has been completed, an application is
 * encouraged to notify the run-time library by calling this method. This
 * could be used by the RTLib implementation to collect suitable information
 * and statistics on reconfigurations.
 *
 * @param ech the handler of the EXC to configure
 */
typedef void (*RTLIB_Notify_PostConfigure)(
		RTLIB_ExecutionContextHandler_t ech);

/**
 * @brief Notify the RTLib a "run" is starting
 *
 * Once a new processing cycle is starting, an application is encouraged to
 * notify the run-time library by calling this method. This could be used by
 * the RTLib implementation to collect suitable information and statistics on
 * application processing.
 *
 * @param ech the handler of the EXC to configure
 */
typedef void (*RTLIB_Notify_PreRun)(
		RTLIB_ExecutionContextHandler_t ech);


/**
 * @brief Notify the RTLib a "run" has completed
 *
 * Once a new processing cycle has been completed, an application is
 * encouraged to notify the run-time library by calling this method. This
 * could be used by the RTLib implementation to collect suitable information
 * and statistics on application processing.
 *
 * @param ech the handler of the EXC to configure
 */
typedef void (*RTLIB_Notify_PostRun)(
		RTLIB_ExecutionContextHandler_t ech);

/**
 * @brief Notify the RTLib a "monitor" is starting
 *
 * Once a new QoS monitor is starting, an application is encouraged to
 * notify the run-time library by calling this method. This could be used by
 * the RTLib implementation to collect suitable information and statistics on
 * application performance monitoring.
 *
 * @param ech the handler of the EXC to configure
 */
typedef void (*RTLIB_Notify_PreMonitor)(
		RTLIB_ExecutionContextHandler_t ech);


/**
 * @brief Notify the RTLib a "monitor" has completed
 *
 * Once a new QoS monitor has been completed, an application is
 * encouraged to notify the run-time library by calling this method. This
 * could be used by the RTLib implementation to collect suitable information
 * and statistics on application performance monitoring.
 *
 * @param ech the handler of the EXC to configure
 */
typedef void (*RTLIB_Notify_PostMonitor)(
		RTLIB_ExecutionContextHandler_t ech);

/**
 * @brief Notify the RTLib a "suspend" is starting
 *
 * Once a working mode suspension is starting, an application is encouraged to
 * notify the run-time library by calling this method. This could be used by
 * the RTLib implementation to collect suitable information and statistics on
 * application suspension overheads.
 *
 * @param ech the handler of the EXC to configure
 */
typedef void (*RTLIB_Notify_PreSuspend)(
		RTLIB_ExecutionContextHandler_t ech);


/**
 * @brief Notify the RTLib a "suspend" has completed
 *
 * Once a working mode suspension has been completed, an application is
 * encouraged to notify the run-time library by calling this method. This
 * could be used by the RTLib implementation to collect suitable information
 * and statistics on application suspension overheads.
 *i
 * @param ech the handler of the EXC to configure
 */
typedef void (*RTLIB_Notify_PostSuspend)(
		RTLIB_ExecutionContextHandler_t ech);

/**
 * @brief Notify the RTLib a "resume" is starting
 *
 * Once a working mode resume is starting, an application is encouraged to
 * notify the run-time library by calling this method. This could be used by
 * the RTLib implementation to collect suitable information and statistics on
 * application resume overheads.
 *
 * @param ech the handler of the EXC to configure
 */
typedef void (*RTLIB_Notify_PreResume)(
		RTLIB_ExecutionContextHandler_t ech);


/**
 * @brief Notify the RTLib a "resume" has completed
 *
 * Once a working mode resume has been completed, an application is
 * encouraged to notify the run-time library by calling this method. This
 * could be used by the RTLib implementation to collect suitable information
 * and statistics on application resume overheads.
 *i
 * @param ech the handler of the EXC to configure
 */
typedef void (*RTLIB_Notify_PostResume)(
		RTLIB_ExecutionContextHandler_t ech);

/*******************************************************************************
 *    RTLib Services Descriptor (RSD)
 ******************************************************************************/

/**
 * @brief Information passed to the application at RTLib initialization time.
 *
 * This struct aggregate all the services that the Barbeque provides to
 * application (e.g., version, registeration of "execution contexts" and
 * service functions). This struct is passed to each application at library
 * initialization time.
*/
struct RTLIB_Services {
	/** Current version of the plugins API */
	RTLIB_APIVersion_t version;
	/** Execution contexts registration
	 * Applications use this function at RTLib library initialization
	 * time to register each "execution context" they want. */
	RTLIB_Register_t Register;
	/** Execution contexts scheduing
	 * Applications use this function to ask resources for a specified EXCs
	 * (ar all the registered ones).*/
	RTLIB_Enable_t Enable;
	/** Notify synchronization and get the assigned Working Mode.
	 * Applications call this method, during the initialization or after a
	 * false returning sync, to get a reference to the new assigned
	 * working mode. */
	RTLIB_GetWorkingMode_t GetWorkingMode;
	/** Constraints assertion on a recipe working modes An execution
	 * context could set a boundary on a set of working modes to consider
	 * at run-time for resource scheduling. The Barbeque RTRM resource
	 * scheduling of working modes should satisfying these requirement. */
	RTLIB_SetConstraints_t SetConstraints;
	/** Constraints removal on recipe working modes */
	RTLIB_ClearConstraints_t ClearConstraints;
	/** Execution contexts release
	 * Applications use this function to release resources for a specified
	 * EXCs (ar all the scheduled ones).*/
	RTLIB_Disable_t Disable;
	/** Execution contexts un-registration
	 * Applications use this function to un-register an "execution
	 * context" */
	RTLIB_Unregister_t Unregister;

	/* Performance estimation and notification interface */
	struct {
		/** Setup notifier */
		RTLIB_Notify_Setup Setup;
		/** Initialization notifier */
		RTLIB_Notify_Init Init;
		/** Finalization notifier */
		RTLIB_Notify_Exit Exit;
		/** Pre-Configuration notifier */
		RTLIB_Notify_PreConfigure PreConfigure;
		/** Post-Configuration notifier */
		RTLIB_Notify_PostConfigure PostConfigure;
		/** Pre-Run notifier */
		RTLIB_Notify_PreRun PreRun;
		/** Post-Run notifier */
		RTLIB_Notify_PostRun PostRun;
		/** Pre-Monitor notifier */
		RTLIB_Notify_PreMonitor PreMonitor;
		/** Post-Monitor notifier */
		RTLIB_Notify_PostMonitor PostMonitor;
		/** Pre-Suspend notifier */
		RTLIB_Notify_PreSuspend PreSuspend;
		/** Post-Suspend notifier */
		RTLIB_Notify_PostSuspend PostSuspend;
		/** Pre-Resume notifier */
		RTLIB_Notify_PreResume PreResume;
		/** Post-Resume notifier */
		RTLIB_Notify_PostResume PostResume;
	} Notify;
};


/*******************************************************************************
 *    RTLib Utils
 ******************************************************************************/

/**
 * @brief A stringified rapresentation of error messages
 */
extern const char *RTLIB_errorStr[];

/**
 * @brief Get a string description for the specified RTLib error
 */
inline char const *RTLIB_ErrorStr(RTLIB_ExitCode_t result) {
	assert(result < RTLIB_EXIT_CODE_COUNT);
	return RTLIB_errorStr[result];
}


#ifdef  __cplusplus
}
#endif

#endif // BBQUE_RTLIB_H_

