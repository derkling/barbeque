/**
*       @file  application.cc
*      @brief  Application descriptor implementation
*
* This implements the application descriptor.
* Such descriptor includes static and dynamic information upon application
* execution. It embeds usual information about name, priority, user, PID
* (could be different from the one given by OS) plus a reference to the
* recipe object, the list of enabled working modes and resource constraints.
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

#include "bbque/app/application.h"

#include <limits>

#include "bbque/application_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/app/overheads.h"
#include "bbque/app/working_mode.h"
#include "bbque/res/resource_accounter.h"

namespace ba = bbque::app;
namespace br = bbque::res;
namespace bp = bbque::plugins;

namespace bbque { namespace app {

char const *ApplicationStatusIF::stateStr[] = {
	"DISABLED",
	"READY",
	"SYNC",
	"RUNNING",
	"FINISHED"
};

char const *ApplicationStatusIF::syncStateStr[] = {
	"STARTING",
	"RECONF",
	"MIGREC",
	"MIGRATE",
	"BLOCKED",
	"NONE"
};

// Compare two working mode values.
// This is used to sort the list of enabled working modes.
bool CompareAWMsByValue(const AwmPtr_t & wm1, const AwmPtr_t & wm2) {
		return wm1->Value() < wm2->Value();
}

Application::Application(std::string const & _name, AppPid_t _pid,
		uint8_t _exc_id) :
	name(_name),
	pid(_pid),
	exc_id(_exc_id) {

	// Get a logger
	std::string logger_name(APPLICATION_NAMESPACE"." + _name);
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	assert(logger);

	// Format the EXC string identifier
	::snprintf(str_id, 16, "%05d:%6s:%02d",
			Pid(), Name().substr(0,6).c_str(), ExcId());

	// Initialized scheduling state
	schedule.state = DISABLED;
	schedule.preSyncState = DISABLED;
	schedule.syncState = SYNC_NONE;

	logger->Info("Built new EXC [%s]", StrId());
}

Application::~Application() {
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());

	logger->Debug("Destroying EXC [%s]", StrId());

	// Release the resources
	if (CurrentAWM())
		ra.ReleaseResources(CurrentAWM()->Owner());

	// Release the recipe used
	recipe.reset();

	// Reset scheduling info
	schedule.awm.reset();

	// Releasing AWMs and Constraints...
	enabled_awms.clear();
	constraints.clear();

}

void Application::SetPriority(AppPrio_t _prio) {
	bbque::ApplicationManager &am(bbque::ApplicationManager::GetInstance());

	// If _prio value is greater then the lowest priority
	// (maximum integer value) it is trimmed to the last one.
	priority = std::min(_prio, am.LowestPriority());
}

void Application::InitWorkingModes(AppPtr_t & papp) {
	// Copy the working modes and set the owner
	AwmMap_t const & wms(recipe->WorkingModesAll());
	AwmMap_t::const_iterator it(wms.begin());
	AwmMap_t::const_iterator end(wms.end());
	for (; it != end; ++it) {
		AwmPtr_t app_awm(new WorkingMode((*(it->second).get())));
		app_awm->SetOwner(papp);
		working_modes.insert(std::pair<uint16_t, AwmPtr_t>(app_awm->Id(),
								app_awm));
		enabled_awms.push_back(app_awm);
	}

	// Sort the enabled list by "value"
	enabled_awms.sort(CompareAWMsByValue);
}

void Application::InitConstraints() {
	// For each static constraint make an assertion
	logger->Debug("%d constraints in the recipe",
			recipe->ConstraintsAll().size());
	ConstrMap_t::const_iterator cons_it(recipe->ConstraintsAll().begin());
	ConstrMap_t::const_iterator end_cons(recipe->ConstraintsAll().end());
	for (; cons_it != end_cons; ++cons_it) {
		// Lower bound
		if ((cons_it->second)->lower > 0)
				SetConstraint(cons_it->first, Constraint::LOWER_BOUND,
								(cons_it->second)->lower);
		// Upper bound
		if ((cons_it->second)->upper > 0)
				SetConstraint(cons_it->first, Constraint::UPPER_BOUND,
								(cons_it->second)->upper);
	}
}

void Application::SetRecipe(RecipePtr_t & _recipe, AppPtr_t & papp) {
	// Set the recipe that the application will use
	assert(_recipe.get() != NULL);
	recipe = _recipe;

	// Init information got from the recipe
	InitWorkingModes(papp);
	InitConstraints();
	plugins_data = PlugDataMap_t(recipe->plugins_data);

	// Debug messages
	logger->Info("%d working modes (enabled = %d).", working_modes.size(),
					enabled_awms.size());
	logger->Info("%d constraints in the application.", constraints.size());
	logger->Info("%d plugins specific attributes.", plugins_data.size());
}

AwmPtr_t Application::GetWorkingMode(uint16_t wmId) {
	AwmPtrList_t::iterator awm_it(enabled_awms.begin());
	AwmPtrList_t::iterator end_awm(enabled_awms.end());

	while(awm_it != end_awm) {
		if ((*awm_it)->Id() == wmId)
			return (*awm_it);
		++awm_it;
	}

	return AwmPtr_t();
}


/*******************************************************************************
 *  EXC State and SyncState Management
 ******************************************************************************/

