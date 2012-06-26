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

#include "bbque/utils/perf.h"

#include <cstdlib>

#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "RTLIB_PERF [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "RTLIB_PERF [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "RTLIB_PERF [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "RTLIB_PERF [ERR]", fmt)

namespace bbque { namespace utils {

Perf::Perf() :
	opened(false) {

}

Perf::~Perf() {
	RegisteredCountersMap_t::iterator it;
	pRegisteredCounter_t prc;

	// Release all counters
	for (it = counters.begin(); it != counters.end(); ++it) {
		// release the registered counter
		prc = (*it).second;
		prc.reset();
	}

	// Release the group leader
	pGroupLeader.reset();

	// clean-up the registered counters list
	counters.clear();
}

int Perf::EventOpen(struct perf_event_attr *attr,
		pid_t pid, int cpu, int group_fd,
		unsigned long flags) {
	int result;

	DB(fprintf(stderr, FMT_DBG("Adding new PERF counter [%"PRIu32":%llu], GL [%d], "
					"for Task [%d] on CPU [%d]...\n"),
					attr->type, attr->config, group_fd, pid, cpu));

	attr->size = sizeof(*attr);
	result = syscall(__NR_perf_event_open, attr, pid, cpu,
			group_fd, flags);
	if (result == -1) {
		fprintf(stderr, FMT_ERR("Opening PERF counters FAILED "
					"(Error: %s)\n"), strerror(errno));
	} else {
		opened = true;
	}

	assert(result >= 0);
	return result;
}

int Perf::AddCounter(perf_type_id type,
		uint64_t config, bool exclude_kernel) {
	pRegisteredCounter_t prc(new RegisteredCounter());

	// Set default counter options
	prc->attr.inherit = 1;
	prc->attr.disabled = 1;
	//prc->attr.exclude_idle = 1;

	// Set kernel & hiper-visor tracking
	if (exclude_kernel) {
		prc->attr.exclude_kernel = 1;
		prc->attr.exclude_hv = 1;
	}

	// Define read format
	prc->attr.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | \
							PERF_FORMAT_TOTAL_TIME_RUNNING;

	// Define the event to read
	prc->attr.type = type;
	prc->attr.config = config;

	// Add a new event counter
	prc->fd = EventOpen(&(prc->attr), gettid(), -1, -1, 0);
	//prc->fd = EventOpen(&(prc->attr), gettid(), -1, GroupLeader(), 0);

	// Keep track of GroupLeader
	if (!IsGroupLeaderDefined()) {
		pGroupLeader = prc;
	}

	counters[prc->fd] = prc;

	fprintf(stderr, FMT_INF("Added new PERF counter [%02d:%d:%02lu]\n"),
			prc->fd, type, config);

	return prc->fd;
}

int Perf::Enable() {
	if (!IsGroupLeaderDefined()) {
		fprintf(stderr, FMT_ERR("Enabling PERF counters FAILED "
					"(Error: Undefined group leader)\n"));
		return -1;
	}

	prctl(PR_TASK_PERF_EVENTS_ENABLE);
	//::ioctl(GroupLeader(), PERF_EVENT_IOC_ENABLE);
	DB(fprintf(stderr, FMT_DBG("PERF counters (GL:%d) ENABLED\n"),
			GroupLeader()));

	return 0;
}

int Perf::Disable() {
	if (!IsGroupLeaderDefined()) {
		fprintf(stderr, FMT_ERR("Disabling PERF counters FAILED "
					"(Error: Undefined group leader)\n"));
		return 0;
	}

	prctl(PR_TASK_PERF_EVENTS_DISABLE);
	//::ioctl(GroupLeader(), PERF_EVENT_IOC_DISABLE);
	DB(fprintf(stderr, FMT_DBG("PERF counters (GL:%d) DISABLED\n"),
				GroupLeader()));

	return 0;
}

int Perf::ReadCounter(int fd, void *buf, size_t n) {
	int bytes = n;

	while (n) {
		int ret = read(fd, buf, n);
		if (ret <= 0)
			return ret;
		n -= ret;
		buf += ret;
	}
	return bytes;
}

#define UPDATE_DELTA(COUNTER)\
	prc->delta.COUNTER = prc->count.COUNTER - old_count.COUNTER

uint64_t Perf::Update(int id, bool delta) {
	pRegisteredCounter_t prc = counters[id];
	ReadFormat_t old_count = prc->count;
	ssize_t bytes;

	if (!opened || !prc) {
		fprintf(stderr, FMT_ERR("Reading PERF counter FAILED "
					"(Error: Counters not opened or invalid counter [%d])\n"),
				id);
		return 0;
	}

	// Reading counters
	bytes = ReadCounter(id, &(prc->count), sizeof(prc->count));
	assert(bytes == sizeof(prc->count));
	(void)bytes; // quite compilation warning on RELEASE build

	// Update deltas since last update
	UPDATE_DELTA(value);
	UPDATE_DELTA(time_enabled);
	UPDATE_DELTA(time_running);

	DB(fprintf(stderr, FMT_DBG("Counter [%d:%"PRIu32":%llu]: "
					"cV [%"PRIu64"], cE [%"PRIu64"], cR [%"PRIu64"] "
					"dV [%"PRIu64"], dE [%"PRIu64"], dR [%"PRIu64"]\n"),
				prc->fd, prc->attr.type, prc->attr.config,
				prc->count.value,
				prc->count.time_enabled,
				prc->count.time_running,
				prc->delta.value,
				prc->delta.time_enabled,
				prc->delta.time_running));

	if (delta)
		return (prc->delta).value;
	return (prc->count).value;
}

uint64_t Perf::Read(int id, bool delta) {
	pRegisteredCounter_t prc = counters[id];
	if (delta)
		return (prc->delta).value;
	return (prc->count).value;
}

uint64_t Perf::Enabled(int id, bool delta) {
	pRegisteredCounter_t prc = counters[id];
	if (delta)
		return (prc->delta).time_enabled;
	return (prc->count).time_enabled;
}

uint64_t Perf::Running(int id, bool delta) {
	pRegisteredCounter_t prc = counters[id];
	if (delta)
		return (prc->delta).time_running;
	return (prc->count).time_running;
}

const char *
Perf::hw_event_names[PERF_COUNT_HW_MAX] = {
	"cycles",
	"instructions",
	"cache-references",
	"cache-misses",
	"branches",
	"branch-misses",
	"bus-cycles",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)
	"stalled-cycles-frontend",
	"stalled-cycles-backend",
#endif
};

const char *
Perf::sw_event_names[PERF_COUNT_SW_MAX] = {
	"cpu-clock",
	"task-clock",
	"page-faults",
	"context-switches",
	"CPU-migrations",
	"minor-faults",
	"major-faults",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)
	"alignment-faults",
	"emulation-faults",
#endif
};

