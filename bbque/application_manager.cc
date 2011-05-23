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
	logger = std::unique_ptr<bp::LoggerIF>
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
		 po::value<app::AppPrio_t>
		 (&lowest_prio)->default_value(BBQUE_APP_PRIO_MIN),
		 "Low application priority (the higher the value, "
		 "the lower the prio")
		;
	po::variables_map opts_vm;
	cm.ParseConfigurationFile(opts_desc, opts_vm);

	// Pre-allocate priority and status vectors
	priority_vec = std::vector<AppsMap_t>(lowest_prio + 1);
	status_vec = std::vector<AppsMap_t>(ba::Application::FINISHED);

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
	priority_vec.clear();

	// Clear the status vector
	logger->Debug("Clearing states vector...");
	status_vec.clear();

	// Clear the applications map
	apps.clear();
	logger->Debug("Cleared applications");
}

bp::RecipeLoaderIF::ExitCode_t ApplicationManager::LoadRecipe(AppPtr_t papp,
		std::string const & recipe_name, RecipePtr_t & recipe,
		bool weak_load) {
	bp::RecipeLoaderIF::ExitCode_t result;

	logger->Debug("Loading recipe [%s]...", recipe_name.c_str());

	assert(rloader);
	if (!rloader) {
		logger->Error("Cannot load recipe [%s] "
				"(Error: missing recipe loader module)",
				recipe_name.c_str());
		return bp::RecipeLoaderIF::RL_ABORTED;
	}

	//---  Checking for previously loaded recipe
	std::map<std::string, RecipePtr_t>::iterator it(
			recipes.find(recipe_name));
	if (it != recipes.end()) {
		// Return a previously loaded recipe
		logger->Debug("recipe [%s] already loaded",
				recipe_name.c_str());
		recipe = (*it).second;
		return bp::RecipeLoaderIF::RL_SUCCESS;
	}

	//---  Loading a new recipe
	logger->Info("Loading NEW recipe [%s]...", recipe_name.c_str());

	// Load the required recipe
	// FIXME the RecipeLoader should be a pure client of this class.
	// See ticket #2
	recipe = RecipePtr_t(new ba::Recipe(recipe_name));
	result = rloader->LoadRecipe(papp, recipe_name, recipe);

	// If a weak load has done, but the we didn't want it,
	// or a parsing error happened: then return an empty recipe
	if (result == bp::RecipeLoaderIF::RL_WEAK_LOAD && !weak_load) {
		logger->Error("Load NEW recipe [%s] FAILED "
				"(Error: weak load not accepted)",
				recipe_name.c_str());
		recipe = RecipePtr_t();
		return result;

	}
	// On all other case just WEAK_LOAD and SUCCESS are acceptable
	if (result >= bp::RecipeLoaderIF::RL_FAILED ) {
		logger->Error("Load NEW recipe [%s] FAILED "
				"(Error: %d)",
				recipe_name.c_str(), result);
		recipe = RecipePtr_t();
		return result;
	}

	logger->Debug("recipe [%s] load DONE", recipe_name.c_str());

	// Place the new recipe object in the map, and return it
	recipes[recipe_name] = recipe;

	return bp::RecipeLoaderIF::RL_SUCCESS;

}

AppPtr_t ApplicationManager::StartApplication(
		std::string const & _name, AppPid_t _pid, uint8_t _exc_id,
		std::string const & _rcp_name, app::AppPrio_t _prio,
		bool _weak_load) {
	RecipeLoaderIF::ExitCode_t result;
	RecipePtr_t rcp_ptr;
	AppPtr_t papp;

	// Create a new descriptor
	papp = AppPtr_t(new ba::Application(_name, _pid, _exc_id));
	papp->SetPriority(_prio);

	logger->Info("Starting new EXC [%s], prio[%d]",
			papp->StrId(), papp->Priority());

	// Load the required recipe
	result = LoadRecipe(papp, _rcp_name, rcp_ptr, _weak_load);
	if (!rcp_ptr) {
		logger->Error("Starting EXC [%s] FAILED "
				"(Error: unable to load recipe [%s])",
				papp->StrId(), _rcp_name.c_str());
		return AppPtr_t();
	}
	papp->SetRecipe(rcp_ptr);

	// Save application descriptors
	apps.insert(AppsMapEntry_t(papp->Pid(), papp));
	priority_vec[papp->Priority()].insert(
			AppsMapEntry_t(papp->Pid(), papp));
	status_vec[papp->CurrentState()].insert(
			AppsMapEntry_t(papp->Pid(), papp));

	logger->Debug("EXC [%s] start DONE",
			papp->StrId(), papp->ExcId());

	return papp;
}