// NOTE: this requires a lock on schedule_mtx
void Application::SetSyncState(SyncState_t sync) {

	logger->Debug("Changing sync state [%s, %d:%s => %d:%s]",
			StrId(),
			SyncState(), SyncStateStr(SyncState()),
			sync, SyncStateStr(sync));

	schedule.syncState = sync;
}

// NOTE: this requires a lock on schedule_mtx
void Application::SetState(State_t state, SyncState_t sync) {

	logger->Debug("Changing state [%s, %d:%s => %d:%s]",
			StrId(),
			State(), StateStr(State()),
			state, StateStr(state));

	// Update pre-sync state
	if (state == SYNC) {
		assert(sync != SYNC_NONE);
		schedule.preSyncState = State();
	} else {
		assert(sync == SYNC_NONE);
		schedule.preSyncState = state;
	}

	// Update resources on disabled
	if ((state == DISABLED) ||
			(state == READY))
		schedule.awm.reset();

	// Update state
	schedule.state = state;
	SetSyncState(sync);
}

// NOTE: this requires a lock on schedule_mtx
void Application::ResetState() {

	assert(Synching());

	logger->Debug("Resetting to pre-sync state for [%s]",
			StrId(),
			State(), StateStr(State()),
			PreSyncState(), StateStr(PreSyncState()));
	
	SetState(PreSyncState());
}

/*******************************************************************************
 *  EXC Destruction
 ******************************************************************************/

Application::ExitCode_t Application::Terminate() {
	bbque::ApplicationManager &am(bbque::ApplicationManager::GetInstance());
	std::unique_lock<std::mutex> state_ul(schedule_mtx);
	AppPtr_t papp = am.GetApplication(Uid());

	// Notify state update
	am.NotifyNewState(papp, FINISHED);

	// Mark the application has finished
	SetState(FINISHED);
	state_ul.unlock();

	logger->Info("EXC [%s] FINISHED", StrId());

	return APP_SUCCESS;
	
}


/*******************************************************************************
 *  EXC Enabling
 ******************************************************************************/

Application::ExitCode_t Application::Enable() {
	bbque::ApplicationManager &am(bbque::ApplicationManager::GetInstance());
	std::unique_lock<std::mutex> state_ul(schedule_mtx, std::defer_lock);
	AppPtr_t papp = am.GetApplication(Uid());

	// Not disabled applications could not be marked as READY
	if (!Disabled()) {
		logger->Crit("Trying to enable already enabled application [%s] "
				"(Error: possible data structure curruption?)",
				StrId());
		assert(Disabled());
		return APP_ABORT;
	}

	state_ul.lock();

	// Notify state update
	am.NotifyNewState(papp, READY);

	// Mark the application has ready to run
	SetState(READY);

	state_ul.unlock();

	logger->Info("EXC [%s] ENABLED", StrId());

	return APP_SUCCESS;
}


/*******************************************************************************
 *  EXC Disabled
 ******************************************************************************/

