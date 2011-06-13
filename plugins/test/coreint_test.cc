/**
 *       @file  coreint_test.cc
 *      @brief  CoreInteractions plugin implementation
 *
 * This provides the implementation of the test plugin CoreInteractions
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

#include "coreint_test.h"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>

#include "bbque/modules_factory.h"
#include "bbque/app/recipe.h"
#include "bbque/app/working_mode.h"
#include "bbque/app/plugin_data.h"
#include "bbque/res/resources.h"
#include "bbque/utils/timer.h"

namespace ba = bbque::app;
namespace bp = bbque::plugins;
namespace br = bbque::res;
namespace bu = bbque::utils;

namespace bbque { namespace plugins {

/** Map of application descriptor shared pointers */
typedef std::map<uint32_t, AppPtr_t> AppsMap_t;

// Test set
std::vector<std::string> res_names = {
	// System-wide resources
	"mem0",
	"dma0",
	// Platform resources
	"arch.mem0",
	"arch.tile0.mem0",
	// Cluster level memories
	"arch.tile0.cluster0.mem0",
	"arch.tile0.cluster1.mem0",
	"arch.tile0.cluster2.mem0",
	"arch.tile0.cluster3.mem0",
	// Cluster level DMAs
	"arch.tile0.cluster0.dma0",
	"arch.tile0.cluster1.dma0",
	"arch.tile0.cluster2.dma0",
	"arch.tile0.cluster3.dma0",
	// Processing elements
	"arch.tile0.cluster0.pe0",
	"arch.tile0.cluster0.pe1",
	"arch.tile0.cluster0.pe2",
	"arch.tile0.cluster0.pe3",
	"arch.tile0.cluster1.pe0",
	"arch.tile0.cluster1.pe1",
	"arch.tile0.cluster1.pe2",
	"arch.tile0.cluster1.pe3",
	"arch.tile0.cluster2.pe0",
	"arch.tile0.cluster2.pe1",
	"arch.tile0.cluster2.pe2",
	"arch.tile0.cluster2.pe3",
	"arch.tile0.cluster3.pe0",
	"arch.tile0.cluster3.pe1",
	"arch.tile0.cluster3.pe2",
	"arch.tile0.cluster3.pe3",
/*	"arch.tile0.cluster2.pe0.mem0",
	"arch.tile0.cluster0.pe1.mem1",
	"arch.tile0.cluster0.pe1.mem0",
	"arch.tile0.cluster1.pe2.mem0"
 */
};

std::vector<std::string> res_units = {
	"Mb",
	"Mbps",

	"Mb",
	"Mb",

	"Kb",
	"Kb",
	"Kb",
	"Kb",

	"Mbps",
	"Mbps",
	"Mbps",
	"Mbps",

	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1",
	"1"
};

std::vector<uint64_t> res_totals = {
	32, 	// System RAM
	200,  	// DMA system bandwidth

	4,   	// Memory architecture level
	2,   	// Memory inter-cluster level

	256, 	// Memory intra-cluster level
	256,
	256,
	256,

	50, 	// DMA intra-cluster bandwidht
	50,
	50,
	50,

	1, 	// Number of processing elements
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1
};

std::vector<std::string> rsrcSearchPaths = {
	"arch.tile0.mem",
	"arch.tile0.cluster",
	"arch.tile.cluster.pe",
	"dma",
	"spi",
	RSRC_CLUSTER,
	"arch.tile.cluster2.pe0"
};


CoreInteractionsTest::CoreInteractionsTest():
	sv(SystemView::GetInstance()),
	am(ApplicationManager::GetInstance()) {

	// Get a logger
	std::string logger_name(COREINT_NAMESPACE);
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	assert(logger);

	if (logger)
		logger->Debug("CoreInteractionsTest: %s", logger_name.c_str());
}


CoreInteractionsTest::~CoreInteractionsTest() {

}

// ===================[ Plugin interfaces ]====================================

void * CoreInteractionsTest::Create(PF_ObjectParams *) {
	return new CoreInteractionsTest();
}


