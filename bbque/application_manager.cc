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
#include "bbque/res/resources.h"


namespace bbque {

ApplicationManager *ApplicationManager::GetInstance() {
	static ApplicationManager *instance;

	if (instance == NULL)
		instance = new ApplicationManager();
	return instance;
}


ApplicationManager::ApplicationManager():
	Object("bq.appman") {

	// Get a logger
	std::string logger_name("bq.appman");
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));

	// Set the lowest application priority
	ConfigurationManager & cm = ConfigurationManager::GetInstance();
	uint16_t lowest_priority = cm.GetOptions()["application.lowest_prio"].as<uint16_t>();

	// Pre-allocate priority and status vectors
	priority_vec = std::vector<AppsMap_t>(lowest_priority + 1);
	status_vec = std::vector<AppsMap_t>(app::Application::FINISHED);

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


AppsMap_t const & ApplicationManager::Applications(uint16_t _prio) {

	// Check priority value bounds
	if (_prio <= lowest_priority)
		return priority_vec[_prio];

	// Return an empty map if priority is out of range
	return empty_map;
}


AppsMap_t const & ApplicationManager::Applications(
		app::Application::ScheduleFlag_t sched_state) {

	// Return a map of application descriptor (pointers) with
	// the same priority
	if (sched_state >= app::Application::READY   &&
	        sched_state <= app::Application::FINISHED) {
		return status_vec[sched_state];
	}
	// Return an empty map if the state value is not valid
	return empty_map;
}


plugins::RecipeLoaderIF::ExitCode_t ApplicationManager::StartApplication(
    std::string const & _name, std::string const & _user, uint16_t _prio,
    uint32_t _pid, std::string const & _rname, bool weak_load = false) {

	// A shared pointer the application object descriptor
	AppPtr_t app_ptr;
	RecipePtr_t recp_ptr;

	// The method return value
	plugins::RecipeLoaderIF::ExitCode_t retcode =
	    plugins::RecipeLoaderIF::RL_SUCCESS;

	// Create a new descriptor
	app_ptr = AppPtr_t(new app::Application(_name, _user, _pid));
	app_ptr->SetPriority(_prio);

	// The recipe has been loaded in the past ?
	std::map<std::string, RecipePtr_t>::iterator it_recp =
		recipes.find(_rname);

	if (it_recp != recipes.end()) {
		// Recipe yet loaded in the past. Get it.
		recp_ptr = it_recp->second;
	}
	else {
		//  Recipe never loaded before...
		//  Get the recipe loader instance
		std::string rloader_name("rloader.xml");
		plugins::RecipeLoaderIF * rloader =
			ModulesFactory::GetRecipeLoaderModule(rloader_name);

		if (!rloader) {
			// Cannot load recipes without the plugin!
			logger->Error("Missing RecipeLoader plugin");
			return plugins::RecipeLoaderIF::RL_ABORTED;
		}
		// Load the "recipe" of the application
		recp_ptr = RecipePtr_t(new app::Recipe(_rname));
		retcode = rloader->LoadRecipe(app_ptr, _rname, recp_ptr);

		// If a weak load has done, but the we didn't want it
		// abort the application descriptor creation
		if ((retcode == plugins::RecipeLoaderIF::RL_WEAK_LOAD && !weak_load)
				|| (retcode == plugins::RecipeLoaderIF::RL_FORMAT_ERROR)) {
			return retcode;
		}
		// Place the new recipe object in the map
		recipes[_rname] = recp_ptr;

	// Set the recipe of the application
	app_ptr->SetRecipe(recp_ptr);
	// Application descriptors map
	apps[app_ptr->Pid()] = app_ptr;
	// Priority map
	priority_vec[app_ptr->Priority()][app_ptr->Pid()] = app_ptr;
	// Status map
	status_vec[static_cast<uint8_t>(app_ptr->CurrentState())][app_ptr->Pid()] =
	    app_ptr;

	return retcode;
}


void ApplicationManager::ChangedSchedule(uint32_t _pid, double _time) {

	// Check existance of application instance with PID = "_pid"
	AppsMap_t::iterator pid_it = apps.find(_pid);
	if (pid_it == apps.end())
		return;

	// Get the pointer to the application descriptor
	AppPtr_t _app = AppPtr_t(pid_it->second);

	// If the scheduled state changes we need to update the application
	// descriptor (pointer) position, moving it in the right map
	if (_app->CurrentState() != _app->NextState()) {

		// Retrieve the runtime map from the status vector
		AppsMap_t curr_state_map = Applications(_app->CurrentState());
		AppsMap_t next_state_map = Applications(_app->NextState());

		// Find the application descriptor the current status map
		AppsMap_t::iterator it = curr_state_map.find(_app->Pid());
		if (it == curr_state_map.end()) {
			logger->Error("Cannot find %s in the expected status",
			              _app->Name().c_str());
			return;
		}
		// Move it from the current to the next status map
		next_state_map[_app->Pid()] = it->second;
		curr_state_map.erase(it);
	}
	// The application descriptor now will manage the change of working mode
	_app->SwitchToNextScheduled(_time);
}


AppPtr_t const ApplicationManager::GetApplication(uint32_t _pid) {
	// Null shared pointer
	AppPtr_t app_ptr;
	app_ptr.reset();

	// Find the application by name
	AppsMap_t::iterator app_it = apps.find(_pid);
	if (app_it != apps.end())
		app_ptr = AppPtr(app_it->second);

	return app_ptr;
}

}   // namespace bbque

