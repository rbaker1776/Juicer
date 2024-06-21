CC = g++
STD = -std=c++20
CFLAGS = -O0

TEST = test
TEST_SRC = bitboard.cpp\
		   position.cpp\
		   testsuite.cpp\

test: $(TEST_SRC)
	$(CC) $(STD) $(CFLAGS) $(TEST_SRC) -o $(TEST) && time ./$(TEST)
