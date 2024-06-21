#include <iostream>
#include "../position.h"
#include "../bitboard.h"
#include "minunit.h"


namespace FENOperations
{
	static void fen_constructor()
	{
		Position pos;
		pos.seed("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		mu_assert(pos.pieces(PAWN) == (RANK2_BB | RANK7_BB));
		mu_assert(pos.pieces(KNIGHT) == (G1 | B1 | G8 | B8));
		mu_assert(pos.pieces(BISHOP) == (F1 | C1 | F8 | C8));
		mu_assert(pos.pieces(ROOK) == (H1 | H8 | A8 | A1));
		mu_assert(pos.pieces(QUEEN) == (D1 | D8));
		mu_assert(pos.pieces(KING) == (E8 | E1));
		mu_assert(pos.pieces() == (RANK1_BB | RANK2_BB | RANK7_BB | RANK8_BB));
		mu_assert(pos.pieces(WHITE) == (RANK1_BB | RANK2_BB));
		mu_assert(pos.pieces(BLACK) == (RANK7_BB | RANK8_BB));
		mu_assert(pos.get_turn() == WHITE);
		mu_assert(pos.get_castling_rights() == 15);
		mu_assert(pos.get_ep_square() == NO_SQUARE);
		mu_assert(pos.get_halfmoves() == 0);
		mu_assert(pos.get_gameply() == 0);

		pos.seed("rNbqkbNr/pppppppp/8/8/8/8/PPPPPPPP/RnBQKBnR w KQkq - 20 11");
		mu_assert(pos.pieces(PAWN) == (RANK2_BB | RANK7_BB));
		mu_assert(pos.pieces(KNIGHT) == (G1 | B1 | G8 | B8));
		mu_assert(pos.pieces(BISHOP) == (F1 | C1 | F8 | C8));
		mu_assert(pos.pieces(ROOK) == (H1 | H8 | A8 | A1));
		mu_assert(pos.pieces(QUEEN) == (D1 | D8));
		mu_assert(pos.pieces(KING) == (E8 | E1));
		mu_assert(pos.pieces() == (RANK1_BB | RANK2_BB | RANK7_BB | RANK8_BB));
		mu_assert(pos.pieces(WHITE, KNIGHT) == (G8 | B8));
		mu_assert(pos.get_turn() == WHITE);
		mu_assert(pos.get_castling_rights() == 15);
		mu_assert(pos.get_ep_square() == NO_SQUARE);
		mu_assert(pos.get_halfmoves() == 20);
		mu_assert(pos.get_gameply() == 20);

		pos.seed("rNbqk1Nr/4b1pp/8/ppppppP1/PPPPPP2/8/7P/RnBQKBnR w KQkq f6 0 19");
		mu_assert(pos.get_ep_square() == F6);
		mu_assert(pos.get_halfmoves() == 0);
		mu_assert(pos.get_turn() == WHITE);
		mu_assert(pos.get_gameply() == 36);
		mu_assert(pos.get_castling_rights() == 15);

		pos.seed("Q4b1r/2q1kp1p/p2p4/4pb2/3P4/P7/P4PPP/R3KB1R b KQ - 0 17");
		mu_assert(pos.get_ep_square() == NO_SQUARE);
		mu_assert(pos.get_halfmoves() == 0);
		mu_assert(pos.get_gameply() == 33);
		mu_assert(pos.get_turn() == BLACK);
		mu_assert(pos.get_castling_rights() == 3);

		pos.seed("8/8/8/8/k7/8/5PPP/4K2R w K e8 0 35");
		mu_assert(pos.pieces() == (A4 | E1 | F2 | G2 | H2 | H1));
		mu_assert(pos.get_ep_square() == NO_SQUARE);
		mu_assert(pos.get_gameply() == 68);
		mu_assert(pos.get_castling_rights() == 1);
	}

	static void suite()
	{
		mu_run(FENOperations::fen_constructor);
	}
}
