/**
 *       @file  resource_accounter.cc
 *      @brief  Implementation of the Resource Accounter component
 *
 * This implements the componter for making resource accounting.
 *
 * Each resource of system/platform should be properly registered in the
 * Resource accounter. It keeps track of the information upon availability,
 * total amount and used resources.
 * The information above are updated through proper methods which must be
 * called when an application working mode has triggered.
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

#include "bbque/res/resource_accounter.h"

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

#include "bbque/system_view.h"
#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/platform_services.h"
#include "bbque/app/application.h"
#include "bbque/app/working_mode.h"

namespace bbque { namespace res {

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

void ResourceAccounter::PrintStatusReport(RViewToken_t vtok,
		bool verbose) const {
	std::set<std::string>::const_iterator path_it(paths.begin());
	std::set<std::string>::const_iterator end_path(paths.end());
	char padded_path[30];
	char app_info[50];

	if (verbose) {
		logger->Info("Report on state view: %d", vtok);
		logger->Notice(
				"------------- Resources ------------- Used ------ Total -");
	} else {
		DB(
		logger->Debug("Report on state view: %d", vtok);
		logger->Debug(
				"------------- Resources ------------- Used ------ Total -");
		);
	}
	for (; path_it != end_path; ++path_it) {
		snprintf(padded_path, 30, "%-30s", (*path_it).c_str());
		if (verbose) {
			StrAppUsingPE(*path_it, app_info, 50, vtok);
			logger->Notice("%s : %10llu | %10llu | %s",
					padded_path, Used(*path_it, vtok), Total(*path_it),
					app_info[0] ? app_info : "");
		} else {
			DB(
			StrAppUsingPE(*path_it, app_info, 50, vtok);
			logger->Debug("%s : %10llu | %10llu | %s",
				padded_path, Used(*path_it, vtok), Total(*path_it),
				app_info[0] ? app_info : "");
			);
		}
	}

	if (verbose) {
		logger->Notice(
				"---------------------------------------------------------");
	} else {
		DB(
		logger->Debug(
				"---------------------------------------------------------");
		);
	}
}

AppPtr_t const ResourceAccounter::AppUsingPE(std::string const & path,
		RViewToken_t vtok) const {
	Resource::ExitCode_t rsrc_ret;
	AppUid_t app_uid;
	AppPtr_t papp;
	uint64_t amount;

	// Get the Resource decriptor
	ResourcePtr_t rsrc(GetResource(path));
	if (!rsrc) {
		logger->Error("Cannot find PE: '%s'", path.c_str());
		return AppPtr_t();
	}

	// Get the App/EXC descriptor
	rsrc_ret = rsrc->UsedBy(app_uid, amount, 0, vtok);
	if (rsrc_ret != Resource::RS_SUCCESS)
		return AppPtr_t();

	papp = am.GetApplication(app_uid);
	if (!papp)
		return AppPtr_t();

	// Skip if the App/EXC hasn't an AWM or the resource is not a PE
	if (!papp->CurrentAWM() || (rsrc->Name().compare("pe") < 0))
		return AppPtr_t();

	return papp;
}

inline char const * ResourceAccounter::StrAppUsingPE(std::string const & path,
		char * buff, size_t size, RViewToken_t vtok) const {

	// Lookup the App/EXC
	AppPtr_t papp(AppUsingPE(path, vtok));
	if (!papp) {
		buff[0] = 0;
		return NULL;
	}

	// Build the string
	snprintf(buff, size, "%s,%d,%d", papp->StrId(), papp->Priority(),
			papp->CurrentAWM()->Id());
	return buff;
}

/************************************************************************
 *                   QUERY METHODS                                      *
 ************************************************************************/

