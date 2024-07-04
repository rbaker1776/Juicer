#include <bitset>
#include "minunit.h"
#include "bitboard.h"
#include "movement.h"
#include "position.h"
#include "engine.h"
#include "uci.h"
#include "xorshiftstar64.h"
#include "juicer.h"


static uint64_t safe_step(Square s, int step)
{{{
	Square to = Square(s + step);
	return (is_ok(to) && SQUARE_DISTANCE[s][to] <= 2) ? square_to_bb(to) : 0;
}}}

static uint64_t sliding_attack(PieceType pt, Square s, uint64_t occupied)
{{{
	uint64_t attacks = 0;
	Direction rook_directions[4] = { N, S, E, W };
	Direction bishop_directions[4] = { NE, SE, NW, SW };

	for (Direction d: (pt == ROOK ? rook_directions : bishop_directions))
	{
		Square from = s;
		while (safe_step(from, d) && !(occupied & from)) 
		{ 
			attacks |= (from += d); 
		}
	}
	return attacks;
}}}


namespace Bitboards
{
static void bb_popcount()
{{{ 
	mu_assert(popcount(0) == 0);
	mu_assert(popcount(1) == 1);
	mu_assert(popcount(0xf) == 4);

	for (int i = 1; i < 64; ++i) 
	{ 
		mu_assert(popcount(1ull << i) == 1);
		mu_assert(popcount((1ull << i) - 1) == i);
	}

	for (int i = 0; i < 10000; ++i)
	{
		uint64_t bb = xrs::rand<uint64_t>();
		mu_assert(popcount(bb) == std::bitset<64>(bb).count());
	}
}}}

static void bb_utils()
{{{
	mu_assert((RANK1_BB | RANK2_BB) == 0xffff);
	mu_assert((RANK2_BB & Square::A1) == 0);
	mu_assert((FILEF_BB | Square::F1) == FILEF_BB);

	for (File f = FILE_A; f <= FILE_H; ++f)
		for (File g = FILE_A; g < f; ++g)
			mu_assert((file_bb(f) & file_bb(g)) == 0);
	
	for (Rank r = RANK_1; r <= RANK_8; ++r)
		for (Rank s = RANK_1; s < r; ++s)
			mu_assert((rank_bb(r) & rank_bb(s)) == 0);

	for (Rank r = RANK_1; r <= RANK_8; ++r)
		for (File f = FILE_A; f <= FILE_H; ++f)
			mu_assert(popcount(rank_bb(r) & file_bb(f)) == 1);

	for (File f = FILE_A; f <= FILE_H; ++f)
	{
		int overlaps = 0;
		for (Square s = A1; s <= H8; ++s)
		{
			if (file_bb(f) & s)
			{	
				mu_assert(popcount(file_bb(f) ^ s) == 7);
				mu_assert(popcount(file_bb(f) | s) == 8);
				++overlaps;
			}
			else
			{
				mu_assert(popcount(file_bb(f) | s) == 9);
				mu_assert(popcount(file_bb(f) ^ s) == 9);
			}
		}
		mu_assert(overlaps == 8);
	}

	for (Rank r = RANK_1; r <= RANK_8; ++r)
	{
		int overlaps = 0;
		for (Square s = A1; s <= H8; ++s)
		{
			if (rank_bb(r) & s)
			{	
				mu_assert(popcount(rank_bb(r) ^ s) == 7);
				mu_assert(popcount(rank_bb(r) | s) == 8);
				++overlaps;
			}
			else
			{
				mu_assert(popcount(rank_bb(r) | s) == 9);
				mu_assert(popcount(rank_bb(r) ^ s) == 9);
			}
		}
		mu_assert(overlaps == 8);
	}

	for (Square s = A1; s <= H8; ++s)
		mu_assert(square_to_bb(s) == (rank_bb(s) & file_bb(s)));

	mu_assert((FILEA_BB | B1) == (A8 | A7 | A6 | A5 | A4 | A3 | A2 | A1 | B1));

	for (Square s: { E4, D4, E5, D5 })
		mu_assert(CENTER_DISTANCE[s] == 0);

	for (Square s: { C3, E3, D3, F3, F4, F5, F6, D6, E6, C6, C4, C5 })
		mu_assert(CENTER_DISTANCE[s] == 1);

	for (Square s: { B2, C2, D2, E2, F2, G2, G3, G4, G5, G6, G7, F7, E7, D7, C7, B7, B6, B5, B4, B3 })
		mu_assert(CENTER_DISTANCE[s] == 2);

	for (Square s: { A1, B1, C1, D1, E1, F1, G1, H1, A8, B8, C8, D8, E8, F8, G8, H8, A2, A3, A4, A5, A6, A7, H2, H3, H4, H5, H6, H7 })
		mu_assert(CENTER_DISTANCE[s] == 3);

	mu_assert(SQUARE_DISTANCE[A1][A1] == 0);
	for (Square s: { A2, B2, B1 })
		mu_assert(SQUARE_DISTANCE[A1][s] == 1);

	for (Square s:  {A3, B3, C3, C2, C1 })
		mu_assert(SQUARE_DISTANCE[s][A1] == 2);

	for (Square s: { C8, C7, C6, C5, C4, D4, D4, E4, F4, G4, H4 })
		mu_assert(SQUARE_DISTANCE[s][F7] == 3, std::to_string(int(s)));

	for (Square s: { H8, H7, H6, H5, H4, H3, H2, H1, A1, B1, C1, D1, E1, F1, G1 })
	{
		mu_assert(SQUARE_DISTANCE[s][A8] == 7);
		mu_assert(SQUARE_DISTANCE[s][B7] == 6);
		mu_assert(SQUARE_DISTANCE[s][C6] == 5);
		mu_assert(SQUARE_DISTANCE[s][D5] == 4);
	}

	for (Square s: { H8, H7, G8 })
	{
		mu_assert(SQUARE_DISTANCE[s][A1] == 7);
		mu_assert(SQUARE_DISTANCE[s][B2] == 6);
		mu_assert(SQUARE_DISTANCE[s][C3] == 5);
		mu_assert(SQUARE_DISTANCE[s][D4] == 4);
		mu_assert(SQUARE_DISTANCE[s][E5] == 3);
		mu_assert(SQUARE_DISTANCE[s][F6] == 2);
		mu_assert(SQUARE_DISTANCE[s][G7] == 1);
	}

	for (Square s1: { A1, B1, C1, D1, E1, F1, G1, H1 })
		for (Square s2: { A8, B8, C8, D8, E8, F8, G8, H8 })
			mu_assert(SQUARE_DISTANCE[s1][s2] == 7);

	for (Square s1: { A1, A2, A3, A4, A5, A6, A7, A8 })
		for (Square s2: { H1, H2, H3, H4, H5, H6, H7, H8 })
			mu_assert(SQUARE_DISTANCE[s1][s2] == 7);
}}}

static void suite()
{{{  
	mu_run(Bitboards::bb_popcount);	
	mu_run(Bitboards::bb_utils);
}}}
}


