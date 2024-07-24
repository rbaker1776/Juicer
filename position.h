#ifndef POSITION_H_E76CAC7920F0
#define POSITION_H_E76CAC7920F0

#include <cassert>
#include <string>
#include <sstream>
#include <string_view>
#include "movement.h"
#include "bitboard.h"
#include "types.h"
#include "juicer.h"


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

	static constexpr Square ep_target(std::string_view fen);
	static constexpr uint64_t bitboard(std::string_view fen, Piece pc);
	static constexpr int halfmoves(std::string_view fen);
} // namespace FEN


class Boardstate
{
public:
	const Color turn;
	const bool has_ep_pawn;
	const bool w_castle_ooo;
	const bool w_castle_oo;
	const bool b_castle_ooo;
	const bool b_castle_oo;

	constexpr Boardstate(Color turn, bool ep, bool w_ooo, bool w_oo, bool b_ooo, bool b_oo):
		turn(turn), has_ep_pawn(ep), w_castle_ooo(w_ooo), w_castle_oo(w_oo), b_castle_ooo(b_ooo), b_castle_oo(b_oo)
	{}

	consteval Boardstate(uint8_t pattern): 
		turn((pattern & 0b100000) ? BLACK : WHITE), has_ep_pawn(bool(pattern & 0b010000)),
		w_castle_ooo(bool(pattern & WHITE_OOO)), w_castle_oo(bool(pattern & WHITE_OO)),
		b_castle_ooo(bool(pattern & BLACK_OOO)), b_castle_oo(bool(pattern & BLACK_OO))
	{}

	constexpr Boardstate(std::string_view fen):
		turn(FEN::info<FEN::TURN>(fen) ? BLACK : WHITE), has_ep_pawn(FEN::info<FEN::HAS_EP>(fen)),
		w_castle_ooo(FEN::info<FEN::W_CASTLE_OOO>(fen)), w_castle_oo(FEN::info<FEN::W_CASTLE_OO>(fen)),
		b_castle_ooo(FEN::info<FEN::B_CASTLE_OOO>(fen)), b_castle_oo(FEN::info<FEN::B_CASTLE_OO>(fen))
	{}

	static consteval Boardstate startpos() { return Boardstate(WHITE, false, true, true, true, true); }

	inline uint8_t pattern() const;
	
	consteval bool can_castle_queenside() const { return turn == WHITE ? w_castle_ooo : b_castle_ooo; }
	consteval bool can_castle_kingside()  const { return turn == WHITE ? w_castle_oo : b_castle_oo; }

	inline bool can_castle_queenside(uint64_t seen, uint64_t occupied, uint64_t rook) const;
	inline bool can_castle_kingside(uint64_t seen, uint64_t occupied, uint64_t rook) const;
	
	inline Boardstate pawn_push()  const { return Boardstate(~turn, true, w_castle_ooo, w_castle_oo, b_castle_ooo, b_castle_oo); }
	inline Boardstate quiet_move() const { return Boardstate(~turn, false, w_castle_ooo, w_castle_oo, b_castle_ooo, b_castle_oo); }
	inline Boardstate king_move()  const;
	inline Boardstate rook_move_queenside() const;
	inline Boardstate rook_move_kingside() const;

private:
	static constexpr uint64_t W_EMPTY_OOO = B1 | C1 | D1;
	static constexpr uint64_t B_EMPTY_OOO = B8 | C8 | D8;
	static constexpr uint64_t W_EMPTY_OO = F1 | G1;
	static constexpr uint64_t B_EMPTY_OO = F8 | G8;

	static constexpr uint64_t W_UNSEEN_OOO = E1 | D1 | C1;
	static constexpr uint64_t B_UNSEEN_OOO = E8 | D8 | C8;
	static constexpr uint64_t W_UNSEEN_OO = E1 | F1 | G1;
	static constexpr uint64_t B_UNSEEN_OO = E8 | F8 | G8;
}; // struct Boardstate


struct Board
{
	const uint64_t wp, wn, wb, wr, wq, wk;
	const uint64_t bp, bn, bb, br, bq, bk;
	