uint16_t ResourceAccounter::ClusteringFactor(std::string const & path) {
	uint16_t clustering_factor;

	// Check if the resource exists
	if (!ExistResource(path))
		return 0;

	// Check if the resource is clustered
	int16_t clust_patt_pos = path.find(RSRC_CLUSTER);
	if (clust_patt_pos < 0)
		return 1;

	// Compute the clustering factor
	clustering_factor = Total(RSRC_CLUSTER);
	if (clustering_factor == 0)
		++clustering_factor;

	return clustering_factor;
}

uint64_t ResourceAccounter::QueryStatus(ResourcePtrList_t const & rsrc_set,
		QueryOption_t _att,
		RViewToken_t vtok,
		AppPtr_t papp) const {
	// Cumulative value to return
	uint64_t val = 0;

	// For all the descriptors in the list add the quantity of resource in the
	// specified state (available, used, total)
	ResourcePtrList_t::const_iterator res_it(rsrc_set.begin());
	ResourcePtrList_t::const_iterator res_end(rsrc_set.end());
	for (; res_it != res_end; ++res_it) {

		switch(_att) {
		// Resource availability
		case RA_AVAIL:
			val += (*res_it)->Available(papp, vtok);
			break;
		// Resource used
		case RA_USED:
			val += (*res_it)->Used(vtok);
			break;
		// Resource total
		case RA_TOTAL:
			val += (*res_it)->Total();
			break;
		}
	}
	return val;
}

