/*
 * memory_monitor.cpp
 *
 */

#include "bbque/rtlib/monitors/memory_monitor.h"
#include <fstream>
#include <sstream>

uint16_t MemoryMonitor::newGoal(uint32_t goal)
{
	return Monitor::newGoal(DataFunction::Average,
				ComparisonFunction::LessOrEqual,
				goal,
				defaultWindowSize);
}

uint16_t MemoryMonitor::newGoal(uint32_t goal, uint16_t windowSize)
{
	return Monitor::newGoal(DataFunction::Average,
				ComparisonFunction::LessOrEqual,
				goal,
				windowSize);
}

uint32_t MemoryMonitor::extractMemoryUsage()
{
	uint32_t memoryUsageKb = 0;
	uint32_t temp;

	//The second number in /proc/self/statm is VmRSS in pages
	//TODO decide whether use VmRSS or VmRSS - sharedPages
	FILE* fp = fopen("/proc/self/statm","r");
	fscanf(fp, "%d %d", &temp, &memoryUsageKb);
	memoryUsageKb = memoryUsageKb*getpagesize()/1024;

	return memoryUsageKb;
}

uint32_t MemoryMonitor::extractMemoryUsage(uint16_t id)
{
	uint32_t memoryUsageKb = extractMemoryUsage();
	goalList[id]->addElement(memoryUsageKb);
	return memoryUsageKb;
}
