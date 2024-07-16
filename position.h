#ifndef POSITION_H_E76CAC7920F0
#define POSITION_H_E76CAC7920F0

#include <cassert>
#include <string>
#include <sstream>
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

	consteval Gamestate(uint8_t pattern): 
		turn((pattern & 0b100000) ? BLACK : WHITE), has_ep_pawn(bool(pattern & 0b010000)),
		w_castle_ooo(bool(pattern & WHITE_OOO)), w_castle_oo(bool(pattern & WHITE_OO)),
		b_castle_ooo(bool(pattern & BLACK_OOO)), b_castle_oo(bool(pattern & BLACK_OO))
	{}

	static consteval Gamestate startpos() { return Gamestate(WHITE, false, true, true, true, true); }

	inline uint8_t pattern() const;
	
	consteval bool can_castle_queenside() const { return turn == WHITE ? w_castle_ooo : b_castle_ooo; }
	consteval bool can_castle_kingside()  const { return turn == WHITE ? w_castle_oo : b_castle_oo; }

	inline bool can_castle_queenside(uint64_t seen, uint64_t occupied, uint64_t rook) const;
	inline bool can_castle_kingside(uint64_t seen, uint64_t occupied, uint64_t rook) const;
	
	inline Gamestate pawn_push()  const { return Gamestate(~turn, true, w_castle_ooo, w_castle_oo, b_castle_ooo, b_castle_oo); }
	inline Gamestate quiet_move() const { return Gamestate(~turn, false, w_castle_ooo, w_castle_oo, b_castle_ooo, b_castle_oo); }
	inline Gamestate king_move()  const;
	inline Gamestate rook_move_queenside() const;
	inline Gamestate rook_move_kingside() const;

private:
	static constexpr uint64_t W_EMPTY_OOO = B1 | C1 | D1;
	static constexpr uint64_t B_EMPTY_OOO = B8 | C8 | D8;
	static constexpr uint64_t W_EMPTY_OO = F1 | G1;
	static constexpr uint64_t B_EMPTY_OO = F8 | G8;

	static constexpr uint64_t W_UNSEEN_OOO = E1 | D1 | C1;
	static constexpr uint64_t B_UNSEEN_OOO = E8 | D8 | C8;
	static constexpr uint64_t W_UNSEEN_OO = E1 | F1 | G1;
	static constexpr uint64_t B_UNSEEN_OO = E8 | F8 | G8;
}; // struct Gamestate


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

	template<FEN::Field F>
	static constexpr bool info(std::string_view fen);

	static constexpr Square ep_square(std::string_view fen);
	static constexpr uint64_t bitboard(std::string_view fen, Piece pc);
} // namespace FEN


struct Board
{
	const uint64_t wp, wn, wb, wr, wq, wk;
	const uint64_t bp, bn, bb, br, bq, bk;
	
	const uint64_t w_pieces, b_pieces;
	const uint64_t pieces;

	constexpr Board():
		wp(RANK2_BB), wn(B1 | G1), wb(C1 | F1), wr(A1 | H1), wq(square_to_bb(D1)), wk(square_to_bb(E1)),
		bp(RANK7_BB), bn(B8 | G8), bb(C8 | F8), br(A8 | H8), bq(square_to_bb(D8)), bk(square_to_bb(E8)),
		w_pieces(wp | wn | wb | wr | wq | wk),
		b_pieces(bp | bn | bb | br | bq | bk),
		pieces(w_pieces | b_pieces)
	{}

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

	static constexpr Board startpos() { return Board(FEN::STARTPOS); }

	template<Color C, PieceType Pt, bool IsCapture>
	inline Board move(Square from, Square to) const;
	
	template<Castling Cr>
	inline Board castles() const;
	
	template<Color C>
	inline Board castles(uint64_t kingswitch, uint64_t rookswitch) const;
	
	template<Color C>
	inline Board enpassant(Square from, Square to) const;
	
	template<Color C, PieceType Pt, bool IsCapture>
	inline Board promote(Square from, Square to) const;
	
	inline Piece piece_on(Square s) const;
	
	std::string to_string() const;

	template<Color C, PieceType Pt = ALL_PIECE_TYPES>
	inline uint64_t bitboard() const;

	template<Color C, PieceType... Pts>
	inline uint64_t bitboards() const { return (bitboard<C, Pts>() | ...); }

