/**
 *       @file  perf.h
 *      @brief  Performance Counters support
 *
 * This class provides basic service to use Linux performance counters.
 * Its implementation has been inspired from the "perf" tool provided with the
 * Linux kernel sources. Special credits for this goes to:
 *  Thomas Gleixner and Ingo Molnar
 * and all the other guys which contributed to the Linux Performance events
 * framework.
 *
 *     @author  Patrick Bellasi (derkling), derkling@google.com
 *
 *   @internal
 *     Created  11/05/2011
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#ifndef BBQUE_PERF_H_
#define BBQUE_PERF_H_

#include <stdint.h>
#include <string.h>

#include <linux/version.h>
#include <linux/perf_event.h>

#include <map>
#include <memory>

#include "bbque/utils/utility.h"

#define PERF_HW(COUNTER) \
	PERF_TYPE_HARDWARE, PERF_COUNT_HW_##COUNTER
#define PERF_SW(COUNTER) \
	PERF_TYPE_SOFTWARE, PERF_COUNT_SW_##COUNTER
#define PERF_HC(COUNTER) \
	PERF_TYPE_HW_CACHE, COUNTER

#define PERF_COLOR_NORMAL   ""
#define PERF_COLOR_RESET    "\033[m"
#define PERF_COLOR_BOLD     "\033[1m"
#define PERF_COLOR_RED      "\033[31m"
#define PERF_COLOR_GREEN    "\033[32m"
#define PERF_COLOR_YELLOW   "\033[33m"
#define PERF_COLOR_BLUE     "\033[34m"
#define PERF_COLOR_MAGENTA  "\033[35m"
#define PERF_COLOR_CYAN     "\033[36m"
#define PERF_COLOR_BG_RED   "\033[41m"


// These are a set of value suitable configurations to monitor CACHE usage
#define L1DC_RA (PERF_COUNT_HW_CACHE_L1D      | \
		(PERF_COUNT_HW_CACHE_OP_READ << 8)        | \
		(PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))
#define L1DC_RM (PERF_COUNT_HW_CACHE_L1D      | \
		(PERF_COUNT_HW_CACHE_OP_READ << 8)        | \
		(PERF_COUNT_HW_CACHE_RESULT_MISS << 16))
#define L1DC_WA (PERF_COUNT_HW_CACHE_L1D      | \
		(PERF_COUNT_HW_CACHE_OP_WRITE << 8)       | \
		(PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))
#define L1DC_WM (PERF_COUNT_HW_CACHE_L1D      | \
		(PERF_COUNT_HW_CACHE_OP_WRITE << 8)       | \
		(PERF_COUNT_HW_CACHE_RESULT_MISS << 16))
#define L1DC_PA (PERF_COUNT_HW_CACHE_L1D      | \
		(PERF_COUNT_HW_CACHE_OP_PREFETCH << 8)    | \
		(PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))
#define L1DC_PM (PERF_COUNT_HW_CACHE_L1D      | \
		(PERF_COUNT_HW_CACHE_OP_PREFETCH << 8)    | \
		(PERF_COUNT_HW_CACHE_RESULT_MISS << 16))
#define L1IC_RA (PERF_COUNT_HW_CACHE_L1I      | \
		(PERF_COUNT_HW_CACHE_OP_READ << 8)        | \
		(PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))
#define L1IC_RM (PERF_COUNT_HW_CACHE_L1I      | \
		(PERF_COUNT_HW_CACHE_OP_READ << 8)        | \
		(PERF_COUNT_HW_CACHE_RESULT_MISS << 16))
#define LLC_RA (PERF_COUNT_HW_CACHE_LL        | \
		(PERF_COUNT_HW_CACHE_OP_READ << 8)        | \
		(PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))
#define LLC_RM (PERF_COUNT_HW_CACHE_LL        | \
		(PERF_COUNT_HW_CACHE_OP_READ << 8)        | \
		(PERF_COUNT_HW_CACHE_RESULT_MISS << 16))
#define LLC_WA (PERF_COUNT_HW_CACHE_LL        | \
		(PERF_COUNT_HW_CACHE_OP_WRITE << 8)       | \
		(PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))
#define LLC_WM (PERF_COUNT_HW_CACHE_LL        | \
		(PERF_COUNT_HW_CACHE_OP_WRITE << 8)       | \
		(PERF_COUNT_HW_CACHE_RESULT_MISS << 16))
#define DTLB_RA (PERF_COUNT_HW_CACHE_DTLB    | \
		(PERF_COUNT_HW_CACHE_OP_READ << 8)        | \
		(PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))
#define DTLB_RM (PERF_COUNT_HW_CACHE_DTLB    | \
		(PERF_COUNT_HW_CACHE_OP_READ << 8)        | \
		(PERF_COUNT_HW_CACHE_RESULT_MISS << 16))
#define DTLB_WA (PERF_COUNT_HW_CACHE_DTLB    | \
		(PERF_COUNT_HW_CACHE_OP_WRITE << 8)       | \
		(PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))
#define DTLB_WM (PERF_COUNT_HW_CACHE_DTLB    | \
		(PERF_COUNT_HW_CACHE_OP_WRITE << 8)       | \
		(PERF_COUNT_HW_CACHE_RESULT_MISS << 16))
#define ITLB_RA (PERF_COUNT_HW_CACHE_ITLB    | \
		(PERF_COUNT_HW_CACHE_OP_READ << 8)        | \
		(PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))
#define ITLB_RM (PERF_COUNT_HW_CACHE_ITLB    | \
		(PERF_COUNT_HW_CACHE_OP_READ << 8)        | \
		(PERF_COUNT_HW_CACHE_RESULT_MISS << 16))
#define BPU_RA (PERF_COUNT_HW_CACHE_BPU      | \
		(PERF_COUNT_HW_CACHE_OP_READ << 8)        | \
		(PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))
#define BPU_RM (PERF_COUNT_HW_CACHE_BPU      | \
		(PERF_COUNT_HW_CACHE_OP_READ << 8)        | \
		(PERF_COUNT_HW_CACHE_RESULT_MISS << 16))


namespace bbque { namespace utils {

class Perf {

public:

	/**
	 * @brief Build a new Perf object
	 */
	Perf();

	/**
	 * @brief Release all counters
	 */
	~Perf();

	/**
	 * @brief Add performance counter
	 *
	 * Add a new performance counter to this group of counters. Counters
	 * on a group will be co-scheduled, if possible, otherwise none of them
	 * will count.
	 * The first added counter is the "group leader" of the successive added
	 * counter.
	 * Once a counter has been added, an index is reqiured which could be used
	 * by the following read methods to get back the counter values and other
	 * attributed, e.g. enabled and running time.
	 *
	 * @return the ID of the added counter on SUCCESS, -1 on error
	 */
	int AddCounter(perf_type_id type, uint64_t config,
			bool exclude_kernel = false);

	/**
	 * @brief Add a new counter of type HARDWARE
	 */
