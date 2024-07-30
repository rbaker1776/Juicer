CXX = g++
STD = -std=c++20
CFLAGS = -O3 -Wall -Wextra -pedantic
PRODUCTION_FLAGS = -O3 -mllvm -inline-threshold=65535

SRC =
CLI_SRC = $(SRC) cli.cpp
CLI = juicer

.PHONY: opt

cli: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) $(CLI_SRC) -o $(CLI) && ./$(CLI)

production: $(CLI_SRC)
	$(CXX) $(STD) $(PRODUCTION_FLAGS) $(CLI_SRC) -o $(CLI)

bench-perft:
	$(CXX) $(STD) $(PRODUCTION_FLAGS) ./bench/perft.cpp -o ./bench/perft && ./bench/perft && rm ./bench/perft

test-perft:
	$(CXX) $(STD) $(CFLAGS) ./test/perft.cpp -o ./test/perft && ./test/perft && rm ./test/perft

memcheck: $(TEST)
	leaks --atExit -- ./juicer

opt: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) -Rpass-missed=.* -mllvm -inline-threshold=65535 $(CLI_SRC) -o $(CLI) 2> ./opt/opt_remarks.txt && python3 ./opt/analyzer.py ./opt/opt_remarks.txt

gvn: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) -Rpass-missed=gvn $(CLI_SRC) -o $(CLI) 2> ./opt/gvn_remarks.txt && grep 'remark' ./opt/gvn_remarks.txt | wc -l

licm: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) -Rpass-missed=licm $(CLI_SRC) -o $(CLI) 2> ./opt/licm_remarks.txt && grep 'remark' ./opt/licm_remarks.txt | wc -l

inline: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) -Rpass-missed=inline $(CLI_SRC) -o $(CLI) 2> ./opt/inline_remarks.txt && grep 'remark' ./opt/inline_remarks.txt | wc -l

slp: $(CLI_SRC)
	$(CXX) $(STD) $(CFLAGS) -Rpass-missed=slp-vectorizer $(CLI_SRC) -o $(CLI) 2> ./opt/slp_remarks.txt && grep 'remark' ./opt/slp_remarks.txt | wc -l
	
