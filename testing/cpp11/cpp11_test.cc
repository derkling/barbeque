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

#define  __STDC_FORMAT_MACROS
#include <inttypes.h>

//===== CONTAINERS
#include <set>
typedef std::set<int> intSet_t;


//===== EXCEPTIONS
#include <stdexcept>
void test_exceptions() {
	std::runtime_error("");
}


//===== TRAITS
#include <type_traits>
typedef std::integral_constant<int, 2> two_t;


//===== Data Types
#include <cstdint>
void test_function_types() {
	uint64_t u64 = 0xDEADBEEF;
	int64_t  i64 = 0xDEADBEEF;
	printf("u64(%p): %"PRIu64" (0x%08"PRIx64")\n", &u64, u64, u64);
	printf("i64(%p): %"PRIu64" (0x%08"PRIx64")\n", &i64, i64, i64);
}


//===== RANDOM
#warning Testing RANDOM support
#define _GLIBCXX_USE_C99_STDINT_TR1
#include <random>
std::uniform_int_distribution<uint16_t> dist;

void test_stats() {
	// Random number generator
	std::random_device rd;
	std::mt19937 rng_engine(time(0));

	std::uniform_int_distribution<> dis(1, 6);
	for (int n=0; n<10; ++n)
		printf("%d ", dis(rng_engine));

	printf("\n");
}

//===== REFERECE
#include <functional>
void ref_test(int &i, const int &j) {
	printf("ref_i: %d, cref_j: %d\n", i, j);
}

void test_function_ref() {
	int i = 1;
	int j = 2;
	ref_test(std::ref(i), std::cref(j));
}

//===== BIND
typedef std::function<void(void)> BindedFunction_t;
typedef std::function<bool(int, float)> BindedFunction2_t;

class BindTest {
public:
	void Test() {
		printf("This is a BindTest::Test() test method "
			"(0x%p)\n", this);
	}
	bool Test2(int i, float f) {
		printf("This is a BindTest::Test2() test metod"
			"(0x%p)\n", this);
		return true;
	}
};

void bind_test_caller(const char *name, BindedFunction_t func = NULL) {
	printf("Calling binded function [%s]...\n", name);
	if (func) func();
}

//bool bind_test_caller2(const char *name, BindedFunction2_t func, int i, float f) {
//	printf("Calling binded function2 [%s]...\n", name);
//	if (func)
//		return func(i, f);
//	return false;
//}

#include <iostream>
void bind_test_f(int n1, int n2, int n3, const int& n4, int n5) {
    std::cout << n1 << ' ' << n2 << ' ' << n3 << ' ' << n4 << ' ' << n5 << '\n';
}

int bind_test_g(int n1) {
    return n1;
}

void test_function_bind() {
	BindTest bt;

	printf("Calling un-binded function...\n");
	bind_test_caller("Empty");
	printf("Calling Test binded from BindTest (0x%p)...\n", &bt);
	bind_test_caller("BindTest::Test", std::bind(&BindTest::Test, &bt));
	
//	printf("Calling Test2 binded from BindTest (0x%p)...\n", &bt);
//	bind_test_caller("BindTest::Test2", std::bind(&BindTest::Test, &bt, 1, 2.0));

// These features are NOT supported by GCC 4.4
#if 0
	using namespace std::placeholders;
	// demonstrates argument reordering and pass-by-reference
	int n = 7;
	auto f1 = std::bind(bind_test_f, _2, _1, 42, std::cref(n), n);
	n = 10;
	f1(1, 2, 1001); // 1 is bound by _1, 2 is bound by _2, 1001 is unused

	// nested bind subexpressions share the placeholders
	auto f2 = std::bind(bind_test_f, _3, std::bind(bind_test_g, _3), _3, 4, 5);
	f2(10, 11, 12);
#endif
}


//===== MEMORY
#include <memory>
void test_function_memory() {
	struct ds_t {
		int first;
		int second;
	};

	printf("Testing shared_ptr...");
	std::shared_ptr<void> pds(new ds_t);
	//std::shared_ptr<ds_t> pva(std::static_pointer_cast<ds_t>(vpds));
	pds = NULL;
	printf(" OK!\n");
}

//===== RATIO
#include <bbque/cpp11/ratio.h>
#include <algorithm>
void test_function_ratio() {
	int s = std::__static_sign<-10>::value;
	printf("Sign<-10>: %d\n", s);
	int m =std::__safe_multiply<10,2>::value;
	printf("Multiply<10,2>: %d\n", m);
	int a = std::__safe_add<2,3>::value;
	printf ("Add<2,3>: %d\n", a);
	int rn = std::ratio<3,7>::num;
	int rd = std::ratio<3,7>::den;
	printf("Ratio<3,7>: N[%d], D[%d]\n", rn, rd);
	// NOTE This format string generate a warning on the first or second
	// formatter, depending on the target platform being either 32 or
	// 64bits.
	//printf("Ratio<Kilo>: N[%ld], D[%lld]\n", std::kilo::num, std::kilo::den);

	printf("Min(2,3): %d, Max(2,3): %d\n", std::min(2,3), std::max(2,3));
}