const char *
Perf::hw_cache[PERF_COUNT_HW_CACHE_MAX][MAX_ALIASES] = {
	{ "L1-dcache",	"l1-d",	"l1d", "L1-data", },
	{ "L1-icache",	"l1-i",	"l1i", "L1-instruction", },
	{ "LLC", "L2", },
	{ "dTLB", "d-tlb", "Data-TLB",	},
	{ "iTLB", "i-tlb", "Instruction-TLB", },
	{ "branch", "branches", "bpu", "btb", "bpc", },
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)
	{ "node", },
#endif
};

const char *
Perf::hw_cache_op[PERF_COUNT_HW_CACHE_OP_MAX][MAX_ALIASES] = {
	{ "load", "loads", "read", },
	{ "store", "stores", "write", },
	{ "prefetch", "prefetches", "speculative-read", "speculative-load", },
};

const char *
Perf::hw_cache_result[PERF_COUNT_HW_CACHE_RESULT_MAX][MAX_ALIASES] = {
	{ "refs", "Reference", "ops", "access", },
	{ "misses", "miss", },
};

/*
 * cache operartion stat
 * L1I : Read and prefetch only
 * ITLB and BPU : Read-only
 */
unsigned long Perf::hw_cache_stat[C(MAX)] = {
 /* [C(L1D)]   = */ (CACHE_READ | CACHE_WRITE | CACHE_PREFETCH),
 /* [C(L1I)]   = */ (CACHE_READ | CACHE_PREFETCH),
 /* [C(LL)]    = */ (CACHE_READ | CACHE_WRITE | CACHE_PREFETCH),
 /* [C(DTLB)]  = */ (CACHE_READ | CACHE_WRITE | CACHE_PREFETCH),
 /* [C(ITLB)]  = */ (CACHE_READ),
 /* [C(BPU)]   = */ (CACHE_READ),
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)
 /* [C(NODE)]  = */ (CACHE_READ | CACHE_WRITE | CACHE_PREFETCH),
