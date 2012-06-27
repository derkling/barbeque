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

#ifndef BBQUE_MONITOR_H_
#define BBQUE_MONITOR_H_

#include <map>
#include <vector>
#include <cstdint>
#include <functional>

#include <bbque/monitors/generic_window.h>

namespace bbque { namespace rtlib { namespace as {

/**
 * @brief A generic monitor
 * @ingroup rtlib_sec04_mon
 *
 * @details
 * This class provides a general monitor which functions are common to many
 * classes of metrics specific monitors such as time and throughput monitors.
 */
template <typename dataType>
class Monitor {
public:

	/**
	 * @brief Deletes all structures and variables
	 */
	virtual ~Monitor();

	/**
	 * @brief Creates a new monitor with a window keeping track of old
	 * values
	 *
	 * @param metricName Name of the metric associated to the goal
	 * @param goal Required goal value
	 * @param fType DataFunction to use for the evaluation of the goal
	 * @param cType ComparisonFunction to use for the evaluation of the goal
	 * @param windowSize Number of elements in the window of values
	 */
	virtual uint16_t newGoal(std::string metricName,
				 DataFunction fType,
				 ComparisonFunction cType,
				 dataType goal,
				 uint16_t windowSize = defaultWindowSize);

	/**
	 * @brief Creates a new monitor with a window keeping track of old values
	 *
	 * @param metricName Name of the metric associated to the goal
	 * @param targets List of targets for the current goal
	 * @param windowSize Number of elements in the window of values
	 */
	virtual uint16_t newGoal(std::string metricName,
				 typename GenericWindow<dataType>::TargetsPtr targets,
				 uint16_t windowSize = defaultWindowSize);

	/**
	 * @brief Creates a monitor (without goals) with a window keeping track
	 * of old values
	 *
	 * @param windowSize Number of elements in the window of values
	 */
	virtual uint16_t newEmptyGoal(uint16_t windowSize = defaultWindowSize);

	/**
	 * @brief Checks whether the goal has been respected or not
	 *
	 * @param id Identifies monitor and corresponding list
	 */
	virtual bool checkGoal(uint16_t id);

	/**
	 * @brief Checks whether the goal has been respected or not
	 *
	 * @param id Identifies monitor and corresponding list
	 * @param relativeErrors Output parameter for the relative errors of
	 * each of the targets of the goal
	 */
	virtual bool checkGoal(uint16_t id, std::vector<float> &relativeErrors);


	/**
	 * @brief Check if the goal has been achieved
	 *
	 * It returns a GoalInfoPtr, a pointer to a structure containing all the
	 * information useful to deal with goal management
	 *
	 * @param id Identifier of the monitor
	 */
	virtual GoalInfoPtr fullCheckGoal(uint16_t id);

	/**
	 * @brief Resets current goal
	 *
	 * @param id Identifies monitor and corresponding list
	 */
	virtual void resetGoal(uint16_t id);

	/**
	 * @brief Deletes current goal
	 */
	virtual void deleteGoal(uint16_t id);

	/**
	 * @brief Returns a new unique id
	 */
	uint16_t getUniqueId() const;

	/**
	 * @brief Returns the window of the monitor with the given id
	 *
	 * @param id Identifies a monitor and its corresponding window
	 */
	GenericWindow<dataType>* getWindow(uint16_t id);

	/**
	 * @brief Returns the maximum value from the window
	 *
	 * @param id Identifies a monitor and its corresponding window
	 */
	virtual dataType getMax(uint16_t id) const;

	/**
	 * @brief Returns the minimum value from the window
	 *
	 * @param id Identifies a monitor and its corresponding window
	 */
	virtual dataType getMin(uint16_t id) const;

	/**
	 * @brief Returns the average value from the window
	 *
	 * @param id Identifies a monitor and its corresponding window
	 */
	virtual dataType getAverage(uint16_t id) const;

	/**
	 * @brief Returns the variance of the values inside the window
	 *
	 * @param id Identifies a monitor and its corresponding window
	 */
	virtual dataType getVariance(uint16_t id) const;

	/**
	 * @brief Adds an element into the window
	 *
	 * @param element Element to be inserted
	 * @param id Identifies a monitor and its corresponding window
	 *
	 */
	void addElement(uint16_t id, dataType element);

protected:

