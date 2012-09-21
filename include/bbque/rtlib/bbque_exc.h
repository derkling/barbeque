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

#ifndef BBQUE_EXC_H_
#define BBQUE_EXC_H_

#include <bbque/rtlib.h>
#include <bbque/utils/timer.h>
#include <bbque/cpp11/thread.h>

#include <map>
#include <string>

namespace bbque { namespace rtlib {

/**
 * @brief The AEM base-class
 * @ingroup rtlib_sec02_aem
 *
 * This is a base class suitable for the implementation of an EXC that should
 * be managed by the Barbeque RTRM.
 */
class BbqueEXC {

public:

/**
 * @brief Build a new EXC
 *
 * A new EXecution Context could be build by specifying a <i>name</i>,
 * which identifies it within the system and it is used mostly for
 * logging statements, and a <i>recipe</i>, which specifies the set of AWM
 * supported by the specific stream processing application being
 * defined.
 *
 * The creation of a new EXC requires also a valid handler to the RTLib, which
 * will be used for the (application transparent) communication with the
 * BarbequeRTRM. Thus, an application willing to instantiate an EXC should
 * foremost initialize the RTLib, which could be done as explained in @ref
 * rtlib_sec03_plain.
 *
 * This is an example snippet showing how to properly instantiate an EXC:
 * \code
 *
 * #include <bbque/bbque_exc.h>
 *
 * class ExampleEXC : public BbqueEXC {
 * // definition of the application specific EXC
 * };
 *
 * // The EXC handler
 * pBbqueEXC_t pexc;
 * pexc = pBbqueEXC_t(new ExampleEXC("YourExcName", "YourRecipeName", rtlib));
 *
 * // Checking registration was successful
 * if (!pexc || !pexc->isRegistered()) {
 * 	fprintf(stderr, "EXC registration FAILED\n");
 * 	return EXIT_FAILURE;
 * }
 *
 * \endcode
 *
 * @note To properly exploit the RTLib provided instrumentation for the
 * profiling of run-time behaviors of the EXC, the application integrator
 * should avoid to get resources, such as spawning threads (e.g. setup a
 * thread pool), from within the EXC constructor.
 * This method should be used just to pass the EXC a set of configuration
 * parameters to be saved locally to the derived class, while the actual
 * initialization code should be placed into the \ref onSetup method, which
 * indeed it is called by the base class right after the constructor.
 *
 * @see onSetup
 *
 * @param name the name of the EXC
 * @param recipe the recipe to run-time manage this EXC
 * @param rtlib a reference to the RTLib
 *
 * @ingroup rtlib_sec02_aem_exc
 */
	BbqueEXC(std::string const & name,
			std::string const & recipe,
			RTLIB_Services_t *rtlib);

/**
 * @brief Destory the EXC
 */
	virtual ~BbqueEXC();


/*******************************************************************************
 *    AEM EXecution Context Management
 ******************************************************************************/

/**
 * @brief Check if the EXC has been properly registered
 *
 * The instantiation of a new EXC triggers also its registration within the
 * BarbequeRTRM. This method could be used to check if the registration has
 * been completed with success.
 *
 * @ingroup rtlib_sec02_aem_exc
 */
	inline bool isRegistered() const {
		return registered;
	}

/**
 * @brief Start the EXC thus asking resources
 *
 * Once an EXC has been successfully registered, by default, it is considered
 * disabled by the BarbequeRTRM; thus no resources are assigned to it until this
 * method is called.
 * A call to this method actually enable the AEM defined state machine and
 * issue a resources scheduling request to the system-wide run time manager.
 *
 * Once this call returns, the EXC could get resources assigned to it by an
 * asynchronous call to its \ref onConfigure method.
 *
 * @note the first time an EXC should be started there is not need to
 * explicitely \ref Enable it.
 *
 * @ingroup rtlib_sec02_aem_exc
 */
	RTLIB_ExitCode_t Start();

/**
 * @brief Wait for the completion of workload processing
 *
 * The main application which create EXCs could sync with their completion
 * by simply issuing a call to this method.
 * This is a blocking method which returns only when the corresponding EXC has
 * completed its processing.
 *
 * @ingroup rtlib_sec02_aem_exc
 */
	RTLIB_ExitCode_t WaitCompletion();

/**
 * @brief Stop the EXC thus releasing all resources
 *
 * A call to this method force the EXC to terminate right after the completion
 * of the currently running processing cycle.
 * This is a blocking method which returns only when the corresponding EXC has
 * been terminated.
 *
 * @ingroup rtlib_sec02_aem_exc
 */
	RTLIB_ExitCode_t Terminate();

/**
 * @brief Enable the EXC to resources assignment
 *
 * EXC could be enabled/disabled at run-time; a disabled EXC will not be
 * assigned resources by the BarbequeRTRM until it will be re-enabled.
 * This method could be used to re-enable a <i>previously disabled</i> EXC.
 *
 * Once this call returns, the EXC could get resources assigned to it by an
 * asynchronous call to itd \ref onConfigure method.
 *
 * @ingroup rtlib_sec02_aem_exc
 */
	RTLIB_ExitCode_t Enable();

/**
 * @brief Disable the EXC to resources assignment
 *
 * An EXC could be disabled at run-time to temporarily release all resources
 * eventually assigned to it. Moreover, this call notifies the BarbequeRTRM
 * that it should not care about assigning resources to it.
 *
 * @ingroup rtlib_sec02_aem_exc
 */
	RTLIB_ExitCode_t Disable();


/*******************************************************************************
 *    AEM Constraints Management
 ******************************************************************************/

/**
 * @brief Update constraints on AWM selection
 *
 * This method provides support for the "mandatory" selection of AWM which are
 * considered acceptable at run-time according to the specific needs of an
 * application. Indeed, by calling this method the application could define a
 * subset of all the possible working modes defined by the associated recipe,
 * which represents the only acceptable AWM assignment for the application.
 *
 * A set of constraints could be defined with a single call of this method.
 * Each constraint:
 * <ul><li>
 * refers to a specific <i>AWM ID</i>
 * </li><li>
 * could be of type: <i>upper bound</i>, <i>lower bound</i> or
 * <i>exact value</i>
 * </li><li>
 * could be <i>added</i> or <i>removed</i>
 * </li></ul>
 * By mixing addition of new constraints and removal of previously defined
 * ones, it is possible to completely customize the set of overall recipe
 * defined AWMs to shape a custom set of valid AWMs.
 * Examples:
 * <ul><li>
 * <code>[2, UpperBound, Add]</code>: add an upper bound on the set of valid AWM, all the
 * AWM with ID > 2 will be disabled
 * </li><li>
 * <code>[4, ExactValue, Add]</code>: add an exact value on the set of valid
 * AWM, AWM with ID 4 is considered enabled
 * </li></ul>
 * Thus, at the end of such a configuration, all AWMs but
 * <code>{0,1,2,4}</code> are considered disabled and thus ignored by the
 * BarbequeRTRM for this specific EXC.
 *
 * @note
 * A call to this method is granted to be <i>atomic</i> at the BarbeqeRTRM
 * side, which means that all the specified constraints actions are processed
 * before eventually triggering a new re-scheduling.
 *
 * @note
 * It is worth to notice that this is a <b>mandatory API</b> which means that,
 * once some AWM has been invalidated, if the BarbequeRTRM is not able to
 * assign one of the remaining AWMs to the application (e.g. because there are
 * not resources available to accommodate one of them), than the application
 * will be forcibly suspended.
 *
 * @return RTLIB_OK on success, RTLIB_ERROR or one of the other exit codes
 * otherwise.
 *
 * @ingroup rtlib_sec02_aem_constr
 */
	RTLIB_ExitCode_t SetConstraints(
		RTLIB_Constraint_t *constraints,
		uint8_t count);

/**
 * @brief Clear all constraints on AWM selection
 *
 * Release all the previously asserted constraints, thus considering valid all
 * the AWM refined by the EXC defined recipe.
 *
 * @return RTLIB_OK on success, RTLIB_ERROR or one of the other exit codes
 * otherwise.
 *
 * @ingroup rtlib_sec02_aem_constr
 */
	RTLIB_ExitCode_t ClearConstraints();

/**
 * @brief Assert a new value for the actual Normalized Actual Penalty (NAP)
 *
 * This method provides support for the "best-effort" selection of a different
 * AWM.  If the application is not satisfied with the performances obtained by
 * executing in the currently assigned AWM, i.e. the performances obtained are
 * lower than the one estimated at profile time, than the application could
 * ask the BarbequeRTRM to be assigned more resources by switching to an
 * higher value AWM.
 *
 * An estimate of how far an EXC is from its performance goal is named
 * <b>Normalized Actual Penalty</b> when this distance is normalized and
 * expressed with a percentage, e.g. I expect 30 fps, but only 25 fps could be
 * obtained, than the NAP is <code>(1 - 25/30) = 16%</code>.
 *
 * The BarbequeRTRM uses the NAP value to figure out how to repartition the
 * available resources in order to make all concurrently running applications
 * to have the same (possible ZERO) penalty value.
 *
 * @note
 * A NAP value should be asserted, by a call to this method, only if the
 * application is getting <i>lower performances</i> with respect of the
 * expectations for the currently selected AWM. To the contrary, if the
 * application is getting better performances than the expected one or if the
 * application is not completely exploiting all the resources assigned, the
 * NAP value would be negative and it should not be asserted using this call.
 * Indeed, it is up to the BarbequeRTRM to monitor how well applications are
 * exploiting assigned resources and eventually reclaim unused ones.
 *
 * @return RTLIB_OK on success, RTLIB_ERROR or one of the other exit codes
 * otherwise.
 *
 * @ingroup rtlib_sec02_aem_constr
 */
	RTLIB_ExitCode_t SetGoalGap(uint8_t percent);


/*******************************************************************************
 *    AEM Utilities
 ******************************************************************************/

/**
 * @brief Get the EXC Unique IDentifier
 *
 * Each EXC is assigned a Unique IDentifier (UID) which is used for logging
 * both at RTLib and BqrbequeRTRM side. This method gives a reference to a
 * string representing this UID.
 *
 * @return a pointer to the UID string identitying this EXC
 *
 * @ingroup rtlib_sec02_aem_utils
 */
	const char *GetUid() const;

/**
 * @brief Set the cycle rate for this EXC
 *
 * The AEM defined by this API runs a processing cycle which corresponds to a
 * loop of \ref onRun and \ref onMonitor calls.
 *
 * This method allows to set the maximum cycle rate to the specified <i>Cycles
 * Per Second (CPS)</i> value. If the cycle loop should correspond to a rate
 * higher, then the configured CPS value is enforce by the introduction of a
 * properly calibrated delay.
 *
 * @return RTLIB_OK on success, RTLIB_ERROR or one of the other exit codes
 * otherwise.
 *
 * @ingroup rtlib_sec02_aem_utils
 */
	RTLIB_ExitCode_t SetCPS(float cps);

/**
 * @brief Set the cycle time for this EXC
 *
 * The AEM defined by this API runs a processing cycle which corresponds to a
 * loop of \ref onRun and \ref onMonitor calls.
 *
 * This method allows to set the minimum cycle time to the specified amount of
 * milliseconds [ms] value. If the cycle loop should correspond to a lower
 * cycle time, then, the configured cycle time value is enforce by the
 * introduction of a properly calibrated delay.
 *
 * @return RTLIB_OK on success, RTLIB_ERROR or one of the other exit codes
 * otherwise.
 *
 * @ingroup rtlib_sec02_aem_utils
 */
	RTLIB_ExitCode_t SetCTimeUs(uint32_t us);

/**
 * @brief The number of completed cycles
 *
 * The RTLib keeps track of the completed processing cycles count.
 * This method return the total numer of cycles completed since the starting
 * of the EXC.
 *
 * @return RTLIB_OK on success, RTLIB_ERROR or one of the other exit codes
 * otherwise.
 *
 * @ingroup rtlib_sec02_aem_utils
 */
	inline uint32_t Cycles() const {
		return cycles_count;
	}

/**
 * @brief The currently assigned AWM
 *
 * Each time the EXC is running it has an assigned AWM. This method return a
 * referece to the currently assigned application working mode.
 *
 * @return RTLIB_OK on success, RTLIB_ERROR or one of the other exit codes
 * otherwise.
 *
 * @ingroup rtlib_sec02_aem_utils
 */
	RTLIB_WorkingModeParams_t const & WorkingModeParams() const {
		return wmp;
	}

/**
 * @brief Check if the processing has completed
 *
 * This method return true just when the processing cycle has been completed
 * and the EXC is going to terminate.
 *
 * @return true if the EXC is going to terminate, false otherwise.
 *
 * @ingroup rtlib_sec02_aem_utils
 */
	inline bool Done() const {
		return done;
	}

/**
 * @brief ID of the last assigned AWM
 *
 * Each time the EXC is running it has an assigned AWM. This method return the
 * identified (ID) to the currently (or last) assigned application working mode.
 *
 * @return the ID of the last assigned AWM
 *
 * @ingroup rtlib_sec02_aem_utils
 */
	int8_t CurrentAWM() const {
		if (suspended)
			return -1;
		return wmp.awm_id;
	}

protected:

/**
 * @brief The name of this EXC
 */
	std::string const exc_name;

/**
 * @brief The name of the Recipe used by this EXC
 */
	std::string const rpc_name;

/*******************************************************************************
 *    AEM Application Callbacks
 ******************************************************************************/

/**
 * @brief Setup the EXC for execution
 *
 * This is the application defined call-back method to host all the EXC
 * initialization code. This method will be called by the base class right
 * after the constructor and it's the right place to host all the code to
 * prepare the ground for the stream processing, e.g. opening input and output
 * channels, setup internal data structures for the processing cycle to start.
 *
 * @return RTLIB_OK if everything is ready to start the processing,
 * RTLIB_ERROR or one of the others exit codes otherwise.
 *
 * @ingroup rtlib_sec02_aem_app
 */
	virtual RTLIB_ExitCode_t onSetup();

/**
 * @brief Configure a new AWM for the EXC to continue its execution
 *
 * This is the application defined call-back method to host the required
 * configuration code to switch to a new assigned AWM. The currently assigned
 * working mode is defined by its ID with reference to the IDs defined by the
 * \ref Recipe specified at the EXC instantiation time.
 *
 * @note It is up to the application developer to know the proper mapping
 * between an AWM ID and the corresponding application parameters.
 * However, the high level abstraction \ref rtlib_sec04_rtrm provides a
 * set of automatized mechanisms to define a compile time a look-up table for
 * such parameters starting form the content of a recipe file.
 *
 * @param awm_id the ID of the currently assigned AWM
 *
 * @return RTLIB_OK on success, RTLIB_ERROR or one of the other exit codes
 * otherwise.
 *
 * @ingroup rtlib_sec02_aem_app
 */
	virtual RTLIB_ExitCode_t onConfigure(uint8_t awm_id);

/**
 * @brief Suspend the execution of this EXC
 *
 * If the execution of the EXC should be suspended, for example because there
 * are temporary no resources available for it, than this method is called
 * back right after the completion of the current processing cycle.
 *
 * This method is intended to host all the necessary code required to keep
 * safe all the resources allocated on the accelerator in anticipation of an
 * imminent suspension of the EXC. Once the execution will be resumed the
 * corresponding \ref onResume method is called.
 *
 * @return RTLIB_OK on success, RTLIB_ERROR or one of the other exit codes
 * otherwise.
 *
 * @ingroup rtlib_sec02_aem_app
 */
	virtual RTLIB_ExitCode_t onSuspend();

/**
 * @brief Resume the execution of this EXC
 *
 * This method is intended to host all the necessary code required to resume
 * the execution of a previously suspended EXC.
 *
 * @see onSuspend
 *
 * @return RTLIB_OK on success, RTLIB_ERROR or one of the other exit codes
 * otherwise.
 *
 * @ingroup rtlib_sec02_aem_app
 */
	virtual RTLIB_ExitCode_t onResume();

/**
 * @brief Run the next processing cycle
 *
 * This is one of the main call-back methods which is required to be defined
 * by an application. It is intended to host all the code required to actually
 * process a cycle of the stream processing application, e.g. decoding a single
 * video frame.
 *
 * The amount of data to be processed by a single call of this method it is
 * not technically upper bounded. However, it is worth to consider its
 * execution time and the effects on the latencies forced on the run-time
 * management of the whole system.
 *
 * Once there are not more data to be processed, this method should return
 * RTLIB_EXC_WORKLOAD_NONE to actually terminate the processing cycle. In this
 * case the next \ref onMonitor will not be called and the execution will
 * continue with the \ref onRelease.
 *
 * @note
 * The BarbequeRTRM does its best in avoiding to interrupt an application
 * while it is executing this method. However, in case the latency introduced
 * by a single call of this method should not be compliant with the run-time
 * management goals (e.g. scheduling a just started high-priority application)
 * the RTRM has the capability to force a termination of this method, i.e.
 * eventually also by kill a "not responding" application.
 *
 * @note
 * The code to monitor execution performances of a single call of this
 * method should be better placed into the \ref onMonitor call-back.
 *
 * @see onMonitor
 *
 * @return RTLIB_OK if the processing cycle has been properly completed,
 * RTLIB_EXC_WORKLOAD_NONE if there is not more workload to be processed,
 * RTLIB_ERROR or one of the others exit codes otherwise.
 *
 * @ingroup rtlib_sec02_aem_app
 */
	virtual RTLIB_ExitCode_t onRun();

/**
 * @brief Monitor performances of the last processing cycle
 *
 * This method is called right after each execution of \ref onRun, thus this
 * is the most suitable place for all the run-time monitoring code.  The
 * application developer could exploit this method to implement an
 * <b>application specific</b> run-time management policy, e.g. to tune some
 * application specific parameters based on the behaviors obtained during the
 * previous cycle execution.
 *
 * @return RTLIB_OK if the monitoring has been properly completed,
 * RTLIB_EXC_WORKLOAD_NONE if there is not more workload to be processed,
 * RTLIB_ERROR or one of the others exit codes otherwise.
 *
 * @ingroup rtlib_sec02_aem_app
 */
	virtual RTLIB_ExitCode_t onMonitor();

/**
 * @brief Release all EXC resources before termination
 *
 * This is the application defined call-back method to host all the EXC
 * shutdown code. This method will be called by the base class right before
 * the destructor and it's the right place to host all the code to clean-up
 * everything since the stream processing application is going to be
 * terminated, e.g. closing input and output channels, release internal data
 * structures.
 *
 * @return RTLIB_OK if everything has been properly released,
 * RTLIB_ERROR or one of the others exit codes otherwise.
 *
 * @ingroup rtlib_sec02_aem_app
 */
	virtual RTLIB_ExitCode_t onRelease();


private:

/*******************************************************************************
 *    EXC Handlers and Status
 ******************************************************************************/