//===== CHRONO
#include <bbque/cpp11/chrono.h>

#include <ctime>
#include <locale>
#include <iostream>

void test_function_chrono() {
	char mbstr[100];

	std::chrono::time_point<std::chrono::system_clock> now;
	now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(
				now - std::chrono::hours(24)
			);
	if (std::strftime(mbstr, 100, "%F %T", std::localtime(&now_c))) {
		std::cout << "One day ago, the time was " << mbstr << std::endl;
	}

	printf("Milliseconds: %lld[ms]\n", std::chrono::milliseconds(100));
}

//===== MUTEX
#include <bbque/cpp11/mutex.h>
#include <bbque/cpp11/condition_variable.h>

std::mutex test_mtx;
std::recursive_mutex test_rmtx;
std::condition_variable test_cv;
std::cv_status test_cv_sattus;

void lock_guard_test() {
	std::lock_guard <std::recursive_mutex> rlg(test_rmtx);
	std::lock_guard <std::mutex> lg(test_mtx);

	test_cv.notify_all();
}

void test_function_mutex() {
	std::unique_lock<std::mutex> test_ul(test_mtx, std::defer_lock);
	std::lock_guard <std::recursive_mutex> rlg(test_rmtx);

	printf("Testing try_lock... ");
	fflush(stdout);
	test_mtx.try_lock();
	test_mtx.unlock();
	printf("OK!\n");

	printf("Testing condition_variable, wait_for 5432[ms]... ");
	fflush(stdout);
	test_mtx.lock();
	test_cv.wait_for(test_ul, std::chrono::milliseconds(5432));
	test_mtx.unlock();
	printf("OK!\n");

	printf("Testing lock_guard and recursive_mutex... ");
	lock_guard_test();
	printf("OK!\n");

}

//===== THREAD
#include <bbque/cpp11/thread.h>

class Test {
public:
	void Run (void) {
		printf("Hello threads!\n");
		printf("Sleeping for 2500[ms]... ");
		fflush(stdout);
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		printf("Wakeup.\n");

	}
};

void test_function_thread(void) {
	Test test;
	std::thread test_trd = std::thread(&Test::Run, &test);
	test_trd.join();
}

//===== FUTURE and PROMISE
#include <bbque/cpp11/future.h>

enum ExitCode {
	OK,
	ERROR
};

typedef std::promise<ExitCode> resp_prm_t;
typedef std::future<ExitCode> resp_ftr_t;

struct Resp {
	resp_prm_t resp_prm;
	resp_ftr_t resp_ftr;
	int value;
};

typedef std::shared_ptr<Resp> pResp_t;

class Executor {
public:
	void Run(pResp_t presp) {
		std::unique_lock<std::mutex> test_ul(test_mtx);

		printf("===> Executor START\n");
		printf("Sleeping for 3[s]...\n");
		std::this_thread::sleep_for(std::chrono::seconds(3));

		printf("Testing condition_variable (FROM A THREAD), wait_for 10123[ms]... ");
		fflush(stdout);
		test_cv.wait_for(test_ul, std::chrono::milliseconds(10123));
		printf("OK!\n");

		printf("Setting promise...\n");
		presp->value = 0xDEADBEEF;
		presp->resp_prm.set_value(OK);

		printf("<=== Executor END\n");
	}
};

void test_function_future() {
	std::thread executor_trd;
	pResp_t presp(new Resp);
	Executor exc;

	printf("Spawn executor...\n");
	executor_trd = std::thread(&Executor::Run, &exc, presp);
	executor_trd.detach();

	printf("Setup the promise, thus unlocking executor...\n");
	presp->resp_ftr = (presp->resp_prm).get_future();

	printf("Waiting for promise (max 60[s])...\n");
	(presp->resp_ftr).wait_for(std::chrono::seconds(60));

	printf("Promise satisfied, value: 0x%X\n", presp->value);

}

//===== Tests Entry Point
int main(int argc, const char *argv[]) {

	printf("\n\nStandard C++ library FEATURES test\n");

	printf("\n\n=====[ Testing Datatypes.. ]========\n");
	test_function_types();

	printf("\n\n=====[ Testing Statistic.. ]========\n");
	test_stats();

	printf("\n\n=====[ Testing Refs ]===============\n");
	test_function_ref();

	printf("\n\n=====[ Testing Bind ]===============\n");
	test_function_bind();

	printf("\n\n=====[ Testing Memory ]=============\n");
	test_function_memory();

	printf("\n\n=====[ Testing Ratio ]==============\n");
	test_function_ratio();

	printf("\n\n=====[ Testing Chrono ]=============\n");
	test_function_chrono();

	printf("\n\n=====[ Testing Mutex ]==============\n");
	test_function_mutex();

	printf("\n\n=====[ Testing Threads ]============\n");
	test_function_thread();

	printf("\n\n=====[ Testing Future ]=============\n");
	test_function_future();

	printf("Wait after thread completion, 3[s]...\n");
	std::this_thread::sleep_for(std::chrono::seconds(3));

	printf("\n\n\n");
	return 0;
}
