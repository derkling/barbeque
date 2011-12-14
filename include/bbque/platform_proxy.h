/**
 *       @file  platform_proxy.h
 *      @brief  A proxy to interact with the target platform
 *
 * Definition of the class used by Barbeque RTRM core modules to communicate
 * with the platform by means of a platforms-specific abstraction and
 * integration layer. This class provides all the basic services which are
 * required by the core modules in order to:
 * <li><i>get a platform description:</i> by collecting a description of
 * resources availability and their run-time sate</li>
 * <li><i>control resource partitioning:</i> by binding resources to the
 * assignee applications</li>
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  11/23/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#ifndef BBQUE_PLATFORM_PROXY_H_
#define BBQUE_PLATFORM_PROXY_H_

#include "bbque/plugins/logger.h"
#include "bbque/app/application.h"
#include "bbque/res/resource_accounter.h"

#include <memory>
#include <thread>

#define PLATFORM_PROXY_NAMESPACE "bq.pp"

using bbque::app::AppPtr_t;
using bbque::res::RViewToken_t;
using bbque::res::UsagesMapPtr_t;

namespace bbque {

/**
 * @brief The Platform Proxy module
 */
class PlatformProxy {

public:

	/**
	 * @brief Exit codes returned by methods of this class
	 */
	typedef enum ExitCode {
		OK = 0,
		PLATFORM_INIT_FAILED,
		PLATFORM_ENUMERATION_FAILED,
		PLATFORM_NODE_PARSING_FAILED,
		PLATFORM_DATA_NOT_FOUND,
		PLATFORM_DATA_PARSING_ERROR,
		MAPPING_FAILED
	} ExitCode_t;

/**
 * @defgroup group_plt_prx Platform Proxy
 * @{
 *
 * @name Basic Infrastructure and Initialization
 * @{
 * The <tt>PlatformProxy</tt> is the BarbequeRTRM core module in charge to
 * manage the communication with the target platform by means of a
 * platforms-specific abstraction and integration layer.
 *
 * This class provides all the basic services which are required by the core
 * modules in order to:
 * <ul>
 * <li><i>get a platform description:</i> by collecting a description of
 * resources availability and their run-time sate</li>
 * <li><i>control resource partitioning:</i> by binding resources to the
 * assignee applications</li>
 * </ul>
 */

	/**
	 * @brief Get a reference to the paltform proxy
	 */
	static PlatformProxy & GetInstance();

	/**
	 * @brief Release all the platform proxy resources
	 */
	~PlatformProxy();

	/**
	 * @brief Start the platform monitoring thread
	 *
	 * The platform integration layer provides support for monitoring
	 * resources status at run-time. This methods setup and trigger the
	 * execution of the proper platform monitoring thread.
	 */
	void Start();

	/**
	 * @brief Stop the platform monitoring thread
	 *
	 * The platform integration layer provides support for monitoring
	 * resources status at run-time. This methods setup and trigger the
	 * execution of the proper platform monitoring thread.
	 */
	void Stop();

/**
 * @}
 * @name Platform description
 * @{
 */

	/**
	 * @brief Look-up for a platform description and load it.
	 *
	 * A description of all platform resources is looked-up by a call of this
	 * method which provides also their registration into the ResourceManager
	 * module.
	 */
	ExitCode_t LoadPlatformData();

/**
 * @}
 * @name Resource state monitoring
 * @{
 */


/**
 * @}
 * @name Resource binding
 * @{
 */

	/**
	 * @brief Setup platform data required to manage the specified applciation
	 *
	 * Once a new application enter the system, a set of platform specific
	 * applications data could be require in order to properly set-up the
	 * platform for run-time management. This method should be called to
	 * perpare the groud for run-time platform control.
	 */
	ExitCode_t Setup(AppPtr_t papp);


	/**
	 * @brief Release platform data used to manage the specified application
	 *
	 * Once an application exit the system, this method should be called to
	 * properly release all the platform specific data used to manage it at
	 * run-time.
	 */
	ExitCode_t Release(AppPtr_t papp);

	/**
	 * @brief Release all the resources assigned to the specified application.
	 *
	 * @param papp The application which resources are reclaimed
	 */
	ExitCode_t ReclaimResources(AppPtr_t papp);

	/**
	 * @brief Bind the specified resources to the specified application.
	 *
	 * @param papp The application which resources are assigned
	 * @param pres The resources to be assigned
	 * @param excl If true the specified resources are assigned for exclusive
	 * usage to the application
	 */
	ExitCode_t MapResources(AppPtr_t papp, UsagesMapPtr_t pres,
			bool excl = true);

/**
 * @}
 * @}
 */

private:


	/**
	 * @brief The platform monitoring thread
	 */
	std::thread monitor_thd;

	/**
	 * @brief Status of the monitoring thread
	 *
	 * This variable is set true when the monitoring thread has been autorized
	 * to run.
	 */
	bool trdRunning;


	/**
	 * @brief Set true to terminate the moniroting thread
	 *
	 * This is set to true to end the monitoring thread.
	 */
	bool done;

	/**
	 * @brief Mutex controlling the thread execution
	 */
	std::mutex trdStatus_mtx;

	/**
	 * @brief Conditional variable used to signal the monitoring thread
	 */
	std::condition_variable trdStatus_cv;

	/**
	 * @brief The platform monitoring thread.
	 */
	void Monitor();


protected:

	/**
	 * @brief The logger module
	 */
	plugins::LoggerIF *logger;

	/**
	 * @biref Build a new platform proxy
	 */
	PlatformProxy();

/*******************************************************************************
 *  Platform Specific (low-level) methods
 ******************************************************************************/

	/**
	 * @brief Platform specific resource setup interface.
	 */
	virtual ExitCode_t _Setup(AppPtr_t papp) = 0;

	/**
	 * @brief Platform specific resources enumeration
	 */
	virtual ExitCode_t _LoadPlatformData() = 0;

	/**
	 * @brief Platform specific resources release interface.
	 */
	virtual ExitCode_t _Release(AppPtr_t papp) = 0;

	/**
	 * @brief Platform specific resource claiming interface.
	 */
	virtual ExitCode_t _ReclaimResources(AppPtr_t papp) = 0;


	/**
	 * @brief Platform specifi resource binding interface.
	 */
	virtual ExitCode_t _MapResources(AppPtr_t papp, UsagesMapPtr_t pres,
			RViewToken_t rvt, bool excl) = 0;


};

} /* bbque */

#endif /* end of include guard: BBQUE_PLATFORM_PROXY_H_ */