namespace Attacks
{
static void magics()
{{{
		for (int i = 0; i < 100'000; ++i)
		{
			const Square s = Square(xrs::rand<uint8_t>() & 63);
			const uint64_t occupied = xrs::sparse_rand<uint64_t>() & ~square_to_bb(s);
			mu_assert(sliding_attack(ROOK, s, occupied) == ROOK_MAGICS[s].attacks[ROOK_MAGICS[s].index(occupied)]);
		}
		for (int i = 0; i < 100'000; ++i)
		{
			const Square s = Square(xrs::rand<uint8_t>() & 63);
			const uint64_t occupied = xrs::rand<uint64_t>() & ~square_to_bb(s);
			mu_assert(sliding_attack(ROOK, s, occupied) == ROOK_MAGICS[s].attacks[ROOK_MAGICS[s].index(occupied)]);
		}

		for (int i = 0; i < 100'000; ++i)
		{
			const Square s = Square(xrs::rand<uint8_t>() & 63);
			const uint64_t occupied = xrs::sparse_rand<uint64_t>() & ~square_to_bb(s);
			mu_assert(sliding_attack(BISHOP, s, occupied) == BISHOP_MAGICS[s].attacks[BISHOP_MAGICS[s].index(occupied)]);
		}
		for (int i = 0; i < 100'000; ++i)
		{
			const Square s = Square(xrs::rand<uint8_t>() & 63);
			const uint64_t occupied = xrs::rand<uint64_t>() & ~square_to_bb(s);
			mu_assert(sliding_attack(BISHOP, s, occupied) == BISHOP_MAGICS[s].attacks[BISHOP_MAGICS[s].index(occupied)]);
		}
}}}

static void piece_attacks()
{{{
	mu_assert(PIECE_ATTACKS[KNIGHT][A1] == (B3 | C2));
	mu_assert(PIECE_ATTACKS[KNIGHT][D4] == (C2 | E2 | B3 | F3 | B5 | F5 | C6 | E6));
	mu_assert(PIECE_ATTACKS[KNIGHT][H2] == (F1 | F3 | G4));
	mu_assert(PIECE_ATTACKS[KNIGHT][G3] == (H1 | F1 | E2 | E4 | F5 | H5));
	mu_assert(PIECE_ATTACKS[KNIGHT][B6] == (A8 | C8 | A4 | C4 | D7 | D5));
	mu_assert(PIECE_ATTACKS[KNIGHT][F7] == (H8 | D8 | H6 | D6 | G5 | E5));
	mu_assert(PIECE_ATTACKS[KNIGHT][H1] == (F2 | G3));
	mu_assert(PIECE_ATTACKS[KNIGHT][E5] == (D7 | F7 | G6 | C6 | F3 | D3 | G4 | C4));

	mu_assert(PIECE_ATTACKS[BISHOP][A1] == (B2 | C3 | D4 | E5 | F6 | G7 | H8));
	mu_assert(PIECE_ATTACKS[BISHOP][C2] == (B1 | D1 | B3 | A4 | D3 | E4 | F5 | G6 | H7));
	mu_assert(PIECE_ATTACKS[BISHOP][C6] == (B7 | A8 | D5 | E4 | F3 | G2 | H1 | B5 | A4 | D7 | E8));

	for (Square s = A1; s <= H8; ++s)
	{
		mu_assert((PIECE_ATTACKS[BISHOP][s] & (file_bb(s) | rank_bb(s))) == 0);
		mu_assert((PIECE_ATTACKS[KNIGHT][s] & (file_bb(s) | rank_bb(s))) == 0);
		mu_assert((PIECE_ATTACKS[KNIGHT][s] & PIECE_ATTACKS[QUEEN][s]) == 0);
	}

	for (Square s = A1; s <= H8; ++s)
		mu_assert(PIECE_ATTACKS[ROOK][s] == ((rank_bb(s) | file_bb(s)) & ~square_to_bb(s)));

	for (Square s = A1; s <= H8; ++s)
		mu_assert(PIECE_ATTACKS[QUEEN][s] == (PIECE_ATTACKS[BISHOP][s] | PIECE_ATTACKS[ROOK][s]));

	for (Square s = B7; s <= G7; ++s)
		mu_assert(PIECE_ATTACKS[KING][s] == ((RANK8_BB | RANK7_BB | RANK6_BB) & (file_bb(s) | file_bb(s-1) | file_bb(s+1)) & ~square_to_bb(s)));

	mu_assert(PIECE_ATTACKS[KING][A1] == (A2 | B2 | B1));

	mu_assert(PAWN_ATTACKS[BLACK][A2] == square_to_bb(B1));
	mu_assert(PAWN_ATTACKS[WHITE][E2] == (D3 | F3));
}}}

static void suite()
{{{ 
	mu_run(Attacks::magics);
	mu_run(Attacks::piece_attacks);
}}}
}


namespace Positions
{
static void fen_constructor()
{{{
	Gamestate gs;
	Position pos;
	pos.seed("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", gs);
	mu_assert(pos.pieces(PAWN) == (RANK2_BB | RANK7_BB));
	mu_assert(pos.pieces(KNIGHT) == (G1 | B1 | G8 | B8));
	mu_assert(pos.pieces(BISHOP) == (F1 | C1 | F8 | C8));
	mu_assert(pos.pieces(ROOK) == (H1 | H8 | A8 | A1));
	mu_assert(pos.pieces(QUEEN) == (D1 | D8));
	mu_assert(pos.pieces(KING) == (E8 | E1));
	mu_assert(pos.pieces() == (RANK1_BB | RANK2_BB | RANK7_BB | RANK8_BB));
	mu_assert(pos.pieces(WHITE) == (RANK1_BB | RANK2_BB));
	mu_assert(pos.pieces(BLACK) == (RANK7_BB | RANK8_BB));
	mu_assert(pos.side_to_move() == WHITE);
	mu_assert(pos.castling_rights() == 15);
	mu_assert(pos.ep_square() == NO_SQUARE);
	mu_assert(pos.rule_50() == 0);
	mu_assert(pos.ply() == 0);
	mu_assert(pos.pieces(WHITE, ROOK, KNIGHT, BISHOP, QUEEN, KING) == RANK1_BB);
	mu_assert(pos.pieces(ROOK, KNIGHT, BISHOP, QUEEN, KING) == (RANK1_BB | RANK8_BB));
	
	pos.seed("rNbqkbNr/pppppppp/8/8/8/8/PPPPPPPP/RnBQKBnR w KQkq - 20 11", gs);
	mu_assert(pos.pieces(PAWN) == (RANK2_BB | RANK7_BB));
	mu_assert(pos.pieces(KNIGHT) == (G1 | B1 | G8 | B8));
	mu_assert(pos.pieces(BISHOP) == (F1 | C1 | F8 | C8));
	mu_assert(pos.pieces(ROOK) == (H1 | H8 | A8 | A1));
	mu_assert(pos.pieces(QUEEN) == (D1 | D8));
	mu_assert(pos.pieces(KING) == (E8 | E1));
	mu_assert(pos.pieces() == (RANK1_BB | RANK2_BB | RANK7_BB | RANK8_BB));
	mu_assert(pos.pieces(WHITE, KNIGHT) == (G8 | B8));
	mu_assert(pos.side_to_move() == WHITE);
	mu_assert(pos.castling_rights() == 15);
	mu_assert(pos.ep_square() == NO_SQUARE);
	mu_assert(pos.rule_50() == 20);
	mu_assert(pos.ply() == 20);

	pos.seed("rNbqk1Nr/4b1pp/8/ppppppP1/PPPPPP2/8/7P/RnBQKBnR w KQkq f6 0 19", gs);
	mu_assert(pos.ep_square() == F6);
	mu_assert(pos.rule_50() == 0);
	mu_assert(pos.side_to_move() == WHITE);
	mu_assert(pos.ply() == 36);
	mu_assert(pos.castling_rights() == 15);

	pos.seed("Q4b1r/2q1kp1p/p2p4/4pb2/3P4/P7/P4PPP/R3KB1R b KQ - 0 17", gs);
	mu_assert(pos.ep_square() == NO_SQUARE);
	mu_assert(pos.rule_50() == 0);
	mu_assert(pos.ply() == 33);
	mu_assert(pos.side_to_move() == BLACK);
	mu_assert(pos.castling_rights() == 3);
}}}

static void make_simple_moves()
{{{
	Gamestate gs;
	Position pos;
	pos.seed("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", gs);

	Gamestate gs2;
	pos.make_move(Move::make(E2, E4), gs2);
	mu_assert(pos.pieces(W_PAWN) == (A2 | B2 | C2 | D2 | E4 | F2 | G2 | H2));
	mu_assert(pos.side_to_move() == BLACK);
	mu_assert(pos.ply() == 1);
	mu_assert(pos.pieces(WHITE) == (RANK1_BB | A2 | B2 | C2 | D2 | E4 | F2 | G2 | H2));

	Gamestate gs3;
	pos.make_move(Move::make(C7, C5), gs3);
	mu_assert(pos.pieces(W_PAWN) == (A2 | B2 | C2 | D2 | E4 | F2 | G2 | H2));
	mu_assert(pos.pieces(B_PAWN) == (A7 | B7 | C5 | D7 | E7 | F7 | G7 | H7));
	mu_assert(pos.side_to_move() == WHITE);
	mu_assert(pos.ply() == 2);

	Gamestate gs4;
	pos.make_move(Move::make(G1, F3), gs4);
	mu_assert(pos.pieces(W_KNIGHT) == (B1 | F3));
	mu_assert(pos.side_to_move() == BLACK);
	mu_assert(pos.ply() == 3);

	Gamestate gs5;
	pos.seed("rnbqkbnr/pp2pppp/3p4/8/3NP3/8/PPP2PPP/RNBQKB1R b KQkq - 0 4", gs);
	pos.make_move(Move::make(G8, F6), gs5);
	mu_assert(pos.pieces(B_KNIGHT) == (B8 | F6));
	mu_assert(pos.side_to_move() == WHITE);
	mu_assert(pos.ply() == 8);

	Gamestate gs6;
	pos.make_move(Move::make(F1, B5), gs6);
	mu_assert(pos.pieces(WHITE, BISHOP) == (B5 | C1));
	mu_assert(pos.ply() == 9);
}}}

static void make_captures()
{{{ 
	Gamestate gs;
	Position pos;
	pos.seed("rnbqkb1r/5ppp/p2ppn2/3N4/Pp1NP3/7P/1PP2PP1/R1BQKB1R w KQkq - 0 9", gs);

	Gamestate gs2;
	pos.make_move(Move::make(D5, F6), gs2);
	mu_assert(pos.pieces(W_KNIGHT) == (D4 | F6));
	mu_assert(pos.pieces(B_KNIGHT) == square_to_bb(B8));
	mu_assert(pos.piece_on(F6) == W_KNIGHT);
	mu_assert(gs2.previous == &gs);
	mu_assert(gs2.rule_50 == 0);
	
	Gamestate gs3;
	pos.make_move(Move::make(D8, F6), gs3);
	mu_assert(pos.pieces(W_KNIGHT) == square_to_bb(D4));
	mu_assert(pos.pieces(B_QUEEN) == square_to_bb(F6));
	mu_assert(pos.piece_on(F6) == B_QUEEN);
	mu_assert(gs3.previous == &gs2);
	mu_assert(gs3.rule_50 == 0);

	Gamestate gs4, gs5, gs6;
	pos.make_move(Move::make(F1, D3), gs4);
	pos.make_move(Move::make(F8, E7), gs5);
	pos.make_move(Move::make(C2, C3), gs6);

	Gamestate gs7;
	pos.make_move(Move::make(B4, C3), gs7);
	mu_assert(pos.piece_on(C3) == B_PAWN);
	mu_assert(pos.side_to_move() == WHITE);
}}}

static void make_castles()
{{{ 
	Gamestate gs;
	Position pos;
	pos.seed("rnbqk2r/ppp2ppp/4pn2/3p4/2PP4/P1P2P2/4P1PP/R1BQKBNR b KQkq - 0 6", gs);

	Gamestate gs2;
	pos.make_move(Move::make(E8, H8, CASTLES), gs2);
	mu_assert(pos.piece_on(G8) == B_KING);
	mu_assert(pos.piece_on(F8) == B_ROOK);
	mu_assert(pos.piece_on(H8) == NO_PIECE);
	mu_assert(pos.piece_on(E8) == NO_PIECE);
	mu_assert(pos.castling_rights() == 3);

	pos.seed("rn1q1rk1/p4ppp/bp3n2/2pp4/3P4/P1PBPP2/4N1PP/R1BQK2R w KQ - 2 12", gs);
	pos.make_move(Move::make(E1, H1, CASTLES), gs2);
	mu_assert(pos.piece_on(E1) == NO_PIECE);
	mu_assert(pos.piece_on(F1) == W_ROOK);
	mu_assert(pos.piece_on(G1) == W_KING);
	mu_assert(pos.piece_on(H1) == NO_PIECE);
	mu_assert(pos.castling_rights() == 0);

	pos.seed("r1b1k2r/pppq1ppp/n3pn2/1N1P4/1PP5/P7/2Q2PPP/R3KBNR w KQkq - 1 13", gs);
	pos.make_move(Move::make(E1, A1, CASTLES), gs2);
	mu_assert(pos.piece_on(E1) == NO_PIECE);
	mu_assert(pos.piece_on(D1) == W_ROOK);
	mu_assert(pos.piece_on(C1) == W_KING);
	mu_assert(pos.piece_on(A1) == NO_PIECE);
	mu_assert(pos.castling_rights() == 12);

	pos.seed("r3k2r/ppp2ppp/n4n2/1N1P1b2/1PB5/P7/5PPP/2KR2NR b kq - 1 16", gs);
	pos.make_move(Move::make(E8, A8, CASTLES), gs2);
	mu_assert(pos.piece_on(E8) == NO_PIECE);
	mu_assert(pos.piece_on(D8) == B_ROOK);
	mu_assert(pos.piece_on(C8) == B_KING);
	mu_assert(pos.piece_on(A8) == NO_PIECE);
	mu_assert(pos.castling_rights() == 0);
}}}

static void make_ep_captures()
{{{
	Gamestate gs;
	Position pos;
	pos.seed("rnbqkbnr/pp2pppp/2p5/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3", gs);

	Gamestate gs2;
	mu_assert(pos.ep_square() == D6);
	pos.make_move(Move::make(E5, D6, EN_PASSANT), gs2);
	mu_assert(pos.piece_on(E5) == NO_PIECE);
	mu_assert(pos.piece_on(D5) == NO_PIECE);
	mu_assert(pos.piece_on(D6) == W_PAWN);
	mu_assert(pos.ep_square() == NO_SQUARE);

	pos.seed("rnbqkbnr/p2ppppp/2p5/Pp6/8/8/1PPPPPPP/RNBQKBNR w KQkq b6 0 3", gs);
	pos.make_move(Move::make(A5, B6, EN_PASSANT), gs2);
	mu_assert(pos.piece_on(B6) == W_PAWN);
	mu_assert(pos.piece_on(B5) == NO_PIECE);
	mu_assert(pos.piece_on(A5) == NO_PIECE);
}}}

static void make_promotions()
{{{ 
	Gamestate gs;
	Position pos;
	pos.seed("rnbqkbnr/ppppp1P1/8/8/7p/8/PPPP1PPP/RNBQKBNR w KQkq - 0 5", gs);

	Gamestate gs2;
	pos.make_move(Move::make(G7, H8, PROMOTION, QUEEN), gs2);
	mu_assert(pos.piece_on(H8) == W_QUEEN);
	mu_assert(pos.piece_on(G7) == NO_PIECE);
	mu_assert(pos.pieces(WHITE, QUEEN) == (D1 | H8));

	pos.seed("rnbqkbnr/pp1ppppp/8/8/8/1K6/PPP2pPP/RNBQ1BNR b kq - 1 5", gs);
	pos.make_move(Move::make(F2, G1, PROMOTION, KNIGHT), gs2);
	mu_assert(pos.piece_on(G1) == B_KNIGHT);
	mu_assert(pos.piece_on(F2) == NO_PIECE);
	mu_assert(pos.pieces(BLACK, KNIGHT) == (G1 | B8 | G8));
}}}

static void fen_conversions()
{{{
	Gamestate gs;
	Position pos;

	for (std::string fen : {
		"rnbqkbnr/ppppp1P1/8/8/7p/8/PPPP1PPP/RNBQKBNR w KQkq - 0 5",
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		"rnbqkbnr/pppp1ppp/8/4p3/3PP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 3",
		"rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
		"r1bqkb1r/pppp1ppp/2n2n2/1B2P3/4P3/5N2/PPP2PPP/RNBQK2R b KQkq - 4 5",
		"rnbqkb1r/ppp2ppp/4pn2/3p4/3P4/5N2/PPP1PPPP/RNBQKB1R w KQkq - 0 5",
		"r1bqk1nr/pppp1ppp/2n5/2b1P3/2B5/5N2/PPPP1PPP/RNBQK2R w KQkq - 2 5",
		"rnbqkbnr/pppp1ppp/8/4p3/2P5/8/PP1PPPPP/RNBQKBNR b KQkq - 0 2",
		"rnbqkb1r/ppp2ppp/4pn2/3p4/3PP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 4",
		"rnbqkbnr/pp2pppp/8/2pp4/2B5/8/PPPP1PPP/RNBQK1NR w KQkq - 0 4",
		"r1bqkbnr/1pp1pppp/p1n5/1B2P3/4P3/5N2/PPP2PPP/RNBQK2R w KQkq - 0 4",
		"8/8/8/8/8/4k3/8/4K2R w - - 0 1",
		"r3k2r/ppp1qppp/2n2n2/3p4/3P4/2N2N2/PPP1QPPP/R1B1KB1R w KQkq - 0 9",
		"r2q1rk1/pb1n1pp1/1p2pn1p/2p1P3/2B1P3/2N2N2/PPP2PPP/R1BQ1RK1 w - - 0 10",
	}) mu_assert(pos.seed(fen, gs).fen() == fen, fen + "\n\t" + pos.fen());
}}}

static void play_game()
{{{
	Position pos;
	Gamestate gs[21];
	Move moves[20] = 
	{
		Move::make(D2, D4), Move::make(A7, A5),
		Move::make(D4, D5), Move::make(E7, E5),
		Move::make(C1, G5), Move::make(C7, C5),
		Move::make(D5, C6, EN_PASSANT), Move::make(A5, A4),
		Move::make(G5, D8), Move::make(E8, D8),
		Move::make(E2, E4), Move::make(A4, A3),
		Move::make(F1, D3), Move::make(A3, B2),
		Move::make(G1, E2), Move::make(B2, A1, PROMOTION, KNIGHT),
		Move::make(E1, H1, CASTLES), Move::make(D7, D5),
		Move::make(B1, A3), Move::make(A1, C2),
	};
	std::string fenstrings[21] = 
	{
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		"rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1",
		"rnbqkbnr/1ppppppp/8/p7/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2",
		"rnbqkbnr/1ppppppp/8/p2P4/8/8/PPP1PPPP/RNBQKBNR b KQkq - 0 2",
		"rnbqkbnr/1ppp1ppp/8/p2Pp3/8/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 3",
		"rnbqkbnr/1ppp1ppp/8/p2Pp1B1/8/8/PPP1PPPP/RN1QKBNR b KQkq - 1 3",
		"rnbqkbnr/1p1p1ppp/8/p1pPp1B1/8/8/PPP1PPPP/RN1QKBNR w KQkq c6 0 4",
		"rnbqkbnr/1p1p1ppp/2P5/p3p1B1/8/8/PPP1PPPP/RN1QKBNR b KQkq - 0 4",
		"rnbqkbnr/1p1p1ppp/2P5/4p1B1/p7/8/PPP1PPPP/RN1QKBNR w KQkq - 0 5",
		"rnbBkbnr/1p1p1ppp/2P5/4p3/p7/8/PPP1PPPP/RN1QKBNR b KQkq - 0 5",
		"rnbk1bnr/1p1p1ppp/2P5/4p3/p7/8/PPP1PPPP/RN1QKBNR w KQ - 0 6",
		"rnbk1bnr/1p1p1ppp/2P5/4p3/p3P3/8/PPP2PPP/RN1QKBNR b KQ - 0 6",
		"rnbk1bnr/1p1p1ppp/2P5/4p3/4P3/p7/PPP2PPP/RN1QKBNR w KQ - 0 7",
		"rnbk1bnr/1p1p1ppp/2P5/4p3/4P3/p2B4/PPP2PPP/RN1QK1NR b KQ - 1 7",
		"rnbk1bnr/1p1p1ppp/2P5/4p3/4P3/3B4/PpP2PPP/RN1QK1NR w KQ - 0 8",
		"rnbk1bnr/1p1p1ppp/2P5/4p3/4P3/3B4/PpP1NPPP/RN1QK2R b KQ - 1 8",
		"rnbk1bnr/1p1p1ppp/2P5/4p3/4P3/3B4/P1P1NPPP/nN1QK2R w K - 0 9",
		"rnbk1bnr/1p1p1ppp/2P5/4p3/4P3/3B4/P1P1NPPP/nN1Q1RK1 b - - 1 9",
		"rnbk1bnr/1p3ppp/2P5/3pp3/4P3/3B4/P1P1NPPP/nN1Q1RK1 w - - 0 10",
		"rnbk1bnr/1p3ppp/2P5/3pp3/4P3/N2B4/P1P1NPPP/n2Q1RK1 b - - 1 10",
		"rnbk1bnr/1p3ppp/2P5/3pp3/4P3/N2B4/P1n1NPPP/3Q1RK1 w - - 0 11",
	};

	pos.seed(fenstrings[0], gs[0]);
	for (int i = 0; i < 20; ++i)
	{
		pos.make_move(moves[i], gs[i+1]);
		mu_assert(pos.fen() == fenstrings[i+1], fenstrings[i+1] + "\n\t" + pos.fen());
	}
}}}

static void undo_moves()
{{{
	Gamestate gs;
	Position pos;
	pos.seed(STARTPOS_FEN, gs);

	Gamestate gs2;
	pos.make_move(Move::make(E2, E4), gs2);
	pos.undo_move(Move::make(E2, E4));
	mu_assert(pos.fen() == STARTPOS_FEN);

	pos.make_move(Move::make(E2, E4), gs2);
	pos.undo_move(Move::make(E2, E4));
	mu_assert(pos.fen() == STARTPOS_FEN);

	pos.make_move(Move::make(G1, F3), gs2);
	pos.undo_move(Move::make(G1, F3));
	mu_assert(pos.fen() == STARTPOS_FEN);

	Gamestate gss[20];
	Move moves[20] = 
	{
		Move::make(D2, D4), Move::make(A7, A5),
		Move::make(D4, D5), Move::make(E7, E5),
		Move::make(C1, G5), Move::make(C7, C5),
		Move::make(D5, C6, EN_PASSANT), Move::make(A5, A4),
		Move::make(G5, D8), Move::make(E8, D8),
		Move::make(E2, E4), Move::make(A4, A3),
		Move::make(F1, D3), Move::make(A3, B2),
		Move::make(G1, E2), Move::make(B2, A1, PROMOTION, KNIGHT),
		Move::make(E1, H1, CASTLES), Move::make(D7, D5),
		Move::make(B1, A3), Move::make(A1, C2),
	};

	for (int i = 0; i < 20; ++i) pos.make_move(moves[i], gss[i]);
	for (int i = 19; i >= 0; --i) pos.undo_move(moves[i]);

	mu_assert(pos.fen() == STARTPOS_FEN);
}}}

static void suite()
{{{ 
	mu_run(Positions::fen_constructor);
	mu_run(Positions::make_simple_moves);
	mu_run(Positions::make_captures);
	mu_run(Positions::make_castles);
	mu_run(Positions::make_ep_captures);
	mu_run(Positions::make_promotions);
	mu_run(Positions::fen_conversions);
	mu_run(Positions::play_game);
	mu_run(Positions::undo_moves);
}}}
}


namespace Movegen
{
static void perft()
{{{
	Engine juicer;
	const std::string positions[128] =
	{
		std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
		std::string("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"),
		std::string("4k3/8/8/8/8/8/8/4K2R w K - 0 1"),
		std::string("4k3/8/8/8/8/8/8/R3K3 w Q - 0 1"),
		std::string("4k2r/8/8/8/8/8/8/4K3 w k - 0 1"),
		std::string("r3k3/8/8/8/8/8/8/4K3 w q - 0 1"),
		std::string("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1"),
		std::string("r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1"),
		std::string("8/8/8/8/8/8/6k1/4K2R w K - 0 1"),
		std::string("8/8/8/8/8/8/1k6/R3K3 w Q - 0 1"),
		std::string("4k2r/6K1/8/8/8/8/8/8 w k - 0 1"),
		std::string("r3k3/1K6/8/8/8/8/8/8 w q - 0 1"),
		std::string("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"),
		std::string("r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1"),
		std::string("r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1"),
		std::string("r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1"),
		std::string("1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1"),
		std::string("2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1"),
		std::string("r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1"),
		std::string("4k3/8/8/8/8/8/8/4K2R b K - 0 1"),
		std::string("4k3/8/8/8/8/8/8/R3K3 b Q - 0 1"),
		std::string("4k2r/8/8/8/8/8/8/4K3 b k - 0 1"),
		std::string("r3k3/8/8/8/8/8/8/4K3 b q - 0 1"),
		std::string("4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1"),
		std::string("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1"),
		std::string("8/8/8/8/8/8/6k1/4K2R b K - 0 1"),
		std::string("8/8/8/8/8/8/1k6/R3K3 b Q - 0 1"),
		std::string("4k2r/6K1/8/8/8/8/8/8 b k - 0 1"),
		std::string("r3k3/1K6/8/8/8/8/8/8 b q - 0 1"),
		std::string("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1"),
		std::string("r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1"),
		std::string("r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1"),
		std::string("r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1"),
		std::string("1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1"),
		std::string("2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1"),
		std::string("r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1"),
		std::string("8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1"),
		std::string("8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1"),
		std::string("8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1"),
		std::string("K7/8/2n5/1n6/8/8/8/k6N w - - 0 1"),
		std::string("k7/8/2N5/1N6/8/8/8/K6n w - - 0 1"),
		std::string("8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1"),
		std::string("8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1"),
		std::string("8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1"),
		std::string("K7/8/2n5/1n6/8/8/8/k6N b - - 0 1"),
		std::string("k7/8/2N5/1N6/8/8/8/K6n b - - 0 1"),
		std::string("B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1"),
		std::string("8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1"),
		std::string("k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1"),
		std::string("K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1"),
		std::string("B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1"),
		std::string("8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1"),
		std::string("k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1"),
		std::string("K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1"),
		std::string("7k/RR6/8/8/8/8/rr6/7K w - - 0 1"),
		std::string("R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1"),
		std::string("7k/RR6/8/8/8/8/rr6/7K b - - 0 1"),
		std::string("R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1"),
		std::string("6kq/8/8/8/8/8/8/7K w - - 0 1"),
		std::string("6KQ/8/8/8/8/8/8/7k b - - 0 1"),
		std::string("K7/8/8/3Q4/4q3/8/8/7k w - - 0 1"),
		std::string("6qk/8/8/8/8/8/8/7K b - - 0 1"),
		std::string("6KQ/8/8/8/8/8/8/7k b - - 0 1"),
		std::string("K7/8/8/3Q4/4q3/8/8/7k b - - 0 1"),
		std::string("8/8/8/8/8/K7/P7/k7 w - - 0 1"),
		std::string("8/8/8/8/8/7K/7P/7k w - - 0 1"),
		std::string("K7/p7/k7/8/8/8/8/8 w - - 0 1"),
		std::string("7K/7p/7k/8/8/8/8/8 w - - 0 1"),
		std::string("8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1"),
		std::string("8/8/8/8/8/K7/P7/k7 b - - 0 1"),
		std::string("8/8/8/8/8/7K/7P/7k b - - 0 1"),
		std::string("K7/p7/k7/8/8/8/8/8 b - - 0 1"),
		std::string("7K/7p/7k/8/8/8/8/8 b - - 0 1"),
		std::string("8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1"),
		std::string("8/8/8/8/8/4k3/4P3/4K3 w - - 0 1"),
		std::string("4k3/4p3/4K3/8/8/8/8/8 b - - 0 1"),
		std::string("8/8/7k/7p/7P/7K/8/8 w - - 0 1"),
		std::string("8/8/k7/p7/P7/K7/8/8 w - - 0 1"),
		std::string("8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1"),
		std::string("8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1"),
		std::string("8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1"),
		std::string("k7/8/3p4/8/3P4/8/8/7K w - - 0 1"),
		std::string("8/8/7k/7p/7P/7K/8/8 b - - 0 1"),
		std::string("8/8/k7/p7/P7/K7/8/8 b - - 0 1"),
		std::string("8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1"),
		std::string("8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1"),
		std::string("8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1"),
		std::string("k7/8/3p4/8/3P4/8/8/7K b - - 0 1"),
		std::string("7k/3p4/8/8/3P4/8/8/K7 w - - 0 1"),
		std::string("7k/8/8/3p4/8/8/3P4/K7 w - - 0 1"),
		std::string("k7/8/8/7p/6P1/8/8/K7 w - - 0 1"),
		std::string("k7/8/7p/8/8/6P1/8/K7 w - - 0 1"),
		std::string("k7/8/8/6p1/7P/8/8/K7 w - - 0 1"),
		std::string("k7/8/6p1/8/8/7P/8/K7 w - - 0 1"),
		std::string("k7/8/8/3p4/4p3/8/8/7K w - - 0 1"),
		std::string("k7/8/3p4/8/8/4P3/8/7K w - - 0 1"),
		std::string("7k/3p4/8/8/3P4/8/8/K7 b - - 0 1"),
		std::string("7k/8/8/3p4/8/8/3P4/K7 b - - 0 1"),
		std::string("k7/8/8/7p/6P1/8/8/K7 b - - 0 1"),
		std::string("k7/8/7p/8/8/6P1/8/K7 b - - 0 1"),
		std::string("k7/8/8/6p1/7P/8/8/K7 b - - 0 1"),
		std::string("k7/8/6p1/8/8/7P/8/K7 b - - 0 1"),
		std::string("k7/8/8/3p4/4p3/8/8/7K b - - 0 1"),
		std::string("k7/8/3p4/8/8/4P3/8/7K b - - 0 1"),
		std::string("7k/8/8/p7/1P6/8/8/7K w - - 0 1"),
		std::string("7k/8/p7/8/8/1P6/8/7K w - - 0 1"),
		std::string("7k/8/8/1p6/P7/8/8/7K w - - 0 1"),
		std::string("7k/8/1p6/8/8/P7/8/7K w - - 0 1"),
		std::string("k7/7p/8/8/8/8/6P1/K7 w - - 0 1"),
		std::string("k7/6p1/8/8/8/8/7P/K7 w - - 0 1"),
		std::string("3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1"),
		std::string("7k/8/8/p7/1P6/8/8/7K b - - 0 1"),
		std::string("7k/8/p7/8/8/1P6/8/7K b - - 0 1"),
		std::string("7k/8/8/1p6/P7/8/8/7K b - - 0 1"),
		std::string("7k/8/1p6/8/8/P7/8/7K b - - 0 1"),
		std::string("k7/7p/8/8/8/8/6P1/K7 b - - 0 1"),
		std::string("k7/6p1/8/8/8/8/7P/K7 b - - 0 1"),
		std::string("3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1"),
		std::string("8/Pk6/8/8/8/8/6Kp/8 w - - 0 1"),
		std::string("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1"),
		std::string("8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1"),
		std::string("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1"),
		std::string("8/Pk6/8/8/8/8/6Kp/8 b - - 0 1"),
		std::string("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1"),
		std::string("8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1"),
		std::string("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1"),
		std::string("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"),
		std::string("rnbqkb1r/ppppp1pp/7n/4Pp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3")
	};

	const uint64_t perft_results[128][6] = 
	{
		{20, 400, 8902, 197281, 4865609, 119060324},
		{48, 2039, 97862, 4085603, 193690690, 8031647685},
		{15, 66, 1197, 7059, 133987, 764643},
		{16, 71, 1287, 7626, 145232, 846648},
		{5, 75, 459, 8290, 47635, 899442},
		{5, 80, 493, 8897, 52710, 1001523},
		{26, 112, 3189, 17945, 532933, 2788982},
		{5, 130, 782, 22180, 118882, 3517770},
		{12, 38, 564, 2219, 37735, 185867},
		{15, 65, 1018, 4573, 80619, 413018},
		{3, 32, 134, 2073, 10485, 179869},
		{4, 49, 243, 3991, 20780, 367724},
		{26, 568, 13744, 314346, 7594526, 179862938},
		{25, 567, 14095, 328965, 8153719, 195629489},
		{25, 548, 13502, 312835, 7736373, 184411439},
		{25, 547, 13579, 316214, 7878456, 189224276},
		{26, 583, 14252, 334705, 8198901, 198328929},
		{25, 560, 13592, 317324, 7710115, 185959088},
		{25, 560, 13607, 320792, 7848606, 190755813},
		{5, 75, 459, 8290, 47635, 899442},
		{5, 80, 493, 8897, 52710, 1001523},
		{15, 66, 1197, 7059, 133987, 764643},
		{16, 71, 1287, 7626, 145232, 846648},
		{5, 130, 782, 22180, 118882, 3517770},
		{26, 112, 3189, 17945, 532933, 2788982},
		{3, 32, 134, 2073, 10485, 179869},
		{4, 49, 243, 3991, 20780, 367724},
		{12, 38, 564, 2219, 37735, 185867},
		{15, 65, 1018, 4573, 80619, 413018},
		{26, 568, 13744, 314346, 7594526, 179862938},
		{26, 583, 14252, 334705, 8198901, 198328929},
		{25, 560, 13592, 317324, 7710115, 185959088},
		{25, 560, 13607, 320792, 7848606, 190755813},
		{25, 567, 14095, 328965, 8153719, 195629489},
		{25, 548, 13502, 312835, 7736373, 184411439},
		{25, 547, 13579, 316214, 7878456, 189224276},
		{14, 195, 2760, 38675, 570726, 8107539},
		{11, 156, 1636, 20534, 223507, 2594412},
		{19, 289, 4442, 73584, 1198299, 19870403},
		{3, 51, 345, 5301, 38348, 588695},
		{17, 54, 835, 5910, 92250, 688780},
		{15, 193, 2816, 40039, 582642, 8503277},
		{16, 180, 2290, 24640, 288141, 3147566},
		{4, 68, 1118, 16199, 281190, 4405103},
		{17, 54, 835, 5910, 92250, 688780},
		{3, 51, 345, 5301, 38348, 588695},
		{17, 278, 4607, 76778, 1320507, 22823890},
		{21, 316, 5744, 93338, 1713368, 28861171},
		{21, 144, 3242, 32955, 787524, 7881673},
		{7, 143, 1416, 31787, 310862, 7382896},
		{6, 106, 1829, 31151, 530585, 9250746},
		{17, 309, 5133, 93603, 1591064, 29027891},
		{7, 143, 1416, 31787, 310862, 7382896},
		{21, 144, 3242, 32955, 787524, 7881673},
		{19, 275, 5300, 104342, 2161211, 44956585},
		{36, 1027, 29215, 771461, 20506480, 525169084},
		{19, 275, 5300, 104342, 2161211, 44956585},
		{36, 1027, 29227, 771368, 20521342, 524966748},
		{2, 36, 143, 3637, 14893, 391507},
		{2, 36, 143, 3637, 14893, 391507},
		{6, 35, 495, 8349, 166741, 3370175},
		{22, 43, 1015, 4167, 105749, 419369},
		{2, 36, 143, 3637, 14893, 391507},
		{6, 35, 495, 8349, 166741, 3370175},
		{3, 7, 43, 199, 1347, 6249},
		{3, 7, 43, 199, 1347, 6249},
		{1, 3, 12, 80, 342, 2343},
		{1, 3, 12, 80, 342, 2343},
		{7, 35, 210, 1091, 7028, 34834},
		{1, 3, 12, 80, 342, 2343},
		{1, 3, 12, 80, 342, 2343},
		{3, 7, 43, 199, 1347, 6249},
		{3, 7, 43, 199, 1347, 6249},
		{5, 35, 182, 1091, 5408, 34822},
		{2, 8, 44, 282, 1814, 11848},
		{2, 8, 44, 282, 1814, 11848},
		{3, 9, 57, 360, 1969, 10724},
		{3, 9, 57, 360, 1969, 10724},
		{5, 25, 180, 1294, 8296, 53138},
		{8, 61, 483, 3213, 23599, 157093},
		{8, 61, 411, 3213, 21637, 158065},
		{4, 15, 90, 534, 3450, 20960},
		{3, 9, 57, 360, 1969, 10724},
		{3, 9, 57, 360, 1969, 10724},
		{5, 25, 180, 1294, 8296, 53138},
		{8, 61, 411, 3213, 21637, 158065},
		{8, 61, 483, 3213, 23599, 157093},
		{4, 15, 89, 537, 3309, 21104},
		{4, 19, 117, 720, 4661, 32191},
		{5, 19, 116, 716, 4786, 30980},
		{5, 22, 139, 877, 6112, 41874},
		{4, 16, 101, 637, 4354, 29679},
		{5, 22, 139, 877, 6112, 41874},
		{4, 16, 101, 637, 4354, 29679},
		{3, 15, 84, 573, 3013, 22886},
		{4, 16, 101, 637, 4271, 28662},
		{5, 19, 117, 720, 5014, 32167},
		{4, 19, 117, 712, 4658, 30749},
		{5, 22, 139, 877, 6112, 41874},
		{4, 16, 101, 637, 4354, 29679},
		{5, 22, 139, 877, 6112, 41874},
		{4, 16, 101, 637, 4354, 29679},
		{5, 15, 102, 569, 4337, 22579},
		{4, 16, 101, 637, 4271, 28662},
		{5, 22, 139, 877, 6112, 41874},
		{4, 16, 101, 637, 4354, 29679},
		{5, 22, 139, 877, 6112, 41874},
		{4, 16, 101, 637, 4354, 29679},
		{5, 25, 161, 1035, 7574, 55338},
		{5, 25, 161, 1035, 7574, 55338},
		{7, 49, 378, 2902, 24122, 199002},
		{5, 22, 139, 877, 6112, 41874},
		{4, 16, 101, 637, 4354, 29679},
		{5, 22, 139, 877, 6112, 41874},
		{4, 16, 101, 637, 4354, 29679},
		{5, 25, 161, 1035, 7574, 55338},
		{5, 25, 161, 1035, 7574, 55338},
		{7, 49, 378, 2902, 24122, 199002},
		{11, 97, 887, 8048, 90606, 1030499},
		{24, 421, 7421, 124608, 2193768, 37665329},
		{18, 270, 4699, 79355, 1533145, 28859283},
		{24, 496, 9483, 182838, 3605103, 71179139},
		{11, 97, 887, 8048, 90606, 1030499},
		{24, 421, 7421, 124608, 2193768, 37665329},
		{18, 270, 4699, 79355, 1533145, 28859283},
		{24, 496, 9483, 182838, 3605103, 71179139},
		{14, 191, 2812, 43238, 674624, 11030083},
		{31, 570, 17546, 351806, 11139762, 244063299},
	};

	uint64_t total_perft = 0;
	for (int i = 0; i < 128; ++i)
	{
		juicer.set_position(positions[i]);
		for (int depth = 1; depth <= 6; ++depth)
		{
			uint64_t perft = juicer.perft(depth, 1, 1);
			mu_assert(perft == perft_results[i][depth-1], positions[i]);
			total_perft += perft;
			#if QUICK_TEST == true
				if (depth == 4) break;
			#endif
		}
	}
	std::cout << "Total nodes searched: " << total_perft << std::endl;
}}}

static void suite()
{{{ 
	mu_run(perft);
}}}
}


int main()
{
	init_bitboards();

	mu_suite(Bitboards::suite);
	mu_suite(Attacks::suite);
	mu_suite(Positions::suite);
	mu_suite(Movegen::suite);

	return 0;
}
