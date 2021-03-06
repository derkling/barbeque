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

#ifndef BBQUE_APPLICATION_MANAGER_H_
#define BBQUE_APPLICATION_MANAGER_H_

#include <map>
#include <vector>

#include "bbque/config.h"
#include "bbque/application_manager_conf.h"
#include "bbque/utils/deferrable.h"
#include "bbque/plugins/logger.h"
#include "bbque/cpp11/mutex.h"

#define APPLICATION_MANAGER_NAMESPACE "bq.am"

using bbque::app::Application;
using bbque::app::RecipePtr_t;
using bbque::utils::Deferrable;
using bbque::plugins::LoggerIF;
using bbque::plugins::RecipeLoaderIF;

namespace bbque {

// Forward declaration
class PlatformProxy;

/**
 * @brief The class provides interfaces for managing the applications lifecycle.
 * @ingroup sec03_am
 *
 * This provides the interface for managing applications registration and keep
 * track of their schedule status changes. The class provides calls to
 * register applications, retrieving application descriptors, the maps of
 * application descriptors given their scheduling status or priority level.
 * Moreover to signal the scheduling change of status of an application, and
 * to know which is lowest priority level (maximum integer value) managed by
 * Barbeque RTRM.
 */
class ApplicationManager: public ApplicationManagerConfIF {

public:

	/**
	 * @brief Get the ApplicationManager instance
	 */
	static ApplicationManager & GetInstance();

	/**
	 * @brief The destructor
	 */
	virtual ~ApplicationManager();

	/**
	 * @see ApplicationManagerConfIF
	 */
	 AppPtr_t CreateEXC(
			std::string const & name, AppPid_t pid, uint8_t exc_id,
			std::string const & recipe,
			AppPrio_t prio = BBQUE_APP_PRIO_LEVELS-1,
			bool weak_load = false);

	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t DestroyEXC(AppPid_t pid);

	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t DestroyEXC(AppPtr_t papp);

	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t DestroyEXC(AppPid_t pid, uint8_t exc_id);


	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t SetConstraintsEXC(AppPtr_t papp,
			RTLIB_Constraint_t *constraints, uint8_t count);

	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t SetConstraintsEXC(AppPid_t pid, uint8_t exc_id,
			RTLIB_Constraint_t *constraints, uint8_t count);

	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t ClearConstraintsEXC(AppPtr_t papp);

	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t ClearConstraintsEXC(AppPid_t pid, uint8_t exc_id);

	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t SetGoalGapEXC(AppPtr_t papp, uint8_t gap);

	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t SetGoalGapEXC(AppPid_t pid, uint8_t exc_id, uint8_t gap);

	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t EnableEXC(AppPtr_t papp);

	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t EnableEXC(AppPid_t pid, uint8_t exc_id);

	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t DisableEXC(AppPtr_t papp);

	/**
	 * @see ApplicationManagerConfIF
	 */
	ExitCode_t DisableEXC(AppPid_t pid, uint8_t exc_id);

/*******************************************************************************
 *     Thread-Safe Queue Access Functions
 ******************************************************************************/

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppPtr_t GetFirst(AppsUidMapIt & it);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppPtr_t GetNext(AppsUidMapIt & it);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppPtr_t GetFirst(AppPrio_t prio,
			AppsUidMapIt & it);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppPtr_t GetNext(AppPrio_t prio,
			AppsUidMapIt & it);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppPtr_t GetFirst(ApplicationStatusIF::State_t state,
			AppsUidMapIt & it);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppPtr_t GetNext(ApplicationStatusIF::State_t state,
			AppsUidMapIt & it);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppPtr_t GetFirst(ApplicationStatusIF::SyncState_t state,
			AppsUidMapIt & it);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppPtr_t GetNext(ApplicationStatusIF::SyncState_t state,
			AppsUidMapIt & it);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	bool HasApplications (AppPrio_t prio);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	bool HasApplications (ApplicationStatusIF::State_t state);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	bool HasApplications (ApplicationStatusIF::SyncState_t state);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	uint16_t AppsCount (AppPrio_t prio) const;

	/**
	 * @see ApplicationManagerStatusIF
	 */
	uint16_t AppsCount (ApplicationStatusIF::State_t state) const;

	/**
	 * @see ApplicationManagerStatusIF
	 */
	uint16_t AppsCount (ApplicationStatusIF::SyncState_t state) const;

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppPtr_t HighestPrio(ApplicationStatusIF::State_t state);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppPtr_t HighestPrio(ApplicationStatusIF::SyncState_t syncState);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppPtr_t const GetApplication(AppPid_t pid, uint8_t exc_id);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppPtr_t const GetApplication(AppUid_t uid);