int32_t CoreInteractionsTest::Destroy(void * plugin) {
	if (!plugin)
		return -1;
	delete (CoreInteractionsTest *) plugin;
	return 0;
}


void RegisterSomeResources() {

	// Get ResourceAccounter instance
	br::ResourceAccounter & ra = br::ResourceAccounter::GetInstance();

	std::cout << "names=" << res_names.size()
		<< " units=" << res_units.size()
		<< " totals=" << res_totals.size()
		<< std::endl;

	bu::Timer _t(true);
	// Start register resources
	for (uint16_t i=0; i < res_names.size(); ++i) {
		printf(" >>> Registering... :%s\n", res_names[i].c_str());
		ra.RegisterResource(res_names[i], res_units[i],
				res_totals[i]);
	}

	_t.stop();
	std::cout << "\nResources registered in " << _t.getElapsedTimeUs() << " us"
		<< std::endl;

	// Print a tree-like view of the resources
	ra.TreeView();
	std::cout << "Press a key..." << std::endl;
	getchar();
}


// ===================[ Print functions ]=======================================

void PrintResourceAvailabilities(SystemView & sv) {

	std::cout
		<<
		"\n______________________| Resource availabilities |___________________\n"
		<< std::endl;

	// Print resource availabilities info
	for (uint16_t i = 0; i < res_names.size(); ++i) {
		std::cout << std::setw(50) << std::left << res_names[i].c_str() << "| "
			<< std::setw(15) << std::right <<
			sv.ResourceAvailability(res_names[i])
			<< " |" << std::endl;
	}
	std::cout
		<<
		"____________________________________________________________________\n"
		<< std::endl;
}


void PrintScheduleInfo(AppPtr_t & papp) {

	if (!papp) {
		std::cout
			<< "Null application descriptor pointer passed"
			<< std::endl;
		return;
	}

	if (!(papp->CurrentAWM())) {
		std::cout << "! - Current AWM not set - " << std::endl;
	}
	else {
		std::cout
			<< papp->Name().c_str() << ": "
			<< "Curr sched = "
			<< "AWM" << papp->CurrentAWM()->Id()
			<< " " << papp->CurrentAWM()->Name()
			<< " | State " << papp->State() << std::endl;
	}

	if (papp->NextAWM().get() == NULL) {
		std::cout << "[!] Next AWM not set" << std::endl;
	}
	else {
		std::cout
			<< papp->Name().c_str() << ": "
			<< "Next sched = "
			<< "AWM" << papp->NextAWM()->Id()
			<< " " << papp->NextAWM()->Name() << std::endl;
	}

	std::cout << "Press a key..." << std::endl;
	getchar();
}


int PrintWorkingModesInfo(AppPtr_t papp) {

	// Application descriptor pointer is valid?
	if (!papp) {
		std::cout << "Application is NULL" << std::endl;
		return 1;
	}

	// Get all the enabled working modes
	AwmPtrList_t const * awms = papp->WorkingModes();
	if (awms->empty()) {
		std::cout << "Cannot find any working modes" << std::endl;
		return 2;
	}

	// For each working mode...
	AwmPtrList_t::const_iterator wm_it = awms->begin();
	AwmPtrList_t::const_iterator endm = awms->end();
	for (; wm_it != endm; ++wm_it) {

		fprintf(stderr, "\n\n *** AWM%d [ %s ] (value = %d) %d resource usages ***\n",
				(*wm_it)->Id(),
				(*wm_it)->Name().c_str(),
				(*wm_it)->Value(),
				(int)(*wm_it)->ResourceUsages().size());

		std::cout
			<< "\n--------------------------------[ "
			<< (*wm_it)->Id() << " "
			<< (*wm_it)->Name()
			<< " ]----------------------------" << std::endl;

		// Casting to WorkingModeStatusIF
		AwmPtr_t c_awm = *wm_it;

		UsagesMap_t::const_iterator res_it = c_awm->ResourceUsages().begin();
		UsagesMap_t::const_iterator res_end = c_awm->ResourceUsages().end();

		// Print resource usage values
		for ( ; res_it != res_end; ++res_it) {
			std::cout <<
				std::setw(50) << std::left	<< (res_it->first).c_str() << ":"
				<< std::setw(15) << std::right
				<< c_awm->ResourceUsageValue(res_it->first) << " |" << std::endl;
		}

		std::cout
			<< "-------------------------------------------------------------"
			<< "-----" << std::endl;
	}

	AwmPtr_t l_awm = papp->LowValueAWM();
	std::cout
		<< l_awm->Name()
		<< " is the working mode with the lowest value [" << l_awm->Value()
		<< "]" << std::endl;

	std::cout << "Press a key..." << std::endl;
	getchar();

	return 0;
}


