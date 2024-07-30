import chess
import random


def gen_rand_pos():
	board = chess.Board()
	num_moves = random.randint(30,400)

	for _ in range(num_moves):
		legal_moves = list(board.legal_moves)
		if not legal_moves:
			return False
		move = random.choice(legal_moves)
		board.push(move)

	return board.fen()

for i in range(128):
	s = False
	while s == False:
		s = gen_rand_pos()

	print(s)