ResourceAccounter::ExitCode_t ResourceAccounter::CheckAvailability(
		UsagesMapPtr_t const & usages,
		RViewToken_t vtok,
		AppPtr_t papp) const {
	UsagesMap_t::const_iterator usages_it(usages->begin());
	UsagesMap_t::const_iterator usages_end(usages->end());

	// Check availability for each ResourceUsage object
	for (; usages_it != usages_end; ++usages_it) {
		uint64_t avail = Available(usages_it->second, vtok, papp);
		if (avail < usages_it->second->value) {
			logger->Debug("ChkAvail: Exceeding request for {%s}"
					"[USG:%llu | AV:%llu | TOT:%llu] ",
					usages_it->first.c_str(),
					usages_it->second->value,
					avail,
					Total(usages_it->second));
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

	return RA_SUCCESS;
}

ResourceAccounter::ExitCode_t ResourceAccounter::BookResources(AppPtr_t papp,
		UsagesMapPtr_t const & resource_set,
		RViewToken_t vtok,
		bool do_check) {
	std::unique_lock<std::recursive_mutex> status_ul(status_mtx);

	// Check to avoid null pointer segmentation fault
	if (!papp) {
		logger->Fatal("Booking: Null pointer to the application descriptor");
		return RA_ERR_MISS_APP;
	}

	// Check that the next working mode has been set
	if ((!resource_set) || (resource_set->empty())) {
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
		logger->Debug("Booking: [%s] currently using a resource set yet",
				papp->StrId());
		return RA_ERR_APP_USAGES;
	}

	// Check resource availability (if this is not a sync session)
	if ((do_check) && !(sync_ssn.started)) {
		if (CheckAvailability(resource_set, vtok) == RA_ERR_USAGE_EXC) {
			logger->Debug("Booking: Cannot allocate the resource set");
			return RA_ERR_USAGE_EXC;
		}
	}

	// Increment the booking counts and save the reference to the resource set
	// used by the application
	IncBookingCounts(resource_set, papp, vtok);
	apps_usages->insert(std::pair<AppUid_t, UsagesMapPtr_t>(papp->Uid(),
				resource_set));
	logger->Debug("Booking: [%s] now holds %d resources", papp->StrId(),
			resource_set->size());

	return RA_SUCCESS;
}

void ResourceAccounter::ReleaseResources(AppPtr_t papp, RViewToken_t vtok) {
	std::unique_lock<std::recursive_mutex> status_ul(status_mtx);

	// Check to avoid null pointer seg-fault
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

	// Release resources from sync view
	if ((sync_ssn.started) && (papp->Active()) && (vtok != sync_ssn.view))
		ReleaseResources(papp, sync_ssn.view);
}

/************************************************************************
 *                   STATE VIEWS MANAGEMENT                             *
 ************************************************************************/

ResourceAccounter::ExitCode_t ResourceAccounter::GetView(std::string req_path,
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
	logger->Info("SyncMode: Start");
	std::unique_lock<std::mutex>(sync_ssn.mtx);
	// If the counter has reached the maximum, reset
	if (sync_ssn.count == std::numeric_limits<uint32_t>::max()) {
		logger->Debug("SyncMode: Session counter reset");
		sync_ssn.count = 0;
	}

	// Build the path for getting the resource view token
	char token_path[TOKEN_PATH_MAX_LEN];
	snprintf(token_path, TOKEN_PATH_MAX_LEN, SYNC_RVIEW_PATH"%d",
			++sync_ssn.count);
	logger->Debug("SyncMode [%d]: Requiring resource state view for %s",
			sync_ssn.count,	token_path);

	// Set the flag and get the resources sync view
	sync_ssn.started = true;
	if (GetView(token_path, sync_ssn.view) != RA_SUCCESS) {
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
	AppPtr_t papp;

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
		AppPtr_t const & papp) {
	// Check next AWM
	if (!papp->NextAWM()) {
		logger->Fatal("SyncMode [%d]: [%s] missing the next AWM",
				sync_ssn.count, papp->StrId());
		return RA_ERR_MISS_AWM;
	}

	// Resource set to acquire
	UsagesMapPtr_t const &usages(papp->NextAWM()->GetResourceBinding());

	// Check that we are in a synchronized session
	if (!sync_ssn.started) {
		logger->Error("SyncMode [%d]: Session not open", sync_ssn.count);
		return RA_ERR_SYNC_START;
	}

	// Acquire resources
	return BookResources(papp, usages, sync_ssn.view, false);
}

void ResourceAccounter::SyncAbort() {
	PutView(sync_ssn.view);
	SyncFinalize();
	logger->Info("SyncMode [%d]: Session aborted", sync_ssn.count);
}

ResourceAccounter::ExitCode_t ResourceAccounter::SyncCommit() {
	ResourceAccounter::ExitCode_t result = RA_SUCCESS;
	if (SetView(sync_ssn.view) != sync_ssn.view) {
		logger->Fatal("SyncMode [%d]: Unable to set the new system resource"
				"state view", sync_ssn.count);
		result = RA_ERR_SYNC_VIEW;
	}

	SyncFinalize();
	if (result == RA_SUCCESS) {
		logger->Info("SyncMode [%d]: Session committed", sync_ssn.count);
		// Release the last scheduled view, by setting it to the system view
		SetScheduledView(sys_view_token);
	}

	PrintStatusReport();
	return result;
}

/************************************************************************
 *                   RESOURCE ACCOUNTING                                *
 ************************************************************************/

void ResourceAccounter::IncBookingCounts(UsagesMapPtr_t const & app_usages,
		AppPtr_t const & papp,
		RViewToken_t vtok) {
	ExitCode_t result;

	// Book resources for the application
	UsagesMap_t::const_iterator usages_it(app_usages->begin());
	UsagesMap_t::const_iterator usages_end(app_usages->end());
	for (; usages_it != usages_end;	++usages_it) {
		// Current required resource (ResourceUsage object)
		UsagePtr_t pusage(usages_it->second);
		logger->Debug("Booking: [%s] requires resource {%s}",
				papp->StrId(), usages_it->first.c_str());

		// Do booking for this resource
		result = DoResourceBooking(papp, pusage, vtok);
		if (result != RA_SUCCESS)  {
			logger->Crit("Booking: unexpected fail! "
					"%s [USG:%llu | AV:%llu | TOT:%llu]",
				usages_it->first.c_str(), pusage->value,
				Available(usages_it->first, vtok, papp),
				Total(usages_it->first));

			// Print the report table of the resource assignments
			PrintStatusReport();
		}

		assert(result == RA_SUCCESS);
		logger->Info("Booking: SUCCESS - %s [USG:%llu | AV:%llu | TOT:%llu]",
				usages_it->first.c_str(), pusage->value,
				Available(usages_it->first, vtok, papp),
				Total(usages_it->first));
	}
}

ResourceAccounter::ExitCode_t ResourceAccounter::DoResourceBooking(
		AppPtr_t const & papp,
		UsagePtr_t & pusage,
		RViewToken_t vtok) {
	// Get the set of resources referenced in the view
	ResourceViewsMap_t::iterator rsrc_view(rsrc_per_views.find(vtok));
	assert(rsrc_view != rsrc_per_views.end());
	ResourceSetPtr_t rsrc_set(rsrc_view->second);

	// Amount of resource to book
	uint64_t usage_val = pusage->value;

	// Get the list of resource binds
	ResourcePtrListIterator_t it_bind(pusage->binds.begin());
	ResourcePtrListIterator_t end_it(pusage->binds.end());
	while ((usage_value > 0) && (it_bind != end_it)) {
		// If the current resource bind has enough availability, reserve the
		// whole quantity requested here. Otherwise split it in more "sibling"
		// resource binds.
		uint64_t availab = (*it_bind)->Available(vtok);
		if (usage_value < availab)
			usage_value -= (*it_bind)->Acquire(papp, usage_value, vtok);
		else
			usage_value -=
				(*it_bind)->Acquire(papp, availab, vtok);

		// Add the resource to the set of resources used in the view
		// referenced by 'vtok'
		rsrc_set->insert(*it_bind);

		++it_bind;
	}

	// Critical error. This means that the availability of resources
	// mismatches the one checked in the scheduling phase
	if (usage_value != 0)
		return RA_ERR_USAGE_EXC;

	return RA_SUCCESS;
}

void ResourceAccounter::DecBookingCounts(UsagesMapPtr_t const & app_usages,
		AppPtr_t const & papp,
		RViewToken_t vtok) {
	// Maps of resource usages per Application/EXC
	UsagesMap_t::const_iterator usages_it(app_usages->begin());
	UsagesMap_t::const_iterator usages_end(app_usages->end());
	logger->Debug("DecCount: [%s] holds %d resources", papp->StrId(),
			app_usages->size());

	// Release the all the resources hold by the Application/EXC
	for (; usages_it != usages_end; ++usages_it) {
		UsagePtr_t pusage(usages_it->second);
		UndoResourceBooking(papp, pusage, vtok);
		logger->Debug("DecCount: [%s] has freed {%s} of %llu", papp->StrId(),
				usages_it->first.c_str(), pusage->value);
	}
}

void ResourceAccounter::UndoResourceBooking(AppPtr_t const & papp,
		UsagePtr_t & pusage,
		RViewToken_t vtok) {
	// Get the set of resources referenced in the view
	ResourceViewsMap_t::iterator rsrc_view(rsrc_per_views.find(vtok));
	ResourceSetPtr_t rsrc_set;
	if (rsrc_view != rsrc_per_views.end())
		rsrc_set = rsrc_view->second;

	// Keep track of the amount of resource freed
	uint64_t usage_freed = 0;

	// For each resource binding release the amount allocated to the App/EXC
	ResourcePtrListIterator_t it_bind(pusage->binds.begin());
	ResourcePtrListIterator_t end_it(pusage->binds.end());
	while (usage_freed < pusage->value) {
		assert(it_bind != end_it);
		usage_freed += (*it_bind)->Release(papp, vtok);

		// If there are no more applications using the resource remove it
		// from the set of resources referenced in the view
		if (rsrc_set) {
			logger->Debug("ApplicationsCount: %s = %d",
					(*it_bind)->Name().c_str(),
					(*it_bind)->ApplicationsCount());
			if ((*it_bind)->ApplicationsCount() == 0)
				rsrc_set->erase(*it_bind);
		}

		++it_bind;
	}
	assert(usage_freed == pusage->value);
}

}   // namespace res

}   // namespace bbque