	template<Color C> inline Square king_sq() const { if constexpr (C == WHITE) return lsb(wk); else return lsb(bk); }

	inline uint64_t empty() const { return ~pieces; }
}; // struct Board


inline uint8_t Gamestate::pattern() const
{
	return ((turn == BLACK) << 5) | (has_ep_pawn << 4)
	        | (w_castle_ooo << 3) | (w_castle_oo << 2)
			| (b_castle_ooo << 1) | (b_castle_oo);
}

inline bool Gamestate::can_castle_queenside(uint64_t seen, uint64_t occupied, uint64_t rook) const
{
	if (turn == WHITE && w_castle_ooo)
	{
		if (occupied & W_EMPTY_OOO || seen & W_UNSEEN_OOO)
			return false;
		if (rook & A1)
			return true;
	}
	else if (turn == BLACK && b_castle_ooo)
	{
		if (occupied & B_EMPTY_OOO || seen & B_UNSEEN_OOO)
			return false;
		if (rook & A8)
			return true;
	}
	return false;
}

inline bool Gamestate::can_castle_kingside(uint64_t seen, uint64_t occupied, uint64_t rook) const
{
	if (turn == WHITE && w_castle_oo)
	{
		if (occupied & W_EMPTY_OO || seen & W_UNSEEN_OO) 
			return false;
		if (rook & H1) 
			return true;
	}
	else if (turn == BLACK && b_castle_oo)
	{
		if (occupied & B_EMPTY_OO || seen & B_UNSEEN_OO) 
			return false;
		if (rook & H8) 
			return true;
	}
	return false;
}

inline Gamestate Gamestate::king_move() const
{
	if (turn == WHITE)
		return Gamestate(BLACK, false, false, false, b_castle_ooo, b_castle_oo);
	else
		return Gamestate(WHITE, false, w_castle_ooo, w_castle_oo, false, false);
}

inline Gamestate Gamestate::rook_move_queenside() const
{
	if (turn == WHITE)
		return Gamestate(BLACK, false, false, w_castle_oo, b_castle_ooo, b_castle_oo);
	else
		return Gamestate(WHITE, false, w_castle_ooo, w_castle_oo, false, b_castle_oo);
}

inline Gamestate Gamestate::rook_move_kingside() const
{
	if (turn == WHITE)
		return Gamestate(BLACK, false, w_castle_ooo, false, b_castle_ooo, b_castle_oo);
	else
		return Gamestate(WHITE, false, w_castle_ooo, w_castle_oo, b_castle_ooo, false);
}


static constexpr Square FEN::ep_square(std::string_view fen)
{
	const char* token = &fen[0];

	for (int count = 0; count < 3; count += isspace(*token++)) {}

	char rank, file;
	if (((file = *token++) && (file >= 'a' && file <= 'h')) && ((rank = *token++) && (rank >= '1' && rank <= '8')))
		return make_square(File(file - 'a'), Rank(rank - '1'));

	return NO_SQUARE;
}

template<FEN::Field F>
static constexpr bool FEN::info(std::string_view fen)
{
	const char* token = &fen[0];

	while (!isspace(*token++)) {}

	Color turn = *token++ == 'w' ? WHITE : BLACK;

	if constexpr (F == FEN::Field::TURN) { return turn == BLACK; }

	#if (DEBUG)
		assert(isspace(*token));
	#endif

	++token;
	while (!isspace(*token))
	{
		if constexpr (F == FEN::W_CASTLE_OOO)
			if (*token == 'Q')
				return true;
		if constexpr (F == FEN::W_CASTLE_OO) 
			if (*token == 'K')
				return true;
		if constexpr (F == FEN::B_CASTLE_OOO)
			if (*token == 'q')
				return true;
		if constexpr (F == FEN::B_CASTLE_OO) 
			if (*token == 'k')
				return true;
		++token;
	}

	if constexpr (F == FEN::Field::W_CASTLE_OO || F == FEN::Field::W_CASTLE_OOO || F == FEN::Field::B_CASTLE_OO || F == FEN::Field::B_CASTLE_OOO)
		return false;

	++token;
	char rank, file;
	if (((file = *token++) && (file >= 'a' && file <= 'h')) && ((rank = *token++) && (rank >= '1' && rank <= '8')))
		if constexpr (F == FEN::Field::HAS_EP)
			return true;

	if constexpr (F == FEN::Field::HAS_EP)
		return false;
}

