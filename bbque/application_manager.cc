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


	// Set the lowest application priority
	ConfigurationManager & cm = ConfigurationManager::GetInstance();
	uint16_t lowest_priority =
		cm.GetOptions()["application.lowest_prio"].as<uint16_t>();

	// Pre-allocate priority and status vectors
	priority_vec = std::vector<AppsMap_t>(lowest_priority + 1);
	status_vec = std::vector<AppsMap_t>(ba::Application::FINISHED);

	// Debug logging
	logger->Debug("Min priority = %d", lowest_priority);
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

RecipePtr_t ApplicationManager::LoadRecipe(AppPtr_t _app_ptr,
		std::string const & _rname, bool _weak_load) {
	bp::RecipeLoaderIF::ExitCode_t retcode;
	RecipePtr_t recp_ptr;

	//---  Checking for previously loaded recipe
	std::map<std::string, RecipePtr_t>::iterator it = recipes.find(_rname);
	if (it != recipes.end()) {
		// Return a previously loaded recipe
		return (*it).second;
	}

	//---  Loading a new recipe

	//  Get the recipe loader instance
	std::string rloader_name(RECIPE_LOADER_NAMESPACE);
	bp::RecipeLoaderIF * rloader =
		ModulesFactory::GetRecipeLoaderModule(rloader_name);
	if (!rloader) {
		// Cannot load recipes without the plugin!
		logger->Fatal("Missing RecipeLoader plugin");
		assert(rloader);
	}

	// Load the required recipe
	recp_ptr = RecipePtr_t(new ba::Recipe(_rname));
	retcode = rloader->LoadRecipe(_app_ptr, _rname, recp_ptr);

	// If a weak load has done, but the we didn't want it,
	// or a parsing error happened: then return an empty recipe
	if ((retcode == bp::RecipeLoaderIF::RL_WEAK_LOAD && !_weak_load)
			|| (retcode == bp::RecipeLoaderIF::RL_FORMAT_ERROR)) {
		return RecipePtr_t();
	}

	// Place the new recipe object in the map, and return it
	recipes[_rname] = recp_ptr;
	return recp_ptr;

}

bp::RecipeLoaderIF::ExitCode_t ApplicationManager::StartApplication(
    std::string const & _name, std::string const & _user, uint16_t _prio,
	uint32_t _pid, uint32_t _exc_id, std::string const & _rname,
	bool _weak_load) {

	// A shared pointer the application object descriptor
	AppPtr_t app_ptr;
	RecipePtr_t recp_ptr;

	// Create a new descriptor
	app_ptr = AppPtr_t(new ba::Application(_name, _user, _pid, _exc_id));
	app_ptr->SetPriority(_prio);

	// Load the required recipe
	recp_ptr = LoadRecipe(app_ptr, _rname, _weak_load);
	if (!recp_ptr) {
		logger->Error("FAILED loading recipe [%s]", _rname.c_str());
		return bp::RecipeLoaderIF::RL_FAILED;
	}
	app_ptr->SetRecipe(recp_ptr);

	// Save the application descriptor
	apps.insert(AppsMapEntry_t(app_ptr->Pid(), app_ptr));
	(priority_vec[app_ptr->Priority()]).
		insert(AppsMapEntry_t(app_ptr->Pid(), app_ptr));
	(status_vec[static_cast<uint8_t>(app_ptr->CurrentState())]).
		insert(AppsMapEntry_t(app_ptr->Pid(), app_ptr));

	return bp::RecipeLoaderIF::RL_SUCCESS;
}


void ApplicationManager::ChangedSchedule(AppPtr_t _papp, double _time) {

	// Check existance of application instance
	assert(_papp);

	// If the scheduled state changes we need to update the application
	// descriptor (pointer) position, moving it in the right map
	if (_papp->CurrentState() != _papp->NextState()) {

		// Retrieve the runtime map from the status vector
		AppsMap_t curr_state_map = Applications(_papp->CurrentState());
		AppsMap_t next_state_map = Applications(_papp->NextState());

		// Find the application descriptor the current status map
		std::pair<AppsMap_t::iterator, AppsMap_t::iterator> range =
			curr_state_map.equal_range(_papp->Pid());
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

		curr_state_map.erase(it);

		// Move it from the current to the next status map
		next_state_map.insert(AppsMapEntry_t(_papp->Pid(), _papp));

	}
	// The application descriptor now will manage the change of working mode
	_papp->SwitchToNextScheduled(_time);

}


AppPtr_t const ApplicationManager::GetApplication(uint32_t _pid,
		uint32_t _exc_id) {
	AppPtr_t app_ptr;

	logger->Debug("Looking for Application [Pid: %d, ExcId: %d]...",
			_pid, _exc_id);

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

