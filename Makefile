CC = g++
STD = -std=c++20
CFLAGS = -Wall -Wextra -Werror -pedantic -O0

SRC = bitboard.cpp
SRC_H = bitboard.h types.h random.h
TEST = test.cpp


test: $(SRC) $(SRC_H) $(TEST)
	$(CC) $(STD) $(CFLAGS) $(SRC) $(TEST) -o test && ./test