static constexpr uint64_t FEN::bitboard(std::string_view fen, Piece pc)
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


template<Color C, PieceType Pt, bool IsCapture>
inline Board Board::move(Square from, Square to) const
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
				if (bk & sqs)
				{
					std::cout << to_string() << std::endl;
				}

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

template<Castling Cr>
inline Board Board::castles() const
{
	if constexpr (Cr == WHITE_OOO)
		return Board(wp, wn, wb, wr ^ (A1 | D1), wq, wk ^ (E1 | C1), bp, bn, bb, br, bq, bk);
	if constexpr (Cr == WHITE_OO)
		return Board(wp, wn, wb, wr ^ (H1 | F1), wq, wk ^ (E1 | G1), bp, bn, bb, br, bq, bk);
	if constexpr (Cr == BLACK_OOO)
		return Board(wp, wn, wb, wr, wq, wk, bp, bn, bb, br ^ (A8 | D8), bq, bk ^ (E8 | C8));
	if constexpr (Cr == BLACK_OO)
		return Board(wp, wn, wb, wr, wq, wk, bp, bn, bb, br ^ (H8 | F8), bq, bk ^ (E8 | G8));

}

template<Color C>
inline Board Board::enpassant(Square from, Square to) const
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
inline Board Board::promote(Square from, Square to) const
{
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
				assert(RANK1_BB & to);
			#endif
			if constexpr (Pt == QUEEN)  return Board(wp, wn, wb, wr, wq, wk, bp ^ from, bn, bb, br, bq ^ to, bk);
			if constexpr (Pt == KNIGHT) return Board(wp, wn, wb, wr, wq, wk, bp ^ from, bn ^ to, bb, br, bq, bk);
			if constexpr (Pt == ROOK)   return Board(wp, wn, wb, wr, wq, wk, bp ^ from, bn, bb, br ^ to, bq, bk);
			if constexpr (Pt == BISHOP) return Board(wp, wn, wb, wr, wq, wk, bp ^ from, bn, bb ^ to, br, bq, bk);
		}
	}
}

inline Piece Board::piece_on(Square s) const
{
	if (w_pieces & s)
	{
		if (wp & s) return W_PAWN;
		if (wn & s) return W_KNIGHT;
		if (wb & s) return W_BISHOP;
		if (wr & s) return W_ROOK;
		if (wq & s) return W_QUEEN;
		return W_KING;
	}
	else if (b_pieces & s)
	{
		if (bp & s) return B_PAWN;
		if (bn & s) return B_KNIGHT;
		if (bb & s) return B_BISHOP;
		if (br & s) return B_ROOK;
		if (bq & s) return B_QUEEN;
		return B_KING;
	}
	else
		return NO_PIECE;
}

template<Color C, PieceType Pt>
inline uint64_t Board::bitboard() const
{
	if constexpr (C == WHITE)
	{
		if constexpr (Pt == PAWN)   return wp;
		if constexpr (Pt == KNIGHT) return wn;
		if constexpr (Pt == BISHOP) return wb;
		if constexpr (Pt == ROOK)   return wr;
		if constexpr (Pt == QUEEN)  return wq;
		if constexpr (Pt == KING)   return wk;
		if constexpr (Pt == ALL_PIECE_TYPES)
			return w_pieces;
	}
	else
	{
		if constexpr (Pt == PAWN)   return bp;
		if constexpr (Pt == KNIGHT) return bn;
		if constexpr (Pt == BISHOP) return bb;
		if constexpr (Pt == ROOK)   return br;
		if constexpr (Pt == QUEEN)  return bq;
		if constexpr (Pt == KING)   return bk;
		if constexpr (Pt == ALL_PIECE_TYPES)
			return b_pieces;
	}
}

std::string Board::to_string() const
{
	std::ostringstream ss;
	ss << "+---+---+---+---+---+---+---+---+\n";

	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			ss << '|' << ' ' << FEN::PIECES[this->piece_on(make_square(f, r))] << ' ';
		}
		ss << '|' << ' ' << std::to_string(r + 1) << '\n';
		ss << "+---+---+---+---+---+---+---+---+\n";
	}

	ss << "  a   b   c   d   e   f   g   h";
	return ss.str();
}


#endif // POSITION_H_E76CAC7920F0
