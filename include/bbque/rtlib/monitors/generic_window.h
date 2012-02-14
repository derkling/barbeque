/**
 *      @file  generic_window.h
 *      @class  GenericWindow
 *      @brief  Generic window of values
 *
 * This class provides a general window able to contain different types of
 * values and utilities for their management.
 *
 *     @author  Andrea Di Gesare , andrea.digesare@gmail.com
 *     @author  Vincenzo Consales , vincenzo.consales@gmail.com
 *
 *   @internal
 *     Created
 *    Revision
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Di Gesare Andrea, Consales Vincenzo
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */


#ifndef BBQUE_GENERIC_WINDOW_H_
#define BBQUE_GENERIC_WINDOW_H_

#include <mutex>
#include <vector>
#include <cstdint>
#include <functional>
#include <boost/circular_buffer.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/variance.hpp>

/**
 * @brief Name of the metric used for goal checking
 */
enum class DataFunction:uint8_t {
	Max = 0,
	Min,
	Average,
	Variance
};

/**
 * @brief Name of comparison (less, greater, etc) used for goal checking
 */
enum class ComparisonFunction:uint8_t {
	Greater = 0,
	GreaterOrEqual,
	Less,
	LessOrEqual
};

namespace bac = boost::accumulators;

/**
 * @brief Default size of the window of samples
 */
const uint16_t defaultWindowSize = 100;

template <typename dataType>
class GenericWindow {

public:

	/**
	 * @class Target
	 *
	 * @brief Defines a target for a goal
	 */
	class Target{
	public:
		/**
		 * @brief Constructor of the Target class
		 *
		 * @param dataFunction DataFunction to use
		 * @param comparisonFunction ComparisonFunction to use
		 * @param goalValue Value of goal desired (regarding the target)
		 *
		 */
		Target(std::function<dataType(GenericWindow <dataType>*)> dataFunction,
			std::function<bool(dataType, dataType)> comparisonFunction,
			dataType goalValue) :
				dataFunction(dataFunction),
				comparisonFunction(comparisonFunction),
				goalValue(goalValue) {
		}

		/**
		 * @brief Constructor of the Target class
		 *
		 * @param dFun DataFunction to use
		 * @param cFun ComparisonFunction to use
		 * @param goalValue Value of goal desired (regarding the target)
		 *
		 */
		Target(DataFunction dFun,
		       ComparisonFunction cFun,
		       dataType goalValue) :
				goalValue(goalValue) {
			comparisonFunction =
				comparisonFunctions[static_cast<uint8_t>(cFun)];
			dataFunction =
				dataFunctions[static_cast<uint8_t>(dFun)];
		}

		/**
		 * @brief Value of the goal desired (regarding the target)
		 */
		dataType goalValue;

		/**
		 * @brief Current dataFunction
		 */
		std::function<dataType(GenericWindow <dataType>*)> dataFunction;

		/**
		 * @brief Current comparisonFunction
		 */
		std::function<bool(dataType, dataType)> comparisonFunction;
	};

	/**
	 * @brief Initializes internal variables
	 */
	GenericWindow(std::vector<typename GenericWindow<dataType>::Target> targets,
			uint16_t windowSize) :
				goalTargets(targets) {
			setCapacity(windowSize);
	}

	/**
	 * @brief Initializes internal variables
	 */
	GenericWindow(std::vector<typename GenericWindow<dataType>::Target> targets) :
		goalTargets(targets) {
		setCapacity(defaultWindowSize);
	}

	/**
	 * @brief Returns the maximum value from the window
	 */
	virtual dataType getMax() const;

	/**
	 * @brief Returns the minimum value from the window
	 */
	virtual dataType getMin() const;

	/**
	 * @brief Returns the average value from the window
	 */
	virtual dataType getAverage() const;

	/**
	 * @brief Returns the variance of the values inside the window
	 */
	virtual dataType getVariance() const;

	/**
	 * @brief Checks whether the goal has been respected or not
	 */
	virtual bool checkGoal();

	/**
	 * @brief Checks whether the goal has been respected or not
	 *
	 * @param gaps Output parameter representing the difference between
	 * the goal's targets and their current values (expressed in percentage)
	 */
	virtual bool checkGoal(std::vector<float> &gaps);

	/**
	 * @brief Removes all values from the window
	 */
	void clear();

	/**
	 * @brief Adds an element into the window
	 *
	 * @param element Element to be inserted
	 */
	void addElement(dataType element);

	/**
	 * @brief Returns last element from the window
	 */
	dataType getLastElement() const;

	/**
	 * @brief Sets a goal for the window of data
	 *
	 * @param target List of Target elements
	 */
	void setGoal(std::vector<typename GenericWindow<dataType>::Target> targets);

	/**
	 * @brief Sets the capacity of the window
	 *
	 * @param windowSize Number of elements of the window
	 */
	void setCapacity(uint16_t windowSize);

	/**
	 * @brief Sets the number of elements used by mathematical function
	 * (average, max, min, variance)
	 *
	 * @param resultSize Number of elements used to compute the result
	 */
	void setResultsWindow(uint16_t resultsSize);