	/**
	 * @brief List of goals and their respective IDs
	 *
	 * Each element of the map is given by an unique key that represents the
	 * ID of the goal and a GenericWindow class for data
	 *
	 * @see GenericWindow
	 **/
	std::map <uint16_t, GenericWindow<dataType>*> goalList;
};

template <typename dataType>
inline GenericWindow<dataType>* Monitor <dataType>::getWindow(uint16_t id) {
	if (goalList.find(id) != goalList.end())
		return goalList[id];
	return NULL;
}

template <typename dataType>
inline uint16_t Monitor <dataType>::getUniqueId() const {
	static uint16_t id = 0;
	return id++;
}

template <typename dataType>
Monitor <dataType>::~Monitor() {
	typename std::map<uint16_t, GenericWindow<dataType>*>::iterator it;

	for (it = goalList.begin(); it != goalList.end(); ++it)
		delete it->second;

	goalList.clear();
}

template <typename dataType>
inline uint16_t Monitor <dataType>::newGoal(std::string metricName,
					    DataFunction fType,
					    ComparisonFunction cType,
					    dataType goal,
					    uint16_t windowSize) {
	typename GenericWindow<dataType>::Target target(fType, cType, goal);
	typename GenericWindow<dataType>::TargetsPtr targets (
			new typename GenericWindow<dataType>::Targets());
	targets->push_back(target);

	return Monitor::newGoal(metricName, targets, windowSize);
}

template <typename dataType>
inline uint16_t Monitor <dataType>::newGoal(std::string metricName,
		typename GenericWindow<dataType>::TargetsPtr targets,
		uint16_t windowSize) {
	GenericWindow<dataType>* gWindow =
		new GenericWindow<dataType>(metricName, targets, windowSize);

	uint16_t id = getUniqueId();
	goalList[id] = gWindow;

	return id;
}

template <typename dataType>
inline uint16_t Monitor <dataType>::newEmptyGoal(uint16_t windowSize) {
	GenericWindow<dataType>* gWindow = new GenericWindow<dataType>(windowSize);

	uint16_t id = getUniqueId();
	goalList[id] = gWindow;

	return id;
}

template <typename dataType>
inline bool Monitor <dataType>::checkGoal(uint16_t id) {
	if (goalList.find(id) == goalList.end())
		return false;

	return (goalList[id]->checkGoal());
}

template <typename dataType>
inline bool Monitor <dataType>::checkGoal(uint16_t id,
					  std::vector<float> &relativeErrors) {
	if (goalList.find(id) == goalList.end()) {
		relativeErrors.clear();
		relativeErrors.push_back(0);
		return false;
	}

	return (goalList[id]->checkGoal(relativeErrors));
}

template <typename dataType>
inline GoalInfoPtr Monitor <dataType>::fullCheckGoal(uint16_t id) {
	//TODO add error handling for the case below
	//if (goalList.find(id) == goalList.end())
	//	(Time to switch to exceptions?)
	return (goalList[id]->fullCheckGoal());
}

template <typename dataType>
inline void Monitor <dataType>::deleteGoal(uint16_t id) {
	delete goalList[id];
	goalList.erase(id);
}

template <typename dataType>
inline void Monitor <dataType>::resetGoal(uint16_t id) {
	goalList[id]->clear();
}

template <typename dataType>
inline dataType Monitor <dataType>::getMax(uint16_t id) const {
	typename std::map<uint16_t, GenericWindow<dataType>*>::const_iterator it;
	it = goalList.find(id);
	if (it != goalList.end())
		return it->second->getMax();
	//TODO think about a better way
	fprintf(stderr,"Goal not found! Return value has no meaning!\n");
	return 0;
}

template <typename dataType>
inline dataType Monitor <dataType>::getMin(uint16_t id) const {
	typename std::map<uint16_t, GenericWindow<dataType>*>::const_iterator it;
	it = goalList.find(id);
	if (it != goalList.end())
		return it->second->getMin();
	//TODO think about a better way
	fprintf(stderr,"Goal not found! Return value has no meaning!\n");
	return 0;
}

template <typename dataType>
inline dataType Monitor <dataType>::getAverage(uint16_t id) const {
	typename std::map<uint16_t, GenericWindow<dataType>*>::const_iterator it;
	it = goalList.find(id);
	if (it != goalList.end())
		return it->second->getAverage();
	//TODO think about a better way
	fprintf(stderr,"Goal not found! Return value has no meaning!\n");
	return 0;
}

template <typename dataType>
inline dataType Monitor <dataType>::getVariance(uint16_t id) const {
	typename std::map<uint16_t, GenericWindow<dataType>*>::const_iterator it;
	it = goalList.find(id);
	if (it != goalList.end())
		return it->second->getVariance();
	//TODO think about a better way
	fprintf(stderr,"Goal not found! Return value has no meaning!\n");
	return 0;
}

template <typename dataType>
inline void Monitor <dataType>::addElement(uint16_t id, dataType element) {
	typename std::map<uint16_t, GenericWindow<dataType>*>::const_iterator it;
	it = goalList.find(id);
	if (it == goalList.end())
		return;
	it->second->addElement(element);
}

} // namespace as

} // namespace rtlib

} // namespace bbque

#endif /* BBQUE_MONITOR_H_ */
