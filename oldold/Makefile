CC = g++
STD = -std=c++20
CFLAGS = -Wall -Wextra -Werror -pedantic -O0

SRC = position.cpp
SRC_H = types.h position.h
TEST = test_juicer.cpp


test: $(SRC) $(SRC_H) $(TEST)
	$(CC) $(STD) $(CFLAGS) $(SRC) $(TEST) -o test_juicer && ./test_juicer