	RTLIB_Services_t * const rtlib;

	RTLIB_ExecutionContextHandler_t exc_hdl;

	/**
	 * @brief The number of onRun executions
	 * 
	 * This counter is incremented at each onRun execution thus allowing to
	 * keep track of the amount of workload processed.
	 * Considering a 30fps video decoding, where an onRun is called for each
	 * frame, a uint32 should allow for +4.5 years continous playback ;-)
	 */
	uint32_t cycles_count;

	bool registered;

	bool started;

	bool enabled;

	bool suspended;

	bool done;

	bool terminated;

	RTLIB_WorkingModeParams_t wmp;

/*******************************************************************************
 *    EXC State Machine Control
 ******************************************************************************/

	std::thread ctrl_trd;

	std::mutex ctrl_mtx;

	std::condition_variable ctrl_cv;


/*******************************************************************************
 *    EXC State Machine Internals
 ******************************************************************************/

	void ControlLoop();

	bool WaitEnable();

	RTLIB_ExitCode_t _Enable();

	RTLIB_ExitCode_t Setup();

	RTLIB_ExitCode_t Configure(uint8_t awm_id, RTLIB_ExitCode_t event);

	RTLIB_ExitCode_t Suspend();

	RTLIB_ExitCode_t Reconfigure(RTLIB_ExitCode_t result);

	RTLIB_ExitCode_t GetWorkingMode();

	RTLIB_ExitCode_t Run();

	RTLIB_ExitCode_t Monitor();

	RTLIB_ExitCode_t Release();

};

} // namespace rtlib

} // namespace bbque

#endif // BBQUE_EXC_H_