// ===================[ Test methods ]========================================

void testPathTemplateSearch(SystemView & sv,
		std::vector<std::string> & rsrc_paths) {

	bu::Timer _t(true);
	std::cout << ".........: Test resource template search :......\n"
		<< std::endl;

	// Make a path template based search of the given resources set
	for (std::vector<std::string>::iterator it = rsrc_paths.begin();
			it != rsrc_paths.end(); ++it) {

		std::cout << std::setw(40) << (*it) << ": ";
		if (sv.ExistResource((*it)))
			std::cout << std::right << "FOUND" << std::endl;
		else
			std::cout << "NOT FOUND" << std::endl;
	}

	_t.stop();
	std::cout << "\nSearch finished in " << _t.getElapsedTimeUs() << " us"
		<< std::endl;

	std::cout << "Press a key..." << std::endl;
	getchar();
}


void testResourceSetSearch(SystemView & sv,
		std::vector<std::string> & rsrc_paths) {

	bu::Timer _t(true);
	std::cout << "_________| Test resource groups search |_______\n"
		<< std::endl;

	// List of matches
	std::list<br::ResourcePtr_t> res_match;

	// Make a path template based search of the given resources set
	for (std::vector<std::string>::iterator rsrc_it = rsrc_paths.begin();
			rsrc_it != rsrc_paths.end(); ++rsrc_it) {

		// How many matchings ?
		res_match = sv.GetResources((*rsrc_it));
		std::cout << "[" << (*rsrc_it) << "] matchings : "
			<< res_match.size() << std::endl;

		// Print out one by one
		br::ResourcePtrList_t::iterator it = res_match.begin();
		br::ResourcePtrList_t::iterator end = res_match.end();
		for(; it != end; ++it)
			std::cout << "\t" << (*it)->Name().c_str() << std::endl;

		// Print amounts information
		std::cout
			<< "\tUSED: "
			<< sv.ResourceUsed((*rsrc_it))
			<< "\tTOT: "
			<< sv.ResourceTotal((*rsrc_it))
			<< "\tAVAIL: "
			<< sv.ResourceAvailability((*rsrc_it))
			<< std::endl << std::endl;
	}

	_t.stop();
	std::cout << "\nSearch finished in " << _t.getElapsedTimeUs() << " us"
		<< std::endl;

	std::cout << "Press a key..." << std::endl;
	getchar();
}


void GetClusteredInfo(SystemView & sv,
		std::vector<std::string> & rsrc_paths) {

	bu::Timer _t(true);
	std::cout
		<< "-----------------| Resource clustering factor |----------------"
		<< std::endl;

	// Test for the clustering factor computation
	for (std::vector<std::string>::iterator it = rsrc_paths.begin();
			it != rsrc_paths.end(); ++it) {

		std::cout
			<< std::setw(40) << std::left
			<< (*it) << " CF = " << sv.ResourceClusterFactor((*it))
			<< std::endl;
	}

	_t.stop();
	std::cout << "\nSearch finished in " << _t.getElapsedTimeUs() << " us"
		<< std::endl;

	std::cout << "Press a key..." << std::endl;
	getchar();
}


