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

#include <cassert>
#include <cmath>
#include <memory>
#include <map>
#include <string>

#include "bbque/modules_factory.h"
#include "bbque/plugin_manager.h"
#include "bbque/platform_services.h"
#include "bbque/app/application.h"
#include "bbque/app/working_mode.h"

namespace ba = bbque::app;

namespace bbque { namespace res {

ResourceAccounter *ResourceAccounter::GetInstance() {
	static ResourceAccounter *instance = NULL;

	if (instance == NULL)
		instance = new ResourceAccounter();
	return instance;
}


ResourceAccounter::ResourceAccounter():
	bbque::Object(RESOURCE_ACCOUNTER_NAMESPACE),
	clustering_factor(0) {

	// Get a logger
	std::string logger_name(RESOURCE_ACCOUNTER_NAMESPACE);
	plugins::LoggerIF::Configuration conf(logger_name.c_str());
	logger =
		std::unique_ptr<plugins::LoggerIF>
		(ModulesFactory::GetLoggerModule(std::cref(conf)));

	if (logger)
		logger->Debug("Resource accounter instanced.");
}


ResourceAccounter::~ResourceAccounter() {
	resources.clear();
	usages.clear();
}


ResourceAccounter::ExitCode_t ResourceAccounter::RegisterResource(
		std::string const & _path, std::string const & _units,
		uint64_t _amount) {

	// Check arguments
	if(_path.empty())
		return RA_ERR_MISS_PATH;

	// Insert a new resource in the tree
	ResourcePtr_t res_ptr;
	res_ptr = resources.insert(_path);
	if (res_ptr.get() == NULL)
		return RA_ERR_MEM;

	// Set the amount of resource on the units base
	res_ptr->SetTotal(ConvertValue(_amount, _units));
	return RA_SUCCESS;
}


uint64_t ResourceAccounter::queryStatus(ResourcePtrList_t const & rsrc_set,
		QueryOption_t _att) const {

	// For all the descriptors the list) add the amount of resource in the
	// specified state (available, used, total)
	ResourcePtrList_t::const_iterator res_it = rsrc_set.begin();
	ResourcePtrList_t::const_iterator res_end = rsrc_set.end();
	uint64_t val = 0;

	for (; res_it != res_end; ++res_it) {
		switch(_att) {
		// Resource availability
		case RA_AVAIL:
			val += (*res_it)->Availability();
			break;
		// Resource used
		case RA_USED:
			val += (*res_it)->Used();
			break;
		// Resource total
		case RA_TOTAL:
			val += (*res_it)->Total();
			break;
		}
	}
	return val;
}


ResourceAccounter::ExitCode_t ResourceAccounter::AcquireUsageSet(
		ba::Application const * _app) {

	// Check to avoid null pointer seg-fault
	if (!_app)
		return RA_ERR_MISS_APP;

	// Check that the next working mode has been set
	if (!_app->NextAWM())
		return RA_ERR_MISS_USAGES;

	// Does the application hold a resource usages set yet?
	std::map<uint32_t, UsagesMap_t const *>::iterator usemap_it;
	usemap_it = usages.find(_app->Pid());
	// Each application can hold just one resource usages set
	if (usemap_it != usages.end())
		// Release current resources
		ReleaseUsageSet(_app);

	// Set resource usages from the next working mode
	usages[_app->Pid()] = &(_app->NextAWM()->ResourceUsages());

	// Increment resources counts
	ExitCode_t ret = incUsageCounts(usages[_app->Pid()], _app);

	// If the resource allocation
	if (ret != RA_SUCCESS)
		usages.erase(_app->Pid());

	return ret;
}


void ResourceAccounter::ReleaseUsageSet(ba::Application const * _app) {

	// Check to avoid null pointer seg-fault
	if (!_app)
		return;

	// Get the map of resource usages of the application
	std::map<uint32_t, UsagesMap_t const *>::iterator usemap_it;
	usemap_it = usages.find(_app->Pid());
	// If there isn't any map return
	if (usemap_it == usages.end())
		return;

	// Decrement resources counts
	decUsageCounts(usemap_it->second, _app);

	// Remove the application from the map of usages
	usages.erase(_app->Pid());
}


inline ResourceAccounter::ExitCode_t ResourceAccounter::incUsageCounts(
		UsagesMap_t const * app_usages, ba::Application const * _app) {

	// ResourceUsages iterators
	std::map<std::string, UsagePtr_t>::const_iterator usages_it =
		app_usages->begin();
	std::map<std::string, UsagePtr_t>::const_iterator usages_end =
		app_usages->end();

	// Resource usage descriptor
	UsagePtr_t curr_usage;

	// For each resource in the usages map make a couple of checks
	for (; usages_it != usages_end; ++usages_it) {
		// Current resource usage descriptor
		curr_usage = usages_it->second;

		// Is there a resource binding ?
		if (curr_usage->binds.empty())
			return RA_ERR_MISS_BIND;

		// Is the request satisfable ?
		if (curr_usage->value > queryStatus(curr_usage->binds, RA_AVAIL))
			return RA_ERR_USAGE_EXC;
	}

	// Checks passed, go with resources reservation
	for (usages_it = app_usages->begin(); usages_it != usages_end;
			++usages_it) {
		// Current resource usage descriptor
		curr_usage = usages_it->second;

		// Current resource binds iterators
		ResourcePtrList_t::iterator it_bind = curr_usage->binds.begin();
		ResourcePtrList_t::iterator end_it = curr_usage->binds.end();

		// Usage value to be acquired/reserved to the application
		uint64_t usage_value = curr_usage->value;

		// Allocate the request to the resources binds
		while ((usage_value > 0) && (it_bind != end_it)) {
			// Check the availability of the current resource bind
			if (usage_value < (*it_bind)->Availability())
				// Allocate the quantity into the current resource bind
				usage_value -= (*it_bind)->Acquire(usage_value, _app);
			else
				// Acquire the whole available quantity of the current
				// resource bind
				usage_value -=
					(*it_bind)->Acquire((*it_bind)->Availability(), _app);
			// Next bind
			++it_bind;
		}
	}
	return RA_SUCCESS;
}


inline void ResourceAccounter::decUsageCounts(UsagesMap_t const * app_usages,
		ba::Application const * _app) {

	// ResourceUsages iterators
	std::map<std::string, UsagePtr_t>::const_iterator usages_it =
		app_usages->begin();
	std::map<std::string, UsagePtr_t>::const_iterator usages_end =
		app_usages->end();

	// For each resource in the usages map...
	for (; usages_it != usages_end; ++usages_it) {
		// Resource usage descriptor
		UsagePtr_t curr_usage = usages_it->second;

		// Count of the amount of resource freed
		uint64_t usage_freed = 0;

		ResourcePtrList_t::iterator it_bind = curr_usage->binds.begin();
		ResourcePtrList_t::iterator end_it = curr_usage->binds.end();

		// Release the resources held
		while (usage_freed < curr_usage->value) {
			// Release the quantity allocated in the current resource bind
			usage_freed += (*it_bind)->Release(_app);
			// Next bind
			++it_bind;
		}
	}
}


}   // namespace res

}   // namespace bbque

