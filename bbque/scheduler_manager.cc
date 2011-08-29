/**
 *       @file  resoruce_scheduler.cc
 *      @brief  The resource scheduler component of Barbque
 *
 * This module defines the Barbeque interface to resource scheduling policies.
 * The core framework view only methods exposed by this component, which is on
 * charge to find, select and load a proper optimization policy, to run it when
 * required by new events happening (e.g. applications starting/stopping,
 * resources state/availability changes) and considering its internal
 * configurabile policy.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  05/25/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include "bbque/scheduler_manager.h"

#include "bbque/configuration_manager.h"
#include "bbque/plugin_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/system_view.h"

#include "bbque/utils/utility.h"

#define SCHEDULER_MANAGER_NAMESPACE "bq.sm"

namespace bp = bbque::plugins;
namespace po = boost::program_options;

namespace bbque {

SchedulerManager & SchedulerManager::GetInstance() {
	static SchedulerManager rs;
	return rs;
}

SchedulerManager::SchedulerManager() {
	std::string opt_policy;

	//---------- Get a logger module
	bp::LoggerIF::Configuration conf(SCHEDULER_MANAGER_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		fprintf(stderr, "RS: Logger module creation FAILED\n");
		assert(logger);
	}

	logger->Debug("Starting resource scheduler...");

	//---------- Loading module configuration
	ConfigurationManager & cm = ConfigurationManager::GetInstance();
	po::options_description opts_desc("Resource Scheduler Options");
	opts_desc.add_options()
		("SchedulerManager.policy",
		 po::value<std::string>
		 (&opt_policy)->default_value(BBQUE_DEFAULT_SCHEDULER_MANAGER_POLICY),
		 "The name of the optimization policy to use")
		;
	po::variables_map opts_vm;
	cm.ParseConfigurationFile(opts_desc, opts_vm);

	//---------- Load the required optimization plugin
	std::string opt_namespace(SCHEDULER_POLICY_NAMESPACE);
	logger->Debug("Loading optimization policy [%s%s]...",
			opt_namespace.c_str(), opt_policy.c_str());
	policy = ModulesFactory::GetSchedulerPolicyModule(
			opt_namespace + opt_policy);
	if (!policy) {
		logger->Fatal("Optimization policy load FAILED "
			"(Error: missing plugin for [%s%s])",
			opt_namespace.c_str(), opt_policy.c_str());
		assert(policy);
	}


}

SchedulerManager::~SchedulerManager() {
}

SchedulerManager::ExitCode_t
SchedulerManager::Schedule() {
	SystemView &sv = SystemView::GetInstance();
	SchedulerPolicyIF::ExitCode result;

	if (!policy) {
		logger->Crit("Resource scheduling FAILED (Error: missing policy)");
		assert(policy);
		return MISSING_POLICY;
	}

	// TODO add here proper tracing/monitoring events
	// for statistics collection

	// TODO here should be plugged a scheduling decision policy
	// Such a policy should decide whter a scheduling sould be run
	// or not, e.g. based on the kind of READY applications or considering
	// stability problems and scheduling overheads.
	// In case of a scheduling is not considered safe proper at this time,
	// a DELAYED exit code should be returned
	logger->Warn("TODO: add scheduling activation policy");

	// Call the current optimization plugin scheduling policy
	logger->Info("Resources scheduling, policy [%s]...",
			policy->Name());

	result = policy->Schedule(sv);
	if (result != SchedulerPolicyIF::SCHED_DONE) {
		logger->Error("Scheduliung policy [%s] failed",
				policy->Name());
		return FAILED;
	}

	return DONE;
}

} // namespace bbque

