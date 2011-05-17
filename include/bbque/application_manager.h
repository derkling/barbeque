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

#include "object.h"
#include "bbque/application_manager_conf.h"
#include "bbque/app/application.h"

#define APPLICATION_MANAGER_NAMESPACE "bq.am"

#ifndef BBQUE_APP_PRIO_MIN
/**
  * @brief The (default) minimum priority for an Application
  *
  * Applicaitons are scheduled according to their priority which is a
  * value ranging from 0 (highest priority) to a Bbque defined minimum values.
  * The default minimum value is defined by BBQUE_APP_PRIO_MIN, but it can be
  * tuned by a Barbeque configuration parameter.
  *
  * Recipies could define the priority of the corresponding Application.
  *
  */
# define BBQUE_APP_PRIO_MIN 10
#endif

using bbque::app::Application;
using bbque::app::Recipe;
using bbque::plugins::RecipeLoaderIF;

namespace bbque {

/** Shared pointer to Recipe object */
typedef std::shared_ptr<Recipe> RecipePtr_t;


/**
* @class ApplicationManager
* @brief The class provides interfaces for managing the applications lifecycle.
*/
class ApplicationManager: public ApplicationManagerConfIF, public Object {

public:

	/**
	 * @brief Get the ApplicationManager instance
	 */
	static ApplicationManager *GetInstance();

	/**
	 * @brief The destructor
	 */
	virtual ~ApplicationManager();

	/**
	 * @brief The entry point for the applications requiring Barbeque.
	 * @param name Application name
	 * @param pid PID of the application (assigned from the OS)
	 * @param The ID of the Execution Context (assigned from the application)
	 * @param recipe The name of the recipe to use for this application
	 * @param prio Application priority
	 * @param weak_load If true a weak load of the recipe is accepted.
	 * It is when some resource requests doesn't match perfectly.
	 *
	 * @return A pointer to the newly allocated application, NULL otherwise.
	 */
	 AppPtr_t StartApplication(
			std::string const & name, AppPid_t pid, uint8_t exc_id,
			std::string const & recipe, app::AppPrio_t prio = BBQUE_APP_PRIO_MIN,
			bool weak_load = false);

	/**
	 * @see ApplicationManagerConfIF
	 */
	void StopApplication(AppPid_t pid);

	/**
	 * @see ApplicationManagerConfIF
	 */
	void StopApplication(AppPid_t pid, uint8_t exc_id);

	/**
	 * @see ApplicationManagerConfIF
	 */
	inline AppsMap_t const * Applications() const {
		return &apps;
	}

	/**
	 * @brief Retrieve registered applications with the specified priority
	 *
	 * @param prio The priority value
	 * @return A pointer to the map of applications with the request priority
	 */
	AppsMap_t const * Applications(app::AppPrio_t prio) const {
		assert(prio<=lowest_prio);
		if (prio>lowest_prio)
			prio=lowest_prio;
		return &(priority_vec[prio]);
	}

	/**
	 * @brief Retrieve reigstered applications with the specified scheduling state
	 *
	 * @param sched_state The scheduling state
	 * @return A pointer to the map of applications with the requested schedule status
	 */
	AppsMap_t const * Applications (
			Application::ScheduleFlag_t sched_state) const {
		return &(status_vec[sched_state]);
	}

	/**
	 * @brief Retrieve an application descriptor (shared pointer) by PID and
	 * Excution Context
	 * @param pid Application PID
	 * @param exc_id Execution Contetx ID
	 */
	AppPtr_t const GetApplication(AppPid_t pid, uint8_t exc_id = 0);

	/**
	 * @brief Return the maximum integer value for the minimum application
	 * priority
	 */
	inline app::AppPrio_t LowestPriority() const {
		return lowest_prio;
	};

	/**
	 * @brief The method is called to notify an application scheduling change
	 *
	 * The ApplicationManager is notified of an application change of
	 * scheduled status from the reconfiguration phase. Thus the manager can
	 * move the application descriptor in the proper state map (if the
	 * schedule state has changed), and then update the application runtime
	 * info.
	 *
	 * @param papp The Application which scheduling has chenged
	 * @param time Working mode switch time measured
	 */
	void ChangedSchedule(AppPtr_t papp, double time);

private:

	/** The recipe loader module used to parse recipes */
	RecipeLoaderIF * rloader;

	/** Lowest application priority value (maximum integer) */
	app::AppPrio_t lowest_prio;

	/**
	 * List of all the applications instances which entered the
	 * resource manager starting from its boot. The map key is the PID of the
	 * application instance. The value is the application descriptor of the
	 * instance.
	 */
	AppsMap_t apps;

	/**
	 * Store all the application recipes. More than one application
	 * instance at once could run. Here we have tow cases :
	 * 	1) Each instance use a different recipes
	 * 	2) A single instance is shared between more than one instance
	 * We assume the possibility of manage both cases.
	 */
	std::map<std::string, RecipePtr_t> recipes;

	/**
	 * A (generic) vector of application maps.
	 * These vectors are used to classify applications, e.g. based on their
	 * priority or current status.
	 */
	typedef std::vector<AppsMap_t> AppsMapVec_t;

	/**
	 * Priority vector of currently scheduled applications (actives).
	 * Vector index expresses the application priority, where "0" labels
	 * "critical" applications. Indices greater than 0 are used for best effort
	 * ones. Each position in the vector points to a set of maps grouping active
	 * applications by priority.
	 */
	AppsMapVec_t priority_vec;

	/**
	 * Vector grouping the applications by status (@see ScheduleFlag).
	 * Each position points to a set of maps pointing applications
	 */
	AppsMapVec_t status_vec;

	/** The constructor */
	ApplicationManager();

	/** Return a pointer to a loaded recipe */
	// FIXME this method should be application independent
	// REFACTOR NEDDED:
	// - save static constraint within the recipe
	// - add a method to get static constraints from a recipe object
	RecipeLoaderIF::ExitCode_t LoadRecipe(AppPtr_t _app_ptr,
			std::string const & _recipe_name,
			RecipePtr_t & _recipe,
			bool weak_load = false);

};

} // namespace bbque

#endif // BBQUE_APPLICATION_MANAGER_H_

