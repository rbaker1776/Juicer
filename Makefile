CC = g++
STD = -std=c++20
CFLAGS = -O0

TEST = testjuicer
TEST_SRC = tests/main.cpp tests/test_bitboard_utils.cpp tests/test_magic_bitboards.cpp bitboard.cpp


test: $(TEST_SRC)
	$(CC) $(STD) $(CFLAGS) $(TEST_SRC) -o $(TEST) && time ./$(TEST)