#endif
};


int Perf::IsCacheOpValid(uint8_t cache_type, uint8_t cache_op) {
	if (hw_cache_stat[cache_type] & COP(cache_op))
		return 1;   /* valid */
	return 0;   /* invalid */
}

const char *Perf::EventCacheName(
		uint8_t cache_type, uint8_t cache_op, uint8_t cache_result) {
	static char name[50];

	if (cache_result) {
		sprintf(name, "%s-%s-%s", hw_cache[cache_type][0],
				hw_cache_op[cache_op][0],
				hw_cache_result[cache_result][0]);
	} else {
		sprintf(name, "%s-%s", hw_cache[cache_type][0],
				hw_cache_op[cache_op][1]);
	}

	return name;
}

const char *Perf::EventName(int type, uint64_t config) {
	static char buf[32];

	if (type == PERF_TYPE_RAW) {
		sprintf(buf, "raw 0x%lx", config);
		return buf;
	}

	switch (type) {
	case PERF_TYPE_HARDWARE:
		if (config < PERF_COUNT_HW_MAX && hw_event_names[config])
			return hw_event_names[config];
		return "unknown-hardware";

	case PERF_TYPE_HW_CACHE:
		uint8_t cache_type, cache_op, cache_result;

		cache_type   = (config >>  0) & 0xff;
		if (cache_type > PERF_COUNT_HW_CACHE_MAX)
			return "unknown-ext-hardware-cache-type";

		cache_op     = (config >>  8) & 0xff;
		if (cache_op > PERF_COUNT_HW_CACHE_OP_MAX)
			return "unknown-ext-hardware-cache-op";

		cache_result = (config >> 16) & 0xff;
		if (cache_result > PERF_COUNT_HW_CACHE_RESULT_MAX)
			return "unknown-ext-hardware-cache-result";

		if (!IsCacheOpValid(cache_type, cache_op))
			return "invalid-cache";

		return EventCacheName(cache_type, cache_op, cache_result);

	case PERF_TYPE_SOFTWARE:
		if (config < PERF_COUNT_SW_MAX && sw_event_names[config])
			return sw_event_names[config];
		return "unknown-software";

	case PERF_TYPE_TRACEPOINT:
		return "unknown-tracepoint (TODO)";
		//return tracepoint_id_to_name(config);

	default:
		break;
	}

	return "unknown";
}

int Perf::FPrintf(FILE *fp, const char *color, const char *fmt, ...) {
	va_list args;
	int r = 0;

	va_start(args, fmt);

	r += fprintf(fp, "%s", color);
	r += vfprintf(fp, fmt, args);
	r += fprintf(fp, "%s", PERF_COLOR_RESET);

	va_end(args);
	return r;
}

} // namespace utils

} // namespace bbque

