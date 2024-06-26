CXX = g++
STD = -std=c++20
CFLAGS = -O0 -Wall -Wextra -pedantic

JUICER_SRC = bitboard.cpp\
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
