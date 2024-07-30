#ifndef MINUNIT_H_96077A456EFB
#define MINUNIT_H_96077A456EFB

#include <iostream>
#include <string>
#include <chrono>


static uint32_t mu_failure_line = 0;
static uint32_t mu_assertion_count = 0;
static uint32_t mu_positive_assertion_count = 0;
static uint32_t mu_test_count = 0;
static uint32_t mu_test_pass_count = 0;

static std::string mu_error_message = std::string();

enum class mu_Status: int
{
	SUCCESS = 0,
	FAILURE = 1,
};

static mu_Status mu_status = mu_Status::SUCCESS;

static auto mu_start_time = std::chrono::steady_clock::now();
static auto mu_end_time = std::chrono::steady_clock::now();
static double mu_suite_duration = 0;
static bool mu_timer_running = false;


#define mu_safe_block(block)\
do {\
	block\
} while (false);


// ASSERTIONS
// Handles assertons with no custom error messages
// ex: mu_assert(1 == 2);
#define mu_assert_no_message(condition) mu_safe_block(\
	++mu_assertion_count;\
	if ((condition) == false && mu_failure_line == 0) {\
		mu_failure_line = __LINE__;\
		mu_error_message = #condition " is false";\
		mu_status = mu_Status::FAILURE;\
	} else if (condition) {\
		++mu_positive_assertion_count;\
	}\
)

// Handles assertions with custom error messages
// ex: mu_assert(1 == 2, one equals two is false);
#define mu_assert_with_message(condition, message) mu_safe_block(\
	++mu_assertion_count;\
	if ((condition) == false && mu_failure_line == 0) {\
		mu_failure_line = __LINE__;\
		mu_error_message = message;\
		mu_status = mu_Status::FAILURE;\
	} else {\
		++mu_positive_assertion_count;\
	}\
)

// Accepts input from mu_assert() to select one of the two above macros
#define mu_select_assertion_macro(_1, _2, MACRO, ...) MACRO

// Accepts a condition and an optional error message
// This is the macro that should be invoked by the user
#define mu_assert(...) mu_safe_block(mu_select_assertion_macro(\
	__VA_ARGS__,\
	mu_assert_with_message,\
	mu_assert_no_message\
)(__VA_ARGS__))


// TIMERS
// Returns the duration in seconds between mu_start_time and mu_end_time
static inline double mu_get_duration()
{
	using std::chrono::duration_cast, std::chrono::microseconds;
	auto duration = duration_cast<microseconds>(mu_end_time - mu_start_time);
	return double(duration.count()) / 1'000'000;
}

// Sets the start time to the current time, "starts the timer"
static inline void mu_tick()
{
	mu_timer_running = true;
	mu_start_time = std::chrono::steady_clock::now();
}

// Sets the end time to the current time, "stops the timer"
static inline void mu_tock()
{
	mu_end_time = std::chrono::steady_clock::now();
	mu_timer_running = false;
}


// UNIT TESTS
// Resets variables associated with unit tests
static inline void mu_reset_test_variables()
{
	mu_failure_line = 0;
	mu_assertion_count = 0;
	mu_positive_assertion_count = 0;
	mu_status = mu_Status::SUCCESS;
}

// Reports the results of a unit test to the console
#define mu_report(test_name) mu_safe_block(\
	std::cout << (mu_status == mu_Status::SUCCESS ? "\x1b[32m" : "\x1b[31m");\
	std::cout << test_name << ": ";\
	std::cout << mu_positive_assertion_count << '/' << mu_assertion_count;\
	std::cout << " assertions passed in ";\
	std::cout << mu_get_duration() << " seconds.";\
	if (mu_status == mu_Status::FAILURE) {\
		std::cout << std::endl << '\t' << mu_error_message;\
		std::cout << ", on line " << mu_failure_line;\
	}\
	std::cout << "\x1b[0m" << std::endl;\
)

// Runs and reports results of a unit test
#define mu_run(test) mu_safe_block(\
	mu_reset_test_variables();\
	++mu_test_count;\
	mu_tick();\
	test();\
	if (mu_timer_running) { mu_tock(); }\
	mu_suite_duration += mu_get_duration();\
	if (mu_positive_assertion_count == mu_assertion_count) {\
		++mu_test_pass_count;\
	}\
	mu_report(#test);\
)


// TEST SUITES
// Resets variables associated with test suites
static inline void mu_reset_suite_variables()
{
	mu_suite_duration = 0;
	mu_test_count = 0;
	mu_test_pass_count = 0;
}

#define mu_suite(suite) mu_safe_block(\
	mu_reset_suite_variables();\
	std::cout << #suite "..." << std::endl;\
	suite();\
	std::cout << #suite ": ";\
	std::cout << mu_test_pass_count << '/' << mu_test_count;\
	std::cout << " tests passed in ";\
	std::cout << mu_suite_duration << " seconds" << std::endl;\
)


#endif // MINUNIT_H_96077A456EFB
