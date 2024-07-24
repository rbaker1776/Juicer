import chess
import chess.engine
import multiprocessing
import subprocess


STOCKFISH_PATH = "../../../../usr/local/bin/stockfish"

with open("middlegamefens.txt", 'r') as infile:
	fens = infile.readlines()
	infile.close()


def analyze(fen):
	depth = 15
	board = chess.Board().set_fen(fen)

	process = subprocess.Popen([STOCKFISH_PATH], universal_newlines=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE)

	process.stdin.write("uci\n")
	process.stdin.flush()

	process.stdin.write(f"position fen {fen}\n")
	process.stdin.write("go depth {depth}\n")
	process.stdin.flush()

	evaluation = None
	while True:	
		line = process.stdout.readline().strip()
		if line.startswith(f"info depth {depth} seldepth"):
			parts = line.split()
			idx = parts.index("score")
			score_type = parts[idx + 1]
			if score_type == "cp":
				evaluation = int(parts[idx + 2])
			break

	process.stdin.write("quit\n")
	process.stdin.flush()
	process.wait()

	print(fen, evaluation)
	return evaluation
	
def main():
	num_processes = 120

	processes = []
	for i in range(num_processes):
		process = multiprocessing.Process(target=analyze, args=(fens[i].strip(),))
		processes.append(process)
		process.start()

	for process in processes:
		process.join()

if __name__ == "__main__":
	main()
