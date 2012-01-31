/*
 * throughput_monitor.cpp
 *
 */

#include "bbque/rtlib/monitors/throughput_monitor.h"
#include "bbque/utils/utility.h"

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
	dynamic_cast<ThroughputWindow*> (goalList[id])->tStart =
		std::chrono::monotonic_clock::now();
}

void ThroughputMonitor::_stop(uint16_t id, const double &data) {
	if (unlikely(goalList.find(id) == goalList.end()))
		return;
	if (unlikely(!dynamic_cast<ThroughputWindow*>(goalList[id])->started))
		return;
	dynamic_cast<ThroughputWindow*>
		(goalList[id])->tStop = std::chrono::monotonic_clock::now();
	uint32_t elapsedTime =
		std::chrono::duration_cast<std::chrono::microseconds> (
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
	uint32_t elapsedTime =
			std::chrono::duration_cast<std::chrono::microseconds>
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
