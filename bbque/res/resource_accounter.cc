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


ResourceAccounter::RegExitCode_t ResourceAccounter::RegisterResource(
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


uint64_t ResourceAccounter::queryState(std::string const & _path,
		AttributeSelector_t _att) const {

	// List of resource descriptors matched
	ResourcePtrList_t matches;

	if (IsPathTemplate(_path))
		// Find all the resources related to the path template
		matches = resources.findAll(_path);
	else {
		// Find all the resources matching the ID-based (or hybrid) path
		matches = resources.findSet(_path);
	}

	// For all the descriptors matched (and thus stored in the list) add the
	// amount of resource in the specified state (available, used, total)
	ResourcePtrList_t::iterator res_it = matches.begin();
	ResourcePtrList_t::iterator res_end = matches.end();
	uint64_t val = 0;

	for (; res_it != res_end; ++res_it) {
		switch(_att) {
		// Resource availability
		case RA_AVAILAB:
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


void ResourceAccounter::changeUsages(ba::Application const * _app,
		UsageAction_t _sel) {

	// Map of resource usages of the application
	UsagesMap_t const * app_usages;

	switch (_sel) {
	case RA_SWITCH:
		// Check the next working mode is set
		if(_app->NextAWM().get() == NULL)
			return;
		// Release current resources
		changeUsages(_app, RA_RELEASE);

		// Set resources usages from the next working mode
		usages[_app->Name()] = &(_app->NextAWM()->ResourceUsages());
		app_usages = usages[_app->Name()];
		break;

	case RA_RELEASE:
		// Get the map of resource usages of the application
		std::map<std::string, UsagesMap_t const *>::iterator usemap_it;
		usemap_it = usages.find(_app->Name());

		// If there isn't any map return
		if (usemap_it == usages.end())
			return;
		// Retrieve the current application map of usages
		app_usages = usemap_it->second;
		if (app_usages->empty())
			return;
	}

	// ResourceUsages iterators
	std::map<std::string, UsagePtr_t>::const_iterator usages_it =
		app_usages->begin();
	std::map<std::string, UsagePtr_t>::const_iterator usages_end =
		app_usages->end();

	// For each resource in the usages map...
	for (; usages_it != usages_end; ++usages_it) {

		// Lookup the resource descriptor
		//ResourcePtrList_t res_binds(GetResources(usages_it->second->bind_path));
		ResourcePtrList_t & res_binds = usages_it->second->binds;
		if (res_binds.empty())
			continue;

		ResourcePtrList_t::iterator it_bind = res_binds.begin();
		ResourcePtrList_t::iterator end_it = res_binds.end();

		uint64_t usage_value = usages_it->second->value;
/*
		while (usage_value > 0) {

			for (; it_bind != end_it; ++it_bind) {
				//
				switch (_sel) {
				case RA_SWITCH:
					if ((*it_bind)->Availability() > usage_value) {
						// Add the amount of resource used by the application
						(*it_bind)->AddUsed(usage_value);

						// Append the pointer to the current application
						// descriptor in the map of the applications using the
						// resource
						(*it_bind)->UsedBy(_app);

						//


						//
						usage_value = 0;
					}
					else {
						//
						(*it_bind)->AddUsed((*it_bind)->Availability());
						usage_value -= (*it_bind)->Availability();
					}
					break;

				case RA_RELEASE:
					// Subtract the amount of resource once used by the
					// application
					(*it_bind)->SubUsed();

					// Remove the pointer to the current application
					// descriptor in the map of the applications using the
					// resource
					(*it_bind)->NoMoreUsedBy(_app);
					break;
				}
			}
		}
		*/
	}
	// If this is a resource release...
	if (_sel == RA_RELEASE)
		// Remove the application from the map of usages
		usages.erase(_app->Name());
}

}   // namespace res

}   // namespace bbque

