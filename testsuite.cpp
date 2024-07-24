#include <bitset>
#include <fstream>
#include <chrono>
#include <string>
#include <sstream>
#include "minunit.h"
#include "xorshiftstar64.h"
#include "bitboard.h"
#include "movement.h"
#include "position.h"
#include "juicer.h"
#include "movegen.h"
#include "engine.h"
#include "uci.h"


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

static uint64_t xray_attack(PieceType pt, Square s, uint64_t occupied)
{{{
	uint64_t attacks = 0;
	Direction rook_directions[4] = { N, S, E, W };
	Direction bishop_directions[4] = { NE, SE, NW, SW };

	for (Direction d: (pt == ROOK ? rook_directions : bishop_directions))
	{
		Square from = s;
		bool breakout = false;
		while (safe_step(from, d) && !(breakout && (occupied & from))) 
		{ 
			if (occupied & from) 
			{
				attacks |= from;
				breakout = true;
			}
			if (breakout)
				attacks |= (from += d); 
			else
				from += d;
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
		mu_assert((Bitboard::RANK1 | Bitboard::RANK2) == 0xffff);
		mu_assert((Bitboard::RANK2 & Square::A1) == 0);
		mu_assert((Bitboard::FILEF | Square::F1) == Bitboard::FILEF);

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

		mu_assert((Bitboard::FILEA | B1) == (A8 | A7 | A6 | A5 | A4 | A3 | A2 | A1 | B1));

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
} // namespace Bitboards


namespace Attacks
{
	static void magics()
	{{{
			for (int i = 0; i < 100'000; ++i)
			{
				const Square s = Square(xrs::rand<uint8_t>() & 63);
				const uint64_t occupied = xrs::sparse_rand<uint64_t>() & ~square_to_bb(s);
				mu_assert(sliding_attack(ROOK, s, occupied) == ROOK_MAGICS[s][occupied]);
			}
			for (int i = 0; i < 100'000; ++i)
			{
				const Square s = Square(xrs::rand<uint8_t>() & 63);
				const uint64_t occupied = xrs::rand<uint64_t>() & ~square_to_bb(s);
				mu_assert(sliding_attack(ROOK, s, occupied) == ROOK_MAGICS[s][occupied]);
			}

			for (int i = 0; i < 100'000; ++i)
			{
				const Square s = Square(xrs::rand<uint8_t>() & 63);
				const uint64_t occupied = xrs::sparse_rand<uint64_t>() & ~square_to_bb(s);
				mu_assert(sliding_attack(BISHOP, s, occupied) == BISHOP_MAGICS[s][occupied]);
			}
			for (int i = 0; i < 100'000; ++i)
			{
				const Square s = Square(xrs::rand<uint8_t>() & 63);
				const uint64_t occupied = xrs::rand<uint64_t>() & ~square_to_bb(s);
				mu_assert(sliding_attack(BISHOP, s, occupied) == BISHOP_MAGICS[s][occupied]);
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
			mu_assert(PIECE_ATTACKS[KING][s] == ((Bitboard::RANK8 | Bitboard::RANK7 | Bitboard::RANK6) & (file_bb(s) | file_bb(s-1) | file_bb(s+1)) & ~square_to_bb(s)));

		mu_assert(PIECE_ATTACKS[KING][A1] == (A2 | B2 | B1));

		mu_assert(PAWN_ATTACKS[BLACK][A2] == square_to_bb(B1));
		mu_assert(PAWN_ATTACKS[WHITE][E2] == (D3 | F3));
	}}}

	static void suite()
	{{{ 
		mu_run(Attacks::magics);
		mu_run(Attacks::piece_attacks);
	}}}
} // namespace Attacks


namespace Positions
{
	static void fen_constructor()
	{{{  
		Board b1("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		mu_assert(b1.wp == Bitboard::RANK2);
		mu_assert(b1.bp == Bitboard::RANK7);
		mu_assert(b1.wn == (G1 | B1));
		mu_assert(b1.bn == (G8 | B8));
		mu_assert(b1.wb == (F1 | C1));	
		mu_assert(b1.bb == (F8 | C8));	
		mu_assert(b1.wr == (A1 | H1));	
		mu_assert(b1.br == (A8 | H8));	
		mu_assert(b1.wq == square_to_bb(D1));
		mu_assert(b1.bq == square_to_bb(D8));
		mu_assert(b1.wk == square_to_bb(E1));
		mu_assert(b1.bk == square_to_bb(E8));
		mu_assert(b1.w_pieces == (Bitboard::RANK1 | Bitboard::RANK2));
		mu_assert(b1.b_pieces == (Bitboard::RANK7 | Bitboard::RANK8));
		mu_assert(b1.pieces == (Bitboard::RANK1 | Bitboard::RANK2 | Bitboard::RANK7 | Bitboard::RANK8));

		Board b2("7k/1QQ5/6Q1/6Q1/2Q5/Q5Q1/8/K2Q2Q1 w - - 0 1");
		mu_assert(b2.wp == 0);
		mu_assert(b2.bp == 0);
		mu_assert(b2.wn == 0);
		mu_assert(b2.bn == 0);
		mu_assert(b2.wb == 0);	
		mu_assert(b2.bb == 0);	
		mu_assert(b2.wr == 0);	
		mu_assert(b2.br == 0);	
		mu_assert(b2.wq == (A3 | D1 | G1 | G3 | C4 | B7 | C7 | G6 | G5));
		mu_assert(b2.bq == 0);
		mu_assert(b2.wk == square_to_bb(A1));
		mu_assert(b2.bk == square_to_bb(H8));
		mu_assert(b2.w_pieces == (A3 | D1 | G1 | G3 | C4 | B7 | C7 | G6 | G5 | A1));
		mu_assert(b2.b_pieces == square_to_bb(H8));
		mu_assert(b2.pieces == (A3 | D1 | G1 | G3 | C4 | B7 | C7 | G6 | G5 | A1 | H8));

		Engine juicer;
		juicer.seed("4r1k1/ppp2pb1/3p1P1p/7Q/2Pq3B/1P5P/P4RP1/6K1 b - - 0 26");
		mu_assert(juicer.boardstate().turn == BLACK);
		mu_assert(juicer.boardstate().has_ep_pawn == false);
		mu_assert(juicer.boardstate().w_castle_ooo == false);
		mu_assert(juicer.boardstate().w_castle_oo  == false);
		mu_assert(juicer.boardstate().b_castle_ooo == false);
		mu_assert(juicer.boardstate().b_castle_oo  == false);

		
		juicer.seed("1rbqkbnr/pppppppp/2n5/8/4P3/5N2/PPPP1PPP/RNBQKBR1 b Qk - 4 3");
		mu_assert(juicer.boardstate().turn == BLACK);
		mu_assert(juicer.boardstate().has_ep_pawn == false);
		mu_assert(juicer.boardstate().w_castle_ooo == true);
		mu_assert(juicer.boardstate().w_castle_oo  == false);
		mu_assert(juicer.boardstate().b_castle_ooo == false);
		mu_assert(juicer.boardstate().b_castle_oo  == true);
	}}}

	static void make_simple_moves()
	{{{
		Engine juicer;

		juicer.make_move(Move(NORMAL, E2, E4, PAWN));
		mu_assert(juicer.board().wp == (A2 | B2 | C2 | D2 | E4 | F2 | G2 | H2));
		mu_assert(juicer.boardstate().turn == BLACK);

		juicer.make_move(Move(NORMAL, E7, E5, PAWN));
		mu_assert(juicer.board().wp == (A2 | B2 | C2 | D2 | E4 | F2 | G2 | H2));
		mu_assert(juicer.board().bp == (A7 | B7 | C7 | D7 | E5 | F7 | G7 | H7));
		mu_assert(juicer.boardstate().turn == WHITE);
	}}}

	static void make_captures()
	{{{  
		Engine juicer;
		juicer.seed("r1bqkb1r/ppp2ppp/2n2n2/3pp1N1/2B1P3/8/PPPP1PPP/RNBQK2R w KQkq - 0 5");
		juicer.make_move(Move(NORMAL, E4, D5, PAWN));
		mu_assert(juicer.board().bp == (A7 | B7 | C7 | E5 | F7 | G7 | H7));
		mu_assert(juicer.board().wp == (A2 | B2 | C2 | D2 | D5 | F2 | G2 | H2));
		mu_assert(juicer.board().piece_on(D5) == W_PAWN);

		juicer.make_move(Move(NORMAL, F6, D5, KNIGHT));
		mu_assert(juicer.board().wp == (A2 | B2 | C2 | D2 | F2 | G2 | H2));
		mu_assert(juicer.board().bn == (C6 | D5));
		mu_assert(juicer.board().piece_on(D5) == B_KNIGHT);

		juicer.make_move(Move(NORMAL, C4, D5, BISHOP));
		mu_assert(juicer.board().wb == (C1 | D5));
		mu_assert(juicer.board().bn == square_to_bb(C6));
		mu_assert(juicer.board().piece_on(D5) == W_BISHOP);
	}}}

	static void make_castles()
	{{{ 
		Engine juicer("r1bqkb1r/ppp2ppp/2n5/3np1N1/2B5/8/PPPP1PPP/RNBQK2R w KQkq - 0 6");
		juicer.make_move(Move(CASTLING, E1, G1, KING));
		mu_assert(juicer.boardstate().w_castle_ooo == false);
		mu_assert(juicer.boardstate().w_castle_oo  == false);
		mu_assert(juicer.boardstate().b_castle_ooo == true);
		mu_assert(juicer.boardstate().b_castle_oo  == true);
		mu_assert(juicer.board().piece_on(G1) == W_KING);
		mu_assert(juicer.board().piece_on(F1) == W_ROOK);
		mu_assert(juicer.board().piece_on(E1) == NO_PIECE);
		mu_assert(juicer.board().piece_on(H1) == NO_PIECE);

		juicer.seed("r3kbnr/ppp1pppp/2nq4/3p4/4P1b1/2NB4/PPPPNPPP/R1B1K2R b KQkq - 5 5");
		juicer.make_move(Move(CASTLING, E8, C8, KING));
		mu_assert(juicer.boardstate().w_castle_ooo == true);
		mu_assert(juicer.boardstate().w_castle_oo  == true);
		mu_assert(juicer.boardstate().b_castle_ooo == false);
		mu_assert(juicer.boardstate().b_castle_oo  == false);
		mu_assert(juicer.board().piece_on(C8) == B_KING);
		mu_assert(juicer.board().piece_on(D8) == B_ROOK);
		mu_assert(juicer.board().piece_on(A8) == NO_PIECE);
		mu_assert(juicer.board().piece_on(E8) == NO_PIECE);
	}}}

	static void make_ep_captures()
	{{{
		Engine juicer;
		juicer.seed("rnbqkbnr/ppp2ppp/4p3/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3");
		juicer.make_move(Move(EN_PASSANT, E5, D6, PAWN));
		mu_assert(juicer.board().piece_on(D6) == W_PAWN);
		mu_assert(juicer.board().piece_on(E5) == NO_PIECE);
		mu_assert(juicer.board().piece_on(D5) == NO_PIECE);

		juicer.seed("rnbqkbnr/ppp2ppp/4p3/4P3/2Pp4/3P4/PP3PPP/RNBQKBNR b KQkq c3 0 4");
		juicer.make_move(Move(EN_PASSANT, D4, C3, PAWN));
		mu_assert(juicer.board().piece_on(C3) == B_PAWN);
		mu_assert(juicer.board().piece_on(D4) == NO_PIECE);
		mu_assert(juicer.board().piece_on(C4) == NO_PIECE);
	}}}

	static void make_promotions()
	{{{ 
		Engine juicer("r2qkbnr/1Ppppppp/n7/8/8/8/1PPPPPPP/RNBQKBNR w KQkq - 1 5");
		juicer.make_move(Move(PROMOTION, B7, A8, QUEEN));
		mu_assert(juicer.board().piece_on(A8) == W_QUEEN);
		juicer.undo_move();

		juicer.make_move(Move(PROMOTION, B7, A8, KNIGHT));
		mu_assert(juicer.board().piece_on(A8) == W_KNIGHT);
		juicer.undo_move();

		juicer.make_move(Move(PROMOTION, B7, B8,ROOK));
		mu_assert(juicer.board().piece_on(B8) == W_ROOK);
		juicer.undo_move();
	}}}

	static void suite()
	{{{ 
		mu_run(Positions::fen_constructor);
		mu_run(Positions::make_simple_moves);
		mu_run(Positions::make_captures);
		mu_run(Positions::make_castles);
		mu_run(Positions::make_ep_captures);
		mu_run(Positions::make_promotions);
	}}}
}


namespace Movegen
{
	// thank you to Gigantua for this data: https://github.com/Gigantua/Gigantua/blob/main/Gigantua/Chess_Test.hpp
	static std::string positions[128] =
	{
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ;20 ;400 ;8902 ;197281 ;4865609 ;119060324",
		"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ;48 ;2039 ;97862 ;4085603 ;193690690",
		"4k3/8/8/8/8/8/8/4K2R w K - 0 1 ;15 ;66 ;1197 ;7059 ;133987 ;764643",
		"4k3/8/8/8/8/8/8/R3K3 w Q - 0 1 ;16 ;71 ;1287 ;7626 ;145232 ;846648",
		"4k2r/8/8/8/8/8/8/4K3 w k - 0 1 ;5 ;75 ;459 ;8290 ;47635 ;899442",
		"r3k3/8/8/8/8/8/8/4K3 w q - 0 1 ;5 ;80 ;493 ;8897 ;52710 ;1001523",
		"4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1 ;26 ;112 ;3189 ;17945 ;532933 ;2788982",
		"r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1 ;5 ;130 ;782 ;22180 ;118882 ;3517770",
		"8/8/8/8/8/8/6k1/4K2R w K - 0 1 ;12 ;38 ;564 ;2219 ;37735 ;185867",
		"8/8/8/8/8/8/1k6/R3K3 w Q - 0 1 ;15 ;65 ;1018 ;4573 ;80619 ;413018",
		"4k2r/6K1/8/8/8/8/8/8 w k - 0 1 ;3 ;32 ;134 ;2073 ;10485 ;179869",
		"r3k3/1K6/8/8/8/8/8/8 w q - 0 1 ;4 ;49 ;243 ;3991 ;20780 ;367724",
		"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1 ;26 ;568 ;13744 ;314346 ;7594526 ;179862938",
		"r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1 ;25 ;567 ;14095 ;328965 ;8153719 ;195629489",
		"r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1 ;25 ;548 ;13502 ;312835 ;7736373 ;184411439",
		"r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1 ;25 ;547 ;13579 ;316214 ;7878456 ;189224276",
		"1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1 ;26 ;583 ;14252 ;334705 ;8198901 ;198328929",
		"2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1 ;25 ;560 ;13592 ;317324 ;7710115 ;185959088",
		"r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1 ;25 ;560 ;13607 ;320792 ;7848606 ;190755813",
		"4k3/8/8/8/8/8/8/4K2R b K - 0 1 ;5 ;75 ;459 ;8290 ;47635 ;899442",
		"4k3/8/8/8/8/8/8/R3K3 b Q - 0 1 ;5 ;80 ;493 ;8897 ;52710 ;1001523",
		"4k2r/8/8/8/8/8/8/4K3 b k - 0 1 ;15 ;66 ;1197 ;7059 ;133987 ;764643",
		"r3k3/8/8/8/8/8/8/4K3 b q - 0 1 ;16 ;71 ;1287 ;7626 ;145232 ;846648",
		"4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1 ;5 ;130 ;782 ;22180 ;118882 ;3517770",
		"r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1 ;26 ;112 ;3189 ;17945 ;532933 ;2788982",
		"8/8/8/8/8/8/6k1/4K2R b K - 0 1 ;3 ;32 ;134 ;2073 ;10485 ;179869",
		"8/8/8/8/8/8/1k6/R3K3 b Q - 0 1 ;4 ;49 ;243 ;3991 ;20780 ;367724",
		"4k2r/6K1/8/8/8/8/8/8 b k - 0 1 ;12 ;38 ;564 ;2219 ;37735 ;185867",
		"r3k3/1K6/8/8/8/8/8/8 b q - 0 1 ;15 ;65 ;1018 ;4573 ;80619 ;413018",
		"r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1 ;26 ;568 ;13744 ;314346 ;7594526 ;179862938",
		"r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1 ;26 ;583 ;14252 ;334705 ;8198901 ;198328929",
		"r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1 ;25 ;560 ;13592 ;317324 ;7710115 ;185959088",
		"r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1 ;25 ;560 ;13607 ;320792 ;7848606 ;190755813",
		"1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1 ;25 ;567 ;14095 ;328965 ;8153719 ;195629489",
		"2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1 ;25 ;548 ;13502 ;312835 ;7736373 ;184411439",
		"r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1 ;25 ;547 ;13579 ;316214 ;7878456 ;189224276",
		"8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1 ;14 ;195 ;2760 ;38675 ;570726 ;8107539",
		"8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1 ;11 ;156 ;1636 ;20534 ;223507 ;2594412",
		"8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1 ;19 ;289 ;4442 ;73584 ;1198299 ;19870403",
		"K7/8/2n5/1n6/8/8/8/k6N w - - 0 1 ;3 ;51 ;345 ;5301 ;38348 ;588695",
		"k7/8/2N5/1N6/8/8/8/K6n w - - 0 1 ;17 ;54 ;835 ;5910 ;92250 ;688780",
		"8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1 ;15 ;193 ;2816 ;40039 ;582642 ;8503277",
		"8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1 ;16 ;180 ;2290 ;24640 ;288141 ;3147566",
		"8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1 ;4 ;68 ;1118 ;16199 ;281190 ;4405103",
		"K7/8/2n5/1n6/8/8/8/k6N b - - 0 1 ;17 ;54 ;835 ;5910 ;92250 ;688780",
		"k7/8/2N5/1N6/8/8/8/K6n b - - 0 1 ;3 ;51 ;345 ;5301 ;38348 ;588695",
		"B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1 ;17 ;278 ;4607 ;76778 ;1320507 ;22823890",
		"8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1 ;21 ;316 ;5744 ;93338 ;1713368 ;28861171",
		"k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1 ;21 ;144 ;3242 ;32955 ;787524 ;7881673",
		"K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1 ;7 ;143 ;1416 ;31787 ;310862 ;7382896",
		"B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1 ;6 ;106 ;1829 ;31151 ;530585 ;9250746",
		"8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1 ;17 ;309 ;5133 ;93603 ;1591064 ;29027891",
		"k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1 ;7 ;143 ;1416 ;31787 ;310862 ;7382896",
		"K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1 ;21 ;144 ;3242 ;32955 ;787524 ;7881673",
		"7k/RR6/8/8/8/8/rr6/7K w - - 0 1 ;19 ;275 ;5300 ;104342 ;2161211 ;44956585",
		"R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1 ;36 ;1027 ;29215 ;771461 ;20506480 ;525169084",
		"7k/RR6/8/8/8/8/rr6/7K b - - 0 1 ;19 ;275 ;5300 ;104342 ;2161211 ;44956585",
		"R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1 ;36 ;1027 ;29227 ;771368 ;20521342 ;524966748",
		"6kq/8/8/8/8/8/8/7K w - - 0 1 ;2 ;36 ;143 ;3637 ;14893 ;391507",
		"6KQ/8/8/8/8/8/8/7k b - - 0 1 ;2 ;36 ;143 ;3637 ;14893 ;391507",
		"K7/8/8/3Q4/4q3/8/8/7k w - - 0 1 ;6 ;35 ;495 ;8349 ;166741 ;3370175",
		"6qk/8/8/8/8/8/8/7K b - - 0 1 ;22 ;43 ;1015 ;4167 ;105749 ;419369",
		"6KQ/8/8/8/8/8/8/7k b - - 0 1 ;2 ;36 ;143 ;3637 ;14893 ;391507",
		"K7/8/8/3Q4/4q3/8/8/7k b - - 0 1 ;6 ;35 ;495 ;8349 ;166741 ;3370175",
		"8/8/8/8/8/K7/P7/k7 w - - 0 1 ;3 ;7 ;43 ;199 ;1347 ;6249",
		"8/8/8/8/8/7K/7P/7k w - - 0 1 ;3 ;7 ;43 ;199 ;1347 ;6249",
		"K7/p7/k7/8/8/8/8/8 w - - 0 1 ;1 ;3 ;12 ;80 ;342 ;2343",
		"7K/7p/7k/8/8/8/8/8 w - - 0 1 ;1 ;3 ;12 ;80 ;342 ;2343",
		"8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1 ;7 ;35 ;210 ;1091 ;7028 ;34834",
		"8/8/8/8/8/K7/P7/k7 b - - 0 1 ;1 ;3 ;12 ;80 ;342 ;2343",
		"8/8/8/8/8/7K/7P/7k b - - 0 1 ;1 ;3 ;12 ;80 ;342 ;2343",
		"K7/p7/k7/8/8/8/8/8 b - - 0 1 ;3 ;7 ;43 ;199 ;1347 ;6249",
		"7K/7p/7k/8/8/8/8/8 b - - 0 1 ;3 ;7 ;43 ;199 ;1347 ;6249",
		"8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1 ;5 ;35 ;182 ;1091 ;5408 ;34822",
		"8/8/8/8/8/4k3/4P3/4K3 w - - 0 1 ;2 ;8 ;44 ;282 ;1814 ;11848",
		"4k3/4p3/4K3/8/8/8/8/8 b - - 0 1 ;2 ;8 ;44 ;282 ;1814 ;11848",
		"8/8/7k/7p/7P/7K/8/8 w - - 0 1 ;3 ;9 ;57 ;360 ;1969 ;10724",
		"8/8/k7/p7/P7/K7/8/8 w - - 0 1 ;3 ;9 ;57 ;360 ;1969 ;10724",
		"8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1 ;5 ;25 ;180 ;1294 ;8296 ;53138",
		"8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1 ;8 ;61 ;483 ;3213 ;23599 ;157093",
		"8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1 ;8 ;61 ;411 ;3213 ;21637 ;158065",
		"k7/8/3p4/8/3P4/8/8/7K w - - 0 1 ;4 ;15 ;90 ;534 ;3450 ;20960",
		"8/8/7k/7p/7P/7K/8/8 b - - 0 1 ;3 ;9 ;57 ;360 ;1969 ;10724",
		"8/8/k7/p7/P7/K7/8/8 b - - 0 1 ;3 ;9 ;57 ;360 ;1969 ;10724",
		"8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1 ;5 ;25 ;180 ;1294 ;8296 ;53138",
		"8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1 ;8 ;61 ;411 ;3213 ;21637 ;158065",
		"8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1 ;8 ;61 ;483 ;3213 ;23599 ;157093",
		"k7/8/3p4/8/3P4/8/8/7K b - - 0 1 ;4 ;15 ;89 ;537 ;3309 ;21104",
		"7k/3p4/8/8/3P4/8/8/K7 w - - 0 1 ;4 ;19 ;117 ;720 ;4661 ;32191",
		"7k/8/8/3p4/8/8/3P4/K7 w - - 0 1 ;5 ;19 ;116 ;716 ;4786 ;30980",
		"k7/8/8/7p/6P1/8/8/K7 w - - 0 1 ;5 ;22 ;139 ;877 ;6112 ;41874",
		"k7/8/7p/8/8/6P1/8/K7 w - - 0 1 ;4 ;16 ;101 ;637 ;4354 ;29679",
		"k7/8/8/6p1/7P/8/8/K7 w - - 0 1 ;5 ;22 ;139 ;877 ;6112 ;41874",
		"k7/8/6p1/8/8/7P/8/K7 w - - 0 1 ;4 ;16 ;101 ;637 ;4354 ;29679",
		"k7/8/8/3p4/4p3/8/8/7K w - - 0 1 ;3 ;15 ;84 ;573 ;3013 ;22886",
		"k7/8/3p4/8/8/4P3/8/7K w - - 0 1 ;4 ;16 ;101 ;637 ;4271 ;28662",
		"7k/3p4/8/8/3P4/8/8/K7 b - - 0 1 ;5 ;19 ;117 ;720 ;5014 ;32167",
		"7k/8/8/3p4/8/8/3P4/K7 b - - 0 1 ;4 ;19 ;117 ;712 ;4658 ;30749",
		"k7/8/8/7p/6P1/8/8/K7 b - - 0 1 ;5 ;22 ;139 ;877 ;6112 ;41874",
		"k7/8/7p/8/8/6P1/8/K7 b - - 0 1 ;4 ;16 ;101 ;637 ;4354 ;29679",
		"k7/8/8/6p1/7P/8/8/K7 b - - 0 1 ;5 ;22 ;139 ;877 ;6112 ;41874",
		"k7/8/6p1/8/8/7P/8/K7 b - - 0 1 ;4 ;16 ;101 ;637 ;4354 ;29679",
		"k7/8/8/3p4/4p3/8/8/7K b - - 0 1 ;5 ;15 ;102 ;569 ;4337 ;22579",
		"k7/8/3p4/8/8/4P3/8/7K b - - 0 1 ;4 ;16 ;101 ;637 ;4271 ;28662",
		"7k/8/8/p7/1P6/8/8/7K w - - 0 1 ;5 ;22 ;139 ;877 ;6112 ;41874",
		"7k/8/p7/8/8/1P6/8/7K w - - 0 1 ;4 ;16 ;101 ;637 ;4354 ;29679",
		"7k/8/8/1p6/P7/8/8/7K w - - 0 1 ;5 ;22 ;139 ;877 ;6112 ;41874",
		"7k/8/1p6/8/8/P7/8/7K w - - 0 1 ;4 ;16 ;101 ;637 ;4354 ;29679",
		"k7/7p/8/8/8/8/6P1/K7 w - - 0 1 ;5 ;25 ;161 ;1035 ;7574 ;55338",
		"k7/6p1/8/8/8/8/7P/K7 w - - 0 1 ;5 ;25 ;161 ;1035 ;7574 ;55338",
		"3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1 ;7 ;49 ;378 ;2902 ;24122 ;199002",
		"7k/8/8/p7/1P6/8/8/7K b - - 0 1 ;5 ;22 ;139 ;877 ;6112 ;41874",
		"7k/8/p7/8/8/1P6/8/7K b - - 0 1 ;4 ;16 ;101 ;637 ;4354 ;29679",
		"7k/8/8/1p6/P7/8/8/7K b - - 0 1 ;5 ;22 ;139 ;877 ;6112 ;41874",
		"7k/8/1p6/8/8/P7/8/7K b - - 0 1 ;4 ;16 ;101 ;637 ;4354 ;29679",
		"k7/7p/8/8/8/8/6P1/K7 b - - 0 1 ;5 ;25 ;161 ;1035 ;7574 ;55338",
		"k7/6p1/8/8/8/8/7P/K7 b - - 0 1 ;5 ;25 ;161 ;1035 ;7574 ;55338",
		"3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1 ;7 ;49 ;378 ;2902 ;24122 ;199002",
		"8/Pk6/8/8/8/8/6Kp/8 w - - 0 1 ;11 ;97 ;887 ;8048 ;90606 ;1030499",
		"n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1 ;24 ;421 ;7421 ;124608 ;2193768 ;37665329",
		"8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1 ;18 ;270 ;4699 ;79355 ;1533145 ;28859283",
		"n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1 ;24 ;496 ;9483 ;182838 ;3605103 ;71179139",
		"8/Pk6/8/8/8/8/6Kp/8 b - - 0 1 ;11 ;97 ;887 ;8048 ;90606 ;1030499",
		"n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1 ;24 ;421 ;7421 ;124608 ;2193768 ;37665329",
		"8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1 ;18 ;270 ;4699 ;79355 ;1533145 ;28859283",
		"n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1 ;24 ;496 ;9483 ;182838 ;3605103 ;71179139",
		"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ;14 ;191 ;2812 ;43238 ;674624 ;11030083",
		"rnbqkb1r/ppppp1pp/7n/4Pp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3 ;31 ;570 ;17546 ;351806 ;11139762;244063299",
	};

	static void perft()
	{{{
		Engine juicer;
		uint64_t total_nodes = 0;

		for (const std::string& position: Movegen::positions)
		{
			std::istringstream ss(position);
			std::string segment;
			std::string fen;
			
			std::getline(ss, fen, ';');
			juicer.seed(fen);

			for (int i = 1; std::getline(ss, segment, ';'); ++i)
			{
				uint64_t perftnodes = juicer.perft<false>(i);
				uint64_t reference = std::stoi(segment);
				mu_assert(perftnodes == reference, position + "\n\tExpected: " + std::to_string(reference) + " Actual: " + std::to_string(perftnodes));
				total_nodes += perftnodes;
			}
		}

		std::cout << "Total nodes searched: " << total_nodes << std::endl;
	}}}

	static void suite()
	{{{ 
		mu_run(perft);
	}}}
}


int main()
{
	mu_suite(Bitboards::suite);
	mu_suite(Attacks::suite);
	mu_suite(Positions::suite);
	mu_suite(Movegen::suite);

	return 0;
}