	const uint64_t w_pieces, b_pieces;
	const uint64_t pieces;

	constexpr Board():
		wp(Bitboard::RANK2), wn(B1 | G1), wb(C1 | F1), wr(A1 | H1), wq(square_to_bb(D1)), wk(square_to_bb(E1)),
		bp(Bitboard::RANK7), bn(B8 | G8), bb(C8 | F8), br(A8 | H8), bq(square_to_bb(D8)), bk(square_to_bb(E8)),
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

	static consteval Board startpos() { return Board(FEN::STARTPOS); }

	template<Color C, PieceType Pt, bool IsCapture>
	inline Board move(Square from, Square to) const;

	template<PieceType Pt, bool IsCapture>
	inline Board move(Square from, Square to) const;
	
	template<Castling Cr>
	inline Board castles() const;
	
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


struct Gamestate
{
	const Square ep_target;
	const int rule_50;

	constexpr Gamestate(Square ep, int r50): ep_target(ep), rule_50(r50) {}
	
	constexpr Gamestate(std::string_view fen): 
		ep_target(FEN::ep_target(fen)), rule_50(FEN::halfmoves(fen))
	{}

	static consteval Gamestate startpos() { return Gamestate(FEN::STARTPOS); }

	inline Gamestate pawn_step() const { return Gamestate(NO_SQUARE, 0); }
	inline Gamestate pawn_push(Square to) const { return Gamestate(to, 0); }
	inline Gamestate capture() const { return Gamestate(NO_SQUARE, 0); }
	inline Gamestate quiet_move() const { return Gamestate(NO_SQUARE, rule_50+1); }
}; // struct Gamestate


struct Position
{
	const Board board;
	const Boardstate boardstate;
	const Gamestate gamestate;

	constexpr Position(const Board& board, const Boardstate& bs, const Gamestate& gs):
		board(board), boardstate(bs), gamestate(gs)
	{}

	constexpr Position(std::string_view fen): board(fen), boardstate(fen), gamestate(fen) {}

	static constexpr Position startpos() { return Position(Board(), Boardstate::startpos(), Gamestate(NO_SQUARE, 0)); }

	inline Position make_move(const Move& m) const;