	/**
	 * @see ApplicationManagerStatusIF
	 */
	inline app::AppPrio_t LowestPriority() const {
		return BBQUE_APP_PRIO_LEVELS-1;
	};

	/**
	 * @brief Request the synchronization of an application
	 *
	 * @param papp the application to synchronize
	 * @param state the synchronization state required
	 *
	 * @return AM_SUCCESS if the synchronization request has been accepted,
	 * AM_ABORT on synchronization request errors
	 */
	ExitCode_t SyncRequest(AppPtr_t papp, Application::SyncState_t state);

	/**
	 * @brief Commit the synchronization for the specified application
	 *
	 * @param papp the application which has been synchronized
	 *
	 * @return AM_SUCCESS on commit succesfull, AM_ABORT on errors.
	 */
	ExitCode_t SyncCommit(AppPtr_t papp);

	/**
	 * @brief Abort the synchronization for the specified application
	 *
	 * @param papp the application which has been synchronized
	 */
	void SyncAbort(AppPtr_t papp);

	/**
	 * @brief Notify an application is changin state
	 *
	 * This method should be called by the Application once it is changing its
	 * scheduling state so that the ApplicationManager could update its
	 * internal maps.
	 *
	 * @param papp a pointer to the interested application
	 * @param next the new state the application is entering
	 *
	 * @note this method must acquire the mutex of both current and next state
	 * queues.
	 *
	 * @return AM_SUCCESS on internal maps update success, AM_ABORT on
	 * failure.
	 */
	ExitCode_t NotifyNewState(AppPtr_t papp, Application::State_t next);

	/**
	 * @brief Commit the "continue to run" for the specified application
	 *
	 * @param papp a pointer to the interested application
	 * @return AM_SUCCESS on success, AM_ABORT on failure
	 */
	ExitCode_t RunningCommit(AppPtr_t papp);

	/**
	 * @brief Dump a logline to report on current Status queue counts
	 */
	void ReportStatusQ(bool verbose = false) const;

	/**
	 * @brief Dump a logline to report on current Status queue counts
	 */
	void ReportSyncQ(bool verbose = false) const;

	/**
	 * @see ApplicationManagerStatusIF
	 */
	void PrintStatusReport(bool verbose = false);

private:

	/** The logger used by the application manager */
	LoggerIF  *logger;

	/** The recipe loader module used to parse recipes */
	RecipeLoaderIF * rloader;

	/** Lowest application priority value (maximum integer) */
	app::AppPrio_t lowest_prio;

	/** The PlatformProxy, used to setup/release platform specific data */
	PlatformProxy & pp;

	/**
	 * MultiMap of all the applications instances which entered the
	 * resource manager starting from its boot. The map key is the PID of the
	 * application instance. The value is the application descriptor of the
	 * instance.
	 */
	AppsMap_t apps;

	/**
	 * Mutex to protect concurrent access to the map of applications.
	 */
	std::mutex apps_mtx;


	/**
	 * Map of all the applications instances which entered the
	 * resource manager starting from its boot. The map key is the UID of the
	 * application instance. The value is the application descriptor of the
	 * instance.
	 */
	AppsUidMap_t uids;

	/**
	 * Mutex to protect concurrent access to the map of applications UIDs.
	 */
	std::recursive_mutex uids_mtx;

	/**
	 * Array of iterator retainers for "in loop erase" support on UID map
	 */
	AppsUidMapItRetainer_t uids_ret;


	/**
	 * Store all the application recipes. More than one application
	 * instance at once could run. Here we have tow cases :
	 * 	1) Each instance use a different recipes
	 * 	2) A single instance is shared between more than one instance
	 * We assume the possibility of manage both cases.
	 */
	std::map<std::string, RecipePtr_t> recipes;

	/**
	 * A mutex for serializing recipe loading. This prevents unexpected
	 * behaviors if more applications/EXC are loading the same recipe in
	 * parallel.
	 */
	std::mutex recipes_mtx;


	/**
	 * Priority vector of currently scheduled applications (actives).
	 * Vector index expresses the application priority, where "0" labels
	 * "critical" applications. Indices greater than 0 are used for best effort
	 * ones. Each position in the vector points to a set of maps grouping active
	 * applications by priority.
	 */
	AppsUidMap_t prio_vec[BBQUE_APP_PRIO_LEVELS];

	/**
	 * Array of mutexes protecting the priority queues
	 */
	std::mutex prio_mtx[BBQUE_APP_PRIO_LEVELS];

