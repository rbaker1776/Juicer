#ifndef POSITION_H_E76CAC7920F0
#define POSITION_H_E76CAC7920F0

#include <cassert>
#include <string>
#include <string_view>
#include "bitboard.h"
#include "types.h"
#include "juicer.h"


class Gamestate
{
public:
	const Color turn;
	const bool has_ep_pawn;
	const bool w_castle_ooo;
	const bool w_castle_oo;
	const bool b_castle_ooo;
	const bool b_castle_oo;

	constexpr Gamestate(Color turn, bool ep, bool w_ooo, bool w_oo, bool b_ooo, bool b_oo):
		turn(turn), has_ep_pawn(ep), w_castle_ooo(w_ooo), w_castle_oo(w_oo), b_castle_ooo(b_ooo), b_castle_oo(b_oo)
	{}

	constexpr Gamestate(uint8_t pattern): 
		turn((pattern & 0b100000) ? BLACK : WHITE), has_ep_pawn(bool(pattern & 0b010000)),
		w_castle_ooo(bool(pattern & WHITE_OOO)), w_castle_oo(bool(pattern & WHITE_OO)),
		b_castle_ooo(bool(pattern & BLACK_OOO)), b_castle_oo(bool(pattern & BLACK_OO))
	{}

	inline constexpr uint8_t pattern() const
	{
		return ((turn == BLACK) << 5) | (has_ep_pawn << 4) | (w_castle_ooo << 3) | (w_castle_oo << 2) | (b_castle_ooo << 1) | b_castle_oo;
	}
	
	constexpr bool can_castle() const { return turn == WHITE ? (w_castle_ooo || w_castle_oo) : (b_castle_ooo || b_castle_oo); }
	constexpr bool can_castle_kingside() const { return turn == WHITE ? w_castle_oo : b_castle_oo; }
	constexpr bool can_castle_queenside() const { return turn == WHITE ? w_castle_ooo : b_castle_ooo; }

	constexpr uint64_t rookswitch_kingside() const { return turn == WHITE ? (F1 | H1) : (F8 | H8); }
	constexpr uint64_t rookswitch_queenside() const { return turn == WHITE ? (A1 | D1) : (A8 | D8); }

	inline constexpr bool can_castle_queenside(uint64_t seen, uint64_t occupied, uint64_t rook) const
	{
		if (turn == WHITE && w_castle_ooo)
		{
			if (occupied & W_EMPTY_OOO || seen & W_UNSEEN_OOO) return false;
			if (rook & W_ROOK_OOO) return true;
		}
		else if (turn == BLACK && b_castle_ooo)
		{
			if (occupied & B_EMPTY_OOO || seen & B_UNSEEN_OOO) return false;
			if (rook & B_ROOK_OOO) return true;
		}
		return false;
	}

	inline constexpr bool can_castle_kingside(uint64_t seen, uint64_t occupied, uint64_t rook) const
	{
		if (turn == WHITE && w_castle_oo)
			if (occupied & W_EMPTY_OO || seen & W_UNSEEN_OO) 
				return false;
			if (rook & W_ROOK_OO) 
				return true;
		else if (turn == BLACK && b_castle_oo)
			if (occupied & B_EMPTY_OO || seen & B_UNSEEN_OO) 
				return false;
			if (rook & B_ROOK_OO) 
				return true;
		return false;
	}

	constexpr bool is_queenside_rook(uint64_t rook) const { return turn == WHITE ? rook == W_ROOK_OOO : rook == B_ROOK_OOO; }
	constexpr bool is_kingside_rook(uint64_t rook) const { return turn == WHITE ? rook == W_ROOK_OO : rook == B_ROOK_OO; }

	constexpr Gamestate pawn_push() const { return Gamestate(~turn, true, w_castle_ooo, w_castle_oo, b_castle_ooo, b_castle_oo); }
	constexpr Gamestate quiet_move() const { return Gamestate(~turn, false, w_castle_ooo, w_castle_oo, b_castle_ooo, b_castle_oo); }

	constexpr Gamestate king_move() const
	{
		if (turn == WHITE)
			return Gamestate(BLACK, false, false, false, b_castle_ooo, b_castle_oo);
		else
			return Gamestate(WHITE, false, w_castle_ooo, w_castle_oo, false, false);
	}

