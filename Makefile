CC = g++
STD = -std=c++20
CFLAGS = -O0

JUICER_SRC = bitboard.cpp\
		     position.cpp\
			 movegen.cpp
TEST_SRC = $(JUICER_SRC) testsuite.cpp
CLI_SRC = $(JUICER_SRC) cli.cpp

TEST = testjuicer
CLI = juicer

test: $(TEST_SRC)
	$(CC) $(STD) $(CFLAGS) $(TEST_SRC) -o $(TEST) && time ./$(TEST)

cli: $(CLI_SRC)
	$(CC) $(STD) $(CFLAGS) $(CLI_SRC) -o $(CLI) && ./$(CLI)
