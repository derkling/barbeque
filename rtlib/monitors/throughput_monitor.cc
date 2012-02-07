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

#include "bbque/rtlib/monitors/throughput_monitor.h"
#include "bbque/utils/utility.h"

using std::chrono::duration_cast;
using std::chrono::duration;
typedef std::chrono::monotonic_clock chr_mc;

uint16_t ThroughputMonitor::newGoal(double goal, uint16_t windowSize) {
	ThroughputWindow::Target target(DataFunction::Average,
			ComparisonFunction::GreaterOrEqual,
			goal);
	ThroughputWindow::TargetsPtr targets (new ThroughputWindow::Targets());
	targets->push_back(target);

	return ThroughputMonitor::newGoal(targets, windowSize);
}

uint16_t ThroughputMonitor::newGoal(DataFunction fType,
		ComparisonFunction cType,
		double goal,
		uint16_t windowSize) {
	ThroughputWindow::Target target(fType, cType, goal);
	ThroughputWindow::TargetsPtr targets (new ThroughputWindow::Targets());
	targets->push_back(target);

	return ThroughputMonitor::newGoal(targets, windowSize);
}

uint16_t ThroughputMonitor::newGoal(ThroughputWindow::TargetsPtr targets,
		uint16_t windowSize) {
	ThroughputWindow * tWindow = new ThroughputWindow(targets, windowSize);

	tWindow->started = false;

	uint16_t id = getUniqueId();
	goalList[id] = tWindow;

	return id;
}

void ThroughputMonitor::resetGoal(uint16_t id) {
	Monitor<double>::resetGoal(id);
	dynamic_cast<ThroughputWindow*> (goalList[id])->started = false;
}

void ThroughputMonitor::start(uint16_t id) {
	std::lock_guard<std::mutex> lg(timerMutex);
	_start(id);
}

void ThroughputMonitor::stop(uint16_t id, double data) {
	std::lock_guard<std::mutex> lg(timerMutex);
	_stop(id, data);
}

void ThroughputMonitor::_start(uint16_t id) {
	if (unlikely(goalList.find(id) == goalList.end()))
		return;
	if (unlikely(dynamic_cast<ThroughputWindow*>(goalList[id])->started))
		return;

	dynamic_cast<ThroughputWindow*> (goalList[id])->started = true;
	dynamic_cast<ThroughputWindow*> (goalList[id])->tStart = chr_mc::now();
}

void ThroughputMonitor::_stop(uint16_t id, const double &data) {
	if (unlikely(goalList.find(id) == goalList.end()))
		return;
	if (unlikely(!dynamic_cast<ThroughputWindow*>(goalList[id])->started))
		return;

	dynamic_cast<ThroughputWindow*>(goalList[id])->tStop = chr_mc::now();

	uint32_t elapsedTime = duration_cast<std::chrono::microseconds> (
			dynamic_cast<ThroughputWindow*> (goalList[id])->tStop -
			dynamic_cast<ThroughputWindow*> (goalList[id])->tStart
			).count();
	double tPut = data * (1000000.0 / elapsedTime);
	goalList[id]->addElement(tPut);
	dynamic_cast<ThroughputWindow*>(goalList[id])->started = false;
}

void ThroughputMonitor::_start() {
	if (unlikely(started))
		return;

	started = true;
	tStart = std::chrono::monotonic_clock::now();
}

double ThroughputMonitor::_getThroughput(const double &data) {
	if (unlikely(!started))
		return 0;

	started = false;
	tStop = std::chrono::monotonic_clock::now();
	uint32_t elapsedTime = duration_cast<std::chrono::microseconds>
			(tStop - tStart).count();

	return data * (1000000.0 / elapsedTime);
}

void ThroughputMonitor::start() {
	std::lock_guard<std::mutex> lg(timerMutex);
	_start();
}

double ThroughputMonitor::getThroughput(double data) {
	std::lock_guard<std::mutex> lg(timerMutex);
	return _getThroughput(data);
}