	constexpr Gamestate rook_move_queenside() const { return turn == WHITE ? Gamestate(BLACK, false, false, w_castle_oo, b_castle_ooo, b_castle_oo) : Gamestate(WHITE, false, w_castle_ooo, w_castle_oo, false, b_castle_oo); }
	constexpr Gamestate rook_move_kingside() const { return turn == WHITE ? Gamestate(BLACK, false, w_castle_ooo, false, b_castle_ooo, b_castle_oo) : Gamestate(WHITE, false, w_castle_ooo, w_castle_oo, b_castle_ooo, false); }

	static constexpr Gamestate DEFAULT() { return Gamestate(WHITE, false, true, true, true, true); }

private:
	static constexpr uint64_t W_EMPTY_OOO = B1 | C1 | D1;
	static constexpr uint64_t B_EMPTY_OOO = B8 | C8 | D8;
	static constexpr uint64_t W_EMPTY_OO = F1 | G1;
	static constexpr uint64_t B_EMPTY_OO = F8 | G8;

	static constexpr uint64_t W_UNSEEN_OOO = E1 | D1 | C1;
	static constexpr uint64_t B_UNSEEN_OOO = E8 | D8 | C8;
	static constexpr uint64_t W_UNSEEN_OO = E1 | F1 | G1;
	static constexpr uint64_t B_UNSEEN_OO = E8 | F8 | G8;

	static constexpr uint64_t W_ROOK_OOO = square_to_bb(A1);
	static constexpr uint64_t B_ROOK_OOO = square_to_bb(A8);
	static constexpr uint64_t W_ROOK_OO = square_to_bb(H1);
	static constexpr uint64_t B_ROOK_OO = square_to_bb(H8);
};


namespace FEN
{
static constexpr std::string_view STARTPOS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
static constexpr std::string_view PIECES = " PNBRQK  pnbrqk";

enum Field: int
{
	TURN,
	HAS_EP,
	W_CASTLE_OOO,
	W_CASTLE_OO,
	B_CASTLE_OOO,
	B_CASTLE_OO,
};

static constexpr uint64_t ep_square(std::string_view fen)
{
	const char* token = &fen[0];

	while (!isspace(*token++)) {}

	Color turn = *token++ == 'w' ? WHITE : BLACK;
	token++;

	while (!isspace(*token++)) {}

	char rank, file;
	if (((file = *token++) && (file >= 'a' && file <= 'h')) && ((rank = *token++) && (rank >= '1' && rank <= '8')))
		return square_to_bb(make_square(File(file - 'a'), Rank(rank - '1')));

	return 0ull;
}

template<FEN::Field F>
static constexpr bool info(std::string_view fen)
{
	const char* token = &fen[0];

	while (!isspace(*token++)) {}

	Color turn = *token++ == 'w' ? WHITE : BLACK;

	if constexpr (F == FEN::Field::TURN) { return turn == BLACK; }

	while (!isspace(*token++))
	{
		if constexpr (F == FEN::Field::W_CASTLE_OOO) if (*token == 'Q') return true;
		if constexpr (F == FEN::Field::W_CASTLE_OO)  if (*token == 'K') return true;
		if constexpr (F == FEN::Field::B_CASTLE_OOO) if (*token == 'q') return true;
		if constexpr (F == FEN::Field::B_CASTLE_OO)  if (*token == 'k') return true;
	}

	if constexpr (F == FEN::Field::W_CASTLE_OO || F == FEN::Field::W_CASTLE_OOO || F == FEN::Field::B_CASTLE_OO || F == FEN::Field::B_CASTLE_OOO) return false;

	char rank, file;
	if (((file = *token++) && (file >= 'a' && file <= 'h')) && ((rank = *token++) && (rank >= '1' && rank <= '8')))
		if constexpr (F == FEN::Field::HAS_EP) return true;

	if constexpr (F == FEN::Field::HAS_EP) return false;
}

static constexpr uint64_t bitboard(std::string_view fen, Piece pc)
{
	const char* token = &fen[0];
	uint64_t bb = 0;

	for (Square s = A8; !isspace(*token); ++token)
	{
		if (isnumber(*token))
			s += Direction((*token - '0') * Direction::E);
		else if (*token == '/')
			s += Direction::SS;
		else if (FEN::PIECES[pc] == *token)
		{
			bb |= s;
			++s;
		}
		else
			++s;
	}

	return bb;
}
} // namespace FEN


