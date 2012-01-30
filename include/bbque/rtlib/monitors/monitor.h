/**
 *      @file   monitor.h
 *      @class  Monitor
 *      @brief  Base class for monitor
 *
 * This class provides a general monitor with a structure useful for many
 * types of monitor. Its structures and its functions are common to many types
 * of monitors like time and throughput monitors.
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


#ifndef BBQUE_MONITOR_H_
#define BBQUE_MONITOR_H_

#include <map>
#include <vector>
#include <cstdint>
#include <functional>

#include "generic_window.h"

template <typename dataType>
class Monitor {
public:

	/**
	 * @brief Deletes all structures and variables
	 */
	~Monitor();

	/**
	 * @brief Creates a new monitor with a window keeping track of old
	 * values
	 *
	 * @param fType Selects the DataFunction for the evaluation of the goal
	 * @param cType Selects the ComparisonFunction for the evaluation of
	 * the goal
	 * @param goal Value of goal required
	 */
	virtual uint16_t newGoal(DataFunction sType,
				 ComparisonFunction cType,
				 dataType goal);

	/**
	 * @brief Creates a new monitor with a window keeping track of old
	 * values
	 *
	 * @param goal Required goal value
	 * @param fType DataFunction to use for the evaluation of the goal
	 * @param cType ComparisonFunction to use for the evaluation of the goal
	 * @param windowSize Number of elements in the window of values
	 */
	virtual uint16_t newGoal(DataFunction sType,
				 ComparisonFunction cType,
				 dataType goal,
				 uint16_t windowSize);

	/**
	 * @brief Creates a new monitor with a window keeping track of old
	 * values
	 *
	 * @param targets List of targets for the current goal
	 */
	virtual uint16_t newGoal(
		std::vector<typename GenericWindow<dataType>::Target> targets);

	/**
	 * @brief Creates a new monitor with a window keeping track of old values
	 *
	 * @param targets List of targets for the current goal
	 * @param windowSize Number of elements in the window of values
	 */
	virtual uint16_t newGoal(
		std::vector<typename GenericWindow<dataType>::Target> targets,
		uint16_t windowSize);

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
	 * @param gaps Output parameter representing the difference between
	 * the goal's targets and their current values (expressed in percentage)
	 */
	virtual bool checkGoal(uint16_t id, std::vector<float> &gaps);

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

protected:

	/**
	 * @brief List of goals and their respective IDs
	 *
	 * Each element of the map is given by an unique key that represents the
	 * ID of the goal and a GenericWindow class for data
	 *
	 * @see GenericWindow
	 **/
	std::map <uint16_t, GenericWindow <dataType>*> goalList;
};

template <typename dataType>
inline GenericWindow<dataType>* Monitor <dataType>::getWindow(uint16_t id)
{
	if (goalList.find(id) != goalList.end())
		return goalList[id];
	return NULL;
}

template <typename dataType>
inline uint16_t Monitor <dataType>::getUniqueId() const
{
	static uint16_t id = 0;
	return id++;
}

template <typename dataType>
Monitor <dataType>::~Monitor()
{
	typename std::map<uint16_t, GenericWindow<dataType>*>::iterator it;

	for (it = goalList.begin(); it != goalList.end(); ++it) {
		delete it->second;
	}
	goalList.clear();
}

template <typename dataType>
inline uint16_t Monitor <dataType>::newGoal(DataFunction fType,
					    ComparisonFunction cType,
					    dataType goal)
{
	return Monitor::newGoal(fType, cType, goal, defaultWindowSize);
}

template <typename dataType>
inline uint16_t Monitor <dataType>::newGoal(DataFunction fType,
					    ComparisonFunction cType,
					    dataType goal,
					    uint16_t windowSize)
{
	typename GenericWindow<dataType>::Target target(fType, cType, goal);
	std::vector<typename GenericWindow<dataType>::Target> targets;
	targets.push_back(target);

	return Monitor::newGoal(targets, windowSize);
}

template <typename dataType>
inline uint16_t Monitor <dataType>::newGoal(
		std::vector<typename GenericWindow<dataType>::Target> targets)
{
	return Monitor::newGoal(targets, defaultWindowSize);
}

template <typename dataType>
inline uint16_t Monitor <dataType>::newGoal(
		std::vector<typename GenericWindow<dataType>::Target> targets,
		uint16_t windowSize)
{
	GenericWindow<dataType>* gWindow =
			new GenericWindow<dataType>(targets, windowSize);

	uint16_t id = getUniqueId();
	goalList[id] = gWindow;

	return id;
}

template <typename dataType>
inline bool Monitor <dataType>::checkGoal(uint16_t id)
{
	if (goalList.find(id) == goalList.end())
		return false;

	return (goalList[id]->checkGoal());
}

template <typename dataType>
inline bool Monitor <dataType>::checkGoal(uint16_t id, std::vector<float> &gaps)
{
	if (goalList.find(id) == goalList.end()) {
		gaps.clear();
		gaps.push_back(0);
		return false;
	}

	return (goalList[id]->checkGoal(gaps));
}

template <typename dataType>
inline void Monitor <dataType>::deleteGoal(uint16_t id)
{
	delete goalList[id];
	goalList.erase(id);
}

template <typename dataType>
inline void Monitor <dataType>::resetGoal(uint16_t id)
{
	goalList[id]->clear();
}

#endif /* BBQUE_MONITOR_H_ */
