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

#include "bbque/utils/deferrable.h"
#include "bbque/modules_factory.h"

#define DEFERRABLE_NAMESPACE "bq.df"

namespace bp = bbque::plugins;

namespace bbque { namespace utils {

Deferrable::Deferrable(const char *name,
		DeferredFunction_t func,
		milliseconds period) :
	name(name),
	func(func),
	max_time(period),
	next_time(system_clock::now()),
	done(false) {

	//---------- Get a logger module
	char logName[64];
	snprintf(logName, 64, DEFERRABLE_NAMESPACE".%s", name);
	bp::LoggerIF::Configuration conf(logName);
	logger = ModulesFactory::GetLoggerModule(std::cref(conf));
	if (!logger) {
		fprintf(stderr, "DF: Logger module creation FAILED\n");
		assert(logger);
	}

	if (max_time == SCHEDULE_NONE)
		logger->Debug("Starting new \"on-demand\" deferrable [%s]...",
				Name());
	else
		logger->Debug("Starting new \"repetitive\" deferrable [%s], period %d[ms]...",
				Name(), max_time);

	// Spawn the executor thread
	executor_thd = std::thread(&Deferrable::Executor, this);

	// Start "periodis" deferrables
	Start();

}

Deferrable::~Deferrable() {
	// Stop the executor thread
	Stop();
}

void Deferrable::Schedule(milliseconds time) {
	std::unique_lock<std::mutex> ul(trdStatus_mtx);
	DeferredTime_t request_time = system_clock::now();
	DeferredTime_t schedule_time;

	// Handle immediate scheduling
	if (time == SCHEDULE_NOW) {
		logger->Debug("DF[%s] immediate scheduling required", Name());
		next_timeout = SCHEDULE_NOW;
		trdStatus_cv.notify_one();
		return;
	}

	// Return if _now_ (i.e. schedule_time) we already have a pending
	// schedule which is nearest then the required schedule time
	if (likely(next_time > request_time)) {

		logger->Debug("DF[%s] checking for future schedule...", Name());

		schedule_time = request_time + time;
		if (schedule_time >= next_time) {
			logger->Debug("DF[%s] nearest then %d[ms] schedule pending",
					Name(), time);
			return;
		}
	}

	// Update for next nearest schedule time
	logger->Debug("DF[%s] update nearest schedule to %d[ms]", Name(), time);
	next_time = schedule_time;
	next_timeout = time;
	trdStatus_cv.notify_one();

}

void Deferrable::SetPeriodic(milliseconds period) {
	std::unique_lock<std::mutex> ul(trdStatus_mtx);

	if (period == SCHEDULE_NONE) {
		// Use the SetOnDemand call for this purpose
		assert(period != SCHEDULE_NONE);
		return;
	}

	logger->Info("DF[%s] set \"repetitive\" mode, period %d[ms]",
			Name(), period);

	max_time = period;
	trdStatus_cv.notify_one();
}

void Deferrable::SetOnDemand() {
	std::unique_lock<std::mutex> ul(trdStatus_mtx);
	logger->Info("DF[%s] set \"on-demand\" mode", Name());
	max_time = SCHEDULE_NONE;
	trdStatus_cv.notify_one();
}

void Deferrable::Start() {
	std::unique_lock<std::mutex> ul(trdStatus_mtx);

	logger->Debug("DF[%s] starting deferrable...", Name());
	trdRunning = true;
	trdStatus_cv.notify_one();
}

void Deferrable::Stop() {
	std::unique_lock<std::mutex> ul(trdStatus_mtx);

	if (done == true)
		return;

	logger->Debug("DF[%s] stopping deferrable...", Name());
	done = true;
	trdStatus_cv.notify_one();
	ul.unlock();

	// Waiting for the thread to exit
	if (executor_thd.joinable()) {
		logger->Debug("DF[%s] joining executor...", Name());
		executor_thd.join();
	}
}

#define RESET_TIMEOUT \
	do { \
		next_timeout = SCHEDULE_NONE; \
		if (max_time != SCHEDULE_NONE) \
			next_timeout = max_time; \
		next_time = system_clock::now() + next_timeout; \
	} while(0)

void Deferrable::Executor() {
	std::unique_lock<std::mutex> trdStatus_ul(trdStatus_mtx);
	std::cv_status wakeup_reason;

	// Set the module name
	char thdName[64];
	snprintf(thdName, 64, DEFERRABLE_NAMESPACE".%s", Name());
	if (prctl(PR_SET_NAME, (long unsigned int)thdName, 0, 0, 0) != 0) {
		logger->Error("Set name FAILED! (Error: %s)\n", strerror(errno));
	}

	// Waiting for thread authorization to start
	while (!trdRunning)
		trdStatus_cv.wait(trdStatus_ul);

	// Schedule next execution if we are "repetitive"
	RESET_TIMEOUT;

	logger->Info("DF[%s] Deferrable thread STARTED", Name());

	DB(tmr.start());
	while (!done) {

		// Wait for next execution or re-scheduling
		if (next_timeout == SCHEDULE_NONE) {
			DB(logger->Debug("DF[%s: %9.3f] on-demand waiting...",
					Name(), tmr.getElapsedTimeMs()));

			DB(tmr.start());
			trdStatus_cv.wait(trdStatus_ul);
			DB(logger->Debug("DF[%s: %9.3f] wakeup ON-DEMAND",
					Name(), tmr.getElapsedTimeMs()));

		} else {
			DB(logger->Debug("DF[%s: %9.3f] waiting for %d[ms]...",
					Name(), tmr.getElapsedTimeMs(),
					next_timeout));

			DB(tmr.start());
			wakeup_reason = trdStatus_cv.wait_for(trdStatus_ul, next_timeout);
			if (wakeup_reason == std::cv_status::timeout) {
				DB(logger->Debug("DF[%s: %9.3f] wakeup TIMEOUT",
						Name(), tmr.getElapsedTimeMs()));
				next_timeout = SCHEDULE_NOW;
			}

		}

		if (unlikely(done)) {
			logger->Info("DF[%s] exiting executor...", Name());
			continue;
		}

		// Timeout rescheduling due to nearest schedule
		if (next_timeout != SCHEDULE_NOW) {
			DB(logger->Debug("DF[%s: %9.3f] rescheduling timeout in %d[ms]",
					Name(), tmr.getElapsedTimeMs(),
					next_timeout));
			continue;
		}

		// Execute the deferred task
		logger->Info("DF[%s] execution START", Name());
		Execute();
		logger->Info("DF[%s] execution DONE", Name());

		// Setup the next timeout
		RESET_TIMEOUT;

	}

	logger->Info("DF[%s] Deferrable thread ENDED", Name());

}

} // namespace utils

} // namespace bbque