struct Board
{
	const uint64_t wp, wn, wb, wr, wq, wk;
	const uint64_t bp, bn, bb, br, bq, bk;
	
	const uint64_t w_pieces, b_pieces;
	const uint64_t pieces;

	constexpr Board(
		uint64_t wp, uint64_t wn, uint64_t wb, uint64_t wr, uint64_t wq, uint64_t wk,
		uint64_t bp, uint64_t bn, uint64_t bb, uint64_t br, uint64_t bq, uint64_t bk
	):
		wp(wp), wn(wn), wb(wb), wr(wr), wq(wq), wk(wk),
		bp(bp), bn(bn), bb(bb), br(br), bq(bq), bk(bk),
		w_pieces(wp | wn | wb | wr | wq | wk),
		b_pieces(bp | bn | bb | br | bq | bk),
		pieces(w_pieces | b_pieces)
	{}

	constexpr Board(std::string_view fen): Board(
		FEN::bitboard(fen, W_PAWN), FEN::bitboard(fen, W_KNIGHT), FEN::bitboard(fen, W_BISHOP), FEN::bitboard(fen, W_ROOK), FEN::bitboard(fen, W_QUEEN), FEN::bitboard(fen, W_KING),
		FEN::bitboard(fen, B_PAWN), FEN::bitboard(fen, B_KNIGHT), FEN::bitboard(fen, B_BISHOP), FEN::bitboard(fen, B_ROOK), FEN::bitboard(fen, B_QUEEN), FEN::bitboard(fen, B_KING)
	) {}

	template<Color C, PieceType Pt, bool IsCapture>
	constexpr Board move(Square from, Square to) const
	{
		const uint64_t sqs = from | to;
		if constexpr (IsCapture)
		{
			#if (DEBUG)
				assert(pieces & to);
			#endif

			const uint64_t rem = ~square_to_bb(to);

			if constexpr (C == WHITE)
			{
				#if (DEBUG)
					assert(w_pieces & from);
					assert(!(bk & sqs));
				#endif
				if constexpr (Pt == PAWN)   return Board(wp ^ sqs, wn, wb, wr, wq, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk);
				if constexpr (Pt == KNIGHT) return Board(wp, wn ^ sqs, wb, wr, wq, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk);
				if constexpr (Pt == BISHOP) return Board(wp, wn, wb ^ sqs, wr, wq, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk);
				if constexpr (Pt == ROOK)   return Board(wp, wn, wb, wr ^ sqs, wq, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk);
				if constexpr (Pt == QUEEN)  return Board(wp, wn, wb, wr, wq ^ sqs, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk);
				if constexpr (Pt == KING)   return Board(wp, wn, wb, wr, wq, wk ^ sqs, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk);
			}
			else // (C == BLACK)
			{
				#if (DEBUG)
					assert(b_pieces & from);
					assert(!(wk & sqs));
				#endif
				if constexpr (Pt == PAWN)   return Board(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ sqs, bn, bb, br, bq, bk);
				if constexpr (Pt == KNIGHT) return Board(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn ^ sqs, bb, br, bq, bk);
				if constexpr (Pt == BISHOP) return Board(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn, bb ^ sqs, br, bq, bk);
				if constexpr (Pt == ROOK)   return Board(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn, bb, br ^ sqs, bq, bk);
				if constexpr (Pt == QUEEN)  return Board(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn, bb, br, bq ^ sqs, bk);
				if constexpr (Pt == KING)   return Board(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn, bb, br, bq, bk ^ sqs);
			}
		}
		else // (!IsCapture)
		{
			#if (DEBUG)
				assert(~pieces & to);
			#endif

			if constexpr (C == WHITE)
			{
				#if (DEBUG)
					assert(w_pieces & from);
					assert(!(bk & sqs));
				#endif
				if constexpr (Pt == PAWN)   return Board(wp ^ sqs, wn, wb, wr, wq, wk, bp, bn, bb, br, bq, bk);
				if constexpr (Pt == KNIGHT) return Board(wp, wn ^ sqs, wb, wr, wq, wk, bp, bn, bb, br, bq, bk);
				if constexpr (Pt == BISHOP) return Board(wp, wn, wb ^ sqs, wr, wq, wk, bp, bn, bb, br, bq, bk);
				if constexpr (Pt == ROOK)   return Board(wp, wn, wb, wr ^ sqs, wq, wk, bp, bn, bb, br, bq, bk);
				if constexpr (Pt == QUEEN)  return Board(wp, wn, wb, wr, wq ^ sqs, wk, bp, bn, bb, br, bq, bk);
				if constexpr (Pt == KING)   return Board(wp, wn, wb, wr, wq, wk ^ sqs, bp, bn, bb, br, bq, bk);
			}
			else // (C == BLACK)
			{
				#if (DEBUG)
					assert(b_pieces & from);
					assert(!(wk & sqs));
				#endif
				if constexpr (Pt == PAWN)   return Board(wp, wn, wb, wr, wq, wk, bp ^ sqs, bn, bb, br, bq, bk);
				if constexpr (Pt == KNIGHT) return Board(wp, wn, wb, wr, wq, wk, bp, bn ^ sqs, bb, br, bq, bk);
				if constexpr (Pt == BISHOP) return Board(wp, wn, wb, wr, wq, wk, bp, bn, bb ^ sqs, br, bq, bk);
				if constexpr (Pt == ROOK)   return Board(wp, wn, wb, wr, wq, wk, bp, bn, bb, br ^ sqs, bq, bk);
				if constexpr (Pt == QUEEN)  return Board(wp, wn, wb, wr, wq, wk, bp, bn, bb, br, bq ^ sqs, bk);
				if constexpr (Pt == KING)   return Board(wp, wn, wb, wr, wq, wk, bp, bn, bb, br, bq, bk ^ sqs);
			}
		}
	}