ApplicationManager::ExitCode_t
ApplicationManager::StopApplication(AppPid_t pid) {
	std::pair<AppsMap_t::iterator, AppsMap_t::iterator> range;
	AppsMapVec_t::iterator vit;
	AppsMapVec_t::iterator end;
	AppsMap_t::iterator ait;

	logger->Debug("Stopping all EXCs for application [%d] ...", pid);
	range = apps.equal_range(pid);
	ait = range.first;
	for ( ; ait!=range.second; ++ait) {
		((*ait).second)->StopExecution();
	 }

	// Remove application (EXCs) descriptors from status map
	logger->Debug("Releasing [%d] EXCs from status maps...", pid);
	vit  = status_vec.begin();
	end = status_vec.end();
	for ( ; vit!=end; ++vit) {
		(*vit).erase(pid);
	}

	// Remove application (EXCs) descriptors from priority map
	logger->Debug("Releasing [%d] EXCs from priority maps...", pid);
	vit  = priority_vec.begin();
	end = priority_vec.end();
	for ( ; vit!=end; ++vit) {
		(*vit).erase(pid);
	}

	// Remove application (EXCs) descriptors from applications map
	logger->Debug("Releasing [%d] EXCs from applications maps...", pid);
	apps.erase(pid);

	return AM_SUCCESS;
}

ApplicationManager::ExitCode_t
ApplicationManager::PriorityRemove(AppPtr_t papp) {
	std::pair<AppsMap_t::iterator, AppsMap_t::iterator> range;
	AppsMap_t::iterator it;

	// Remove execution context descriptor from priority map
	logger->Debug("Releasing [%s] EXCs from priority maps...",
			papp->StrId());
	range = priority_vec[papp->Priority()].equal_range(papp->Pid());
	it = range.first;
	while (it != range.second &&
		((*it).second)->ExcId() != papp->ExcId()) {
		++it;
	}
	assert(it != range.second);
	if (it == range.second) {
		logger->Crit("EXCs [%s] not found in priority maps "
				"(Error: possible data structure corruption?)",
			papp->StrId());
		return AM_DATA_CORRUPT;
	}
	priority_vec[papp->Priority()].erase(it);

	return AM_SUCCESS;
}

ApplicationManager::ExitCode_t
ApplicationManager::StatusRemove(AppPtr_t papp) {
	std::pair<AppsMap_t::iterator, AppsMap_t::iterator> range;
	AppsMap_t::iterator it;

	logger->Debug("Releasing [%s] EXCs from status maps...",
			papp->StrId());
	range = status_vec[papp->CurrentState()].equal_range(papp->Pid());
	it = range.first;
	while (it != range.second &&
		((*it).second)->ExcId() != papp->ExcId()) {
		++it;
	}
	assert(it != range.second);
	if (it == range.second) {
		logger->Crit("EXCs [%s] not found in status maps "
				"(Error: possible data structure corruption)",
			papp->StrId());
		return AM_DATA_CORRUPT;
	}
	status_vec[papp->CurrentState()].erase(it);

	return AM_SUCCESS;
}

ApplicationManager::ExitCode_t
ApplicationManager::StopApplication(AppPtr_t papp) {
	std::pair<AppsMap_t::iterator, AppsMap_t::iterator> range;
	AppsMap_t::iterator it;
	ExitCode_t result;

	logger->Debug("Removing EXC [%s] ...", papp->StrId());

	// Remove execution context form priority and status maps
	result = PriorityRemove(papp);
	if (result != AM_SUCCESS)
		return result;

	result = StatusRemove(papp);
	if (result != AM_SUCCESS)
		return result;

	range = apps.equal_range(papp->Pid());
	it = range.first;
	while (it != range.second &&
		((*it).second)->ExcId() != papp->ExcId()) {
		++it;
	}
	assert(it != range.second);
	if (it == range.second) {
		logger->Crit("EXCs [%s] not found in application map "
				"(Error: possible data structure corruption)",
			papp->StrId());
		return AM_DATA_CORRUPT;
	}
	apps.erase(it);

	return AM_SUCCESS;
}

