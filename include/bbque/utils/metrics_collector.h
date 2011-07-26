/**
 *       @file  metrics_collector.h
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


#ifndef BBQUE_METRICS_COLLECTOR_H_
#define BBQUE_METRICS_COLLECTOR_H_

#include "bbque/plugins/logger.h"
#include "bbque/utils/timer.h"

#include <map>
#include <memory>
#include <mutex>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/variance.hpp>

using namespace boost::accumulators;
using bbque::plugins::LoggerIF;
using bbque::utils::Timer;

namespace bbque { namespace utils {

class MetricsCollector {

public:

	/**
	 * @brief Return codes generated by methods of this class
	 */
	typedef enum {
		OK,
		DUPLICATE,
		UNSUPPORTED,
		UNKNOWEN
	} ExitCode_t;

	/**
	 * @ibrief The supported kinds of metrics
	 */
	typedef enum {
		/** A generic "u64" event counter */
		COUNTER = 0,
		/** A generic "u64" value which could be increased/decreased */
		VALUE,
		/** A generic "double" sample for statistical computations */
		SAMPLE,

		/** The number of metrics classes */
		CLASSES_COUNT // This MUST be the last value

	} MetricClass_t;

	/** The handler of a registered metrics */
	typedef size_t MetricHandler_t;

	/**
	 * @brief A collection of metrics to be registerd
	 *
	 * A client module could register a set of metrics by declaring an array
	 * of this structures, filled with proper values describing each single
	 * metrics of interest, and passing this to the provided registration
	 * method.
	 */
	typedef struct MetricsCollection {
		/** The name of the metric */
		const char *name;
		/** A textual description of the metric */
		const char *desc;
		/** The class of this metirc */
		MetricClass_t mc;
		/** A metric handler returned by the registration method */
		MetricHandler_t mh;
	} MetricsCollection_t;

	/**
	 * @brief The base class for a registered metric
	 *
	 * This is the data structure representing the maximum set of information
	 * common to all metric classes. Specific metrics are specified by
	 * deriving this class.
	 */
	typedef struct Metric {
		/** The name of the metric */
		const char *name;
		/** A textual description of the metric */
		const char *desc;
		/** The class of this metirc */
		MetricClass_t mc;

		/** Mutex protecting concurrent access to statistical data */
		std::mutex mtx;

		Metric(const char *name, const char *desc, MetricClass_t mc) :
			name(name), desc(desc), mc(mc) {
		};

	} Metric_t;

	/** A pointer to a (base class) registered metrics */
	typedef std::shared_ptr<Metric_t> pMetric_t;

	/**
	 * @brief A counting metric
	 *
	 * This is a simple metric which could be used to count events. Indeed
	 * this metrics supports only the "increment" operation.
	 */
	typedef struct CounterMetric : public Metric {
		uint64_t count;

		CounterMetric(const char *name, const char *desc) :
			Metric(name, desc, COUNTER),
			count(0) {};

	} CounterMetric_t;

	/**
	 * @brief A value accounting metrics
	 *
	 * This is a simple metric which could be used to keep track of a certain
	 * value (e.g. amount of memory, total time spent) which chould both
	 * increment or decrement of a specified value. The metrics keep track
	 * also of the "maximum" and "minimum" value for the metric.
	 */
	typedef struct ValueMetric : public Metric {
		/** The current value */
		uint64_t value;
		/** Statistics on Value */
		accumulator_set<uint64_t,
			stats<tag::count, tag::min, tag::max>> stat;

		ValueMetric(const char *name, const char *desc) :
			Metric(name, desc, VALUE),
			value(0) {};

	} ValueMetric_t;

	/**
	 * @brief A statistic collection metrics
	 *
	 * This is a metrics which could be useed to compute a statistic on a set
	 * of samples. Indeed, this metric support only the sample addition
	 * operations: each time a new sample is added to the metric a set of
	 * statistics are updated. So far the supported statistics are:
	 * minumum, maximum, mead and variance.<br>
	 * Mean and variance are computed on the <i>complete population</i>, i.e.
	 * considering all the samples collected so far.
	 */
	typedef struct SamplesMetric : public Metric {
		/** Statistics on collected Samples */
		accumulator_set<double,
			stats<tag::min, tag::max, tag::variance>> stat;

		SamplesMetric(const char *name, const char *desc) :
			Metric(name, desc, SAMPLE) {};

	} SamplesMetric_t;


	/**
	 * @brief Get a reference to the metrics collector
	 * The MetricsCollector is a singleton class providing the glue logic for
	 * the Barbeque metrics collection, management and query.
	 *
	 * @return  a reference to the MetricsCollector singleton instance
	 */
	static MetricsCollector & GetInstance();

	/**
	 * @brief  Clean-up the metrics collector data structures.
	 */
	~MetricsCollector();

	/**
	 * @brief Register a new system metric
	 *
	 * Once a new metric is needed it should be first registered to the
	 * MetricsCollector class by using this method. The registeration requires
	 * a metric name, which is used to uniquely identify it, as well as the
	 * metric class. This method returns, on success, a "metrics handler"
	 * which is required by many other access/modification methods.
	 */
	ExitCode_t Register(const char *name,
			const char *desc,
			MetricClass_t mc,
			MetricHandler_t & mh);

	/**
	 * @brief Register the sepcified set of metrics
	 *
	 * A client module could register a set of metrics by using this method
	 * which requires a pointer to a MetricsCollection, i.e. a pre-loaded
	 * vector of metrics descriptors.
	 */
	ExitCode_t Register(MetricsCollection_t *mc, uint8_t count);

	/**
	 * @brief Increase the specified counter metric
	 *
	 * This method is reserved to metrics of COUNT class and allows to
	 * increment the counter by the specified amount (by default 1).
	 */
	ExitCode_t Count(MetricHandler_t mh, uint64_t amount = 1);

	/**
	 * @brief Add the specified amount to a value metric
	 *
	 * This method is reserved to metrics of VALUE class and allows to augment
	 * the current metric value by the specified amount.
	 */
	ExitCode_t Add(MetricHandler_t mh, double amount);

	/**
	 * @brief Subtract the specified amount to a value metric
	 *
	 * This method is reserved to metrics of VALUE class and allows to
	 * decrease the current metric value by the specified amount.
	 */
	ExitCode_t Remove(MetricHandler_t mh, double amount);

	/**
	 * @brief Reset the specified value metric
	 *
	 * This method is reserved to metrics of VALUE class and allows to reset
	 * the current value of the specified metric.
	 */
	ExitCode_t Reset(MetricHandler_t mh);

	/**
	 * @brief Add a new sample to a SAMPLE metric
	 *
	 * This method is reserved to metrics of SAMPLE class and allows to collect
	 * one more sample, thus updating the metrics statistics.
	 */
	ExitCode_t AddSample(MetricHandler_t mh, double sample);

	/**
	 * @brief Dump on screen a report of all the registered metrics
	 *
	 * This method is intended mainly dor debugging and allows to report on
	 * screen the current values for all the registered metrics.
	 */
	void DumpMetrics();

