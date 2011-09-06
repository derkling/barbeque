/**
 *       @file  metrics_collector.cc
 *      @brief  A class to collect metrics
 *
 * Metrics allows to account and analyze different system parameters. This is a
 * base class which provides a centralized repository for system metrics which
 * could be dynamically defined, updated and queryed.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  08/24/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "bbque/utils/metrics_collector.h"

#include "bbque/modules_factory.h"

#define METRICS_COLLECTOR_NAMESPACE "bq.mc"

namespace bp = bbque::plugins;

namespace bbque { namespace utils {

MetricsCollector & MetricsCollector::GetInstance() {
	static MetricsCollector mc;
	return mc;
}

MetricsCollector::MetricsCollector() {

	//---------- Get a logger module
	bp::LoggerIF::Configuration conf(METRICS_COLLECTOR_NAMESPACE);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		fprintf(stderr, "MC: Logger module creation FAILED\n");
		assert(logger);
	}

	logger->Debug("Starting metrics collector...");
}

MetricsCollector::~MetricsCollector() {
}


MetricsCollector::MetricHandler_t
MetricsCollector::GetHandler(const char *name) {
	return std::hash<const char *>()(name);
}

MetricsCollector::pMetric_t
MetricsCollector::GetMetric(MetricHandler_t hdlr) {
	MetricsMap_t::iterator it = metricsMap.find(hdlr);
	// Lookup for metrics
	if (it != metricsMap.end())
		return (*it).second;
	// Return NULL pointer
	return pMetric_t();
}

MetricsCollector::pMetric_t
MetricsCollector::GetMetric(const char *name) {
	MetricHandler_t hdlr = GetHandler(name);
	return GetMetric(hdlr);
}

MetricsCollector::ExitCode_t
MetricsCollector::Register(const char *name, const char *desc,
		MetricClass_t mc, MetricHandler_t & mh) {
	std::unique_lock<std::mutex> metrics_ul(metrics_mtx);
	pMetric_t pm = GetMetric(name);

	// Check if the metric has not yet been registered
	if (pm) {
		logger->Error("Metric [%s] registration FAILED "
				"(Error: metric already registered)", name);
		DB(assert(!pm));
		return DUPLICATE;
	}

	// Build a new metric container
	assert(!pm);
	switch(mc) {
	case COUNTER:
		pm = pMetric_t(new CounterMetric(name, desc));
		break;
	case VALUE:
		pm = pMetric_t(new ValueMetric(name, desc));
		break;
	case SAMPLE:
		pm = pMetric_t(new SamplesMetric(name, desc));
		break;
	default:
		logger->Error("Metric [%s] registration FAILED "
				"(Error: metric class not supported)", name);
		return UNSUPPORTED;
	}

	// Compute the metric handler
	mh = GetHandler(name);

	// Save the metric containter into proper map
	assert(mc < CLASSES_COUNT);
	metricsMap.insert(MetricsMapEntry_t(mh, pm));
	metricsVec[mc].insert(MetricsMapEntry_t(mh, pm));

	logger->Debug("New metrics [%s:%s => %s] registered",
			metricClassName[pm->mc], pm->name, pm->desc);

	return OK;
}

MetricsCollector::ExitCode_t
MetricsCollector::Register(MetricsCollection_t *mc, uint8_t count) {
	ExitCode_t result;
	uint8_t idx;

	for (idx = 0; idx < count; idx++) {
		result = Register(mc[idx].name, mc[idx].desc,
				mc[idx].mc, mc[idx].mh);
		if (result != OK) {
			logger->Error("Metrics collection registration FAILED");
			return result;
		}
	}

	return OK;
}

MetricsCollector::ExitCode_t
MetricsCollector::Count(MetricHandler_t mh, uint64_t amount) {
	pMetric_t pm = GetMetric(mh);
	CounterMetric_t *m;

	// Check if the metric has not yet been registered
	if (!pm) {
		logger->Error("Counting FAILED "
				"(Error: metric not registered)");
		return UNKNOWEN;
	}

	// Check the metrics is of compatible type
	if (pm->mc != COUNTER) {
		logger->Error("Counting FAILED "
				"(Error: wrong metric class)");
		assert(pm->mc == COUNTER);
		return UNSUPPORTED;
	}

	// Lock this metric
	std::unique_lock<std::mutex> ul(pm->mtx);

	// Get the SAMPLE metrics
	m = (CounterMetric_t*)pm.get();

	// Increase the counter for the specified value
	m->count += amount;

	return OK;
}

MetricsCollector::ExitCode_t
MetricsCollector::UpdateValue(MetricHandler_t mh, double amount) {
	pMetric_t pm = GetMetric(mh);
	ValueMetric_t *m;

	// Check if the metric has not yet been registered
	if (!pm) {
		logger->Error("Value update FAILED "
				"(Error: metric not registered)");
		return UNKNOWEN;
	}

	// Check the metrics is of compatible type
	if (pm->mc != VALUE) {
		logger->Error("Value update FAILED "
				"(Error: wrong metric class)");
		assert(pm->mc == VALUE);
		return UNSUPPORTED;
	}

	// Lock this metric
	std::unique_lock<std::mutex> ul(pm->mtx);

	// Get the SAMPLE metrics
	m = (ValueMetric_t*)pm.get();

	// Update the value if not zero, otherwise reset it
	if (amount)
		m->value += amount;
	else
		m->value = 0;

	m->stat(m->value);
	return OK;
}


MetricsCollector::ExitCode_t
MetricsCollector::Add(MetricHandler_t mh, double amount) {
	if (!amount)
		return OK;

	return UpdateValue(mh, amount);
}

MetricsCollector::ExitCode_t
MetricsCollector::Remove(MetricHandler_t mh, double amount) {
	if (!amount)
		return OK;

	return UpdateValue(mh, -amount);
}

MetricsCollector::ExitCode_t
MetricsCollector::Reset(MetricHandler_t mh) {
	return UpdateValue(mh, 0);
}



MetricsCollector::ExitCode_t
MetricsCollector::AddSample(MetricHandler_t mh, double sample) {
	pMetric_t pm = GetMetric(mh);
	SamplesMetric_t *m;

	// Check if the metric has not yet been registered
	if (!pm) {
		logger->Error("Add sample FAILED "
				"(Error: metric not registered)");
		return UNKNOWEN;
	}

	// Check the metrics is of compatible type
	if (pm->mc != SAMPLE) {
		logger->Error("Add sample FAILED "
				"(Error: wrong metric class)");
		assert(pm->mc == SAMPLE);
		return UNSUPPORTED;
	}

	// Lock this metric
	std::unique_lock<std::mutex> ul(pm->mtx);

	// Get the SAMPLE metrics
	m = (SamplesMetric_t*)pm.get();

	// Push-in the new sample into the accumulator
	m->stat(sample);

	return OK;
}

void
MetricsCollector::DumpCounter(CounterMetric_t *m) {
	logger->Notice(
		" %-20s | %9ld : %s",
		m->name, m->count, m->desc);
}

void
MetricsCollector::DumpValue(ValueMetric_t *m) {
	double _min = 0, _max = 0;

	if (count(m->stat)) {
		_min = min(m->stat);
		_max = max(m->stat);
	}
	logger->Notice(
		" %-20s | %9ld | %9ld | %9ld : %s",
		m->name, m->value, _min, _max, m->desc);
}

void
MetricsCollector::DumpSample(SamplesMetric_t *m) {
	double _min = 0, _max = 0, _avg = 0, _var = 0;

	if (count(m->stat)) {
		_min = min(m->stat);
		_max = max(m->stat);
		_avg = mean(m->stat);
		_var = variance(m->stat);
	}
	logger->Notice(
		" %-20s | %9.3f | %9.3f | %9.3f | %9.3f : %s",
		m->name, _min, _max, _avg, ::sqrt(_var), m->desc);

}

#define METRICS_COUNTER_HEADER \
"  Metric              |  Count    |  Description"
#define METRICS_COUNTER_SEPARATOR \
"----------------------+-----------+----------------------"

#define METRICS_VALUE_HEADER \
"  Metric              |  Value    |  Min      |  Max      |  Description"
#define METRICS_VALUE_SEPARATOR \
"----------------------+-----------+-----------+-----------+----------------------"

#define METRICS_SAMPLES_HEADER \
"  Metric              |  Min      |  Max      |  Avg      |  StdDev   |  Description"
#define METRICS_SAMPLES_SEPARATOR \
"----------------------+-----------+-----------+-----------+-----------+----------------------"

void
MetricsCollector::DumpMetrics() {
	MetricsMap_t::iterator it;

	logger->Notice("");
	logger->Notice("==========[ Counter Metrics ]=========="
			"========================================");
	logger->Notice("");

	// Dumping COUTNER metrics
	logger->Notice(METRICS_COUNTER_HEADER);
	logger->Notice(METRICS_COUNTER_SEPARATOR);
	it = metricsVec[COUNTER].begin();
	for ( ; it != metricsVec[COUNTER].end(); ++it) {
		DumpCounter((CounterMetric_t*)(((*it).second).get()));
	}
	logger->Notice(METRICS_COUNTER_SEPARATOR);


	logger->Notice("");
	logger->Notice("==========[ Value Metrics ]============"
			"========================================");
	logger->Notice("");

	// Dumping VALUE metrics
	logger->Notice(METRICS_VALUE_HEADER);
	logger->Notice(METRICS_VALUE_SEPARATOR);
	it = metricsVec[VALUE].begin();
	for ( ; it != metricsVec[VALUE].end(); ++it) {
		DumpValue((ValueMetric_t*)(((*it).second).get()));
	}
	logger->Notice(METRICS_VALUE_SEPARATOR);


	logger->Notice("");
	logger->Notice("==========[ Sample Metrics ]==========="
			"========================================");
	logger->Notice("");

	// Dumping SAMPLES metrics
	logger->Notice(METRICS_SAMPLES_HEADER);
	logger->Notice(METRICS_SAMPLES_SEPARATOR);
	it = metricsVec[SAMPLE].begin();
	for ( ; it != metricsVec[SAMPLE].end(); ++it) {
		DumpSample((SamplesMetric_t*)(((*it).second).get()));
	}
	logger->Notice(METRICS_SAMPLES_SEPARATOR);

}


const char *
MetricsCollector::metricClassName[MetricsCollector::CLASSES_COUNT] = {
	"Counter",
	"Value",
	"Samples"
};


} // namespace utils

} // namespace bbque