	template<Color C>
	constexpr Board castles(uint64_t kingswitch, uint64_t rookswitch) const
	{
		if constexpr (C == WHITE)
			return Board(wp, wn, wb, wr ^ rookswitch, wq, wk ^ kingswitch, bp, bn, bb, br, bq, bk);
		else
			return Board(wp, wn, wb, wr, wq, wk, bp, bn, bb, br ^ rookswitch, bq, bk ^ kingswitch);
	}

	template<Color C>
	constexpr Board enpassant(Square from, Square to) const
	{
		const Square enemy = make_square(file_of(to), rank_of(from));
		const uint64_t sqs = to | from;

		#if (DEBUG)
			assert(!(pieces & to));
			assert((wp | bp) & enemy);
			assert((wp | bp) & from);
		#endif

		if constexpr (C == WHITE)
			return Board(wp ^ sqs, wn, wb, wr, wq, wk, bp & ~square_to_bb(enemy), bn, bb, br, bq, bk);
		else
			return Board(wp & ~square_to_bb(enemy), wn, wb, wr, wq, wk, bp ^ sqs, bn, bb, br, bq, bk);
	}

	template<Color C, PieceType Pt, bool IsCapture>
	constexpr Board promote(Square from, Square to) const
	{
		const uint64_t sqs = from | to;
		if constexpr (IsCapture)
		{
			#if (DEBUG)
				assert(pieces & to);
			#endif

			const uint64_t rem = ~square_to_bb(to);

			if constexpr (C == WHITE)
			{
				#if (DEBUG)
					assert(w_pieces & from);
					assert(!(bk & sqs));
					assert(RANK8_BB & to);
				#endif
				if constexpr (Pt == QUEEN)  return Board(wp ^ from, wn, wb, wr, wq ^ to, wk, bp, bn & rem, bb & rem, br & rem, bq & rem, bk);
				if constexpr (Pt == KNIGHT) return Board(wp ^ from, wn ^ to, wb, wr, wq, wk, bp, bn & rem, bb & rem, br & rem, bq & rem, bk);
				if constexpr (Pt == ROOK)   return Board(wp ^ from, wn, wb, wr ^ to, wq, wk, bp, bn & rem, bb & rem, br & rem, bq & rem, bk);
				if constexpr (Pt == BISHOP) return Board(wp ^ from, wn, wb ^ to, wr, wq, wk, bp, bn & rem, bb & rem, br & rem, bq & rem, bk);
			}
			else // (C == BLACK)
			{
				#if (DEBUG)
					assert(b_pieces & from);
					assert(!(wk & sqs));
					assert(RANK1_BB & to);
				#endif
				if constexpr (Pt == QUEEN)  return Board(wp, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ from, bn, bb, br, bq ^ to, bk);
				if constexpr (Pt == KNIGHT) return Board(wp, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ from, bn ^ to, bb, br, bq, bk);
				if constexpr (Pt == ROOK)   return Board(wp, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ from, bn, bb, br ^ to, bq, bk);
				if constexpr (Pt == BISHOP) return Board(wp, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ from, bn, bb ^ to, br, bq, bk);
			}
		}
		else // (!IsCapture)
		{
			#if (DEBUG)
				assert(~pieces & to);
			#endif

			if constexpr (C == WHITE)
			{
				#if (DEBUG)
					assert(w_pieces & from);
					assert(!(bk & sqs));
					assert(RANK8_BB & to);
				#endif
				if constexpr (Pt == QUEEN)  return Board(wp ^ from, wn, wb, wr, wq ^ to, wk, bp, bn, bb, br, bq, bk);
				if constexpr (Pt == KNIGHT) return Board(wp ^ from, wn ^ to, wb, wr, wq, wk, bp, bn, bb, br, bq, bk);
				if constexpr (Pt == ROOK)   return Board(wp ^ from, wn, wb, wr ^ to, wq, wk, bp, bn, bb, br, bq, bk);
				if constexpr (Pt == BISHOP) return Board(wp ^ from, wn, wb ^ to, wr, wq, wk, bp, bn, bb, br, bq, bk);
			}
			else // (C == BLACK)
			{
				#if (DEBUG)
					assert(b_pieces & from);
					assert(!(wk & sqs));
					assert(RANK1_BB & to);
				#endif
				if constexpr (Pt == QUEEN)  return Board(wp, wn, wb, wr, wq, wk, bp ^ from, bn, bb, br, bq ^ to, bk);
				if constexpr (Pt == KNIGHT) return Board(wp, wn, wb, wr, wq, wk, bp ^ from, bn ^ to, bb, br, bq, bk);
				if constexpr (Pt == ROOK)   return Board(wp, wn, wb, wr, wq, wk, bp ^ from, bn, bb, br ^ to, bq, bk);
				if constexpr (Pt == BISHOP) return Board(wp, wn, wb, wr, wq, wk, bp ^ from, bn, bb ^ to, br, bq, bk);
			}
		}
	}