#define AddCounterHW(TYPE) \
	AddCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_ ## TYPE)

	/**
	 * @brief Add a new counter of type HARDWARE (excluding kernel)
	 */
#define AddCounterHW_NOKERN(TYPE) \
	AddCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_ ## TYPE, true)

	/**
	 * @brief Add a new counter of type SOFTWARE
	 */
#define AddCounterSW(TYPE) \
	AddCounter(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_ ## TYPE)

	/**
	 * @brief Add a new counter of type SOFTWARE (excluding kernel)
	 */
#define AddCounterSW_NOKERN(TYPE) \
	AddCounter(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_ ## TYPE, true)

	/**
	 * @brief Add a new counter of type TRACEPOINT
	 */
	int AddCounterTP(uint64_t config, bool exclude_kernel = false) {
		return AddCounter(PERF_TYPE_TRACEPOINT, config, exclude_kernel);
	}

	/**
	 * @brief Add a new counter of type HARDWARE CACHE
	 */
	int AddCounterHC(uint64_t config, bool exclude_kernel = false) {
		return AddCounter(PERF_TYPE_HW_CACHE, config, exclude_kernel);
	}

	/**
	 * @brief Add a new counter of type RAW (i.e. platform defined)
	 */
	int AddCounterRAW(uint64_t config, bool exclude_kernel = false) {
		return AddCounter(PERF_TYPE_RAW, config, exclude_kernel);
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)
	/**
	 * @brief Add a new counter of type BREACKPOINT
	 */
	int AddCounterBP(uint64_t config, bool exclude_kernel = false) {
		return AddCounter(PERF_TYPE_BREAKPOINT, config, exclude_kernel);
	}
#endif

	/**
	 * @brief Enable associated performance counters
	 */
	int Enable();

	/**
	 * @brief Disable the associated performance counters
	 */
	int Disable();

	/**
	 * @brief Update the specified performance counter
	 * @return the counter absolute value, or a relative difference since last
	 * update if delta is true.
	 */
	uint64_t Update(int id, bool delta = true);

	/**
	 * @brief Read the performance counter value
	 */
	uint64_t Read(int id, bool delta = true);

	/**
	 * @brief The time the counter has been enabled
	 */
	uint64_t Enabled(int id, bool delta = true);

	/**
	 * @brief The time the counter has been running
	 */
	uint64_t Running(int id, bool delta = true);

	/**
	 * @brief Get the string name of the specified event
	 */
	static const char *EventName(int type, uint64_t config);

	/**
	 * @brief Color print a log/debug message
	 */
	static int FPrintf(FILE *fp, const char *color, const char *fmt, ...);

private:

	/**
	 * @brief Perf counter group leader
	 */
	int fd_group;

	/**
	 * @brief The format of bytes readed from kernel space
	 */
	typedef struct ReadFormat {
		uint64_t value;
		uint64_t time_enabled;
		uint64_t time_running;
	} ReadFormat_t;

	/**
	 * @brief Informations on a registered counter
	 */
	typedef struct RegisteredCounter {
		/** The counter associated FD */
		int fd;
		/** The counter monitored PID */
		pid_t pid;
		/** The attributed of this counter */
		struct perf_event_attr attr;

		/** Counters values as of last last update */
		ReadFormat_t count;

		/** Counters values since last update */
		ReadFormat_t delta;

		RegisteredCounter() :
			fd(-1), pid(-1) {
			memset(&attr,  0, sizeof(attr));
			memset(&count, 0, sizeof(count));
			memset(&delta, 0, sizeof(delta));
		};

		~RegisteredCounter() {
			if (fd != -1)
				close(fd);
		}

	} RegisteredCounter;

	/**
	 * @brief A pointer to a registered counter
	 */
	typedef std::shared_ptr<RegisteredCounter> pRegisteredCounter_t;

	/**
	 * @brief Map registered counters on their handler
	 */
	typedef std::map<int, pRegisteredCounter_t> RegisteredCountersMap_t;

	/**
	 * @brief An entry of the RegisteredCounterMap_t
	 */
	typedef std::pair<int, pRegisteredCounter_t> RegisteredCountersMapEntry_t;

	/**
	 * @brief True if counters has been successfully opened
	 */
	bool opened;

	/**
	 * @brief A map of registered performance counters
	 */
	RegisteredCountersMap_t counters;

	/**
	 * @brief The Group Leader counter
	 */
	pRegisteredCounter_t pGroupLeader;

	/**
	 * @brief Return the number of registered counters
	 */
	uint8_t RegisteredCountersCount() {
		return counters.size();
	}

	/**
	 * @brief Return the ID of the counter leader
	 */
	int GroupLeader() {
		if (!pGroupLeader)
			return -1;
		return pGroupLeader->fd;
	}

	/**
	 * @brief Check if a counter leader has been defined
	 */
	bool IsGroupLeaderDefined() {
		return (GroupLeader() != -1);
	}

	/**
	 * @brief Register the specified counter in kernel space
	 */
	int EventOpen(struct perf_event_attr *attr,
			pid_t pid, int cpu, int group_fd,
			unsigned long flags);

	/**
	 * @brief Ensure a proper reading of a counter from kernel space
	 */
	int ReadCounter(int fd, void *buf, size_t n);

	/**
	 * @brief Check if the specified event is a valid CACHE event
	 */
	static int IsCacheOpValid(uint8_t cache_type, uint8_t cache_op);

	/**
	 * @brief Get the string name of CACHE event
	 */
	static const char *EventCacheName(uint8_t cache_type, uint8_t cache_op,
							uint8_t cache_result);

#define C(x)        	PERF_COUNT_HW_CACHE_##x
#define CACHE_READ  	(1 << C(OP_READ))
#define CACHE_WRITE 	(1 << C(OP_WRITE))
#define CACHE_PREFETCH	(1 << C(OP_PREFETCH))
#define COP(x)      	(1 << x)

	/**
	 * @brief Valid configurations for CACHE events monitoring
	 */
	static unsigned long hw_cache_stat[C(MAX)];

	/**
	 * @brief String names for HW events
	 */
	static const char *hw_event_names[PERF_COUNT_HW_MAX];

	/**
	 * @brief String names for SW events
	 */
	static const char *sw_event_names[PERF_COUNT_SW_MAX];

#define MAX_ALIASES 8

	/**
	 * @brief String names for different CACHES types
	 */
	static const char *hw_cache[PERF_COUNT_HW_CACHE_MAX][MAX_ALIASES];

	/**
	 * @brief String names for different CACHES operations
	 */
	static const char *hw_cache_op[PERF_COUNT_HW_CACHE_OP_MAX][MAX_ALIASES];

	/**
	 * @brief String names for different CACHE operation results
	 */
	static const char
		*hw_cache_result[PERF_COUNT_HW_CACHE_RESULT_MAX][MAX_ALIASES];

};

} // namespace utils

} // namespace bbque

#endif // BBQUE_PERF_H_

