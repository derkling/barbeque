/**
 *       @file  constraints.cc
 *      @brief  Test for constraints assertion/removal
 *
 * This provides a testcase for the AWM constraints assertion and removal API.
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  01/10/2012
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2011, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =============================================================================
 */

#include "tests.h"

// These are a set of useful debugging log formatters
#define FMT_DBG(fmt) BBQUE_FMT(COLOR_LGRAY,  "CONSTR     [DBG]", fmt)
#define FMT_INF(fmt) BBQUE_FMT(COLOR_GREEN,  "CONSTR     [INF]", fmt)
#define FMT_WRN(fmt) BBQUE_FMT(COLOR_YELLOW, "CONSTR     [WRN]", fmt)
#define FMT_ERR(fmt) BBQUE_FMT(COLOR_RED,    "CONSTR     [ERR]", fmt)

using bbque::rtlib::BbqueEXC;

// The RTLib context;
extern RTLIB_Services_t *rtlib;


typedef struct CheckSteps {
	uint8_t count;
	RTLIB_Constraint_t *pcons;
} CheckSteps_t;

#define DEFINE_CHECK_STEP(STEP)\
RTLIB_Constraint_t pcons ## STEP []

#define ADD_CHECK_STEP(STEP)\
	{(sizeof(pcons ## STEP)/sizeof(RTLIB_Constraint_t)), pcons ## STEP}

// Initially all 8 AWMs are valid
#define STEP_FAULT_00 (curAwm > 8)

DEFINE_CHECK_STEP(01) = {
	{8, CONSTRAINT_REMOVE, EXACT_VALUE},
};
#define STEP_FAULT_01 (curAwm > 7)

DEFINE_CHECK_STEP(02) = {
	{6, CONSTRAINT_REMOVE, EXACT_VALUE},
	{2, CONSTRAINT_REMOVE, EXACT_VALUE},
};
#define STEP_FAULT_02 (curAwm == 2 || curAwm == 6 || curAwm > 7)

DEFINE_CHECK_STEP(03) = {
	{4, CONSTRAINT_ADD, UPPER_BOUND},
	{2, CONSTRAINT_ADD, LOWER_BOUND},
};
#define STEP_FAULT_03 (curAwm < 3 || curAWM > 4)

DEFINE_CHECK_STEP(04) = {
	{6, CONSTRAINT_ADD, UPPER_BOUND},
};
#define STEP_FAULT_04 (curAwm < 3 || curAWM > 6)

DEFINE_CHECK_STEP(05) = {
	{2, CONSTRAINT_ADD, UPPER_BOUND},
	{8, CONSTRAINT_ADD, EXACT_VALUE},
};
#define STEP_FAULT_05 (curAwm != 8 )

DEFINE_CHECK_STEP(06) = {
	{4, CONSTRAINT_ADD, LOWER_BOUND},
};
#define STEP_FAULT_06 (curAwm < 4 )

DEFINE_CHECK_STEP(07) = {
	{4, CONSTRAINT_ADD, UPPER_BOUND},
};
#define STEP_FAULT_07 (curAwm != 4 )

// The set of constraints to be verified
static CheckSteps_t CheckSteps[] = {
	ADD_CHECK_STEP(01),
	ADD_CHECK_STEP(02),
	ADD_CHECK_STEP(03),
	ADD_CHECK_STEP(04),
	ADD_CHECK_STEP(05),
	ADD_CHECK_STEP(06),
	ADD_CHECK_STEP(07),
};

#define CHECK_STEPS (sizeof(CheckSteps)/sizeof(CheckSteps_t))


class CCheckEXC : public BbqueEXC {

public:

		CCheckEXC(std::string const & name,
                std::string const & recipe,
                RTLIB_Services_t *rtlib):
        BbqueEXC(name, recipe, rtlib),
		testPassed(true),
		checkStep(0) {
        }

        ~CCheckEXC() {
        }
		
		// Test passed?
		bool testPassed;

private:

		// The expected AWM being selected from BBQ
		uint8_t curAwm;
	
		// The checking step
		uint8_t checkStep;

		// The minimum run loops on each CheckStep
		int16_t runLoops;

        RTLIB_ExitCode_t CheckStep();
        RTLIB_ExitCode_t onConfigure(uint8_t awm_id);
        RTLIB_ExitCode_t onRun();
        RTLIB_ExitCode_t onMonitor();

};

#define TEST_FAILED_IF(COND)\
	if (COND) {\
		fprintf(stderr, FMT_ERR("Test FAILED, error at step %d: "\
					"got invalid AWM [%d]\n"), checkStep, curAwm);\
		result = RTLIB_EXC_WORKLOAD_NONE;\
		testPassed = false;\
		goto exit_failed;\
	}

#define CHECK_STEP(STEP)\
	case STEP:\
		TEST_FAILED_IF(STEP_FAULT_##STEP);\
		break\

RTLIB_ExitCode_t
CCheckEXC::CheckStep() {
	RTLIB_ExitCode_t result = RTLIB_OK;

//=====[ CHECKING CODE ]=======================================================

	switch (checkStep) {
	CHECK_STEP(00);
	CHECK_STEP(01);
	CHECK_STEP(02);
	default:
		// If we get here... something is not working well
		TEST_FAILED_IF(true);
	}

	runLoops = 5;
	checkStep++;

exit_failed:
	return result;
}

RTLIB_ExitCode_t
CCheckEXC::onConfigure(uint8_t awm_id) {
	curAwm = awm_id;
	fprintf(stderr, FMT_INF("onConfigure AWM: %d, CheckStep: %d\n"),
			curAwm, checkStep);

	::usleep(100);
	CheckStep();

	return RTLIB_OK;
}

RTLIB_ExitCode_t
CCheckEXC::onRun() {
	RTLIB_ExitCode_t result = RTLIB_OK;

	if (!testPassed || checkStep > CHECK_STEPS)
		result = RTLIB_EXC_WORKLOAD_NONE;

	::usleep(200000);
	return result;
}

RTLIB_ExitCode_t
CCheckEXC::onMonitor() {
	RTLIB_ExitCode_t result = RTLIB_OK;

	
	// After 10 loops without reconfiguration, force a check
	// This is needed to manage the case when a constraints assertion does not
	// invalidate the current AWM and thus a re-scheduling is not triggered.
	if (runLoops < -10)
		CheckStep();

	if (--runLoops)
		goto exit_continue;
	
//=====[ CONSTRAINTS ASSERTION ]===============================================

	fprintf(stderr, FMT_INF("onMonitor: setting constraints for "
				"checkStep [%d]\n"), checkStep);
	
//fprintf(stderr, "pcons01 @%p\n", (void*)pcons01);
//fprintf(stderr, "RTLIBStruct @%p: %luBytes\n",
//		(void*)CheckSteps[checkStep-1].pcons, sizeof(RTLIB_Constraint_t));

//fprintf(stderr, "Constraints count: %d, tot size: %d Bytes\n",
//		CheckSteps[checkStep-1].count,
//		sizeof(*(CheckSteps[checkStep-1].pcons)));
	ClearConstraints();
	SetConstraints(CheckSteps[checkStep-1].pcons,
			CheckSteps[checkStep-1].count);


exit_continue:

	::usleep(100);
	return result;
}



TestResult_t test_constraints(int argc, char *argv[]) {
	CCheckEXC *pcheck;
	(void)argc;
	(void)argv;

	fprintf(stderr, FMT_INF("Here is the Constraints test\n"));

	// Initializing the RTLib library
	fprintf(stderr, FMT_INF("Init RTLib library...\n"));

	RTLIB_Init("BbqTesting", &rtlib);
	assert(rtlib);


	// Buid a new constraint testing EXC
	pcheck = new CCheckEXC("TestCCheck", "Test_Testing8", rtlib);
	if (!pcheck) {
		fprintf(stderr, FMT_ERR("CCheckEXC creation FAILED\n"));
		return TEST_FAILED;
	}

	// Start this EXC
	pcheck->Start();

	// Sit waiting for processing to complete
	pcheck->WaitCompletion();


	::sleep(5);



	return TEST_PASSED;
}

