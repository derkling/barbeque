/**
 *       @file  application_manager.cc
 *      @brief  Application Manager implementation
 *
 * This implements the ApplicationManager class.
 *
 *     @author  Giuseppe Massari (jumanix), joe.massanga@gmail.com
 *
 *   @internal
 *     Created  06/04/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Giuseppe Massari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/application_manager.h"

#include "bbque/configuration_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"

#include "bbque/app/application.h"
#include "bbque/app/recipe.h"
#include "bbque/app/constraints.h"
#include "bbque/plugins/recipe_loader.h"
#include "bbque/res/resources.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

namespace ba = bbque::app;
namespace bp = bbque::plugins;
namespace po = boost::program_options;

namespace bbque {

ApplicationManager *ApplicationManager::GetInstance() {
	static ApplicationManager *instance;

	if (instance == NULL)
		instance = new ApplicationManager();
	return instance;
}


ApplicationManager::ApplicationManager():
	Object(APPLICATION_MANAGER_NAMESPACE) {

	// Get a logger
	std::string logger_name(APPLICATION_MANAGER_NAMESPACE);
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger =
		std::unique_ptr<bp::LoggerIF>
		(ModulesFactory::GetLoggerModule(std::cref(conf)));

	//  Get the recipe loader instance
	rloader = ModulesFactory::GetRecipeLoaderModule();
	if (!rloader) {
		logger->Fatal("Missing RecipeLoader plugin");
		assert(rloader);
	}

	// Read the lowest application priority from configuraiton file
	ConfigurationManager & cm = ConfigurationManager::GetInstance();
	po::options_description opts_desc("Application Manager Options");
	opts_desc.add_options()
		("app.lowest_prio",
		 po::value<app::AppPrio_t>(&lowest_prio)->default_value(BBQUE_APP_PRIO_MIN),
		 "Low application priority (the higher the value, the lower the prio")
		;
	po::variables_map opts_vm;
	cm.ParseConfigurationFile(opts_desc, opts_vm);

	// Pre-allocate priority and status vectors
	priority_vec.resize(lowest_prio + 1);
	status_vec.resize(ba::Application::FINISHED);

	// Debug logging
	logger->Debug("Min priority = %d", lowest_prio);
	logger->Debug("Priority vector size = %d", priority_vec.size());
	logger->Debug("Status vector size = %d", status_vec.size());
}


ApplicationManager::~ApplicationManager() {

	// Clear the recipes
	logger->Debug("Clearing recipes...");
	recipes.clear();

	// Clear the priority vector
	logger->Debug("Clearing priority vector...");
	std::vector<AppsMap_t>::iterator it = priority_vec.begin();
	std::vector<AppsMap_t>::iterator endv = priority_vec.end();
	for (; it < endv; ++it)
		it->clear();
	priority_vec.clear();

	// Clear the status vector
	logger->Debug("Clearing states vector...");
	status_vec.clear();

	// Clear the applications map
	apps.clear();
	logger->Debug("Cleared applications");
}

bp::RecipeLoaderIF::ExitCode_t ApplicationManager::LoadRecipe(AppPtr_t _app_ptr,
		std::string const & _recipe_name, RecipePtr_t & _recipe,
		bool _weak_load) {
	bp::RecipeLoaderIF::ExitCode_t result;
	RecipePtr_t recp_ptr;

	logger->Debug("Loading recipe [%s]...", _recipe_name.c_str());

	assert(rloader);
	if (!rloader) {
		logger->Error("Cannot load recipe [%s] (Error: missing recipe loader module)",
				_recipe_name.c_str());
		return bp::RecipeLoaderIF::RL_ABORTED;
	}

	//---  Checking for previously loaded recipe
	std::map<std::string, RecipePtr_t>::iterator it = recipes.find(_recipe_name);
	if (it != recipes.end()) {
		// Return a previously loaded recipe
		logger->Debug("recipe [%s] already loaded", _recipe_name.c_str());
		_recipe = (*it).second;
		return bp::RecipeLoaderIF::RL_SUCCESS;
	}

	//---  Loading a new recipe
	logger->Info("Loading NEW recipe [%s]...", _recipe_name.c_str());

	// Load the required recipe
	recp_ptr = RecipePtr_t(new ba::Recipe(_recipe_name));
	result = rloader->LoadRecipe(_app_ptr, _recipe_name, recp_ptr);

	// If a weak load has done, but the we didn't want it,
	// or a parsing error happened: then return an empty recipe
	if (result == bp::RecipeLoaderIF::RL_WEAK_LOAD && !_weak_load) {
		logger->Error("Load NEW recipe [%s] FAILED "
				"(Error: weak load not accepted)",
				_recipe_name.c_str());
		_recipe = RecipePtr_t();
		return result;

	}
	// On all other case just WEAK_LOAD and SUCCESS are acceptable
	if (result >= bp::RecipeLoaderIF::RL_FAILED ) {
		logger->Error("Load NEW recipe [%s] FAILED "
				"(Error: %d)",
				_recipe_name.c_str(), result);
		_recipe = RecipePtr_t();
		return result;
	}

	logger->Debug("recipe [%s] load DONE", _recipe_name.c_str());

	// Place the new recipe object in the map, and return it
	recipes[_recipe_name] = recp_ptr;
	_recipe = recp_ptr;

	return bp::RecipeLoaderIF::RL_SUCCESS;

}

AppPtr_t ApplicationManager::StartApplication(
		std::string const & _name, AppPid_t _pid, uint8_t _exc_id,
		std::string const & _rcp_name, app::AppPrio_t _prio,
		bool _weak_load) {
	RecipeLoaderIF::ExitCode_t result;
	RecipePtr_t rcp_ptr;
	AppPtr_t app_ptr;

	logger->Info("Starting NEW application [%s]...", _name.c_str());

	// Create a new descriptor
	app_ptr = AppPtr_t(new ba::Application(_name, _pid, _exc_id));
	app_ptr->SetPriority(_prio);

	// Load the required recipe
	result = LoadRecipe(app_ptr, _rcp_name, rcp_ptr, _weak_load);
	if (!rcp_ptr) {
		logger->Error("Start application [%s] FAILED (Error: unable to load recipe [%s])",
				_name.c_str(), _rcp_name.c_str());
		return AppPtr_t();
	}
	app_ptr->SetRecipe(rcp_ptr);

	// Save the application descriptor
	apps.insert(AppsMapEntry_t(app_ptr->Pid(), app_ptr));
	(priority_vec[app_ptr->Priority()]).
		insert(AppsMapEntry_t(app_ptr->Pid(), app_ptr));
	(status_vec[static_cast<uint8_t>(app_ptr->CurrentState())]).
		insert(AppsMapEntry_t(app_ptr->Pid(), app_ptr));

	logger->Debug("application [%s] started DONE", _name.c_str());

	return app_ptr;
}

void ApplicationManager::StopApplication(AppPid_t pid) {

	// Get all the execution context of the application
	std::pair<AppsMap_t::iterator, AppsMap_t::iterator> range =
		apps.equal_range(pid);
	if (range.first == range.second)
		return;

	// Stop executions and update scheduling information
	AppsMap_t::iterator it = range.first;
	for ( ; it!=range.second; it++) {
		((*it).second)->StopExecution();
		ChangedSchedule((*it).second, 0);

		// Remove application (ECs) descriptors from global and priority map
		priority_vec[((*it).second)->Priority()].erase(pid);
	}

	apps.erase(pid);
}


void ApplicationManager::StopApplication(AppPid_t pid, uint8_t exc_id) {

	// Execution context pointer
	AppPtr_t _pexc;

	// Get all the execution context of the application
	std::pair<AppsMap_t::iterator, AppsMap_t::iterator> range =
		apps.equal_range(pid);
	if (range.first == range.second)
		return;

	// Find the execution context "exc_id", stop it and update scheduling
	// information
	AppsMap_t::iterator it = range.first;
	for ( ; it!=range.second; ++it)

		if (((*it).second)->ExcId() == exc_id) {
			((*it).second)->StopExecution();
			ChangedSchedule((*it).second, 0);

			// Remove execution context descriptor from global map
			_pexc = AppPtr_t((*it).second);
			apps.erase(it);
			break;
		}

	if (!_pexc)
		return;

	// Get all the execution context from the priority of the application
	std::pair<AppsMap_t::iterator, AppsMap_t::iterator> prio_range =
		priority_vec[_pexc->Priority()].equal_range(pid);
	if (prio_range.first == prio_range.second)
		return;

	// Remove application (EC) descriptor from priority map
	AppsMap_t::iterator prio_it = prio_range.first;
	for (; prio_it != prio_range.second; ++prio_it)
		if (((*prio_it).second)->ExcId() == exc_id) {
			priority_vec[_pexc->Priority()].erase(prio_it);
		}
}

void ApplicationManager::ChangedSchedule(AppPtr_t _papp, double _time) {

	// Check existance of application instance
	assert(_papp);

	// If the scheduled state changes we need to update the application
	// descriptor (pointer) position, moving it in the right map
	if (_papp->CurrentState() != _papp->NextState()) {

		// Retrieve the runtime map from the status vector
		AppsMap_t * curr_state_map = &(status_vec[_papp->CurrentState()]);
		AppsMap_t * next_state_map = &(status_vec[_papp->NextState()]);

		assert(*curr_state_map != *next_state_map);

		// Find the application descriptor the current status map
		std::pair<AppsMap_t::iterator, AppsMap_t::iterator> range =
			(*curr_state_map).equal_range(_papp->Pid());
		if (range.first == range.second) {
			logger->Error("Cannot find %s in the expected status",
			              _papp->Name().c_str());
			return;
		}

		logger->Debug("Looking for Application [Name: %s, Pid: %d, ExcId: %d]...",
				_papp->Name().c_str(),
				_papp->Pid(),
				_papp->ExcId());

		AppsMap_t::iterator it = range.first;
		for ( ; it!=range.second; it++) {
			logger->Debug("Found Application [Name: %s, Pid: %d, ExcId: %d]",
					((*it).second)->Name().c_str(),
					((*it).second)->Pid(),
					((*it).second)->ExcId());
			if (_papp->ExcId() == ((*it).second)->ExcId())
				break;
		}
		// The required application should be found, otherwise a data
		// structure corruption has occurred
		assert(it!=range.second);

		logger->Debug("Changed Application [Name: %s, Pid: %d, ExcId: %d] "
				"status to [%d]",
				((*it).second)->Name().c_str(),
				((*it).second)->Pid(),
				((*it).second)->ExcId(),
				((*it).second)->NextState());

		(*curr_state_map).erase(it);

		// Move it from the current to the next status map
		(*next_state_map).insert(AppsMapEntry_t(_papp->Pid(), _papp));

	}
	// The application descriptor now will manage the change of working mode
	_papp->SwitchToNextScheduled(_time);

}


AppPtr_t const ApplicationManager::GetApplication(AppPid_t _pid,
		uint8_t _exc_id) {
	AppPtr_t app_ptr;

	// Find all the Execution Contexts of the specified application PID
	std::pair<AppsMap_t::iterator, AppsMap_t::iterator> range =
		apps.equal_range(_pid);
	if (range.first == range.second) {
		logger->Debug("Application [Pid: %d] NOT FOUND", _pid);
		return app_ptr;
	}

	// Look for the required Execution Context
	AppsMap_t::iterator it = range.first;
	for ( ; it!=range.second; it++) {
		if (_exc_id == ((*it).second)->ExcId())
			break;
	}
	if (it == range.second) {
		logger->Debug("Execution Context [Pid: %d, ExcId: %d] NOT FOUND",
				_pid, _exc_id);
		return app_ptr;
	}

	logger->Debug("Application Excution Context "
			"[Name: %s, Pid: %d, ExcId: %d] FOUND",
			((*it).second)->Name().c_str(),
			((*it).second)->Pid(),
			((*it).second)->ExcId());

	app_ptr = ((*it).second);

	return app_ptr;
}

}   // namespace bbque