void CoreInteractionsTest::testScheduleSwitch(AppPtr_t & papp,
		uint8_t wm_id,
		double ov_time) {

	// Get working mode wm1
	if (papp.get() == NULL) {
		std::cout << "Null application descriptor pointer passed"
			<< std::endl;
		return;
	}

	// Get working mode descriptor related to "wm"
	AwmPtr_t d_wm = papp->GetWorkingMode(wm_id);
	if (d_wm.get() == NULL) {
		std::cout << "Working mode ID=" << wm_id << " not found"
			<< std::endl;
		return;
	}

	// Do a resource binding!
	UsagesMapPtr_t rsrc_binds(new UsagesMap_t);
	d_wm->BindResource("cluster", RSRC_ID_ANY, 1, rsrc_binds);
//	logger->Info("Cluster... %d", d_wm->UsingCluster());
	logger->Debug("Usages / Binds = %d / %d",
			d_wm->ResourceUsages().size(),
			rsrc_binds->size());

	// Let's set next schedule for the application above
	papp->ScheduleRequest(d_wm, rsrc_binds);
	//papp->SetNextSchedule(d_wm, rsrc_binds);
	PrintScheduleInfo(papp);

	// Now switch!
	am._ChangedSchedule(papp, ov_time);
	PrintScheduleInfo(papp);
}


void CoreInteractionsTest::testApplicationLifecycle(AppPtr_t & papp) {

	am.EnableEXC(papp);

	// Print out working modes details
	PrintWorkingModesInfo(papp);

	// Get the application descriptor
	AppPtr_t app_conf(am.GetApplication(3324, 0));

	// Simulate a schedulation 1
	testScheduleSwitch(app_conf, 1, 0.381);
	PrintResourceAvailabilities(sv);

	// Set a constraint
	app_conf->RemoveConstraint("cacheL3", Constraint::UPPER_BOUND);

	// Simulate a schedulation 2
	testScheduleSwitch(app_conf, 2, 0.445);
	PrintResourceAvailabilities(sv);

	// Come back to awm  1
	testScheduleSwitch(app_conf, 1, 0.409);
	PrintResourceAvailabilities(sv);

	// Stop application
	ApplicationManager::ExitCode_t result = am.DestroyEXC(3324);
	if (result == ApplicationManager::AM_SUCCESS)
		logger->Info("Application correctly exited.");
	else
		logger->Info("Error: Application didn't exit correctly"
				" [ExitCode = %d]", result);
	PrintResourceAvailabilities(sv);
}


// =======================================[ Start the test ]==================

void CoreInteractionsTest::Test() {

	logger->Debug("....: CoreInteractions Test starting :.....\n");


	// Resources
	RegisterSomeResources();
	PrintResourceAvailabilities(sv);

	// Some resource search test
	testPathTemplateSearch(sv, rsrcSearchPaths);
	testResourceSetSearch(sv, rsrcSearchPaths);
	GetClusteredInfo(sv, rsrcSearchPaths);

	// Create an Execution Context
	am.CreateEXC("mp3player", 3324, 0, "r1_platA", 0, true);
	AppPtr_t test_app(am.GetApplication(3324, 0));
	if (!test_app) {
		logger->Error("Application not started.");
		return;
	}

	logger->Debug("Applications loaded = %d",
			sv.ApplicationsReady()->size());

	// Plugin specific data
	std::string * auth =
		(static_cast<std::string *>(
					(test_app->GetAttribute("YaMCa", "author")).get()));
	if (auth)
		logger->Info("Plugin YaMCa: <author> : %s", auth->c_str());

/*
	// Scheduler test
	testScheduling();
	PrintResourceAvailabilities(sv);

	// Stop applications
	AppsMap_t::const_iterator apps_it(sv.ApplicationsRunning()->begin());
	AppsMap_t::const_iterator end_apps(sv.ApplicationsRunning()->end());
	for (; apps_it != end_apps; ++apps_it)
		am.StopApplication(apps_it->second);
*/
}

}   // namespace test

}   // namespace bbque