	inline Color side_to_move() const { return boardstate.turn; }
}; // struct Position


static constexpr Square FEN::ep_target(std::string_view fen)
{
	const char* token = &fen[0];

	while (!(*token++ == ' ')) {}

	Color turn = *token++ == 'w' ? WHITE : BLACK;

	++token;
	while (!(*token++ == ' ')) {}

	char rank, file;
	if (((file = *token++) && (file >= 'a' && file <= 'h')) && ((rank = *token++) && (rank >= '1' && rank <= '8')))
		return make_square(File(file - 'a'), Rank(rank - '1')) + (turn == WHITE ? Direction::S : Direction::N);

	return NO_SQUARE;
}

template<FEN::Field F>
static constexpr bool FEN::info(std::string_view fen)
{
	const char* token = &fen[0];

	while (!(*token++ == ' ')) {}

	Color turn = *token++ == 'w' ? WHITE : BLACK;

	if constexpr (F == FEN::Field::TURN) { return turn == BLACK; }

	#if (DEBUG)
		assert(*token == ' ');
	#endif

	++token;
	while (!(*token == ' '))
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

	for (Square s = A8; !(*token == ' '); ++token)
	{
		if (*token >= '1' && *token <= '8')
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

static constexpr int FEN::halfmoves(std::string_view fen)
{
	const char* token = &fen[0];
	int rule_50 = 0;

	for (int i = 0; i < 4; i += (*token++ == ' ')) {}

	while (*token++ != ' ')
	{
		rule_50 *= 10;
		rule_50 += *token - '0';
	}

	return rule_50;
}


inline uint8_t Boardstate::pattern() const
{
	return ((turn == BLACK) << 5) | (has_ep_pawn << 4)
	        | (w_castle_ooo << 3) | (w_castle_oo << 2)
			| (b_castle_ooo << 1) | (b_castle_oo);
}

inline bool Boardstate::can_castle_queenside(uint64_t seen, uint64_t occupied, uint64_t rook) const
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

inline bool Boardstate::can_castle_kingside(uint64_t seen, uint64_t occupied, uint64_t rook) const
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

inline Boardstate Boardstate::king_move() const
{
	if (turn == WHITE)
		return Boardstate(BLACK, false, false, false, b_castle_ooo, b_castle_oo);
	else
		return Boardstate(WHITE, false, w_castle_ooo, w_castle_oo, false, false);
}

inline Boardstate Boardstate::rook_move_queenside() const
{
	if (turn == WHITE)
		return Boardstate(BLACK, false, false, w_castle_oo, b_castle_ooo, b_castle_oo);
	else
		return Boardstate(WHITE, false, w_castle_ooo, w_castle_oo, false, b_castle_oo);
}

inline Boardstate Boardstate::rook_move_kingside() const
{
	if (turn == WHITE)
		return Boardstate(BLACK, false, w_castle_ooo, false, b_castle_ooo, b_castle_oo);
	else
		return Boardstate(WHITE, false, w_castle_ooo, w_castle_oo, b_castle_ooo, false);
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

template<PieceType Pt, bool IsCapture>
inline Board Board::move(Square from, Square to) const
{
	if (w_pieces & from)
		return move<WHITE, Pt, IsCapture>(from, to);
	else
		return move<BLACK, Pt, IsCapture>(from, to);
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
				assert(Bitboard::RANK8 & to);
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
				assert(Bitboard::RANK1 & to);
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
				assert(Bitboard::RANK8 & to);
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
				assert(Bitboard::RANK1 & to);
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


inline Position Position::make_move(const Move& m) const
{
	switch (m.type)
	{
		case NORMAL:
			switch (m.piece)
			{
				case PAWN:
					if (board.pieces & m.to) // capture
					{
						return Position(board.move<PAWN, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
					}
					else // no capture
					{
						if (SQUARE_DISTANCE[m.from][m.to] == 2) // pawn push
							return Position(board.move<PAWN, false>(m.from, m.to), boardstate.pawn_push(), gamestate.pawn_push(m.to));
						else
							return Position(board.move<PAWN, false>(m.from, m.to), boardstate.quiet_move(), gamestate.pawn_step());
					}

				case KNIGHT:
					if (board.pieces & m.to) // capture
						return Position(board.move<KNIGHT, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
					else // no capture
						return Position(board.move<KNIGHT, false>(m.from, m.to), boardstate.quiet_move(), gamestate.quiet_move());

				case BISHOP:
					if (board.pieces & m.to) // capture
						return Position(board.move<BISHOP, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
					else // no capture
						return Position(board.move<BISHOP, false>(m.from, m.to), boardstate.quiet_move(), gamestate.quiet_move());

				case ROOK:
					if (board.pieces & m.to) // capture
					{
						switch (m.from)
						{
							case A1: case A8: return Position(board.move<ROOK, true>(m.from, m.to), boardstate.rook_move_queenside(), gamestate.capture());
							case H1: case H8: return Position(board.move<ROOK, true>(m.from, m.to), boardstate.rook_move_kingside(),  gamestate.capture());
							default: return Position(board.move<ROOK, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
						}
					}
					else // no capture
					{
						switch (m.from)
						{
							case A1: case A8: return Position(board.move<ROOK, false>(m.from, m.to), boardstate.rook_move_queenside(), gamestate.quiet_move());
							case H1: case H8: return Position(board.move<ROOK, false>(m.from, m.to), boardstate.rook_move_kingside(),  gamestate.quiet_move());
							default: return Position(board.move<ROOK, false>(m.from, m.to), boardstate.quiet_move(), gamestate.quiet_move());
						}
					}

				case QUEEN:
					if (board.pieces & m.to) // capture
						return Position(board.move<QUEEN, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
					else // no capture
						return Position(board.move<QUEEN, false>(m.from, m.to), boardstate.quiet_move(), gamestate.quiet_move());

				case KING:
					if (board.pieces & m.to) // capture
						return Position(board.move<KING, true>(m.from, m.to), boardstate.king_move(), gamestate.capture());
					else // no capture
						return Position(board.move<KING, false>(m.from, m.to), boardstate.king_move(), gamestate.quiet_move());
				default:
					std::cerr << "Error, invalid piece passed into Position::make_move: " << m.piece << std::endl;
			}

		case CASTLING:
			switch (m.to)
			{
				case C1: return Position(board.castles<WHITE_OOO>(), boardstate.king_move(), gamestate.quiet_move());
				case G1: return Position(board.castles<WHITE_OO>(),  boardstate.king_move(), gamestate.quiet_move());
				case C8: return Position(board.castles<BLACK_OOO>(), boardstate.king_move(), gamestate.quiet_move());
				case G8: return Position(board.castles<BLACK_OO>(),  boardstate.king_move(), gamestate.quiet_move());
				default:
					std::cerr << "Error, invalid square for castling: " << m.to << std::endl;
			}

		case PROMOTION:
			switch (m.piece)
			{
				case QUEEN:
					if (board.pieces & m.to) // capture
					{
						if (boardstate.turn == WHITE)
							return Position(board.promote<WHITE, QUEEN, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
						else
							return Position(board.promote<BLACK, QUEEN, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
					}
					else // no capture
					{
						if (boardstate.turn == WHITE)
							return Position(board.promote<WHITE, QUEEN, false>(m.from, m.to), boardstate.quiet_move(), gamestate.quiet_move());
						else
							return Position(board.promote<BLACK, QUEEN, false>(m.from, m.to), boardstate.quiet_move(), gamestate.quiet_move());
					}

				case KNIGHT:
					if (board.pieces & m.to) // capture
					{
						if (boardstate.turn == WHITE)
							return Position(board.promote<WHITE, KNIGHT, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
						else
							return Position(board.promote<BLACK, KNIGHT, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
					}
					else // no capture
					{
						if (boardstate.turn == WHITE)
							return Position(board.promote<WHITE, KNIGHT, false>(m.from, m.to), boardstate.quiet_move(), gamestate.quiet_move());
						else
							return Position(board.promote<BLACK, KNIGHT, false>(m.from, m.to), boardstate.quiet_move(), gamestate.quiet_move());
					}

				case ROOK:
					if (board.pieces & m.to) // capture
					{
						if (boardstate.turn == WHITE)
							return Position(board.promote<WHITE, ROOK, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
						else
							return Position(board.promote<BLACK, ROOK, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
					}
					else // no capture
					{
						if (boardstate.turn == WHITE)
							return Position(board.promote<WHITE, ROOK, false>(m.from, m.to), boardstate.quiet_move(), gamestate.quiet_move());
						else
							return Position(board.promote<BLACK, ROOK, false>(m.from, m.to), boardstate.quiet_move(), gamestate.quiet_move());
					}

				case BISHOP:
					if (board.pieces & m.to) // capture
					{
						if (boardstate.turn == WHITE)
							return Position(board.promote<WHITE, BISHOP, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
						else
							return Position(board.promote<BLACK, BISHOP, true>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
					}
					else // no capture
					{
						if (boardstate.turn == WHITE)
							return Position(board.promote<WHITE, BISHOP, false>(m.from, m.to), boardstate.quiet_move(), gamestate.quiet_move());
						else
							return Position(board.promote<BLACK, BISHOP, false>(m.from, m.to), boardstate.quiet_move(), gamestate.quiet_move());
					}
				
				default:
					std::cerr << "Error, invalid PieceType for promotion: " << m.piece << std::endl;
			}

		case EN_PASSANT:
			if (boardstate.turn == WHITE)
				return Position(board.enpassant<WHITE>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
			else
				return Position(board.enpassant<BLACK>(m.from, m.to), boardstate.quiet_move(), gamestate.capture());
	}
}


#endif // POSITION_H_E76CAC7920F0
