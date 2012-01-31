/*
 * memory_monitor.cpp
 *
 */

#include <stdio.h>

#include "bbque/rtlib/monitors/memory_monitor.h"

uint16_t MemoryMonitor::newGoal(uint32_t goal) {
	return Monitor::newGoal(DataFunction::Average,
				ComparisonFunction::LessOrEqual,
				goal,
				defaultWindowSize);
}

uint16_t MemoryMonitor::newGoal(uint32_t goal, uint16_t windowSize) {
	return Monitor::newGoal(DataFunction::Average,
				ComparisonFunction::LessOrEqual,
				goal,
				windowSize);
}

uint32_t MemoryMonitor::extractMemoryUsage() {
	uint32_t memoryUsageKb = 0;
	int result;

	//The second number in /proc/self/statm is VmRSS in pages
	//TODO decide whether use VmRSS or VmRSS - sharedPages
	FILE* fp = fopen("/proc/self/statm","r");
	result = ::fscanf(fp, "%*d %u", &memoryUsageKb);
	if (result == EOF) {
		perror("MemoryMonitor read FAILED");
		fclose(fp);	//Is it safe to close it here? Is fp valid?
		return 0;
	}
	fclose(fp);

	return (memoryUsageKb * getpagesize() / 1024);
}

uint32_t MemoryMonitor::extractMemoryUsage(uint16_t id) {
	uint32_t memoryUsageKb = extractMemoryUsage();
	goalList[id]->addElement(memoryUsageKb);
	return memoryUsageKb;
}
