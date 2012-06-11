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

#ifndef BBQUE_DEFERRABLE_H_
#define BBQUE_DEFERRABLE_H_

#include "bbque/cpp11/chrono.h"
#include "bbque/cpp11/mutex.h"
#include "bbque/cpp11/thread.h"
#include "bbque/plugins/logger.h"
#include "bbque/utils/utility.h"
#include "bbque/utils/timer.h"

#include <functional>

using std::chrono::time_point;
using std::chrono::milliseconds;
using std::chrono::system_clock;

using bbque::plugins::LoggerIF;
using bbque::utils::Timer;

namespace bbque { namespace utils {

/**
 * @brief Deferrable execution of specified functionalities
 *
 * This class provides the basic support to schedule a deferrable exeuction of
 * a specified task, with the possibility to update the scheduled execution
 * time and also to repete the execution at specified intervals.
 * On each time, the most recent future execution request is executed, by
 * discarding all the (non periodic) older ones.
 */
class Deferrable {

public:

	typedef time_point<system_clock> DeferredTime_t;
	typedef std::function<void(void)> DeferredFunction_t;

#define SCHEDULE_NONE milliseconds(0)

	/**
	 * @brief Build a new "on-demand" or "repetitive" deferrable object
	 *
	 * @param name the name of this executor, used mainly for logging
	 * purposes
	 * @param period when not null, the minimum execution period [ms] for
	 * each execution
	 */
	Deferrable(const char *name, DeferredFunction_t func = NULL,
			milliseconds period = SCHEDULE_NONE);

	/**
	 * @brief Release all deferrable resources
	 */
	virtual ~Deferrable();

	const char *Name() const {
		return name.c_str();
	}

	/**
	 * @brief The operations to execute
	 *
	 * This should be implemeted by derived classes with the set of
	 * operations to be executed at the next nearest scheduled time.
	 */
	virtual void Execute() {
		fprintf(stderr, "Deferrable::Execute\n");
		if (func) func();
	}

#define SCHEDULE_NOW  milliseconds(0)

	/**
	 * @brief Schedule a new execution
	 *
	 * A new execution of this deferred is scheduled into the specified
	 * time, or immediately if time is 0.
	 */
	void Schedule(milliseconds time = SCHEDULE_NOW);

	/**
	 * @brief Update the "repetitive" scheduling period
	 */
	void SetPeriodic(milliseconds period);

	/**
	 * @brief Set the deferrable to be just on-demand
	 */
	void SetOnDemand();

protected:

	/**
	 * @brief The defarrable name
	 */
	const std::string name;

	/**
	 * @brief The logger to use.
	 */
	LoggerIF *logger;

private:

	/**
	 * @brief
	 */
	DeferredFunction_t func;

	/**
	 * @brief The minimum "repetition" period [ms] for the execution
	 *
	 * When this period is 0 this is just an on-demand deferrable which
	 * execute one time for the nearest of the pending schedule.
	 * Otherwise, when not zero, an execution is triggered at least once
	 * every min_time
	 */
	milliseconds max_time;

	/**
	 * @brief The next execution time
	 */
	DeferredTime_t next_time;

	/**
	 * @brief A timeout for the nearest execution time
	 */
	milliseconds next_timeout;

	/**
	 * @brief The deferrable executor thread
	 */
	std::thread executor_thd;

	/**
	 * @brief Status of the executor thread
	 *
	 * This variable is set true when the executor thread has been autorized
	 * to run.
	 */
	bool trdRunning;

	/**
	 * @brief Set true to terminate the executor thread
	 *
	 * This is set to true to end the executor thread.
	 */
	bool done;

	/*
	 * @brief Mutex controlling the thread execution
	 */
	std::mutex trdStatus_mtx;

	/**
	 * @brief Conditional variable used to signal the monitoring thread
	 */
	std::condition_variable trdStatus_cv;

	/**
	 * @brief A timer used for validation
	 */
	DB(Timer tmr);

	/**
	 * @brief Start the executor thread
	 */
	void Start();

	/**
	 * @brief Stop the executor thread
	 */
	void Stop();

	/**
	 * @brief The thread providing the execution context for this deferrable
	 */
	void Executor();

};

} // namespace utils

} // namespace bbque

#endif // BBQUE_DEFERRABLE_H_
