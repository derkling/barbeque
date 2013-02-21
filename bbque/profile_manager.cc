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

#include "bbque/profile_manager.h"
#include "bbque/modules_factory.h"
#include "bbque/app/working_mode.h"
#include "bbque/app/application.h"

#include "bbque/utils/utility.h"

#define PROFILE_MANAGER_NAMESPACE "bq.om"

static const char *prioLevels[] = {
	"Prio level",
	NULL
};

/** Metrics (class SAMPLE) declaration */
#define PM_SAMPLE_METRIC(NAME, DESC)\
 {PROFILE_MANAGER_NAMESPACE "." NAME, DESC, MetricsCollector::SAMPLE, \
	 BBQUE_APP_PRIO_LEVELS, prioLevels, 0}
/** Acquire a new (generic) sample */
#define PM_ADD_SAMPLE(METRICS, INDEX, SAMPLE, PRIO) \
	mc.AddSample(METRICS[INDEX].mh, SAMPLE, PRIO);


namespace bu = bbque::utils;
namespace bp = bbque::plugins;

namespace bbque {

/* Definition of metrics used by this module */
MetricsCollector::MetricsCollection_t
ProfileManager::metrics[PM_METRICS_COUNT] = {

	//----- Sampling statistics
	PM_SAMPLE_METRIC("sch.appv", "Schedule applications value"),
	PM_SAMPLE_METRIC("sch.awmv", "Schedule AWMs value"),
	PM_SAMPLE_METRIC("sch.frns", "Schedule fairness"),
	PM_SAMPLE_METRIC("sch.wmix", "Schedule workload mix")

};


ProfileManager & ProfileManager::GetInstance() {
	static ProfileManager rtrm;
	return rtrm;
}

ProfileManager::ProfileManager() :
	sm(SchedulerManager::GetInstance()),
	am(ApplicationManager::GetInstance()),
	mc(bu::MetricsCollector::GetInstance()) {

	//---------- Get a logger module
	std::string logger_name(PROFILE_MANAGER_NAMESPACE);
	bp::LoggerIF::Configuration conf(logger_name.c_str());
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		fprintf(stderr, "OM: Logger module creation FAILED\n");
		assert(logger);
		return;
	}

	logger->Debug("Starting profile manager...");

	//---------- Setup all the module metrics
	mc.Register(metrics, PM_METRICS_COUNT);

}

ProfileManager::~ProfileManager() {
}


ProfileManager::ExitCode_t
ProfileManager::ProfileScheduleClass(uint16_t prio) {
	accumulator_set<double, stats<tag::min, tag::max, tag::variance>>
		appValueStats, awmValueStats;
	uint16_t actives_count = 0;
	uint16_t running_count = 0;
	double app_avg = 0, app_var = 0;
	double awm_avg = 0, awm_var = 0;
	double wmix_idx = 0;
	double fnes_idx = 0;
	AppsUidMapIt app_it;
	AppPtr_t papp;

	// Profiling ACTIVE applications
	papp = am.GetFirst(prio, app_it);
	while (papp) {
		if (!papp->Active())
			goto loop_continue;
		++actives_count;

		//logger->Debug("Prio[%d], accounting [%s] as ACTIVE",
		//		prio, papp->StrId(), papp->Value());

		// Stats are computed just on RUNNING applications
		if (papp->State() != ApplicationStatusIF::RUNNING)
			goto loop_continue;
		++running_count;

		//logger->Debug("Prio[%d], adding [%s] to stats, value [%.4f]",
		//		prio, papp->StrId(), papp->Value());
		appValueStats(papp->Value());
		awmValueStats(papp->CurrentAWM()->Value());
loop_continue:
		papp = am.GetNext(prio, app_it);
	}

	// We could have applications on a prio level which are just
	// BLOCKED or DISABLED
	if (actives_count == 0)
		return OK;

	// Computing statistics on Applications Value
	app_avg = mean(appValueStats);
	app_var = variance(appValueStats);

	// Computing statistics on AWMs Value
	awm_avg = mean(awmValueStats);
	awm_var = variance(awmValueStats);

	// Workload Mix INDEX: WMix = Apps[RUNNING] / Apps[ACTIVE]
	wmix_idx = static_cast<double>(running_count) / actives_count;

	// Fairness INDEX: F = WMix / (1 + Sched[VARIANCE])
	fnes_idx = wmix_idx / (1 + app_var);


	// Adding SAMPLES to metrics collector
	PM_ADD_SAMPLE(metrics, PM_SCHED_APP_VALUE, app_avg,
			static_cast<uint8_t>(prio));
	PM_ADD_SAMPLE(metrics, PM_SCHED_AWM_VALUE, awm_avg,
			static_cast<uint8_t>(prio));
	PM_ADD_SAMPLE(metrics, PM_SCHED_FAIRNESS, fnes_idx,
			static_cast<uint8_t>(prio));
	PM_ADD_SAMPLE(metrics, PM_SCHED_WORKLOAD_MIX, wmix_idx,
			static_cast<uint8_t>(prio));

	logger->Notice(
		"|  %3d | %3d | %3d | %5.3f | %5.3f | %5.3f | %5.3f | %5.3f | %5.3f |",
		prio,
		actives_count,
		running_count,
		app_avg, app_var,
		awm_avg, awm_var,
		wmix_idx,
		fnes_idx
	);

	return OK;
}

ProfileManager::ExitCode_t
ProfileManager::ProfileSchedule() {
	uint16_t prio;

	logger->Notice(
		"====================================================================");
	logger->Notice(
		"|      |  Apps Cnt |  Apps Values  |  AWMs Values  | WLMix | Fness |");
	logger->Notice(
		"| Prio | Act | Run |  Avg  |  Var  |  Avg  |  Var  |   Idx |   Idx |");
	logger->Notice(
		"|------+-----+-----+-------+-------+-------+-------+-------+-------+");

	// Compute per-priority classes scheduler profiling statistics
	for (prio = 0; prio <= am.LowestPriority(); ++prio) {
		if (!am.HasApplications(prio))
			continue;
		ProfileScheduleClass(prio);
	}

	logger->Notice(
		"====================================================================");
	return OK;
}

} // namespace bbque
