#ifndef MINIUNIT_H_INCLUDED
#define MINIUNIT_H_INCLUDED

#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>



static int  miniunit_line  = 0;  // first failure line
static auto miniunit_start = std::chrono::steady_clock::now();
static auto miniunit_end   = std::chrono::steady_clock::now();
static auto miniunit_duration = std::chrono::duration_cast<std::chrono::microseconds>(miniunit_end - miniunit_start);

static std::string miniunit_message = ""; 
static bool _miniunit_custom_message = false;
static bool _miniunit_custom_tock    = false;
static bool _miniunit_printed_benchmark_header = false;


#define miniunit_block(block) do {\
	block\
} while (false)


#define mu_configure()\
miniunit_block(\
	miniunit_line  = 0;\
	miniunit_start = std::chrono::steady_clock::now();\
	_miniunit_custom_message = false;\
)


#define mu_return()\
miniunit_block(\
	if (!_miniunit_custom_tock) {\
		miniunit_end = std::chrono::steady_clock::now();\
		miniunit_duration = std::chrono::duration_cast<std::chrono::microseconds>(miniunit_end - miniunit_start);\
	}\
	return (miniunit_line);\
)


#define mu_run(test)\
miniunit_block(\
	const int mu_test_result = test ();\
	const double mu_test_time = mu_duration();\
\
	if (mu_test_result == 0)\
	{\
		mu_notify_success(#test, mu_test_time);\
	}\
	else\
	{\
		mu_notify_failure(#test, miniunit_line, miniunit_message);\
	}\
)


#define mu_assert(condition)\
miniunit_block(\
	if (condition == false && miniunit_line == 0)\
	{\
		miniunit_line = __LINE__;\
		if (_miniunit_custom_message == false) { miniunit_message = "("#condition ") is false."; }\
	}\
)


#define mu_assert_strings_equal(s1, s2)\
miniunit_block(\
	if (s1 != s2)\
	{\
		miniunit_line = __LINE__;\
		if (_miniunit_custom_message == false) { miniunit_message = std::string(s1) + " == " + std::string(s2) +  " is false."; }\
	}\
)


#define mu_tick()\
miniunit_block(\
	miniunit_start = std::chrono::steady_clock::now();\
)


#define mu_tock()\
miniunit_block(\
	miniunit_end = std::chrono::steady_clock::now();\
	miniunit_duration = std::chrono::duration_cast<std::chrono::microseconds>(miniunit_end - miniunit_start);\
	_miniunit_custom_tock = true;\
)


#define mu_duration() (double(miniunit_duration.count()) / 1e6)


#define mu_set_error_message(message)\
miniunit_block(\
	if (miniunit_line == 0)\
	{\
		miniunit_message = message;\
		_miniunit_custom_message = true;\
	}\
)


#define mu_notify_success(test_name, elapsed_time)\
miniunit_block(\
	std::cout << "\x1b[32m" << "Test passed: " << std::setw(40) << test_name << " in " << elapsed_time << " seconds."  << "\x1b[0m\n";\
)


#define mu_notify_failure(test_name, line_number, error_message)\
miniunit_block(\
	std::cout << "\x1b[31m" << "Test failed: " << std::setw(32) << test_name << " at line " << line_number << ". "\
			  << error_message << "\x1b[0m\n";\
)


#define mu_show_bench_results(process)\
miniunit_block(\
	std::cout << "\x1b[36mProcess: " << std::setw(40) << #process\
			  << " completed in " << std::setprecision(6)\
			  << process << " seconds.\x1b[0m\n";\
)


#define mu_compare_to_native_process(process_1, process_2)\
miniunit_block(\
	if (!_miniunit_printed_benchmark_header)\
	{\
		std::cout << '\n'\
				  << "|----------------|----------------|----------------|\n"\
				  << "|                |    # completed |  X slower than |\n"\
				  << "|      operation |     per second |    native perf |\n"\
				  << "|----------------|----------------|----------------|\n";\
		_miniunit_printed_benchmark_header = true;\
	}\
	const double perf_delta = (process_1 >= process_2)\
							? double(process_1) / double(process_2)\
							: double(process_2) / double(process_1);\
	const std::string color = (process_1 >= process_2)\
							? "\x1b[36m"\
							: "\x1b[33m";\
	std::cout << "|" << std::setw(15) << #process_1;\
	std::cout << " |" << std::setw(15) << int64_t(process_1);\
	std::cout << " |" << color << std::setw(15) << perf_delta << "\x1b[0m |\n";\
	std::cout << "|----------------|----------------|----------------|\n";\
)



#endif // MINIUNIT_H_INCLUDED