Application::ExitCode_t Application::Disable() {
	bbque::ApplicationManager &am(bbque::ApplicationManager::GetInstance());
	std::unique_lock<std::mutex> state_ul(schedule_mtx, std::defer_lock);
	AppPtr_t papp = am.GetApplication(Uid());

	// Not disabled applications could not be marked as READY
	if (Disabled()) {
		logger->Crit("Trying to enable already enabled application [%s] "
				"(Error: possible data structure curruption?)",
				StrId());
		assert(!Disabled());
		return APP_ABORT;
	}

	state_ul.lock();

	// Notify state update
	am.NotifyNewState(papp, DISABLED);

	// Mark the application has ready to run
	SetState(DISABLED);

	state_ul.unlock();

	logger->Info("EXC [%s] DISABLED", StrId());

	return APP_SUCCESS;
}


/*******************************************************************************
 *  EXC Optimization
 ******************************************************************************/

// NOTE: this requires a lock on schedule_mtx
Application::ExitCode_t Application::RequestSync(SyncState_t sync) {
	bbque::ApplicationManager &am(bbque::ApplicationManager::GetInstance());
	AppPtr_t papp = am.GetApplication(Uid());
	ApplicationManager::ExitCode_t result;

	if (!Active()) {
		logger->Crit("Sync request FAILED (Error: wrong application status)");
		assert(Active());
		return APP_ABORT;
	}

	logger->Debug("Request synchronization [%s, %d:%s]",
			StrId(), sync, SyncStateStr(sync));

	// Ensuring the AM has an hander for this application
	if (!papp) {
		logger->Crit("Request synchronization [%s, %d:%s] FAILED "
				"(Error: unable to get an application handler",
				StrId(), sync, SyncStateStr(sync));
		assert(papp);
		return APP_ABORT;
	}

	// Update our state
	SetState(SYNC, sync);

	// Schedule the sync request
	am.NotifyNewState(papp, Application::SYNC);

	// Request the application manager to synchronization this application
	// accorting to our new state
	result = am.SyncRequest(papp, sync);
	if (result != ApplicationManager::AM_SUCCESS) {
		logger->Error("Request synchronization FAILED (Error: %d)", result);
		ResetState();
		return APP_ABORT;
	}

	logger->Info("Sync scheduled [%s, %d:%s]",
			StrId(), sync, SyncStateStr(sync));

	return APP_SUCCESS;

}

Application::SyncState_t
Application::SyncRequired(AwmPtr_t const & awm) {

	// This must be called only by running applications
	assert(State() == RUNNING);
	assert(CurrentAWM().get());

	// Check if the assigned operating point implies RECONF|MIGREC|MIGRATE
	if ((CurrentAWM()->GetClusterSet() != awm->GetClusterSet()) &&
			(CurrentAWM()->Id() != awm->Id()))
		return MIGREC;

	if (CurrentAWM()->GetClusterSet() != awm->GetClusterSet())
		return MIGRATE;

	if (CurrentAWM()->Id() != awm->Id())
		return RECONF;

	// NOTE: By default no reconfiguration is assumed to be required, thus we
	// return the SYNC_STATE_COUNT which must be read as false values
	return SYNC_NONE;
}

Application::ExitCode_t
Application::Reschedule(AwmPtr_t const & awm) {
	SyncState_t sync;

	// Ready application could be synchronized to start
	if (State() == READY)
		return RequestSync(STARTING);
	
	// Otherwise, the application should be running...
	if (State() != RUNNING) {
		logger->Crit("Rescheduling FAILED (Error: wrong application status)");
		assert(State() == RUNNING);
		return APP_ABORT;
	}

	// Checking if a synchronization is required
	sync = SyncRequired(awm);
	if (sync == SYNC_NONE)
		return APP_SUCCESS;

	// Request a synchronization for the identified reconfiguration
	return RequestSync(sync);
}

Application::ExitCode_t Application::Unschedule() {

	// Ready application remain into ready state
	if (State() == READY)
		return APP_ABORT;

	// Otherwise, the application should be running...
	if (State() != RUNNING) {
		logger->Crit("Rescheduling FAILED (Error: wrong application status)");
		assert(State() == RUNNING);
		return APP_ABORT;
	}

	// The application should be blocked
	return RequestSync(BLOCKED);
}

