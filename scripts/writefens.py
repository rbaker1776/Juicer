import chess
import chess.pgn
import random


def write_middlegames(n: int, pgn_path: str):
	# I've made the very scientific decision that the middlegame occurs from move 10 (20 halfmoves)
	# until half of the pieces have been traded... 16 or fewer pieces remain
	verification = input("You are about to modify middlegamefens.txt. Would you like to continue? (y/n)")
	if (verification != 'y'):
		return

	fens = []
	k = 0
	with open(pgn_path, 'r') as infile:
		while k != n:	
			game = chess.pgn.read_game(infile)
			board = chess.Board()
			moves = list(game.mainline_moves())

			for i in range(len(moves)):
				board.push(moves[i])
				if i < 20:
					continue

				if len(board.piece_map()) <= 16:
					break

				fens.append(board.fen())
				k += 1

				if (k == n):
					break

		infile.close()

	random.shuffle(fens)
	with open("middlegamefens.txt", 'w') as outfile:
		outfile.writelines(fen + '\n' for fen in fens)
		outfile.close()

