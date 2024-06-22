#include <bitset>
#include "minunit.h"
#include "bitboard.h"
#include "position.h"
#include "xorshiftstar64.h"


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


namespace Movegen
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

static void pseudo_attacks()
{{{
	mu_assert(PSEUDO_ATTACKS[KNIGHT][A1] == (B3 | C2));
	mu_assert(PSEUDO_ATTACKS[KNIGHT][D4] == (C2 | E2 | B3 | F3 | B5 | F5 | C6 | E6));
	mu_assert(PSEUDO_ATTACKS[KNIGHT][H2] == (F1 | F3 | G4));
	mu_assert(PSEUDO_ATTACKS[KNIGHT][G3] == (H1 | F1 | E2 | E4 | F5 | H5));
	mu_assert(PSEUDO_ATTACKS[KNIGHT][B6] == (A8 | C8 | A4 | C4 | D7 | D5));
	mu_assert(PSEUDO_ATTACKS[KNIGHT][F7] == (H8 | D8 | H6 | D6 | G5 | E5));
	mu_assert(PSEUDO_ATTACKS[KNIGHT][H1] == (F2 | G3));
	mu_assert(PSEUDO_ATTACKS[KNIGHT][E5] == (D7 | F7 | G6 | C6 | F3 | D3 | G4 | C4));

	mu_assert(PSEUDO_ATTACKS[BISHOP][A1] == (B2 | C3 | D4 | E5 | F6 | G7 | H8));
	mu_assert(PSEUDO_ATTACKS[BISHOP][C2] == (B1 | D1 | B3 | A4 | D3 | E4 | F5 | G6 | H7));
	mu_assert(PSEUDO_ATTACKS[BISHOP][C6] == (B7 | A8 | D5 | E4 | F3 | G2 | H1 | B5 | A4 | D7 | E8));

	for (Square s = A1; s <= H8; ++s)
	{
		mu_assert((PSEUDO_ATTACKS[BISHOP][s] & (file_bb(s) | rank_bb(s))) == 0);
		mu_assert((PSEUDO_ATTACKS[KNIGHT][s] & (file_bb(s) | rank_bb(s))) == 0);
		mu_assert((PSEUDO_ATTACKS[KNIGHT][s] & PSEUDO_ATTACKS[QUEEN][s]) == 0);
	}

	for (Square s = A1; s <= H8; ++s)
		mu_assert(PSEUDO_ATTACKS[ROOK][s] == ((rank_bb(s) | file_bb(s)) & ~square_to_bb(s)));

	for (Square s = A1; s <= H8; ++s)
		mu_assert(PSEUDO_ATTACKS[QUEEN][s] == (PSEUDO_ATTACKS[BISHOP][s] | PSEUDO_ATTACKS[ROOK][s]));

	for (Square s = B7; s <= G7; ++s)
		mu_assert(PSEUDO_ATTACKS[KING][s] == ((RANK8_BB | RANK7_BB | RANK6_BB) & (file_bb(s) | file_bb(s-1) | file_bb(s+1)) & ~square_to_bb(s)));

	mu_assert(PSEUDO_ATTACKS[KING][A1] == (A2 | B2 | B1));
}}}

static void suite()
{{{
	mu_run(Movegen::magics);
	mu_run(Movegen::pseudo_attacks);
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

	pos.seed("8/8/8/8/k7/8/5PPP/4K2R w K e8 0 35", gs);
	mu_assert(pos.pieces() == (A4 | E1 | F2 | G2 | H2 | H1));
	mu_assert(pos.ep_square() == NO_SQUARE);
	mu_assert(pos.ply() == 68);
	mu_assert(pos.castling_rights() == 1);

	pos.seed("8/8/8/8/8/8/8/8 b qk e8 999999 999999", gs);
	mu_assert(pos.pieces() == 0);
	mu_assert(pos.ply() == 2 * 999998 + 1);
	mu_assert(pos.castling_rights() == 12);
}}}

static void make_simple_moves()
{{{
	Gamestate gs;
	Position pos;
	pos.seed("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", gs);

	Gamestate dummy_gs;
	pos.make_move(Move::make(E2, E4), dummy_gs);
	mu_assert(pos.pieces(W_PAWN) == (A2 | B2 | C2 | D2 | E4 | F2 | G2 | H2));
	mu_assert(pos.side_to_move() == BLACK);
	mu_assert(pos.ply() == 1);
	mu_assert(pos.rule_50() == 1);

	pos.make_move(Move::make(C7, C5), dummy_gs);
	mu_assert(pos.pieces(W_PAWN) == (A2 | B2 | C2 | D2 | E4 | F2 | G2 | H2));
	mu_assert(pos.pieces(B_PAWN) == (A7 | B7 | C5 | D7 | E7 | F7 | G7 | H7));
	mu_assert(pos.side_to_move() == WHITE);
	mu_assert(pos.ply() == 2);
	mu_assert(pos.rule_50() == 2);

	pos.make_move(Move::make(G1, F3), dummy_gs);
	mu_assert(pos.pieces(W_KNIGHT) == (B1 | F3));
	mu_assert(pos.side_to_move() == BLACK);
	mu_assert(pos.ply() == 3);
	mu_assert(pos.rule_50() == 3);

	pos.seed("rnbqkbnr/pp2pppp/3p4/8/3NP3/8/PPP2PPP/RNBQKB1R b KQkq - 0 4", gs);
	pos.make_move(Move::make(G8, F6), dummy_gs);
	mu_assert(pos.pieces(B_KNIGHT) == (B8 | F6));
	mu_assert(pos.side_to_move() == WHITE);
	mu_assert(pos.ply() == 8);

	pos.make_move(Move::make(F1, B5), dummy_gs);
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

	Gamestate dummy_gs;
	pos.make_move(Move::make(F1, D3), dummy_gs);
	pos.make_move(Move::make(F8, E7), dummy_gs);
	pos.make_move(Move::make(C2, C3), dummy_gs);

	Gamestate gs4;
	pos.make_move(Move::make(B4, C3), gs4);
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

	pos.seed("rnbqkbnr/p2ppppp/2p5/Pp6/8/8/1PPPPPPP/RNBQKBNR w KQkq b6 0 3", gs2);
	pos.make_move(Move::make(A5, B6, EN_PASSANT), gs2);
	mu_assert(pos.piece_on(B6) == W_PAWN);
	mu_assert(pos.piece_on(B5) == NO_PIECE);
	mu_assert(pos.piece_on(A5) == NO_PIECE);
}}}

static void make_promotions()
{{{
	
}}}

static void suite()
{{{   
	mu_run(Positions::fen_constructor);
	mu_run(Positions::make_simple_moves);
	mu_run(Positions::make_captures);
	mu_run(Positions::make_castles);
	mu_run(Positions::make_ep_captures);
}}}
}


int main()
{
	init_bitboards();

	mu_suite(Bitboards::suite);
	mu_suite(Movegen::suite);
	mu_suite(Positions::suite);

	return 0;
}