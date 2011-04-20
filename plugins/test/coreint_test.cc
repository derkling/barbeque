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

#include <iomanip>
#include <map>
#include <vector>
#include <cstdint>

#include "bbque/modules_factory.h"
#include "bbque/system_view.h"
#include "bbque/app/recipe.h"
#include "bbque/app/working_mode.h"
#include "bbque/app/plugin_data.h"
#include "bbque/res/resources.h"
#include "bbque/utils/timer.h"

namespace ba = bbque::app;
namespace bp = bbque::plugins;
namespace bu = bbque::utils;

namespace bbque { namespace plugins {

/** Map of application descriptor shared pointers */
typedef std::map<uint32_t, ba::AppPtr_t> AppsMap_t;

// Test set
std::vector<std::string> res_names = {
	"mem0",
	"dma0",
	"arch.mem0",
	"arch.clusters.mem0",
	"arch.clusters.cluster0",
	"arch.clusters.cluster0.mem0",
	"arch.clusters.cluster0.dma0",
	"arch.clusters.cluster0.pe0",
	"arch.clusters.cluster1.pe0",
	"arch.clusters.cluster2.pe0",
	"arch.clusters.cluster3.pe0",
	"arch.clusters.cluster0.pe0.mem0",
	"arch.clusters.cluster0.pe0.mem1",
	"arch.clusters.cluster0.pe0.mem2",
	"arch.clusters.cluster1.pe1",
	"arch.clusters.cluster1.pe2",
	"arch.clusters.cluster1.pe0",
	"arch.clusters.cluster1.pe0.mem0",
	"arch.clusters.cluster1.mem0",
	"arch.clusters.cluster0.pe1",
	"arch.clusters.cluster2.mem0",
	"arch.clusters.cluster1.mem0",
	"arch.clusters.cluster2.pe0.mem0",
	"arch.clusters.cluster0.pe1.mem1",
	"arch.clusters.cluster0.pe1.mem0",
	"arch.clusters.cluster1.pe2.mem0"
};

std::vector<std::string> res_types = {
	"SDRAM memory",
	"bus",
	"SDRAM memory",
	"SDRAM memory",
	"cluster",
	"SDRAM memory",
	"bus",
	"cpu",
	"cpu",
	"cpu",
	"cpu",
	"memory",
	"memory",
	"memory",
	"cpu",
	"cpu",
	"cpu",
	"memory",
	"memory",
	"cpu",
	"memory",
	"memory",
	"memory",
	"memory",
	"memory",
	"memory"
};

std::vector<std::string> res_units = {
	"Mb",
	"Mbps",
	"Mb",
	"Mb",
	"1",
	"Mb",
	"Mbps",
	"1",
	"1",
	"1",
	"1",
	"kb",
	"kb",
	"kb",
	"1",
	"1",
	"1",
	"kb",
	"Mb",
	"1",
	"Mb",
	"Mb",
	"kb",
	"kb",
	"kb",
	"kb"
};

std::vector<uint64_t> res_totals = {
	256, // System RAM
	200,  // DMA system bus
	32,   // Memory architecture level
	16,   // Memory inter-cluster level
	4,    // Number of pe in cluster 0
	8,    // Memory in cluster 0
	50,   // DMA for cluster 0
	1,    // Num of pe0 in cluster0
	1,    // Num of pe1 in cluster1
	1,    // Num of pe2 in cluster2
	1,    // Num of pe3 in cluster3
	512,  // cluster1.pe0.mem0
	512,  // cluster
	512,
	1,
	1,
	1,
	512,
	4,
	1,
	8,
	8,
	512,
	512,
	512,
	512
};


CoreInteractionsTest::CoreInteractionsTest():
	bbque::Object(COREINT_NAMESPACE) {

	// Get a logger
	std::string logger_name(COREINT_NAMESPACE);
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger =
		std::unique_ptr<bp::LoggerIF>
		(ModulesFactory::GetLoggerModule(std::cref(conf)));

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


// ===================[ Test functions ]=======================================

void CoreInteractionsTest::RegisterSomeResources() {

	// Get ResourceAccounter instance
	bbque::res::ResourceAccounter * RA =
		bbque::res::ResourceAccounter::GetInstance();

	if (RA == NULL) {
		printf("Error: ResourceAccounter missing");
		return;
	}

	bu::Timer _t(true);
	// Start register resources
	for (uint16_t i=0; i < res_names.size(); ++i) {
		printf(" >>> Registering... :%s\n", res_names[i].c_str());
		RA->RegisterResource(res_names[i], res_types[i], res_units[i],
				res_totals[i]);
	}

	_t.stop();
	std::cout << "\nResources registered in " << _t.getElapsedTimeUs() << " us"
		<< std::endl;

	// Print a tree-like view of the resources
	RA->TreeView();
	std::cout << "Press a key..." << std::endl;
	getchar();
}


void CoreInteractionsTest::PrintResourceAvailabilities() {

	bbque::res::ResourceAccounter * RA =
		bbque::res::ResourceAccounter::GetInstance();

	if (RA == NULL) {
		printf("Error: ResourceAccounter missing");
		return;
	}

	std::cout
		<<
		"\n______________________| Resource availabilities |___________________\n"
		<< std::endl;

	// Print resource availabilities info
	for (uint16_t i = 0; i < res_names.size(); ++i) {
		std::cout << std::setw(50) << std::left << res_names[i].c_str() << "| "
			<< std::setw(15) << std::right << RA->Available(res_names[i])
			<< " |" << std::endl;
	}
	std::cout
		<<
		"____________________________________________________________________\n"
		<< std::endl;
}


int CoreInteractionsTest::WorkingModesDetails(
		std::shared_ptr<ba::ApplicationStatusIF> test_app,
		std::vector<std::string> resources) {

	// Application descriptor pointer is valid?
	if (!test_app) {
		std::cout << "Application is NULL" << std::endl;
		return 1;
	}

	// Get all the enabled working modes
	ba::AwmStatusPtrList_t awms = test_app->WorkingModes();

	if (awms.size() == 0) {
		std::cout << "Cannot find any working modes" << std::endl;
		return 2;
	}

	// For each of them print out resources usages
	std::list<ba::AwmStatusPtr_t>::iterator it2 =  awms.begin();
	std::list<ba::AwmStatusPtr_t>::iterator endm = awms.end();
	for (; it2 != endm; ++it2) {
		fprintf(stderr, "\n\n *** [ %s ] (value = %d) %d resource usages ***\n",
				(*it2)->Name().c_str(), (*it2)->Value(),
				(int)(*it2)->ResourceUsages().size());

		ba::AwmStatusPtr_t awm_get(*it2);

		std::vector<std::string>::iterator res_it = resources.begin();
		std::vector<std::string>::iterator res_end = resources.end();

		std::cout
			<< "\n--------------------------------[ " << awm_get->Name()
			<< " ]----------------------------" << std::endl;

		for ( ; res_it < res_end; ++res_it) {
			std::cout << std::setw(50) << std::left << (*res_it).c_str() << ":"
				<< std::setw(15) << std::right
				<< awm_get->ResourceUsage(*res_it) << " |" << std::endl;
		}
		std::cout
			<< "-------------------------------------------------------------"
			<< "-----" << std::endl;
	}

	ba::AwmStatusPtr_t l_awm = test_app->LowValueAWM();
	std::cout << l_awm->Name() << " is the working mode with the lowest "
		"value [" << l_awm->Value() << "]" << std::endl;

	std::cout << "Press a key..." << std::endl;
	getchar();

	return 0;
}


void CoreInteractionsTest::PrintScheduleInfo(
		std::shared_ptr<ba::Application> test_app) {

	if (test_app.get() == NULL) {
		std::cout << "Null application descriptor pointer passed" <<
			std::endl;
		return;
	}

	std::cout << std::endl
		<< "**************************************************"
		<< std::endl;

	if (test_app->CurrentAWM().get() == NULL) {
		std::cout << "[!] Current AWM not set" << std::endl;
	}
	else {
		std::cout << "Current schedule of " << test_app->Name().c_str() << " is "
			<< test_app->CurrentAWM()->Name() << " "
			<< test_app->CurrentState() << std::endl;
	}

	if (test_app->NextAWM().get() == NULL) {
		std::cout << "[!] Next AWM not set" << std::endl;
	}
	else {
		std::cout << "Next schedule of " << test_app->Name().c_str() << " is "
			<< test_app->NextAWM()->Name() << " "
			<< test_app->NextState() << std::endl;
	}

	std::cout
		<< "**************************************************"
		<< std::endl << std::endl;

	std::cout << "Press a key..." << std::endl;
	getchar();
}


void CoreInteractionsTest::DoScheduleSwitch(
		std::shared_ptr<ba::Application> test_app,
		std::string const & wm, double ov_time) {

	// Get working mode wm1
	if (test_app.get() == NULL) {
		std::cout << "Null application descriptor pointer passed"
			<< std::endl;
		return;
	}
	// Get working mode descriptor related to "wm"
	ba::AwmPtr_t d_wm = test_app->GetRecipe()->WorkingMode(wm);
	if (d_wm.get() == NULL)
		return;

	// Let's set next schedule for the application above
	test_app->SetNextSchedule(d_wm->Name(),	ba::Application::RUNNING);
	PrintScheduleInfo(test_app);

	// Now switch!
	ApplicationManager * appman = ApplicationManager::GetInstance();
	appman->ChangedSchedule(test_app, ov_time);
	PrintScheduleInfo(test_app);
}


void SearchResources(SystemView * sv) {

	bu::Timer _t(true);
	std::cout << ".........: Test resource template search :......\n"
		<< std::endl;

	std::cout << "arch.clusters.mem : ";
	if (sv->ExistResource("arch.clusters.mem"))
		std::cout << "FOUND" << std::endl;
	else
		std::cout << "NOT FOUND" << std::endl;

	std::cout << "arch.clusters.cluster1 : ";
	br::ResourcePtr_t res_ptr = sv->GetResource("arch.clusters.cluster1");
	if (res_ptr.get() == 0)
		std::cout << "FOUND" << std::endl;
	else
		std::cout << "NOT FOUND" << std::endl;

	std::cout << "arch.clusters.cluster : ";
	if (sv->ExistResource("arch.clusters.cluster"))
		std::cout << "FOUND" << std::endl;
	else
		std::cout << "NOT FOUND" << std::endl;

	std::cout << "arch.clusters.cluster.pe : ";
	if (sv->ExistResource("arch.clusters.cluster.pe"))
		std::cout << "FOUND" << std::endl;
	else
		std::cout << "NOT FOUND" << std::endl;

	std::cout << "arch.clusters.pe : ";
	if (sv->ExistResource("arch.clusters.pe"))
		std::cout << "FOUND" << std::endl;
	else
		std::cout << "NOT FOUND" << std::endl;

	std::cout << "dma : ";
	if (sv->ExistResource("dma"))
		std::cout << "FOUND" << std::endl;
	else
		std::cout << "NOT FOUND" << std::endl;

	std::cout << "pci : ";
	if (sv->ExistResource("pci"))
		std::cout << "FOUND" << std::endl;
	else
		std::cout << "NOT FOUND" << std::endl;

	_t.stop();
	std::cout << "\nSearch finished in " << _t.getElapsedTimeUs() << " us"
		<< std::endl;

	std::cout << "Press a key..." << std::endl;
	getchar();
}


void GetClusteredInfo(SystemView * sv) {

	std::cout
		<< "-----------------| Resource clustering factor |----------------"
		<< std::endl;

	std::cout << "dma  c.f. = "
		<< sv->ResourceClusterFactor("dma") << std::endl;
	std::cout << "arch.clusters.mem0 	c.f. = "
		<< sv->ResourceClusterFactor("arch.clusters.mem0") << std::endl;
	std::cout << "arch.clusters.cluster1 c.f. = "
		<< sv->ResourceClusterFactor("arch.clusters.cluster1") << std::endl;
	std::cout << "arch.mem0 c.f. = "
		<< sv->ResourceClusterFactor("arch.mem0") << std::endl;
	std::cout << "arch.clusters.cluster0.pe0  c.f. = "
		<< sv->ResourceClusterFactor("arch.clusters.cluster0.pe0") << std::endl;
	std::cout << "arch.spi  c.f. = "
		<< sv->ResourceClusterFactor("arch.spi") << std::endl;
	std::cout << std::endl;
}


void SearchResourceGroups(SystemView * sv) {

	bu::Timer _t(true);
	std::cout << "_________| Test resource groups search |_______\n"
		<< std::endl;


	// List of matches
	std::list<br::ResourcePtr_t> res_match;

	// Search... cluster
	res_match = sv->GetResources("arch.clusters.cluster");
	std::cout << "[arch.clusters.cluster] matchings : "
		<< res_match.size() << std::endl;

	br::ResourcePtrList_t::iterator it = res_match.begin();
	br::ResourcePtrList_t::iterator end = res_match.end();
	for(; it != end; ++it)
		std::cout << "\t" << (*it)->Name().c_str() << std::endl;

	// Search... cluster.pe
	res_match = sv->GetResources("arch.clusters.cluster.pe");
	std::cout << "[arch.clusters.cluster.pe] matchings : "
		<< res_match.size() << std::endl;
	std::cout << "\tavailability = "
		<< sv->ResourceAvailability("arch.clusters.cluster.pe") << std::endl;
	std::cout << "\tused = "
		<< sv->ResourceUsed("arch.clusters.cluster.pe") << std::endl;
	std::cout << "\ttotal = "
		<< sv->ResourceTotal("arch.clusters.cluster.pe") << std::endl;

	it = res_match.begin();
	end = res_match.end();
	for(; it != end; ++it)
		std::cout << "\t" << (*it)->Name().c_str() << std::endl;

	// Search... pci
	res_match = sv->GetResources("arch.clusters.pci");
	std::cout << "[arch.clusters.pci] matchings : "
		<< res_match.size() << std::endl;

	it = res_match.begin();
	end = res_match.end();
	for(; it != end; ++it)
		std::cout << "\t" << (*it)->Name().c_str() << std::endl;

	// Search... mem
	res_match = sv->GetResources("mem");
	std::cout << "[mem] matchings : "
		<< res_match.size() << std::endl;

	it = res_match.begin();
	end = res_match.end();
	for(; it != end; ++it)
		std::cout << "\t" << (*it)->Name().c_str() << std::endl;

	_t.stop();
	std::cout << "\nSearch finished in " << _t.getElapsedTimeUs() << " us"
		<< std::endl;

	std::cout << "Press a key..." << std::endl;
	getchar();
}


// =======================================[ Start the test ]==================

void CoreInteractionsTest::Test() {

	logger->Debug("....: CoreInteractions Test starting :.....\n");

	// Resources
	RegisterSomeResources();
	PrintResourceAvailabilities();

	// ApplicationManager instance
	ApplicationManager *appman = ApplicationManager::GetInstance();

	if (!appman) {
		logger->Error("Cannot find the application manager");
		return;
	}
	// Start an application
	appman->StartApplication("mp3player", 3324, 0, "r1_platA", 0, true);

	std::shared_ptr<ba::ApplicationStatusIF>
		test_app(appman->GetApplication(3324));

	if (!test_app) {
		logger->Error("Application not started.");
		return;
	}
	// SystemView
	SystemView *sv = SystemView::GetInstance();
	logger->Debug("Applications loaded = %d", sv->ApplicationsReady()->size());

	// Plugin specific data
	ba::PluginsDataContainer pdc;
	ba::PluginDataPtr_t pdata = test_app->GetPluginData("YaMCa");

	if (pdata.get() == NULL) {
		logger->Warn("Unable to get plugin info");
	}
	else {
		std::string pl_author;
		if ((pdata->Get("author", pl_author)) == ba::PluginData::PDATA_SUCCESS)
			std::cout << "Plugin YaMCa - Author :" << pl_author << std::endl;
	}

	// Print out working modes details
	WorkingModesDetails(test_app, res_names);

	// Get the application descriptor
	std::shared_ptr<ba::Application> app_conf(appman->GetApplication(3324));

	// Simulate a schedulation 1
	DoScheduleSwitch(app_conf, "wm1", 0.381);
	PrintResourceAvailabilities();

	// Set a constraint
	app_conf->RemoveConstraint("cacheL3", ba::Constraint::UPPER_BOUND);

	// Simulate a schedulation 2
	DoScheduleSwitch(app_conf, "wm2", 0.445);
	PrintResourceAvailabilities();

	// Come back to awm 1
	DoScheduleSwitch(app_conf, "wm1", 0.409);
	PrintResourceAvailabilities();

	// Stop application
	appman->StopApplication(3324);
	PrintResourceAvailabilities();

	// Some resource search test
	SearchResources(sv);
	SearchResourceGroups(sv);
	GetClusteredInfo(sv);

	delete appman;
}

}   // namespae test

}   // namespace bbque