private:

	/** A map of metrics handlers on correpsonding registered metrics */
	typedef std::map<MetricHandler_t, pMetric_t> MetricsMap_t;

	/** An entry of the metrics maps */
	typedef std::pair<MetricHandler_t, pMetric_t> MetricsMapEntry_t;

	/** A set of metrics maps grouped by class */
	typedef MetricsMap_t MetricsVec_t[CLASSES_COUNT];

	/** A string representation of metric classes */
	static const char *metricClassName[CLASSES_COUNT];

	/**
	 * @brief The logger to use.
	 */
	LoggerIF *logger;

	/**
	 * @brief Map of all registered metrics
	 */
	MetricsMap_t metricsMap;

	/**
	 * @brief The mutex protecting access to metrics maps
	 */
	std::mutex metrics_mtx;

	/**
	 * @brief Vector of registered metrics grouped by class
	 */
	MetricsVec_t metricsVec;

	/**
	 * @brief Build a new MetricsCollector
	 */
	MetricsCollector();

	/**
	 * @brief Get the handler of the metric specified by name
	 *
	 * Return the handler of a metrics with the specified name.
	 */
	MetricHandler_t GetHandler(const char *name);

	/**
	 * @brief Get a reference to the registered metrics with specified handler
	 *
	 * Given the handler of a metrics, this method return a reference to its
	 * base class, or an empty pointer if the metric has not yet been
	 * registered.
	 */
	pMetric_t GetMetric(MetricHandler_t hdlr);

	/**
	 * @brief Get a reference to the registered metrics with specified name
	 *
	 * Given the name of a metrics, this method return a reference to its
	 * base class, or an empty pointer if the metric has not yet been
	 * registered.
	 */
	pMetric_t GetMetric(const char *name);

	/**
	 * @brief Update a metrics of VALUE class of the specified amout.
	 *
	 * The specified amount is added/removed for the metrics, the metrics is
	 * reset if amount is ZERO.
	 */
	ExitCode_t UpdateValue(MetricHandler_t mh, double amount);

	/**
	 * @brief Dump the current value for a metric of class COUNT
	 */
	void DumpCounter(CounterMetric_t *m);

	/**
	 * @brief Dump the current value for a metric of class VALUE
	 */
	void DumpValue(ValueMetric_t *m);

	/**
	 * @brief Dump the current value for a metric of class SAMPLE
	 */
	void DumpSample(SamplesMetric_t *m);

};

} // namespace utils

} // namespace bbque

#endif // BBQUE_METRICS_COLLECTOR_H_
