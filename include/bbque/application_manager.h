/**
 *       @file  application_manager.h
 *      @brief  Application manager component
 *
 * This provides the interface for managing applications registration and keep
 * track of their schedule status changes. The class provides calls to
 * register applications, retrieving application descriptors, the maps of
 * application descriptors given their scheduling status or priority level.
 * Moreover to signal the scheduling change of status of an application, and
 * to know which is lowest priority level (maximum integer value) managed by
 * Barbeque RTRM.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  04/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_APPLICATION_MANAGER_H_
#define BBQUE_APPLICATION_MANAGER_H_

#include <map>
#include <vector>
#include <mutex>

#include "bbque/application_manager_conf.h"
#include "bbque/app/application.h"
#include "bbque/plugins/logger.h"

#define APPLICATION_MANAGER_NAMESPACE "bq.am"

#ifndef BBQUE_APP_PRIO_LEVELS
/**
  * @brief The (default) minimum priority for an Application
  *
  * Applicaitons are scheduled according to their priority which is a
  * value ranging from 0 (highest priority) to a Bbque defined minimum values.
  * The default minimum value is defined by BBQUE_APP_PRIO_LEVELS, but it can be
  * tuned by a Barbeque configuration parameter.
  *
  * Recipies could define the priority of the corresponding Application.
  *
  */
# define BBQUE_APP_PRIO_LEVELS 10
#endif

using bbque::app::Application;
using bbque::app::Recipe;
using bbque::plugins::LoggerIF;
using bbque::plugins::RecipeLoaderIF;

namespace bbque {

/** Shared pointer to Recipe object */
typedef std::shared_ptr<Recipe> RecipePtr_t;


/**
* @class ApplicationManager
* @brief The class provides interfaces for managing the applications lifecycle.
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

	/**
	 * @see ApplicationManagerConfIF
	 */
	inline AppsUidMap_t const * Applications() const {
		return &uids;
	}

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppsUidMap_t const * Applications(app::AppPrio_t prio) const {
		assert(prio<=lowest_prio);
		if (prio>lowest_prio)
			prio=lowest_prio;
		return &(priority_vec[prio]);
	}

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppsUidMap_t const * Applications(Application::State_t state) const {
		return &(status_vec[state]);
	}

	/**
	 * @see ApplicationManagerStatusIF
	 */
	AppsUidMap_t const * Applications(
			Application::SyncState_t sync_state) const {
		return &(sync_vec[sync_state]);
	}

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
	 * @brief papp the application which has been synchronized
	 *
	 * @return AM_SUCCESS on commit succesfull, AM_ABORT on errors.
	 */
	ExitCode_t SyncCommit(AppPtr_t papp);

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
	 * @breif Dump a logline to report on current Status queue counts
	 */
	void ReportStatusQ() const;

	/**
	 * @breif Dump a logline to report on current Status queue counts
	 */
	void ReportSyncQ() const;

	/**
	 * @see ApplicationManagerStatusIF
	 */
	void PrintStatusReport();

private:

	/** The logger used by the application manager */
	LoggerIF  *logger;

	/** The recipe loader module used to parse recipes */
	RecipeLoaderIF * rloader;

	/** Lowest application priority value (maximum integer) */
	app::AppPrio_t lowest_prio;


	/**
	 * MultiMap of all the applications instances which entered the
	 * resource manager starting from its boot. The map key is the PID of the
	 * application instance. The value is the application descriptor of the
	 * instance.
	 */
	AppsMap_t apps;

	/**
	 * Map of all the applications instances which entered the
	 * resource manager starting from its boot. The map key is the UID of the
	 * application instance. The value is the application descriptor of the
	 * instance.
	 */
	AppsUidMap_t uids;

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
	 * Array grouping the applications by status (@see ScheduleFlag).
	 * Each position points to a set of maps pointing applications
	 */
	AppsUidMap_t status_vec[Application::STATE_COUNT];

	/**
	 * Array of mutexes protecting the status queues.
	 */
	std::mutex status_mtx[Application::STATE_COUNT];

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
};

} // namespace bbque

#endif // BBQUE_APPLICATION_MANAGER_H_

