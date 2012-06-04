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

#ifndef BBQUE_GENERIC_WINDOW_H_
#define BBQUE_GENERIC_WINDOW_H_

#include <bbque/cpp11/mutex.h>

#include <cmath>
#include <vector>
#include <memory>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <boost/circular_buffer.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <iostream>
#include <bbque/monitors/goal_info.h>

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


/**
 * @brief GenericWindow interface class
 * @ingroup rtlib_sec04_mon
 *
 * @details
 * This class provides a general interface common for every GenericWindow class
 */
class GenericWindowIF {

public:
	/**
	 * @brief Checks whether the goal has been respected or not
	 */
	virtual bool checkGoal() = 0;

	/**
	 * @brief Checks whether the goal has been respected or not
	 *
	 * @param naps Output parameter representing a normalised value for the
	 * penalty in the range [0,1]
	 */
	virtual bool checkGoal(std::vector<float> &relativeErrors) = 0;

	/**
	 * @brief Check if the goal has been achieved
	 *
	 * It returns a GoalInfoPtr, a pointer to a structure containing all the
	 * information useful to deal with goal management
	 */
	virtual GoalInfoPtr fullCheckGoal() = 0;
};


/**
 * @brief A generic data window
 * @ingroup rtlib_sec04_mon
 *
 * @details
 * This class provides a general window able to contain different types of
 * values and manage them by provided utility functions.
 */
template <typename dataType>
class GenericWindow: public GenericWindowIF {

public:

	typedef std::function<dataType(GenericWindow <dataType>*)> DataFunctor;
	typedef std::function<bool(dataType, dataType)> ComparisonFunctor;

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
		Target(DataFunctor dataFunction,
		       ComparisonFunctor comparisonFunction,
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
		DataFunctor dataFunction;

		/**
		 * @brief Current comparisonFunction
		 */
		ComparisonFunctor comparisonFunction;
	};

	typedef std::vector<GenericWindow<dataType>::Target> Targets;
	typedef std::shared_ptr<Targets> TargetsPtr;

	TargetsPtr getTargets();

	/**
	 * @brief Initializes internal variables
	 */
	GenericWindow(std::string metricName,
		      TargetsPtr targets,
		      uint16_t windowSize = defaultWindowSize) :
		metricName(metricName),
		goalTargets(targets) {
			setCapacity(windowSize);
	}

	/**
	 * @brief Initializes internal variables
	 */
	GenericWindow(uint16_t windowSize = defaultWindowSize) {
		setCapacity(windowSize);
	}

	virtual ~GenericWindow() {
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
	 * @brief Check if the goal has been achieved.
	 *
	 * This function returns a GoalInfoPtr, a pointer to a structure
	 * containing all the information useful to deal with goal management
	 */
	GoalInfoPtr fullCheckGoal();

	/**
	 * @brief Checks whether the goal has been respected or not
	 *
	 * @param naps Output parameter representing a normalised value for the
	 * penalty in the range [0,1]
	 */
	virtual bool checkGoal(std::vector<float> &relativeErrors);

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
	 * @param targets List of Target elements
	 */
	void setGoal(TargetsPtr targets);

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
	 * @param resultsSize Number of elements used to compute the result
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
	 * Name of the metric associated to the goal
	 */
	std::string metricName;

	/**
	 * List of targets that need to be satisfied to achieve the goal
	 */
	TargetsPtr goalTargets;

	/**
	 * @brief Number of samples used to compute the result.
	 */
	uint16_t resultsWindowSize;

	/**
	 * @brief Set of mathematical functions used on a set of value.
	 */
	static const DataFunctor dataFunctions[4];

	/**
	 * @brief Set of logical functions (less, greater, etc. etc.)
	 * used to have the right comparison in the goal-checking phase
	 */
	static const ComparisonFunctor comparisonFunctions[4];
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
inline typename GenericWindow<dataType>::TargetsPtr GenericWindow<dataType>::getTargets() {
	return goalTargets;
}

template <typename dataType>
inline bool GenericWindow<dataType>::checkGoal() {
	bool result = true;
	typename std::vector<Target>::iterator it = goalTargets->begin();
	while (result && it != goalTargets->end()) {
		result = it->comparisonFunction(
				 it->dataFunction(this),
				 it->goalValue);
		++it;
	}

	return result;
}

template <typename dataType>
inline bool GenericWindow<dataType>::checkGoal(std::vector<float> &relativeErrors) {
	typename std::vector<Target>::iterator it;
	bool result = true;
	double goalValue;
	double dfResult;
	double absoluteError;
	double relativeError;

	// Forces a removal of all the content, in order to prevent
	// unpredictability of the output
	relativeErrors.clear();
	relativeErrors.reserve(goalTargets->size());

	for (it = goalTargets->begin(); it != goalTargets->end(); ++it) {
		/*
		 * Forced promotion to double to avoid problems with unsigned
		 * types.
		 * This variables will be used just to compute naps. The
		 * uncasted values will be used instead to have a more accurate
		 * evaluation of the goal.
		 */
		goalValue = it->goalValue;
		dfResult  = it->dataFunction(this);

		absoluteError = dfResult - goalValue;
		relativeError = absoluteError / goalValue;

		result = result && it->comparisonFunction(dfResult, goalValue);

		relativeErrors.push_back(relativeError);
	}

	return result;
}

template <typename dataType>
inline GoalInfoPtr GenericWindow<dataType>::fullCheckGoal() {
	bool result;
	uint8_t nap = 0;
	double goalValue;
	double dfResult;
	double absoluteError;
	double relativeError;

	GoalInfoPtr goalInfo(new GoalInfo(goalTargets->size()));

	goalInfo->metricName = metricName;

	typename std::vector<Target>::iterator it;

	for (it = goalTargets->begin(); it != goalTargets->end(); ++it) {
		/*
		 * Forced promotion to double to avoid problems with unsigned
		 * types.
		 * This variables will be used just to compute naps. The
		 * uncasted values will be used instead to have a more accurate
		 * evaluation of the goal.
		 */
		goalValue = it->goalValue;
		dfResult  = it->dataFunction(this);

		absoluteError = dfResult - goalValue;
		relativeError = absoluteError / goalValue;

		result = it->comparisonFunction(dfResult, goalValue);
		if (!result)
			nap = 100*fabs(absoluteError / (dfResult + goalValue));

		goalInfo->achieved.push_back(result);
		goalInfo->targetGoals.push_back(goalValue);
		goalInfo->relativeErrors.push_back(relativeError);
		goalInfo->observedValues.push_back(dfResult);
		goalInfo->naps.push_back(nap);

	}

	return goalInfo;
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
void GenericWindow<dataType>::setGoal(TargetsPtr targets) {
	goalTargets = targets;
}

template <typename dataType>
void GenericWindow<dataType>::setCapacity(uint16_t windowSize) {
	std::lock_guard<std::mutex> lg(windowMutex);
	windowBuffer.set_capacity(windowSize);
	resultsWindowSize = windowSize;
}

template <typename dataType>
inline void GenericWindow <dataType>::resetResultsWindow() {
	resultsWindowSize = windowBuffer.capacity;
}

/*******************************************************************************
 *    Doxygen Module Documentation
 ******************************************************************************/

/**
 * @defgroup rtlib_sec04_mon Metrics Monitoring
 * @ingroup rtlib_sec04_rtrm
 *
 * ADD MORE DETAILS HERE (Monitors)
 *
 */

#endif /* BBQUE_GENERIC_WINDOW_H_ */
