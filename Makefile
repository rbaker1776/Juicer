CXX = clang++
STD = -std=c++20
CFLAGS = -O3 -Wall -Wextra -pedantic -Rpass-missed=.* -mllvm -inline-threshold=65535

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

gvn: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) -Rpass-missed=gvn $(CLI_SRC) -o $(CLI) 2> gvn_remarks.txt && grep 'remark' gvn_remarks.txt | wc -l

licm: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) -Rpass-missed-licm $(CLI_SRC) -o $(CLI) 2> licm_remarks.txt && grep 'remark' licm_remarks.txt | wc -l
