CXX = g++
STD = -std=c++20
CFLAGS = -O3 -Wall -Wextra -pedantic

SRC =
TEST_SRC = $(SRC) testsuite.cpp
CLI_SRC = $(SRC) cli.cpp

TEST = testjuicer
CLI = juicer

cli: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) $(CLI_SRC) -o $(CLI) && ./$(CLI)

test: $(TEST_SRC)
	$(CXX) $(STD) $(CFLAGS) $(TEST_SRC) -o $(TEST) && time ./$(TEST)

memcheck: $(TEST)
	leaks --atExit -- ./testjuicer