Application::ExitCode_t Application::ScheduleRequest(AwmPtr_t const & awm,
		br::UsagesMapPtr_t & resource_set,
		RViewToken_t vtok) {
	std::unique_lock<std::mutex> schedule_ul(schedule_mtx);
	br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
	br::ResourceAccounter::ExitCode_t booking;
	AppPtr_t papp(awm->Owner());
	ExitCode_t result;

	logger->Debug("Schedule request for [%s] into AWM [%02d:%s]",
			papp->StrId(), awm->Id(), awm->Name().c_str());

	// Get the working mode pointer
	if (!awm) {
		logger->Crit("Schedule request for [%s] FAILED "
				"(Error: AWM not existing)", papp->StrId());
		assert(awm);
		return APP_WM_NOT_FOUND;
	}

	if (Disabled()) {
		logger->Debug("Schedule request for [%s] FAILED "
				"(Error: EXC being disabled)", papp->StrId());
		return APP_DISABLED;
	}

	// Checking for resources availability
	booking = ra.BookResources(papp, resource_set, vtok);

	// If resources are available, bind the resource set into the working
	// mode, and thus reschedule the application. Otherwise unschedule.
	if (booking != br::ResourceAccounter::RA_SUCCESS) {
		logger->Debug("Unscheduling [%s]...", papp->StrId());
		Unschedule();
		return APP_WM_REJECTED;
	}
	
	logger->Debug("Rescheduling [%s] into AWM [%d:%s]...",
			papp->StrId(), awm->Id(), awm->Name().c_str());

	awm->SetResourceBinding(resource_set);
	result = Reschedule(awm);

	if (result != APP_SUCCESS) {
		ra.ReleaseResources(papp, vtok);
		awm->ClearResourceBinding();
		return APP_WM_REJECTED;
	}

	// Set next awm
	schedule.next_awm = awm;
	return APP_WM_ACCEPTED;
}

/*******************************************************************************
 *  EXC Synchronization
 ******************************************************************************/

Application::ExitCode_t Application::SetRunning() {
	bbque::ApplicationManager &am(bbque::ApplicationManager::GetInstance());
	AppPtr_t papp = am.GetApplication(Uid());

	am.NotifyNewState(papp, RUNNING);
	SetState(RUNNING);

	return APP_SUCCESS;

}

Application::ExitCode_t Application::SetBlocked() {
	bbque::ApplicationManager &am(bbque::ApplicationManager::GetInstance());
	AppPtr_t papp = am.GetApplication(Uid());

	// If the application as been marked FINISHED, than it is going to be
	// released
	if (State() == FINISHED)
		return APP_SUCCESS;

	// Otherwise mark it as READY to be re-scheduled when possible
	am.NotifyNewState(papp, READY);
	SetState(READY);
	return APP_SUCCESS;

}

Application::ExitCode_t Application::ScheduleCommit() {
	std::unique_lock<std::mutex> state_ul(schedule_mtx, std::defer_lock);

	// Ignoring applications disabled during a SYNC
	if (Disabled()) {
		logger->Info("Sync completed (on disabled EXC) [%s, %d:%s]",
				StrId(), State(), StateStr(State()));
		return APP_SUCCESS;
	}

	state_ul.lock();

	assert(State() == SYNC);

	switch(SyncState()) {
	case STARTING:
	case RECONF:
	case MIGREC:
	case MIGRATE:
		SetRunning();
		break;
	case BLOCKED:
		SetBlocked();
		break;
	default:
		logger->Crit("Sync for EXC [%s] FAILED"
				"(Error: invalid synchronization state)");
		assert(SyncState() < Application::SYNC_NONE);
		return APP_ABORT;
	}

	logger->Info("Sync completed [%s, %d:%s]",
			StrId(), State(), StateStr(State()));

	return APP_SUCCESS;

}


/*******************************************************************************
 *  EXC Constraints Management
 ******************************************************************************/