	/**
	 * Array of iterator retainers for "in loop erase" support on priority
	 * queues
	 */
	AppsUidMapItRetainer_t prio_ret[BBQUE_APP_PRIO_LEVELS];


	/**
	 * Array grouping the applications by status (@see ScheduleFlag).
	 * Each position points to a set of maps pointing applications
	 */
	AppsUidMap_t status_vec[Application::STATE_COUNT];

	/**
	 * Array of mutexes protecting the status queues.
	 */
	std::mutex status_mtx[Application::STATE_COUNT];

	/**
	 * Array of iterator retainers for "in loop erase" support on STATUS
	 * queues
	 */
	AppsUidMapItRetainer_t status_ret[Application::STATE_COUNT];


	/**
	 * @brief Applications grouping based on next state to be scheduled.
	 *
	 * Array grouping the applicaitons by the value of theis next_sched.state
	 * (@see ScheduleFlag). Each entry is a vector of applications on the
	 * correposnding scheduled status. This view on applicaitons could be
	 * exploited by the synchronization module to update applications.
	 */
	AppsUidMap_t sync_vec[Application::SYNC_STATE_COUNT];

	/**
	 * Array of mutexes protecting the synchronization queues.
	 */
	std::mutex sync_mtx[Application::SYNC_STATE_COUNT];

	/**
	 * Array of iterator retainers for "in loop erase" support on SYNC
	 * queues
	 */
	AppsUidMapItRetainer_t sync_ret[Application::SYNC_STATE_COUNT];

	/**
	 * @brief EXC cleaner deferrable
	 *
	 * This is used to collect and aggregate EXC cleanup requests.
	 * The cleanup will be performed by a call of the Cleanup
	 * method.
	 */
	Deferrable cleanup_dfr;

	/** The constructor */
	ApplicationManager();

	/** Return a pointer to a loaded recipe */
	RecipeLoaderIF::ExitCode_t LoadRecipe(std::string const & _recipe_name,
			RecipePtr_t & _recipe, bool weak_load = false);

	/**
	 * Remove the specified application from the priority maps
	 */
	ExitCode_t PriorityRemove(AppPtr_t papp);

	/**
	 * Remove the specified application from the status maps
	 */
	ExitCode_t StatusRemove(AppPtr_t papp);
	
	/**
	 * Remove the specified application from the apps maps
	 */
	ExitCode_t AppsRemove(AppPtr_t papp);

	/**
	 * @brief In-Loop-Erase-Safe update of iterators on applications
	 * containers
	 */
	void UpdateIterators(AppsUidMapItRetainer_t & ret, AppPtr_t papp);

	/**
	 * @brief Move the application from state vectors
	 *
	 * @param papp a pointer to an application
	 * @param prev previous application status
	 * @param next next application status
	 */
	ExitCode_t UpdateStatusMaps(AppPtr_t papp,
			Application::State_t prev, Application::State_t next);

	/**
	 * @brief Release a synchronization request for the specified application
	 *
	 * @param papp the application to release
	 * @param state the synchronization state to remove
	 */
	void SyncRemove(AppPtr_t papp, Application::SyncState_t state);

	/**
	 * @brief Release any synchronization request for the specified
	 * application
	 *
	 * @param papp the application to release
	 */
	void SyncRemove(AppPtr_t papp);

	/**
	 * @brief Add a synchronization request for the specified application
	 *
	 * @param papp the application to synchronize
	 * @param state the synchronization state to add
	 */
	void SyncAdd(AppPtr_t papp, Application::SyncState_t state);

	/**
	 * @brief Add the configured synchronization request for the specified
	 * application
	 *
	 * @param papp the application to synchronize
	 */
	void SyncAdd(AppPtr_t papp);


	/**
	 * @brief Clean-up the specified EXC
	 *
	 * Release all resources associated with the specified EXC.
	 */
	ExitCode_t CleanupEXC(AppPtr_t papp);

	/**
	 * @brief Clean-up all disabled EXCs
	 *
	 * Once an EXC is disabled and released, all the time consuming
	 * operations realted to releasing internal data structures (e.g.
	 * platform specific data) are performed by an anynchronous call to
	 * this method.
	 * An exeution of this method, which is managed as a deferrable task,
	 * is triggered by the DestroyEXC, this approach allows to:
	 * 1. keep short the critical path related to respond to an RTLib command
	 * 2. aggregate time consuming operations to be performed
	 * asynchronously
	 */
	void Cleanup();
};

} // namespace bbque

#endif // BBQUE_APPLICATION_MANAGER_H_