	constexpr Piece piece_on(Square s) const
	{
		return (w_pieces & s) ? 
				 ( (wp & s) ? W_PAWN
			     : (wn & s) ? W_KNIGHT
				 : (wb & s) ? W_BISHOP
				 : (wr & s) ? W_ROOK
				 : (wq & s) ? W_QUEEN
				 : W_KING)
			 : (b_pieces & s) ?
			     ( (bp & s) ? B_PAWN
			     : (bn & s) ? B_KNIGHT
				 : (bb & s) ? B_BISHOP
				 : (br & s) ? B_ROOK
				 : (bq & s) ? B_QUEEN
				 : B_KING)
			 : NO_PIECE;
	}

	static constexpr Board DEFAULT() { return Board(FEN::STARTPOS); }

	std::string to_string() const;

	template<Color C> constexpr uint64_t hv_sliders_bb() const { if constexpr (C == WHITE) return wr | wq; else return br | bq; }
	template<Color C> constexpr uint64_t diag_sliders_bb() const { if constexpr (C == WHITE) return wb | wq; else return bb | bq; }

	template<Color C> constexpr uint64_t pawn_bb()   const { if constexpr (C == WHITE) return wp; else return bp; }
	template<Color C> constexpr uint64_t knight_bb() const { if constexpr (C == WHITE) return wn; else return bn; }
	template<Color C> constexpr uint64_t bishop_bb() const { if constexpr (C == WHITE) return wb; else return bb; }
	template<Color C> constexpr uint64_t rook_bb()   const { if constexpr (C == WHITE) return wr; else return br; }
	template<Color C> constexpr uint64_t queen_bb()  const { if constexpr (C == WHITE) return wq; else return bq; }
	template<Color C> constexpr uint64_t king_bb()   const { if constexpr (C == WHITE) return wk; else return bk; }

	template<Color C> constexpr Square king_sq() const { if constexpr (C == WHITE) return lsb(wk); else return lsb(bk); }

	template<Color C> constexpr uint64_t pieces_bb() const { if constexpr (C == WHITE) return w_pieces; else return b_pieces; }
	constexpr uint64_t empty() const { return ~pieces; }
};


#endif // POSITION_H_E76CAC7920F0