void Application::WorkingModesEnabling(std::string const & _res_name,
				Constraint::BoundType_t _type,
				uint64_t _constr_value) {
	// Iterate over all the working modes to check the resource usage value
	AwmMap_t::const_iterator it_awm(recipe->WorkingModesAll().begin());
	AwmMap_t::const_iterator awms_end(recipe->WorkingModesAll().end());
	for (; it_awm != awms_end; ++it_awm) {

		// If a resource usage is below an upper bound constraint or
		// above a lower bound one disable the working mode, by removing it
		// from the enabled list
		uint64_t usage_value = (it_awm->second)->ResourceUsageValue(_res_name);
		if (((_type == Constraint::LOWER_BOUND) &&
				(usage_value < _constr_value)) ||
						((_type == Constraint::UPPER_BOUND) &&
							(usage_value > _constr_value))) {
			enabled_awms.remove(it_awm->second);
			logger->Debug("Disabled : %s", (it_awm->second)->Name().c_str());
		}
		else {
			// The working mode is enabled yet ?
			AwmPtrList_t::iterator it_enabl(enabled_awms.begin());
			AwmPtrList_t::iterator enabl_end(enabled_awms.end());
			for (; it_enabl != enabl_end; ++it_awm)
				if ((*it_enabl)->Name().compare((it_awm->second)->Name()) == 0)
					break;

			// If no, enable it
			if (it_enabl == enabl_end) {
				enabled_awms.push_back(it_awm->second);
				logger->Debug("Enabled : %s", (it_awm->second)->Name().c_str());
			}
		}
	}

	// Sort by "value"
	enabled_awms.sort(CompareAWMsByValue);
	logger->Debug("%d working modes enabled", enabled_awms.size());
}

Application::ExitCode_t Application::SetConstraint(
				std::string const & _res_name,
				Constraint::BoundType_t _type,
				uint32_t _value) {
	// Check if the resource exists, and get the descriptor.
	// If doesn't, return.
	ConstrMap_t::iterator it_con(constraints.find(_res_name));
	if (it_con == constraints.end()) {
		br::ResourceAccounter &ra(br::ResourceAccounter::GetInstance());
		br::ResourcePtr_t rsrc_ptr(ra.GetResource(_res_name));
		if (!rsrc_ptr) {
			logger->Warn("Constraint rejected: unknown resource path");
			return APP_RSRC_NOT_FOUND;
		}

		// Create a constraint object, set its resource reference
		// and insert it into the constraints map
		constraints.insert(std::pair<std::string, ConstrPtr_t>(
								_res_name,
								ConstrPtr_t(new Constraint(rsrc_ptr))));
	}

	// Set the constraint bound value
	switch(_type) {
	case Constraint::LOWER_BOUND:
		constraints[_res_name]->lower = _value;
		break;
	case Constraint::UPPER_BOUND:
		constraints[_res_name]->upper = _value;
		break;
	}
	// Check if there are some awms to disable
	WorkingModesEnabling(_res_name, _type, _value);

	return APP_SUCCESS;
}

Application::ExitCode_t Application::RemoveConstraint(
				std::string const & _res_name,
				Constraint::BoundType_t _type) {
	// Lookup the constraint by resource pathname
	ConstrMap_t::iterator it_con(constraints.find(_res_name));
	if (it_con == constraints.end()) {
		logger->Warn("Constraint removing failed: unknown resource path");
		return APP_CONS_NOT_FOUND;
	}

	// Reset the constraint and check for AWM to enable
	switch (_type) {
	case Constraint::LOWER_BOUND :
		it_con->second->lower = 0;
		WorkingModesEnabling(_res_name, _type, it_con->second->lower);
		// If there isn't an upper bound value, remove the constraint object
		if (it_con->second->upper == std::numeric_limits<uint64_t>::max())
			constraints.erase(it_con);
		break;

	case Constraint::UPPER_BOUND :
		it_con->second->upper = std::numeric_limits<uint64_t>::max();
		WorkingModesEnabling(_res_name, _type, it_con->second->upper);
		// If there isn't a lower bound value, remove the constraint object
		if (it_con->second->lower == 0)
			constraints.erase(it_con);
		break;
	}

	return APP_SUCCESS;
}


} // namespace app

} // namespace bbque

