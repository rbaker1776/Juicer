CXX = g++
STD = -std=c++20
CFLAGS = -O3 -Wall -Wextra -pedantic -fconstexpr-steps=393222

JUICER_SRC = movement.cpp\
		     position.cpp\
			 movegen.cpp\
			 engine.cpp\
			 uci.cpp
TEST_SRC = $(JUICER_SRC) testsuite.cpp
CLI_SRC = $(JUICER_SRC) cli.cpp

TEST = testjuicer
CLI = juicer


cli: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) $(CLI_SRC) -o $(CLI) && ./$(CLI)

test: $(TEST_SRC)
	$(CXX) $(STD) $(CFLAGS) $(TEST_SRC) -o $(TEST) && time ./$(TEST)

memcheck: $(TEST)
	leaks --atExit -- ./$(TEST)