	/**
	 * @brief Resets the number of elements used by mathematical function
	 * (average, max, min, variance) as its default size.
	 */
	void resetResultsWindow();

protected:

	/**
	 * @brief Mutex variable associated to the window buffer.
	 */
	std::mutex windowMutex;

	/**
	 * @brief Buffer for the window of values
	 */
	boost::circular_buffer<dataType> windowBuffer;

	/**
	 * List of targets that need to be satisfied to achieve the goal
	 */
	std::vector<Target> goalTargets;

	/**
	 * @brief Number of samples used to compute the result.
	 */
	uint16_t resultsWindowSize;

	/**
	 * @brief Set of mathematical functions used on a set of value.
	 */
	static const std::function<dataType(GenericWindow <dataType>*)>
							dataFunctions[4];
	/**
	 * @brief Set of logical functions (less, greater, etc. etc.)
	 * used to have the right comparison in the goal-checking phase
	 */
	static const std::function<bool(dataType, dataType)>
							comparisonFunctions[4];
};

template <typename dataType>
const std::function<dataType(GenericWindow <dataType>*)>
GenericWindow<dataType>::dataFunctions[4] = {
		&GenericWindow<dataType>::getMax,
		&GenericWindow<dataType>::getMin,
		&GenericWindow<dataType>::getAverage,
		&GenericWindow<dataType>::getVariance
};

template <typename dataType>
const std::function<bool(dataType, dataType)>
GenericWindow<dataType>::comparisonFunctions[4] = {
		std::greater<dataType>(),
		std::greater_equal<dataType>(),
		std::less<dataType>(),
		std::less_equal<dataType>()
};

template <typename dataType>
inline bool GenericWindow<dataType>::checkGoal() {
	bool result = true;
	typename std::vector<Target>::iterator it = goalTargets.begin();
	while (result && it != goalTargets.end()) {
		result = it->comparisonFunction(
				 it->dataFunction(this),
				 it->goalValue);
		++it;
	}

	return result;
}

template <typename dataType>
inline bool GenericWindow<dataType>::checkGoal(std::vector<float> &gaps) {
	bool result = true;
	typename std::vector<Target>::iterator it;
	for (it = goalTargets.begin(); it != goalTargets.end(); ++it) {
		gaps.push_back((it->dataFunction(this) - it->goalValue) /
							 it->goalValue);
		result = result &&
				(it->comparisonFunction (
				 it->dataFunction(this), it->goalValue));
	}

	return result;
}

#define GW_ACCUMULATE()\
	uint16_t count = windowBuffer.size();\
	if (count > resultsWindowSize)\
		count = resultsWindowSize;\
	acc = std::for_each(windowBuffer.end() - count,\
			    windowBuffer.end(), acc);

template <typename dataType>
inline dataType GenericWindow<dataType>::getMax() const {
	bac::accumulator_set<dataType, bac::features<bac::tag::max>> acc;
	GW_ACCUMULATE();
	return (bac::extract::max(acc));
}

template <typename dataType>
inline dataType GenericWindow<dataType>::getMin() const {
	bac::accumulator_set<dataType, bac::features<bac::tag::min>> acc;
	GW_ACCUMULATE();
	return (bac::extract::min(acc));
}

template <typename dataType>
inline dataType GenericWindow<dataType>::getAverage() const {
	bac::accumulator_set<dataType,
		bac::features<bac::tag::mean(bac::immediate)>> acc;
	GW_ACCUMULATE();
	return (bac::extract::mean(acc));
}

template <typename dataType>
inline dataType GenericWindow<dataType>::getVariance() const {
	bac::accumulator_set<dataType, bac::features<bac::tag::variance>> acc;
	GW_ACCUMULATE();
	return (bac::extract::variance(acc));
}

template <typename dataType>
inline dataType GenericWindow<dataType>::getLastElement() const {
	return (windowBuffer.back());
}

template <typename dataType>
inline void GenericWindow<dataType>::setResultsWindow(uint16_t resultSize) {
	resultsWindowSize = resultSize;
}

template <typename dataType>
void GenericWindow<dataType>::addElement(dataType element) {
	std::lock_guard<std::mutex> lg(windowMutex);
	windowBuffer.push_back(element);
}

template <typename dataType>
void GenericWindow<dataType>::clear() {
	std::lock_guard<std::mutex> lg(windowMutex);
	windowBuffer.clear();
}

template <typename dataType>
void GenericWindow<dataType>::setGoal(
		std::vector<typename GenericWindow<dataType>::Target> targets) {
	goalTargets = targets;
}

template <typename dataType>
void GenericWindow<dataType>::setCapacity(uint16_t windowSize) {
	std::lock_guard <std::mutex> lg(windowMutex);
	windowBuffer.set_capacity(windowSize);
	resultsWindowSize = windowSize;
}

template <typename dataType>
inline void GenericWindow <dataType>::resetResultsWindow() {
	resultsWindowSize = windowBuffer.capacity;
}

#endif /* BBQUE_GENERIC_WINDOW_H_ */
