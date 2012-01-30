/*
 * time_monitor.cpp
 *
 */

#include "bbque/rtlib/monitors/time_monitor.h"
#include "bbque/utils/utility.h"
#include <ratio>

uint16_t TimeMonitor::newGoal(uint32_t goal, uint16_t windowSize) {
	TimeWindow::Target target(DataFunction::Average,
			ComparisonFunction::LessOrEqual,
			goal);

	// FIXME This call, as well a the next three similar ones, allocate
	// local vectors which are then passed BY COPY multiple times!
	// For example, this "target" vector is copied all these times:
	// 1. first into TimeMonitor::newGoal
	std::vector<TimeWindow::Target> targets;
	targets.push_back(target);

	return TimeMonitor::newGoal(targets, windowSize);
}

uint16_t TimeMonitor::newGoal(DataFunction fType,
		ComparisonFunction cType,
		uint32_t goal,
		uint16_t windowSize) {
	TimeWindow::Target target(fType, cType, goal);
	std::vector<TimeWindow::Target> targets;
	targets.push_back(target);

	return TimeMonitor::newGoal(targets, windowSize);
}

uint16_t TimeMonitor::newGoal(std::vector<TimeWindow::Target> targets,
		uint16_t windowSize) {
	// 2. then into the constructor of the TimeWindow...
	// 3. which in turn copies it into the GenericWindow..
	// 4. which finally copies it into its private GenericWindow::goalTargets
	//
	// For a total of FOUR VECTOR copies just to initialize a data structure!
	// why not using a shared_pointer and pass everyting by reference?!?
	TimeWindow * tWindow = new TimeWindow(targets, windowSize);

	tWindow->started = false;

	uint16_t id = getUniqueId();
	goalList[id] = tWindow;

	return id;
}

void TimeMonitor::resetGoal(uint16_t id) {
	Monitor<uint32_t>::resetGoal(id);
	dynamic_cast<TimeWindow*>(goalList[id])->started = false;
}

void TimeMonitor::start(uint16_t id) {
	if (goalList.find(id) == goalList.end())
		return;

	dynamic_cast<TimeWindow*>(goalList[id])->started = true;
	dynamic_cast<TimeWindow*>(goalList[id])->tStart =
		std::chrono::monotonic_clock::now();
}

void TimeMonitor::stop(uint16_t id) {
	if (goalList.find(id) == goalList.end())
		return;

	if (!unlikely(dynamic_cast<TimeWindow*>(goalList[id])->started))
		return;

	dynamic_cast<TimeWindow*>(goalList[id])->tStop =
		std::chrono::monotonic_clock::now();

	uint32_t elapsedTime =
		std::chrono::duration_cast<std::chrono::milliseconds>(
				dynamic_cast<TimeWindow*>(goalList[id])->tStop -
				dynamic_cast<TimeWindow*>(goalList[id])->tStart
			).count();
	goalList[id]->addElement(elapsedTime);
	dynamic_cast<TimeWindow*>(goalList[id])->started = false;

}

void TimeMonitor::_start() {
	if (unlikely(started))
		return;
	started = true;
	tStart = std::chrono::monotonic_clock::now();
}

void TimeMonitor::_stop() {
	if (unlikely(!started))
		return;
	started = false;
	tStop = std::chrono::monotonic_clock::now();
}

void TimeMonitor::start() {
	std::lock_guard<std::mutex> lg(timerMutex);
	_start();
}

void TimeMonitor::stop() {
	std::lock_guard<std::mutex> lg(timerMutex);
	_stop();
}

double TimeMonitor::getElapsedTime() {
	std::lock_guard<std::mutex> lg(timerMutex);
	_stop();
	return (std::chrono::duration_cast<std::chrono::duration<double>>
		(tStop - tStart).count());
}

double TimeMonitor::getElapsedTimeMs() {
	std::lock_guard<std::mutex> lg(timerMutex);
	_stop();
	return (std::chrono::duration_cast<std::chrono::duration<double, std::milli>>
		(tStop - tStart).count());
}

double TimeMonitor::getElapsedTimeUs() {
	std::lock_guard<std::mutex> lg(timerMutex);
	_stop();
	return (std::chrono::duration_cast<std::chrono::duration<double, std::micro>>
		(tStop - tStart).count());
}
