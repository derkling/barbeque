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

#include <stdio.h>

#include "bbque/rtlib/monitors/memory_monitor.h"

uint16_t MemoryMonitor::newGoal(std::string metricName, uint32_t goal) {
	return Monitor::newGoal(metricName,
				DataFunction::Average,
				ComparisonFunction::LessOrEqual,
				goal,
				defaultWindowSize);
}

uint16_t MemoryMonitor::newGoal(std::string metricName, uint32_t goal,
				uint16_t windowSize) {
	return Monitor::newGoal(metricName,
				DataFunction::Average,
				ComparisonFunction::LessOrEqual,
				goal,
				windowSize);
}

uint32_t MemoryMonitor::extractMemoryUsage() {
	std::shared_ptr<FILE> fp(fopen("/proc/self/statm", "r"), fclose);
	uint32_t memoryUsageKb = 0;
	int result;

	//The second number in /proc/self/statm is VmRSS in pages
	//TODO decide whether use VmRSS or VmRSS - sharedPages
	result = ::fscanf(fp.get(), "%*d %"SCNu32, &memoryUsageKb);
	if (result == EOF) {
		perror("MemoryMonitor read FAILED");
		return 0;
	}

	return (memoryUsageKb * getpagesize() / 1024);
}

uint32_t MemoryMonitor::extractMemoryUsage(uint16_t id) {
	uint32_t memoryUsageKb = extractMemoryUsage();
	goalList[id]->addElement(memoryUsageKb);
	return memoryUsageKb;
}

uint32_t MemoryMonitor::extractVmPeakSize() {
	std::shared_ptr<FILE> fp(fopen("/proc/self/status", "r"), fclose);
	uint32_t vmPeak_Kb = 0;
	char buf[256];

	while (!feof(fp.get())) {
		if (fgets(buf, 256, fp.get()) == NULL) {
			perror("MemoryMonitor read FAILED");
			return 0;
		}

		if (strncmp(buf, "VmPeak:", 7))
			continue;

		sscanf(buf, "%*s %"SCNu32, &vmPeak_Kb);
		return vmPeak_Kb ;
	}

	return vmPeak_Kb;
}
