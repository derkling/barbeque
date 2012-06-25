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

#include "bbque/rtlib/monitors/time_monitor.h"
#include "bbque/utils/utility.h"
#include "bbque/cpp11/ratio.h"

using std::chrono::duration_cast;
using std::chrono::duration;
typedef std::chrono::monotonic_clock chr_mc;

namespace bbque { namespace rtlib { namespace as {

uint16_t TimeMonitor::newGoal(std::string metricName, uint32_t goal,
			      uint16_t windowSize) {
	TimeWindow::Target target(DataFunction::Average,
			ComparisonFunction::LessOrEqual,
			goal);

	TimeWindow::TargetsPtr targets (new TimeWindow::Targets());
	targets->push_back(target);

	return TimeMonitor::newGoal(metricName, targets, windowSize);
}

uint16_t TimeMonitor::newGoal(std::string metricName,
		DataFunction fType,
		ComparisonFunction cType,
		uint32_t goal,
		uint16_t windowSize) {
	TimeWindow::Target target(fType, cType, goal);
	TimeWindow::TargetsPtr targets (new TimeWindow::Targets());
	targets->push_back(target);

	return TimeMonitor::newGoal(metricName, targets, windowSize);
}

uint16_t TimeMonitor::newGoal(std::string metricName,
		TimeWindow::TargetsPtr targets,
		uint16_t windowSize) {

	TimeWindow * tWindow = new TimeWindow(metricName, targets, windowSize);

	tWindow->started = false;

	uint16_t id = getUniqueId();
	goalList[id] = tWindow;

	return id;
}

uint16_t TimeMonitor::newEmptyGoal(uint16_t windowSize) {
	TimeWindow * tWindow = new TimeWindow(windowSize);

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
	std::lock_guard<std::mutex> lg(timerMutex);
	_start(id);
}

void TimeMonitor::stop(uint16_t id) {
	std::lock_guard<std::mutex> lg(timerMutex);
	_stop(id);
}

void TimeMonitor::_start(uint16_t id) {
	if (unlikely(goalList.find(id) == goalList.end()))
		return;
	if (unlikely(dynamic_cast<TimeWindow*>(goalList[id])->started))
		return;

	dynamic_cast<TimeWindow*>(goalList[id])->started = true;
	dynamic_cast<TimeWindow*>(goalList[id])->tStart = chr_mc::now();
}

void TimeMonitor::_stop(uint16_t id) {
	if (unlikely(goalList.find(id) == goalList.end()))
		return;
	if (unlikely(!dynamic_cast<TimeWindow*>(goalList[id])->started))
		return;

	dynamic_cast<TimeWindow*>(goalList[id])->tStop = chr_mc::now();

	uint32_t elapsedTime = duration_cast<std::chrono::milliseconds>(
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
	return (duration_cast<duration<double>>(tStop - tStart).count());
}

double TimeMonitor::getElapsedTimeMs() {
	std::lock_guard<std::mutex> lg(timerMutex);
	_stop();
	return (duration_cast<duration<double, std::milli>>(tStop - tStart).count());
}

double TimeMonitor::getElapsedTimeUs() {
	std::lock_guard<std::mutex> lg(timerMutex);
	_stop();
	return (duration_cast<duration<double, std::micro>>(tStop - tStart).count());
}

} // namespace as

} // namespace rtlib

} // namespace bbque
