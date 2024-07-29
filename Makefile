CXX = clang++
STD = -std=c++20
CFLAGS = -O3 -Wall -Wextra -pedantic -mllvm -inline-threshold=65535 -Rpass-missed=.*

SRC =
CLI_SRC = $(SRC) cli.cpp
CLI = juicer

cli: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) $(CLI_SRC) -o $(CLI) && ./$(CLI)

perft:
	$(CXX) $(STD) $(CFLAGS) bench_perft.cpp -o benchperft

memcheck: $(TEST)
	leaks --atExit -- ./juicer

gvn: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) -Rpass-missed=gvn $(CLI_SRC) -o $(CLI) 2> ./opt/gvn_remarks.txt && grep 'remark' ./opt/gvn_remarks.txt | wc -l

licm: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) -Rpass-missed=licm $(CLI_SRC) -o $(CLI) 2> ./opt/licm_remarks.txt && grep 'remark' ./opt/licm_remarks.txt | wc -l

opt: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) -Rpass-missed=.* -mllvm -inline-threshold=65535 $(CLI_SRC) -o $(CLI) 2> ./opt/opt_remarks.txt
