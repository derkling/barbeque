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

#include "bbque/utils/metrics_collector.h"

#include "bbque/modules_factory.h"

#define METRICS_COLLECTOR_NAMESPACE "bq.mc"

namespace bp = bbque::plugins;

namespace bbque { namespace utils {

MetricsCollector::CounterMetric::CounterMetric(
		const char *name, const char *desc,
		uint8_t sm_count, const char **sm_desc) :
	Metric(name, desc, COUNTER, sm_count, sm_desc),
	cnt(0), sm_cnt(sm_count) {

}

void MetricsCollector::CounterMetric::Reset() {
	cnt = 0;
	for (uint8_t i = 0; i < sm_cnt.size(); ++i)
		sm_cnt[i] = 0;
}

MetricsCollector::ValueMetric::ValueMetric(
		const char *name, const char *desc,
		uint8_t sm_count, const char **sm_desc) :
	Metric(name, desc, VALUE, sm_count, sm_desc),
	value(0), sm_value(sm_count), sm_pstat(sm_count) {
	pstat = pStatMetric_t(new statMetric_t);
	for (uint8_t i = 0; i < sm_pstat.size(); ++i) {
		sm_value[i] = 0;
		sm_pstat[i] = pStatMetric_t(new statMetric_t);
	}
}

void MetricsCollector::ValueMetric::Reset() {
	value = 0;
	pstat = pStatMetric_t(new statMetric_t);
	for (uint8_t i = 0; i < sm_pstat.size(); ++i) {
		sm_value[i] = 0;
		sm_pstat[i] = pStatMetric_t(new statMetric_t);
	}
}

MetricsCollector::SamplesMetric::SamplesMetric(
		const char *name, const char *desc,
		uint8_t sm_count, const char **sm_desc) :
	Metric(name, desc, SAMPLE, sm_count, sm_desc),
	sm_pstat(sm_count) {
	pstat = pStatMetric_t(new statMetric_t);
	for (uint8_t i = 0; i < sm_pstat.size(); ++i) {
		sm_pstat[i] = pStatMetric_t(new statMetric_t);
	}
}

void MetricsCollector::SamplesMetric::Reset() {
	pstat = pStatMetric_t(new statMetric_t);
	for (uint8_t i = 0; i < sm_pstat.size(); ++i) {
		sm_pstat[i] = pStatMetric_t(new statMetric_t);
	}
}

MetricsCollector::PeriodMetric::PeriodMetric(
		const char *name, const char *desc,
		uint8_t sm_count, const char **sm_desc) :
	Metric(name, desc, PERIOD, sm_count, sm_desc),
	sm_period_tmr(sm_count), sm_pstat(sm_count) {
	pstat = pStatMetric_t(new statMetric_t);
	for (uint8_t i = 0; i < sm_pstat.size(); ++i) {
		sm_pstat[i] = pStatMetric_t(new statMetric_t);
	}
}

void MetricsCollector::PeriodMetric::Reset() {
	pstat = pStatMetric_t(new statMetric_t);
	for (uint8_t i = 0; i < sm_pstat.size(); ++i) {
		sm_pstat[i] = pStatMetric_t(new statMetric_t);
	}
}

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
		MetricClass_t mc, MetricHandler_t & mh,
		uint8_t count, const char **pdescs) {
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
		pm = pMetric_t(new CounterMetric(name, desc, count, pdescs));
		break;
	case VALUE:
		pm = pMetric_t(new ValueMetric(name, desc, count, pdescs));
		break;
	case SAMPLE:
		pm = pMetric_t(new SamplesMetric(name, desc, count, pdescs));
		break;
	case PERIOD:
		pm = pMetric_t(new PeriodMetric(name, desc, count, pdescs));
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

	logger->Debug("New metric [%s:%s => %s] registered, "
			"with [%d] sub-metrics",
			metricClassName[pm->mc], pm->name,
			pm->desc, pm->sm_count);

	return OK;
}

MetricsCollector::ExitCode_t
MetricsCollector::Register(MetricsCollection_t *mc, uint8_t count) {
	ExitCode_t result;
	uint8_t idx;

	for (idx = 0; idx < count; idx++) {
		result = Register(mc[idx].name, mc[idx].desc,
				mc[idx].mc, mc[idx].mh,
				mc[idx].sm_count, mc[idx].sm_desc);
		if (result != OK) {
			logger->Error("Metrics collection registration FAILED");
			return result;
		}
	}

	return OK;
}

MetricsCollector::ExitCode_t
MetricsCollector::Count(MetricHandler_t mh, uint64_t amount, uint8_t idx) {
	pMetric_t pm = GetMetric(mh);
	CounterMetric *m;

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
	m = (CounterMetric*)pm.get();

	// Increase the counter for the specified value
	m->cnt += amount;
	if (m->HasSubmetrics())
		m->sm_cnt[idx] += amount;

	return OK;
}

MetricsCollector::ExitCode_t
MetricsCollector::UpdateValue(MetricHandler_t mh, double amount,
		uint8_t idx) {
	pMetric_t pm = GetMetric(mh);
	ValueMetric *m;

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

	// Get the VALUE metric
	m = (ValueMetric*)pm.get();

	// Update the value if not zero, otherwise reset it
	if (amount) {
		m->value += amount;
		if (m->HasSubmetrics())
			m->sm_value[idx] += amount;
	} else {
		m->value = 0;
		if (m->HasSubmetrics())
			m->sm_value[idx] = 0;
	}

	(*m->pstat)(m->value);
	if (m->HasSubmetrics())
		(*m->sm_pstat[idx])(m->sm_value[idx]);

	return OK;
}


MetricsCollector::ExitCode_t
MetricsCollector::Add(MetricHandler_t mh, double amount, uint8_t idx) {
	if (!amount)
		return OK;

	return UpdateValue(mh, amount, idx);
}

MetricsCollector::ExitCode_t
MetricsCollector::Remove(MetricHandler_t mh, double amount, uint8_t idx) {
	if (!amount)
		return OK;

	return UpdateValue(mh, -amount, idx);
}

MetricsCollector::ExitCode_t
MetricsCollector::Reset(MetricHandler_t mh, uint8_t idx) {
	return UpdateValue(mh, 0, idx);
}


MetricsCollector::ExitCode_t
MetricsCollector::AddSample(MetricHandler_t mh,
		double sample, uint8_t idx) {
	pMetric_t pm = GetMetric(mh);
	SamplesMetric *m;

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
	m = (SamplesMetric*)pm.get();

	// Push-in the new sample into the accumulator
	(*m->pstat)(sample);
	if (m->HasSubmetrics())
		(*m->sm_pstat[idx])(sample);

	return OK;
}

MetricsCollector::ExitCode_t
MetricsCollector::PeriodSample(MetricHandler_t mh,
		double & last_period, uint8_t idx) {
	pMetric_t pm = GetMetric(mh);
	PeriodMetric *m;

	// Check if the metric has not yet been registered
	if (!pm) {
		logger->Error("Period sampling FAILED "
				"(Error: metric not registered)");
		return UNKNOWEN;
	}

	// Check the metrics is of compatible type
	if (pm->mc != PERIOD) {
		logger->Error("Period sampling FAILED "
				"(Error: wrong metric class)");
		assert(pm->mc == PERIOD);
		return UNSUPPORTED;
	}

	// Lock this metric
	std::unique_lock<std::mutex> ul(pm->mtx);

	// Get the SAMPLE metrics
	m = (PeriodMetric*)pm.get();

	// Start the submetrics sampling timer (if not already)
	if (m->HasSubmetrics() &&
		unlikely(!m->sm_period_tmr[idx].Running())) {
		m->sm_period_tmr[idx].start();
	}

	// Just start the sampling timer (if not already)
	if (unlikely(!m->period_tmr.Running())) {
		m->period_tmr.start();
		last_period = 0;
		return OK;
	}

	// Push-in the new timer into the accumulator
	last_period = m->period_tmr.getElapsedTimeMs();
	(*m->pstat)(last_period);
	if (m->HasSubmetrics()) {
		last_period = m->sm_period_tmr[idx].getElapsedTimeMs();
		(*m->sm_pstat[idx])(last_period);
	}

	// Reset timer for next period computation
	m->period_tmr.start();
	if (m->HasSubmetrics()) {
		m->sm_period_tmr[idx].start();
	}

	return OK;
}

void
MetricsCollector::_ResetAll(uint8_t mc) {
	MetricsMap_t::iterator it;
	pMetric_t pm;

	assert(mc < CLASSES_COUNT);
	it = metricsVec[mc].begin();
	for ( ; it != metricsVec[mc].end(); ++it) {
		pm = it->second;
		pm->Reset();
	}

}

void
MetricsCollector::ResetAll() {
	std::unique_lock<std::mutex> metrics_ul(metrics_mtx);
	uint8_t mc;

	for (mc = 0; mc < CLASSES_COUNT; ++mc) {
		logger->Info("Resetting metrics of class [%d]", mc);
		_ResetAll(mc);
	}

}

void
MetricsCollector::DumpCountSM(CounterMetric *m, uint8_t idx) {
	char _name[21], _desc[64];
	uint8_t i;

	// Setup sub-metric name
	snprintf(_name, 21, "%s[%02hu]", m->name, idx);

	// By default use main metrics description
	if ((m->sm_desc == NULL) || (m->sm_desc[0] == NULL)) {
		snprintf(_desc, 64, "%s [%02d]", m->desc, idx);
		goto dump_count_sm;
	}

	// Use the last valid provided description
	for (i = 0; m->sm_desc[i] && (i < idx); ++i) {}
	// This is needed to handle the special case of just one
	// submetric description provided
	if (i < idx || !m->sm_desc[i]) --i;

	// Setup the sub-metric description
	snprintf(_desc, 64, "%s [%02d]", m->sm_desc[i], idx);

dump_count_sm:
	// Dump sub-metric
	logger->Notice(
		" %-20s | %9 " PRIu64 " : %s",
		_name, m->sm_cnt[idx], _desc);
}

void
MetricsCollector::DumpCounter(CounterMetric *m) {
	logger->Notice(
		" %-20s | %9" PRIu64 " : %s",
		m->name, m->cnt, m->desc);

	if (!m->HasSubmetrics())
		return;

	for (uint8_t idx = 0; idx < m->sm_count; ++idx)
		DumpCountSM(m, idx);
}

void
MetricsCollector::DumpValueSM(ValueMetric *m, uint8_t idx,
		MetricStats<uint64_t> &ms) {
	uint8_t i;

	// Setup sub-metric name
	snprintf(ms.name, 21, "%s[%02hu]", m->name, idx);

	// Get sub-metrics statistics
	if (count(*m->sm_pstat[idx])) {
		ms.min = min(*m->sm_pstat[idx]);
		ms.max = max(*m->sm_pstat[idx]);
	}

	// By default use main metrics description
	if ((m->sm_desc == NULL) || (m->sm_desc[0] == NULL)) {
		snprintf(ms.desc, 64, "%s [%02d]", m->desc, idx);
		goto dump_value_sm;
	}

	// Use the last valid provided description
	for (i = 0; m->sm_desc[i] && (i < idx); ++i) {}
	// This is needed to handle the special case of just one
	// submetric description provided
	if (i < idx || !m->sm_desc[i]) --i;

	// Setup the sub-metric description
	snprintf(ms.desc, 64, "%s [%02d]", m->sm_desc[i], idx);

dump_value_sm:
	// Dump sub-metric
	logger->Notice(
		" %-20s | %9" PRIu64 " | %9" PRIu64 " | %9" PRIu64 " : %s",
		ms.name, m->sm_value[idx], ms.min, ms.max, ms.desc);
}

void
MetricsCollector::DumpValue(ValueMetric *m) {
	MetricStats<uint64_t> ms;

	if (count(*m->pstat)) {
		ms.min = min(*m->pstat);
		ms.max = max(*m->pstat);
	}
	logger->Notice(
		" %-20s | %9" PRIu64 " | %9" PRIu64 " | %9" PRIu64 " : %s",
		m->name, m->value, ms.min, ms.max, m->desc);

	if (!m->HasSubmetrics())
		return;

	for (uint8_t idx = 0; idx < m->sm_count; ++idx)
		DumpValueSM(m, idx, ms);
}

void
MetricsCollector::DumpSampleSM(SamplesMetric *m, uint8_t idx,
		MetricStats<double> &ms) {
	uint8_t i;

	// Setup sub-metric name
	snprintf(ms.name, 21, "%s[%02hu]", m->name, idx);

	// Get sub-metrics statistics
	if (count(*m->sm_pstat[idx])) {
		ms.min = min(*m->sm_pstat[idx]);
		ms.max = max(*m->sm_pstat[idx]);
		ms.avg = mean(*m->sm_pstat[idx]);
		ms.var = variance(*m->sm_pstat[idx]);
	} else {
		ms.min = 0; ms.max = 0; ms.avg = 0; ms.var = 0;
	}

	// By default use main metrics description
	if ((m->sm_desc == NULL) || (m->sm_desc[0] == NULL)) {
		snprintf(ms.desc, 64, "%s [%02d]", m->desc, idx);
		goto dump_samples_sm;
	}

	// Use the last valid provided description
	for (i = 0; m->sm_desc[i] && (i < idx); ++i) {}
	// This is needed to handle the special case of just one
	// submetric description provided
	if (i < idx || !m->sm_desc[i]) --i;

	// Setup the sub-metric description
	snprintf(ms.desc, 64, "%s [%02d]", m->sm_desc[i], idx);

dump_samples_sm:
	// Dump sub-metric
	logger->Notice(
			" %-20s | %9.3f | %9.3f | %9.3f | %9.3f :   %s",
			ms.name, ms.min, ms.max, ms.avg, ::sqrt(ms.var), ms.desc);

}

void
MetricsCollector::DumpSample(SamplesMetric *m) {
	MetricStats<double> ms;

	if (count(*m->pstat)) {
		ms.min = min(*m->pstat);
		ms.max = max(*m->pstat);
		ms.avg = mean(*m->pstat);
		ms.var = variance(*m->pstat);
	}
	logger->Notice(
		" %-20s | %9.3f | %9.3f | %9.3f | %9.3f : %s",
		m->name, ms.min, ms.max, ms.avg, ::sqrt(ms.var), m->desc);

	if (!m->HasSubmetrics())
		return;

	for (uint8_t idx = 0; idx < m->sm_count; ++idx)
		DumpSampleSM(m, idx, ms);

}

void
MetricsCollector::DumpPeriodSM(PeriodMetric *m, uint8_t idx,
		MetricStats<double> &ms) {
	uint8_t i;

	// Setup sub-metric name
	snprintf(ms.name, 21, "%s[%02hu]", m->name, idx);

	// Get sub-metrics statistics
	if (count(*m->sm_pstat[idx])) {
		ms.min = min(*m->sm_pstat[idx]);
		ms.max = max(*m->sm_pstat[idx]);
		ms.avg = mean(*m->sm_pstat[idx]);
		ms.var = variance(*m->sm_pstat[idx]);
	} else {
		ms.min = 0; ms.max = 0; ms.avg = 0; ms.var = 0;
	}

	// By default use main metrics description
	if ((m->sm_desc == NULL) || (m->sm_desc[0] == NULL)) {
		snprintf(ms.desc, 64, "%s [%02d]", m->desc, idx);
		goto dump_period_sm;
	}

	// Use the last valid provided description
	for (i = 0; m->sm_desc[i] && (i < idx); ++i) {}
	// This is needed to handle the special case of just one
	// submetric description provided
	if (i < idx || !m->sm_desc[i]) --i;

	// Setup the sub-metric description
	snprintf(ms.desc, 64, "%s [%02d]", m->sm_desc[i], idx);

dump_period_sm:
	// Dump sub-metric
	logger->Notice(
			" %-20s | %10.3f %10.3f | %10.3f %10.3f | %10.3f %10.3f |    %10.3f %10.3f :   %s",
			ms.name,
			ms.min, 1000.0/ms.min,
			ms.max, 1000.0/ms.max,
			ms.avg, 1000.0/ms.avg,
			::sqrt(ms.var), 1000.0/::sqrt(ms.var),
			ms.desc);
}

void
MetricsCollector::DumpPeriod(PeriodMetric *m) {
	MetricStats<double> ms;

	if (count(*m->pstat)) {
		ms.min = min(*m->pstat);
		ms.max = max(*m->pstat);
		ms.avg = mean(*m->pstat);
		ms.var = variance(*m->pstat);
	}
	logger->Notice(
		" %-20s | %10.3f %10.3f | %10.3f %10.3f | %10.3f %10.3f |    %10.3f %10.3f : %s",
		m->name,
		ms.min, 1000.0/ms.min,
		ms.max, 1000.0/ms.max,
		ms.avg, 1000.0/ms.avg,
		::sqrt(ms.var), 1000.0/::sqrt(ms.var),
		m->desc);

	if (!m->HasSubmetrics())
		return;

	for (uint8_t idx = 0; idx < m->sm_count; ++idx)
		DumpPeriodSM(m, idx, ms);

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

#define METRICS_PERIOD_HEADER \
"  Metric              |  Min  [ms]       [Hz] |  Max  [ms]       [Hz] |  Avg  [ms]       [Hz] |  StdDev  [ms]       [Hz] |  Description"
#define METRICS_PERIOD_SEPARATOR \
"----------------------+-----------------------+-----------------------+-----------------------+--------------------------+----------------------"

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
		DumpCounter((CounterMetric*)(((*it).second).get()));
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
		DumpValue((ValueMetric*)(((*it).second).get()));
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
		DumpSample((SamplesMetric*)(((*it).second).get()));
	}
	logger->Notice(METRICS_SAMPLES_SEPARATOR);


	logger->Notice("");
	logger->Notice("==========[ Period Metrics ]==========="
			"========================================");
	logger->Notice("");

	// Dumping PERIOD metrics
	logger->Notice(METRICS_PERIOD_HEADER);
	logger->Notice(METRICS_PERIOD_SEPARATOR);
	it = metricsVec[PERIOD].begin();
	for ( ; it != metricsVec[PERIOD].end(); ++it) {
		DumpPeriod((PeriodMetric*)(((*it).second).get()));
	}
	logger->Notice(METRICS_PERIOD_SEPARATOR);


}


const char *
MetricsCollector::metricClassName[MetricsCollector::CLASSES_COUNT] = {
	"Counter",
	"Value",
	"Samples",
	"Period"
};


} // namespace utils

} // namespace bbque

