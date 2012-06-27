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

#ifndef BBQUE_PLATFORM_PROXY_H_
#define BBQUE_PLATFORM_PROXY_H_

#include "bbque/config.h"
#include "bbque/plugins/logger.h"
#include "bbque/app/application.h"
#include "bbque/resource_accounter.h"
#include "bbque/cpp11/thread.h"

#ifdef CONFIG_BBQUE_TEST_PLATFORM_DATA
# include "bbque/test_platform_data.h"
#endif // CONFIG_BBQUE_TEST_PLATFORM_DATA

#include <memory>

#define PLATFORM_PROXY_NAMESPACE "bq.pp"

using bbque::app::AppPtr_t;
using bbque::res::RViewToken_t;
using bbque::res::UsagesMapPtr_t;

namespace bbque {

/**
 * @brief The Platform Proxy module
 * @ingroup sec20_pp
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
		PLATFORM_COMM_ERROR,
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
	 * @breif Return the string ID of the target platform
	 *
	 * Each platform is uniquely identifyed by a string identifier. This
	 * method return a pointer to that string.
	 *
	 * @return A poiinter to the platform string identifier
	 */
	const char* GetPlatformID() const {
		assert(platformIdentifier != NULL);
		return platformIdentifier;
	};
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
	 * @brief Set true if the PIL has been properly initialized
	 *
	 * This is set to true if the Platform Integration Layer (PIL) has
	 * been properly initialized, thus Barbeque has control on assigned
	 * resources.
	 */
	bool pilInitialized;

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

	/**
	 * @brief Setup platform data required to manage the specified application
	 *
	 * Once a new application enter the system, a set of platform specific
	 * applications data could be require in order to properly set-up the
	 * platform for run-time management. This method should be called to
	 * perpare the groud for run-time platform control.
	 */
	ExitCode_t Setup(AppPtr_t papp);

protected:

	/**
	 * @brief The logger module
	 */
	plugins::LoggerIF *logger;

	/**
	 * @biref Build a new platform proxy
	 */
	PlatformProxy();


	/**
	 * @brief The platform specific string identifier
	 */
	const char *platformIdentifier;


	/**
	 * @brief Set the PIL as initialized
	 *
	 * The Platform Specific (low-level) module is expected to call this
	 * method if the platform dependent initialization has been completed
	 * with success.
	 */
	void SetPilInitialized() {
		pilInitialized = true;
	}

/*******************************************************************************
 *  Platform Specific (low-level) methods
 ******************************************************************************/

	/**
	 * @brief Return the Platform specific string identifier
	 */
	virtual const char* _GetPlatformID() {
		logger->Debug("PLAT PRX: default _GetPlatformID()");
		return "it.polimi.bbque.tpd";
	};

	/**
	 * @brief Platform specific resource setup interface.
	 */
	virtual ExitCode_t _Setup(AppPtr_t papp) {
		(void)papp;
		logger->Debug("PLAT PRX: default _Setup()");
		return OK;
	};

	/**
	 * @brief Platform specific resources enumeration
	 *
	 * The default implementation of this method loads the TPD, is such a
	 * function has been enabled
	 */
	virtual ExitCode_t _LoadPlatformData() {
#ifndef CONFIG_BBQUE_TEST_PLATFORM_DATA
		logger->Debug("PLAT PRX: default _LoadPlatformData()");
#else // !CONFIG_BBQUE_TEST_PLATFORM_DATA
		//---------- Loading TEST platform data
		logger->Debug("PLAT PRX: loading Test Platform Data (TPD)");
		TestPlatformData &tpd(TestPlatformData::GetInstance());
		tpd.LoadPlatformData();
#endif // !CONFIG_BBQUE_TEST_PLATFORM_DATA
		return OK;
	};

	/**
	 * @brief Platform specific resources release interface.
	 */
	virtual ExitCode_t _Release(AppPtr_t papp) {
		(void)papp;
		logger->Debug("PLAT PRX: default _Release()");
		return OK;
	};

	/**
	 * @brief Platform specific resource claiming interface.
	 */
	virtual ExitCode_t _ReclaimResources(AppPtr_t papp) {
		(void)papp;
		logger->Debug("PLAT PRX: default _ReclaimResources()");
		return OK;
	};

	/**
	 * @brief Platform specifi resource binding interface.
	 */
	virtual ExitCode_t _MapResources(AppPtr_t papp, UsagesMapPtr_t pres,
			RViewToken_t rvt, bool excl) {
		(void)papp;
		(void)pres;
		(void)rvt;
		(void)excl;
		logger->Debug("PLAT PRX: default _MapResources()");
		return OK;
	};


};

} // namespace bbque

#endif // BBQUE_PLATFORM_PROXY_H_
