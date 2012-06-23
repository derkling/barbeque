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

#include "bbque/resource_accounter.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <limits>
#include <locale>
#include <memory>
#include <map>
#include <string>
#include <sstream>

#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/platform_services.h"
#include "bbque/app/working_mode.h"

#define RP_DIV1 "============================================================="
#define RP_DIV2 "|-------------------------------+-------------+-------------|"
#define RP_DIV3 "|                               :             |             |"
#define RP_HEAD "|             RESOURCES         |        USED |       TOTAL |"


#define PRINT_NOTICE_IF_VERBOSE(verbose, text)\
	if (verbose)\
		logger->Notice(text);\
	else\
		DB(\
		logger->Debug(text);\
		);


namespace bbque {

ResourceAccounter & ResourceAccounter::GetInstance() {
	static ResourceAccounter instance;
	return instance;
}

ResourceAccounter::ResourceAccounter() :
	am(ApplicationManager::GetInstance()) {

	// Get a logger
	std::string logger_name(RESOURCE_ACCOUNTER_NAMESPACE);
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	assert(logger);

	// Init the system resources state view
	sys_usages_view = AppUsagesMapPtr_t(new AppUsagesMap_t);
	sys_view_token = 0;
	usages_per_views[sys_view_token] = sys_usages_view;
	rsrc_per_views[sys_view_token] = ResourceSetPtr_t(new ResourceSet_t);

	// Init sync session info
	sync_ssn.count = 0;
}

ResourceAccounter::~ResourceAccounter() {
	resources.clear();
	usages_per_views.clear();
	rsrc_per_views.clear();
}

/************************************************************************
 *                   LOGGER REPORTS                                     *
 ************************************************************************/

void ResourceAccounter::PrintStatusReport(RViewToken_t vtok, bool verbose) const {
	std::set<std::string>::const_iterator path_it(paths.begin());
	std::set<std::string>::const_iterator end_path(paths.end());
	char rsrc_path_padded[30];
	char rsrc_text_row[66];
	uint64_t rsrc_used;

	// Print the head of the report table
	if (verbose) {
		logger->Info("Report on state view: %d", vtok);
		logger->Notice(RP_DIV1);
		logger->Notice(RP_HEAD);
		logger->Notice(RP_DIV2);
	}
	else {
		DB(
		logger->Debug("Report on state view: %d", vtok);
		logger->Debug(RP_DIV1);
		logger->Debug(RP_HEAD);
		logger->Debug(RP_DIV2);
		);
	}

	for (; path_it != end_path; ++path_it) {
		// Amount of resource used
		rsrc_used = Used(*path_it, vtok);

		// Build the resource text row
		snprintf(rsrc_path_padded, 30, "%-30s", (*path_it).c_str());
		snprintf(rsrc_text_row, 66, "| %s : %11"PRIu64" | %11"PRIu64" |",
				rsrc_path_padded, rsrc_used, Total(*path_it));

		PRINT_NOTICE_IF_VERBOSE(verbose, rsrc_text_row);

		// No details to print if usage = 0
		if (rsrc_used == 0)
			continue;

		// Print details about how usage is partitioned among applications
		PrintAppDetails(*path_it, vtok, verbose);
	}

	PRINT_NOTICE_IF_VERBOSE(verbose, RP_DIV1);
}

void ResourceAccounter::PrintAppDetails(
		std::string const & path,
		RViewToken_t vtok,
		bool verbose) const {
	Resource::ExitCode_t res_result;
	AppSPtr_t papp;
	AppUid_t app_uid;
	uint64_t rsrc_amount;
	uint8_t app_index = 0;
	char app_info[30];
	char app_text_row[66];

	// Get the resource descriptor
	ResourcePtr_t rsrc(GetResource(path));
	if (!rsrc || rsrc->ApplicationsCount(vtok) == 0)
		return;

	do {
		// How much does the application/EXC use?
		res_result = rsrc->UsedBy(app_uid, rsrc_amount, app_index, vtok);
		if (res_result != Resource::RS_SUCCESS)
			break;

		// Get the App/EXC descriptor
		papp = am.GetApplication(app_uid);
		if (!papp || !papp->CurrentAWM())
			break;

		// Build the row to print
		snprintf(app_info, 30, "%s,P%02d,AWM%02d", papp->StrId(),
				papp->Priority(), papp->CurrentAWM()->Id());
		snprintf(app_text_row, 66, "| %29s : %11"PRIu64" |             |",
				app_info, rsrc_amount);

		PRINT_NOTICE_IF_VERBOSE(verbose, app_text_row);

		// Next application/EXC
		++app_index;
	} while (papp);

	// Print a separator line
	PRINT_NOTICE_IF_VERBOSE(verbose, RP_DIV3);
}

/************************************************************************
 *                   QUERY METHODS                                      *
 ************************************************************************/

uint64_t ResourceAccounter::QueryStatus(
		ResourcePtrList_t const & rsrc_list,
		QueryOption_t _att,
		RViewToken_t vtok,
		AppSPtr_t papp) const {
	// Cumulative value to return
	uint64_t val = 0;

	// For all the descriptors in the list add the quantity of resource in the
	// specified state (available, used, total)
	ResourcePtrList_t::const_iterator res_it(rsrc_list.begin());
	ResourcePtrList_t::const_iterator res_end(rsrc_list.end());
	for (; res_it != res_end; ++res_it) {
		// Current resource descriptor
		ResourcePtr_t const & rsrc(*res_it);

		switch(_att) {
		// Resource availability
		case RA_AVAIL:
			val += rsrc->Available(papp, vtok);
			break;
		// Resource used
		case RA_USED:
			val += rsrc->Used(vtok);
			break;
		// Resource total
		case RA_TOTAL:
			val += rsrc->Total();
			break;
		}
	}
	return val;
}

ResourceAccounter::ExitCode_t ResourceAccounter::CheckAvailability(
		UsagesMapPtr_t const & usages,
		RViewToken_t vtok,
		AppSPtr_t papp) const {
	uint64_t avail = 0;
	UsagesMap_t::const_iterator usages_it(usages->begin());
	UsagesMap_t::const_iterator usages_end(usages->end());

	// Check availability for each Usage object
	for (; usages_it != usages_end; ++usages_it) {
		// Current Usage
		std::string const & rsrc_path(usages_it->first);
		UsagePtr_t const & pusage(usages_it->second);

		// Query the availability of the resources in the list
		avail = QueryStatus(pusage->GetBindingList(), RA_AVAIL, vtok, papp);

		// If the availability is less than the amount required...
		if (avail < pusage->GetAmount()) {
			logger->Debug("Check availability: Exceeding request for {%s}"
					"[USG:%"PRIu64" | AV:%"PRIu64" | TOT:%"PRIu64"] ",
					rsrc_path.c_str(), pusage->GetAmount(), avail,
					QueryStatus(pusage->GetBindingList(), RA_TOTAL));
			return RA_ERR_USAGE_EXC;
		}
	}

	return RA_SUCCESS;
}

ResourceAccounter::ExitCode_t ResourceAccounter::GetAppUsagesByView(
		RViewToken_t vtok,
		AppUsagesMapPtr_t & apps_usages) {
	// Get the map of all the Apps/EXCs resource usages
	AppUsagesViewsMap_t::iterator view_it;
	if (vtok == 0) {
		// Default view / System state
		assert(sys_usages_view);
		apps_usages = sys_usages_view;
		return RA_SUCCESS;
	}

	// "Alternate" state view
	view_it = usages_per_views.find(vtok);
	if (view_it == usages_per_views.end()) {
		logger->Error("Application usages:"
				"Cannot find the resource state view referenced by %d",	vtok);
		return RA_ERR_MISS_VIEW;
	}

	// Set the the map
	apps_usages = view_it->second;
	return RA_SUCCESS;
}

/************************************************************************
 *                   RESOURCE MANAGEMENT                                *
 ************************************************************************/

ResourceAccounter::ExitCode_t ResourceAccounter::RegisterResource(
		std::string const & _path,
		std::string const & _units,
		uint64_t _amount) {
	std::string rsrc_type;

	// Check arguments
	if(_path.empty()) {
		logger->Fatal("Registering: Invalid resource path");
		return RA_ERR_MISS_PATH;
	}

	// Insert a new resource in the tree
	ResourcePtr_t rsrc(resources.insert(_path));
	if (!rsrc) {
		logger->Crit("Registering: Unable to allocate a new resource"
				"descriptor");
		return RA_ERR_MEM;
	}

	// Set the amount of resource considering the units
	rsrc->SetTotal(ConvertValue(_amount, _units));

	// Insert the path in the paths set
	paths.insert(_path);
	path_max_len = std::max((int) path_max_len, (int) _path.length());

	// Track the number of resources per type
	rsrc_type = ResourcePathUtils::GetNameTemplate(_path);
	if (rsrc_count_map.find(rsrc_type) == rsrc_count_map.end())
		rsrc_count_map.insert(std::pair<std::string, uint16_t>(rsrc_type, 1));
	else
		++rsrc_count_map[rsrc_type];

	return RA_SUCCESS;
}

ResourceAccounter::ExitCode_t ResourceAccounter::BookResources(
		AppSPtr_t papp,
		UsagesMapPtr_t const & rsrc_usages,
		RViewToken_t vtok,
		bool do_check) {
	std::unique_lock<std::recursive_mutex> status_ul(status_mtx);

	// Check to avoid null pointer segmentation fault
	if (!papp) {
		logger->Fatal("Booking: Null pointer to the application descriptor");
		return RA_ERR_MISS_APP;
	}

	// Check that the set of resource usages is not null
	if ((!rsrc_usages) || (rsrc_usages->empty())) {
		logger->Fatal("Booking: Empty resource usages set");
		return RA_ERR_MISS_USAGES;
	}

	// Get the map of resources used by the application (from the state view
	// referenced by 'vtok'). A missing view implies that the token is not
	// valid.
	AppUsagesMapPtr_t apps_usages;
	if (GetAppUsagesByView(vtok, apps_usages) == RA_ERR_MISS_VIEW) {
		logger->Fatal("Booking: Invalid resource state view token");
		return RA_ERR_MISS_VIEW;
	}

	// Each application can hold just one resource usages set
	AppUsagesMap_t::iterator usemap_it(apps_usages->find(papp->Uid()));
	if (usemap_it != apps_usages->end()) {
		logger->Warn("Booking: [%s] currently using a resource set yet",
				papp->StrId());
		return RA_ERR_APP_USAGES;
	}

	// Check resource availability (if this is not a sync session)
	if ((do_check) && !(Synching())) {
		if (CheckAvailability(rsrc_usages, vtok) == RA_ERR_USAGE_EXC) {
			logger->Debug("Booking: Cannot allocate the resource set");
			return RA_ERR_USAGE_EXC;
		}
	}

	// Increment the booking counts and save the reference to the resource set
	// used by the application
	IncBookingCounts(rsrc_usages, papp, vtok);
	apps_usages->insert(std::pair<AppUid_t, UsagesMapPtr_t>(papp->Uid(),
				rsrc_usages));
	logger->Debug("Booking: [%s] now holds %d resources", papp->StrId(),
			rsrc_usages->size());

	return RA_SUCCESS;
}

void ResourceAccounter::ReleaseResources(AppSPtr_t papp, RViewToken_t vtok) {
	std::unique_lock<std::recursive_mutex> status_ul(status_mtx);

	// Sanity check
	if (!papp) {
		logger->Fatal("Release: Null pointer to the application descriptor");
		return;
	}

	// Get the map of applications resource usages related to the state view
	// referenced by 'vtok'
	AppUsagesMapPtr_t apps_usages;
	if (GetAppUsagesByView(vtok, apps_usages) == RA_ERR_MISS_VIEW) {
		logger->Fatal("Release: Resource view unavailable");
		return;
	}

	// Get the map of resource usages of the application
	AppUsagesMap_t::iterator usemap_it(apps_usages->find(papp->Uid()));
	if (usemap_it == apps_usages->end()) {
		logger->Fatal("Release: Application referenced misses a resource set."
				" Possible data corruption occurred.");
		return;
	}

	// Decrement resources counts and remove the usages map
	DecBookingCounts(usemap_it->second, papp, vtok);
	apps_usages->erase(papp->Uid());
	logger->Debug("Release: [%s] resource release terminated", papp->StrId());
}

/************************************************************************
 *                   STATE VIEWS MANAGEMENT                             *
 ************************************************************************/

ResourceAccounter::ExitCode_t ResourceAccounter::GetView(
		std::string req_path,
		RViewToken_t & token) {
	// Null-string check
	if (req_path.empty()) {
		logger->Error("GetView: Missing a valid string");
		return RA_ERR_MISS_PATH;
	}

	// Token
	token = std::hash<std::string>()(req_path);
	logger->Debug("GetView: New resource state view. Token = %d", token);

	// Allocate a new view for the applications resource usages
	usages_per_views.insert(std::pair<RViewToken_t, AppUsagesMapPtr_t>(token,
				AppUsagesMapPtr_t(new AppUsagesMap_t)));

	//Allocate a new view for the set of resources allocated
	rsrc_per_views.insert(std::pair<RViewToken_t, ResourceSetPtr_t>(token,
				ResourceSetPtr_t(new ResourceSet_t)));

	return RA_SUCCESS;
}

void ResourceAccounter::PutView(RViewToken_t vtok) {
	// Do nothing if the token references the system state view
	if (vtok == sys_view_token) {
		logger->Warn("PutView: Cannot release the system resources view");
		return;
	}

	// Get the resource set using the referenced view
	ResourceViewsMap_t::iterator rviews_it(rsrc_per_views.find(vtok));
	if (rviews_it == rsrc_per_views.end()) {
		logger->Error("PutView: Cannot find the resource view"
				"referenced by %d", vtok);
		return;
	}

	// For each resource delete the view
	ResourceSet_t::iterator rsrc_set_it(rviews_it->second->begin());
	ResourceSet_t::iterator rsrc_set_end(rviews_it->second->end());
	for (; rsrc_set_it != rsrc_set_end; ++rsrc_set_it)
		(*rsrc_set_it)->DeleteView(vtok);

	// Remove the map of Apps/EXCs resource usages and the resource reference
	// set of this view
	usages_per_views.erase(vtok);
	rsrc_per_views.erase(vtok);

	logger->Debug("PutView: view %d cleared", vtok);
	logger->Debug("PutView: %d resource set and %d usages per views"
			"currently managed",
			rsrc_per_views.size(), usages_per_views.erase(vtok));
}

RViewToken_t ResourceAccounter::SetView(RViewToken_t vtok) {
	RViewToken_t old_sys_vtok;

	// Do nothing if the token references the system state view
	if (vtok == sys_view_token) {
		logger->Debug("SetView: View %d is the system state view yet!", vtok);
		return sys_view_token;
	}

	// Set the system state view pointer to the map of applications resource
	// usages of this view and point to
	AppUsagesViewsMap_t::iterator us_view_it(usages_per_views.find(vtok));
	if (us_view_it == usages_per_views.end()) {
		logger->Fatal("SetView: View %d unknown", vtok);
		return sys_view_token;
	}

	// Save the old view token
	old_sys_vtok = sys_view_token;

	// Update the system state view token and the map of Apps/EXCs resource
	// usages
	sys_view_token = vtok;
	sys_usages_view = us_view_it->second;

	// Put the old view
	PutView(old_sys_vtok);

	logger->Info("SetView: View %d is the new system state view.",
			sys_view_token);
	logger->Debug("SetView: %d resource set and %d usages per views"
			"currently managed",
			rsrc_per_views.size(), usages_per_views.erase(vtok));

	return sys_view_token;
}


/************************************************************************
 *                   SYNCHRONIZATION SUPPORT                            *
 ************************************************************************/

ResourceAccounter::ExitCode_t ResourceAccounter::SyncStart() {
	ResourceAccounter::ExitCode_t result;
	char tk_path[TOKEN_PATH_MAX_LEN];
	std::unique_lock<std::mutex> sync_ul(sync_ssn.mtx);
	logger->Info("SyncMode: Start");

	// If the counter has reached the maximum, reset
	if (sync_ssn.count == std::numeric_limits<uint32_t>::max()) {
		logger->Debug("SyncMode: Session counter reset");
		sync_ssn.count = 0;
	}

	// Build the path for getting the resource view token
	snprintf(tk_path, TOKEN_PATH_MAX_LEN, SYNC_RVIEW_PATH"%d", ++sync_ssn.count);
	logger->Debug("SyncMode [%d]: Requiring resource state view for %s",
			sync_ssn.count,	tk_path);

	// Synchronization has started
	sync_ssn.started = true;
	sync_ul.unlock();

	// Get a resource state view for the synchronization
	result = GetView(tk_path, sync_ssn.view);
	if (result != RA_SUCCESS) {
		logger->Fatal("SyncMode [%d]: Cannot get a resource state view",
				sync_ssn.count);
		SyncFinalize();
		return RA_ERR_SYNC_VIEW;
	}
	logger->Debug("SyncMode [%d]: Resource state view token = %d",
			sync_ssn.count,	sync_ssn.view);

	// Init the view with the resource accounting of running applications
	return SyncInit();
}

ResourceAccounter::ExitCode_t ResourceAccounter::SyncInit() {
	ResourceAccounter::ExitCode_t result;
	AppsUidMapIt apps_it;
	AppSPtr_t papp;

	// Running Applications/ExC
	papp = am.GetFirst(ApplicationStatusIF::RUNNING, apps_it);
	for ( ; papp; papp = am.GetNext(ApplicationStatusIF::RUNNING, apps_it)) {

		logger->Info("SyncInit: [%s] current AWM: %d", papp->StrId(),
				papp->CurrentAWM()->Id());

		// Re-acquire the resources (these should not have a "Next AWM"!)
		result = BookResources(papp, papp->CurrentAWM()->GetResourceBinding(),
				sync_ssn.view, false);
		if (result != RA_SUCCESS) {
			logger->Fatal("SyncInit [%d]: Resource booking failed for %s."
					" Aborting sync session...", sync_ssn.count, papp->StrId());

			SyncAbort();
			return RA_ERR_SYNC_INIT;
		}
	}

	logger->Info("SyncMode [%d]: Initialization finished", sync_ssn.count);
	return RA_SUCCESS;
}

ResourceAccounter::ExitCode_t ResourceAccounter::SyncAcquireResources(
		AppSPtr_t const & papp) {
	// Check next AWM
	if (!papp->NextAWM()) {
		logger->Fatal("SyncMode [%d]: [%s] missing the next AWM",
				sync_ssn.count, papp->StrId());
		return RA_ERR_MISS_AWM;
	}

	// Resource set to acquire
	UsagesMapPtr_t const &usages(papp->NextAWM()->GetResourceBinding());

	// Check that we are in a synchronized session
	if (!Synching()) {
		logger->Error("SyncMode [%d]: Session not open", sync_ssn.count);
		return RA_ERR_SYNC_START;
	}

	// Acquire resources
	return BookResources(papp, usages, sync_ssn.view, false);
}

void ResourceAccounter::SyncAbort() {
	PutView(sync_ssn.view);
	SyncFinalize();
	logger->Error("SyncMode [%d]: Session aborted", sync_ssn.count);
}

ResourceAccounter::ExitCode_t ResourceAccounter::SyncCommit() {
	ResourceAccounter::ExitCode_t result = RA_SUCCESS;
	RViewToken_t view;

	// Set the synchronization view as the new system one
	view = SetView(sync_ssn.view);
	if (view != sync_ssn.view) {
		logger->Fatal("SyncMode [%d]: Unable to set the new system resource"
				"state view", sync_ssn.count);
		result = RA_ERR_SYNC_VIEW;
	}

	// Release the last scheduled view, by setting it to the system view
	if (result == RA_SUCCESS) {
		SetScheduledView(sys_view_token);
		logger->Info("SyncMode [%d]: Session committed", sync_ssn.count);
	}

	// Finalize the synchronization
	SyncFinalize();

	// Log the status report
	PrintStatusReport();
	return result;
}

/************************************************************************
 *                   RESOURCE ACCOUNTING                                *
 ************************************************************************/

void ResourceAccounter::IncBookingCounts(
		UsagesMapPtr_t const & app_usages,
		AppSPtr_t const & papp,
		RViewToken_t vtok) {
	ResourceAccounter::ExitCode_t result;

	// Book resources for the application
	UsagesMap_t::const_iterator usages_it(app_usages->begin());
	UsagesMap_t::const_iterator usages_end(app_usages->end());
	for (; usages_it != usages_end;	++usages_it) {
		// Current required resource (Usage object)
		std::string const & rsrc_path(usages_it->first);
		UsagePtr_t pusage(usages_it->second);
		logger->Debug("Booking: [%s] requires resource {%s}",
				papp->StrId(), rsrc_path.c_str());

		// Do booking for the current resource request
		result = DoResourceBooking(papp, pusage, vtok);
		if (result != RA_SUCCESS)  {
			logger->Crit("Booking: unexpected fail! %s "
					"[USG:%"PRIu64" | AV:%"PRIu64" | TOT:%"PRIu64"]",
				rsrc_path.c_str(), pusage->GetAmount(),
				Available(rsrc_path, vtok, papp),
				Total(rsrc_path));

			// Print the report table of the resource assignments
			PrintStatusReport();
		}

		assert(result == RA_SUCCESS);
		logger->Info("Booking: SUCCESS - %s [USG:%"PRIu64" | AV:%"PRIu64" | TOT:%"PRIu64"]",
				rsrc_path.c_str(), pusage->GetAmount(),
				Available(rsrc_path, vtok, papp),
				Total(rsrc_path));
	}
}

ResourceAccounter::ExitCode_t ResourceAccounter::DoResourceBooking(
		AppSPtr_t const & papp,
		UsagePtr_t & pusage,
		RViewToken_t vtok) {
	bool first_resource;
	uint64_t  requested;

	// When the first resource bind has been tracked this is set false
	first_resource = false;

	// Get the set of resources referenced in the view
	ResourceViewsMap_t::iterator rsrc_view(rsrc_per_views.find(vtok));
	assert(rsrc_view != rsrc_per_views.end());
	ResourceSetPtr_t & rsrc_set(rsrc_view->second);

	// Amount of resource to book
	requested = pusage->GetAmount();

	// Get the list of resource binds
	ResourcePtrListIterator_t it_bind(pusage->GetBindingList().begin());
	ResourcePtrListIterator_t end_it(pusage->GetBindingList().end());
	for (; it_bind != end_it; ++it_bind) {
		// Break if the required resource has been completely allocated
		if (requested == 0)
			break;

		// Add the current resource binding to the set of resources used in
		// the view referenced by 'vtok'
		ResourcePtr_t & rsrc(*it_bind);
		rsrc_set->insert(rsrc);

		// Synchronization: booking according to scheduling decisions
		if (Synching()) {
			SyncResourceBooking(papp, rsrc, requested);
			continue;
		}

		// Scheduling: allocate required resource among its bindings
		SchedResourceBooking(papp, rsrc, requested, vtok);
		if ((requested == pusage->GetAmount()) || first_resource)
			continue;

		// Keep track of the first resource granted from the bindings
		pusage->TrackFirstBinding(papp, it_bind, vtok);
		first_resource = true;
	}

	// Keep track of the last resource granted from the bindings (only if we
	// are in the scheduling case)
	if (!Synching())
		pusage->TrackLastBinding(papp, it_bind, vtok);

	// Critical error: The availability of resources mismatches the one
	// checked in the scheduling phase. This should never happen!
	if (requested != 0)
		return RA_ERR_USAGE_EXC;

	return RA_SUCCESS;
}

bool ResourceAccounter::IsReshuffling(
		UsagesMapPtr_t const & pum_current,
		UsagesMapPtr_t const & pum_next) {
	ResourcePtrListIterator_t presa_it, presc_it;
	UsagesMap_t::iterator auit, cuit;
	ResourcePtr_t presa, presc;
	UsagePtr_t pua, puc;

	// Loop on resources
	for ( cuit = pum_current->begin(), auit = pum_next->begin();
		cuit != pum_current->end() && auit != pum_next->end();
			++cuit, ++auit) {

		// Get the resource usages
		puc = (*cuit).second;
		pua = (*auit).second;

		// Loop on bindings
		presc = puc->GetFirstResource(presc_it);
		presa = pua->GetFirstResource(presa_it);
		while (presc && presa) {
			logger->Debug("Checking: curr [%s:%d] vs next [%s:%d]",
				presc->Name().c_str(),
				presc->ApplicationUsage(
					puc->own_app, 0),
				presa->Name().c_str(),
				presc->ApplicationUsage(
					puc->own_app, pua->view_tk));
			// Check for resource binding differences
			if (presc->ApplicationUsage(puc->own_app, 0) !=
				presc->ApplicationUsage(puc->own_app,
					pua->view_tk)) {
				logger->Debug("AWM Shuffling detected");
				return true;
			}
			// Check next resource
			presc = puc->GetNextResource(presc_it);
			presa = pua->GetNextResource(presa_it);
		}
	}

	return false;
}

inline void ResourceAccounter::SchedResourceBooking(
		AppSPtr_t const & papp,
		ResourcePtr_t & rsrc,
		uint64_t & requested,
		RViewToken_t vtok) {
	// Check the available amount in the current resource binding
	uint64_t available = rsrc->Available(papp, vtok);

	// If it is greater than the required amount, acquire the whole
	// quantity from the current resource binding, otherwise split
	// it among sibling resource bindings
	if (requested < available)
		requested -= rsrc->Acquire(papp, requested, vtok);
	else
		requested -= rsrc->Acquire(papp, available, vtok);

	logger->Debug("DoResBook: %s scheduled to use %s (%d left)",
			papp->StrId(), rsrc->Name().c_str(), requested);
}

inline void ResourceAccounter::SyncResourceBooking(
		AppSPtr_t const & papp,
		ResourcePtr_t & rsrc,
		uint64_t & requested) {
	// Skip the resource binding if the not assigned by the scheduler
	uint64_t sched_usage = rsrc->ApplicationUsage(papp, sch_view_token);
	if (sched_usage == 0)
		return;

	// Acquire the resource according to the amount assigned by the
	// scheduler
	requested -= rsrc->Acquire(papp, sched_usage, sync_ssn.view);
	logger->Debug("DoResBook: %s acquires %s (%d left)",
			papp->StrId(), rsrc->Name().c_str(), requested);
}

void ResourceAccounter::DecBookingCounts(
		UsagesMapPtr_t const & app_usages,
		AppSPtr_t const & papp,
		RViewToken_t vtok) {
	// Maps of resource usages per Application/EXC
	UsagesMap_t::const_iterator usages_it(app_usages->begin());
	UsagesMap_t::const_iterator usages_end(app_usages->end());
	logger->Debug("DecCount: [%s] holds %d resources", papp->StrId(),
			app_usages->size());

	// Release the all the resources hold by the Application/EXC
	for (; usages_it != usages_end; ++usages_it) {
		std::string const & rsrc_path(usages_it->first);
		UsagePtr_t pusage(usages_it->second);

		// Release the resources bound to the current request
		UndoResourceBooking(papp, pusage, vtok);
		logger->Debug("DecCount: [%s] has freed {%s} of %"PRIu64"",
				papp->StrId(), rsrc_path.c_str(), pusage->GetAmount());
	}
}

void ResourceAccounter::UndoResourceBooking(
		AppSPtr_t const & papp,
		UsagePtr_t & pusage,
		RViewToken_t vtok) {
	// Keep track of the amount of resource freed
	uint64_t usage_freed = 0;

	// Get the set of resources referenced in the view
	ResourceViewsMap_t::iterator rsrc_view(rsrc_per_views.find(vtok));
	ResourceSetPtr_t & rsrc_set(rsrc_view->second);

	// For each resource binding release the amount allocated to the App/EXC
	ResourcePtrListIterator_t it_bind(pusage->GetBindingList().begin());
	ResourcePtrListIterator_t  end_it(pusage->GetBindingList().end());
	for(; usage_freed < pusage->GetAmount(); ++it_bind) {
		assert(it_bind != end_it);

		// Release the quantity hold by the Application/EXC
		ResourcePtr_t & rsrc(*it_bind);
		usage_freed += rsrc->Release(papp, vtok);

		// If no more applications are using this resource, remove it from
		// the set of resources referenced in the resource state view
		if ((rsrc_set) && (rsrc->ApplicationsCount() == 0))
			rsrc_set->erase(rsrc);
	}
	assert(usage_freed == pusage->GetAmount());
}


}   // namespace bbque