ApplicationManager::ExitCode_t
ApplicationManager::StopApplication(AppPid_t pid, uint8_t exc_id) {
	AppPtr_t papp;

	// Find the required EXC
	papp = GetApplication(pid, exc_id);
	assert(papp);
	if (!papp) {
		logger->Warn("Stop EXC [%d:*:%d] FAILED "
				"(Error: EXC not found)");
		return AM_EXC_NOT_FOUND;
	}

	return StopApplication(papp);
}

void ApplicationManager::EnableApplication(AppPtr_t papp) {

	// Enabling the execution context
	logger->Debug("Enabling EXC [%s] ...", papp->StrId());
	papp->Enable();

	// Update internal maps
	ChangedSchedule(papp);

}

void ApplicationManager::EnableApplication(AppPid_t pid, uint8_t exc_id) {
	AppPtr_t papp;

	// Find the required EXC
	papp = GetApplication(pid, exc_id);
	assert(papp);
	if (!papp) {
		logger->Warn("Enable EXC [%d:*:%d] FAILED "
				"(Error: EXC not found)");
		return;
	}

	EnableApplication(papp);
}

void ApplicationManager::DisableApplication(AppPtr_t papp) {

	// Disable the execution context
	logger->Debug("Disabling EXC [%s] ...", papp->StrId());
	papp->Disable();

	// Update status map
	ChangedSchedule(papp);
}

void ApplicationManager::DisableApplication(AppPid_t pid, uint8_t exc_id) {
	AppPtr_t papp;

	// Find the required EXC
	papp = GetApplication(pid, exc_id);
	assert(papp);
	if (!papp) {
		logger->Warn("Disable EXC [%d:*:%d] FAILED "
				"(Error: EXC not found)");
		return;
	}

	DisableApplication(papp);
}

void ApplicationManager::ChangedSchedule(AppPtr_t papp, double time) {
	std::pair<AppsMap_t::iterator, AppsMap_t::iterator> range;
	AppsMap_t::iterator it;
	assert(papp);

	logger->Info("Changed EXC [%s] schedule [%d ==> %d]", papp->StrId(),
			papp->CurrentState(), papp->NextState());

	// We need to update the application descriptor (moving it into the
	// right map) just if the scheduled state has changed.
	if (papp->CurrentState() == papp->NextState())
		return;

	// Retrieve the runtime map from the status vector
	AppsMap_t *curr_state_map = &(status_vec[papp->CurrentState()]);
	AppsMap_t *next_state_map = &(status_vec[papp->NextState()]);
	assert(curr_state_map != next_state_map);

	// Find the application descriptor the current status map
	range = curr_state_map->equal_range(papp->Pid());
	assert(range.first != range.second);
	it = range.first;
	while (it != range.second &&
		((*it).second)->ExcId() != papp->ExcId()) {
		++it;
	}
	// The required application should be found, otherwise a data
	// structure corruption has occurred
	assert(it != range.second);
	if (it == range.second) {
		logger->Crit("unexpected state for EXC [%s] "
				"(Error: possible currupted data structures)",
				papp->StrId());
		return;
	}

	// Move it from the current to the next status map
	next_state_map->insert(next_state_map->begin(),
			AppsMapEntry_t(papp->Pid(), papp));
	curr_state_map->erase(it);

	// The application descriptor now will manage the change of
	// working mode
	papp->UpdateScheduledStatus(time);

	logger->Debug("Changed EXC [%s] status to [%d]",
			papp->StrId(),
			papp->CurrentState());
}


AppPtr_t const ApplicationManager::GetApplication(AppPid_t pid,
		uint8_t exc_id) {
	std::pair<AppsMap_t::iterator, AppsMap_t::iterator> range;
	AppsMap_t::iterator it;
	AppPtr_t papp;

	logger->Debug("Looking for EXC [%05d:*:%02d]...",
			pid, exc_id);

	//----- Find the required EXC
	range = apps.equal_range(pid);
	it = range.first;
	while (it != range.second &&
			((*it).second)->ExcId() != exc_id ) {
		++it;
	}
	if (it == range.second) {
		logger->Debug("EXC [%05d:*:%02d] NOT FOUND", pid, exc_id);
		return AppPtr_t();
	}
	papp = (*it).second;

	logger->Debug("EXC [%s] FOUND", papp->StrId());

	return papp;
}

}   // namespace bbque

