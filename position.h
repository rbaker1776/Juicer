#ifndef POSITION_H_E76CAC7920F0
#define POSITION_H_E76CAC7920F0

#include <cassert>
#include <string>
#include <sstream>
#include <string_view>
#include "fen.h"
#include "movement.h"
#include "bitboard.h"
#include "types.h"
#include "juicer.h"


class Boardstate
{
public:
	const Color turn;
	const bool has_ep_pawn;
	const bool w_castle_ooo;
	const bool w_castle_oo;
	const bool b_castle_ooo;
	const bool b_castle_oo;

	consteval Boardstate(uint8_t pattern): 
		turn((pattern & 0b100000) ? BLACK : WHITE), has_ep_pawn(bool(pattern & 0b010000)),
		w_castle_ooo(bool(pattern & 0b001000)), w_castle_oo(bool(pattern & 0b000100)),
		b_castle_ooo(bool(pattern & 0b000010)), b_castle_oo(bool(pattern & 0b000001))
	{}

	consteval bool can_castle_queenside() const { return turn == WHITE ? w_castle_ooo : b_castle_ooo; }
	consteval bool can_castle_kingside()  const { return turn == WHITE ? w_castle_oo : b_castle_oo; }

	inline bool can_castle_queenside(uint64_t seen, uint64_t occupied, uint64_t rook) const __attribute__((const));
	inline bool can_castle_kingside(uint64_t seen, uint64_t occupied, uint64_t rook) const __attribute__((const));

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


struct Position
{
	const uint64_t wp, wn, wb, wr, wq, wk;
	const uint64_t bp, bn, bb, br, bq, bk;
	const uint64_t w_pieces, b_pieces;
	const uint64_t pieces;

	const Color turn;
	const uint64_t castling;

	const Square ep_target;
	const int rule_50;

	constexpr Position(
		uint64_t wp, uint64_t wn, uint64_t wb, uint64_t wr, uint64_t wq, uint64_t wk,
		uint64_t bp, uint64_t bn, uint64_t bb, uint64_t br, uint64_t bq, uint64_t bk,
		Color turn, uint64_t cr, Square ep, int r50
	):
		wp(wp), wn(wn), wb(wb), wr(wr), wq(wq), wk(wk),
		bp(bp), bn(bn), bb(bb), br(br), bq(bq), bk(bk),
		w_pieces(wp | wn | wb | wr | wq | wk),
		b_pieces(bp | bn | bb | br | bq | bk),
		pieces(w_pieces | b_pieces),
		turn(turn), castling(cr), ep_target(ep), rule_50(r50)
	{}

	constexpr Position(std::string_view fen): Position(
		FEN::bitboard(fen, W_PAWN), FEN::bitboard(fen, W_KNIGHT), FEN::bitboard(fen, W_BISHOP), FEN::bitboard(fen, W_ROOK), FEN::bitboard(fen, W_QUEEN), FEN::bitboard(fen, W_KING),
		FEN::bitboard(fen, B_PAWN), FEN::bitboard(fen, B_KNIGHT), FEN::bitboard(fen, B_BISHOP), FEN::bitboard(fen, B_ROOK), FEN::bitboard(fen, B_QUEEN), FEN::bitboard(fen, B_KING),
		FEN::turn(fen), FEN::castling_rights(fen), FEN::ep_target(fen), FEN::halfmoves(fen)
	) {}

	static consteval Position startpos() { return Position(FEN::STARTPOS); }

	inline Position make_move(const Move& m) const __attribute__((const));

	template<PieceType Pt>
	inline Position make_move(Square from, Square to) const __attribute__((const));

	template<PieceType Pt>
	inline Position make_promotion(Square from, Square to) const __attribute__((const));

	inline Piece piece_on(Square s) const __attribute__((const));

	template<Color C, PieceType Pt = ALL_PIECE_TYPES>
	force_inline uint64_t bitboard() const __attribute__((const));

	template<Color C, PieceType... Pts>
	force_inline uint64_t bitboards() const __attribute__((const)) { return (bitboard<C, Pts>() | ...); }

	template<Color C>
	inline Square king_sq() const __attribute__((const));

	inline uint8_t boardstate_pattern() const __attribute__((const));
}; // struct Position


inline bool Boardstate::can_castle_queenside(uint64_t seen, uint64_t occupied, uint64_t rook) const
{
	if (turn == WHITE && w_castle_ooo)
	{
		if (occupied & W_EMPTY_OOO || seen & W_UNSEEN_OOO)
			return false;
		if (rook & Castling::W_OOO)
			return true;
	}
	else if (turn == BLACK && b_castle_ooo)
	{
		if (occupied & B_EMPTY_OOO || seen & B_UNSEEN_OOO)
			return false;
		if (rook & Castling::B_OOO)
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
		if (rook & Castling::W_OO)
			return true;
	}
	else if (turn == BLACK && b_castle_oo)
	{
		if (occupied & B_EMPTY_OO || seen & B_UNSEEN_OO) 
			return false;
		if (rook & Castling::B_OO)
			return true;
	}
	return false;
}


inline Position Position::make_move(const Move& m) const
{
	switch (m.type)
	{
		case NORMAL:
			switch (m.piece)
			{
				case PAWN:   return make_move<PAWN>(m.from, m.to);
				case KNIGHT: return make_move<KNIGHT>(m.from, m.to);
				case BISHOP: return make_move<BISHOP>(m.from, m.to);
				case ROOK:   return make_move<ROOK>(m.from, m.to);
				case QUEEN:  return make_move<QUEEN>(m.from, m.to);
				case KING:   return make_move<KING>(m.from, m.to);
				default:
					std::cerr << "Invalid PieceType: " << m.piece << std::endl;
			}
		case CASTLING:
			switch (m.to)
			{
				case C1: return Position(wp, wn, wb, wr ^ (A1 | D1), wq, wk ^ (E1 | C1), bp, bn, bb, br, bq, bk, BLACK, castling & Castling::B_CASTLES, NO_SQUARE, 0);
				case G1: return Position(wp, wn, wb, wr ^ (H1 | F1), wq, wk ^ (E1 | G1), bp, bn, bb, br, bq, bk, WHITE, castling & Castling::B_CASTLES, NO_SQUARE, 0);
				case C8: return Position(wp, wn, wb, wr, wq, wk, bp, bn, bb, br ^ (A8 | D8), bq, bk ^ (E8 | C8), WHITE, castling & Castling::W_CASTLES, NO_SQUARE, 0);
				case G8: return Position(wp, wn, wb, wr, wq, wk, bp, bn, bb, br ^ (H8 | F8), bq, bk ^ (E8 | G8), WHITE, castling & Castling::W_CASTLES, NO_SQUARE, 0);
				default:
					std::cerr << "Invalid castling move: " << m.to << "->" << m.from << std::endl;
			}
		case PROMOTION:
			switch (m.piece)
			{
				case QUEEN:  return make_promotion<QUEEN>(m.from, m.to);
				case KNIGHT: return make_promotion<KNIGHT>(m.from, m.to);
				case ROOK:   return make_promotion<ROOK>(m.from, m.to);
				case BISHOP: return make_promotion<BISHOP>(m.from, m.to);
				default:
					std::cerr << "Invalid PieceType for promotion: " << m.piece << std::endl;
			}
		case EN_PASSANT:
			const uint64_t sqs = m.from | m.to;
			if (turn == WHITE)
				return Position(wp ^ sqs, wn, wb, wr, wq, wk, bp & ~square_to_bb(ep_target), bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, 0);
			else
				return Position(wp & ~square_to_bb(ep_target), wn, wb, wr, wq, wk, bp ^ sqs, bn, bb, br, bq, bk, WHITE, castling, NO_SQUARE, 0);
	}
}

template<PieceType Pt>
inline Position Position::make_move(Square from, Square to) const
{
	const uint64_t sqs = from | to;
	const bool is_capture = pieces & to;

	if (is_capture)
	{
		const uint64_t rem = ~square_to_bb(to);

		if (turn == WHITE)
		{
			if constexpr (Pt == PAWN)   return Position(wp ^ sqs, wn, wb, wr, wq, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == KNIGHT) return Position(wp, wn ^ sqs, wb, wr, wq, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == BISHOP) return Position(wp, wn, wb ^ sqs, wr, wq, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == ROOK)   return Position(wp, wn, wb, wr ^ sqs, wq, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & ~sqs, NO_SQUARE, 0);
			if constexpr (Pt == QUEEN)  return Position(wp, wn, wb, wr, wq ^ sqs, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == KING)   return Position(wp, wn, wb, wr, wq, wk ^ sqs, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem & Castling::B_CASTLES, NO_SQUARE, 0);
		}
		else // (turn == BLACK)
		{
			if constexpr (Pt == PAWN)   return Position(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ sqs, bn, bb, br, bq, bk, WHITE, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == KNIGHT) return Position(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn ^ sqs, bb, br, bq, bk, WHITE, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == BISHOP) return Position(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn, bb ^ sqs, br, bq, bk, WHITE, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == ROOK)   return Position(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn, bb, br ^ sqs, bq, bk, WHITE, castling & ~sqs, NO_SQUARE, 0);
			if constexpr (Pt == QUEEN)  return Position(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn, bb, br, bq ^ sqs, bk, WHITE, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == KING)   return Position(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn, bb, br, bq, bk ^ sqs, WHITE, castling & rem & Castling::W_CASTLES, NO_SQUARE, 0);
		}
	}
	else // (!is_capture)
	{
		if (turn == WHITE)
		{
			if constexpr (Pt == PAWN)
			{
				const bool ep = (SQUARE_DISTANCE[to][from] == 2 && bp & PAWN_ATTACKS[WHITE][from + pawn_step<WHITE>()]);
				return Position(wp ^ sqs, wn, wb, wr, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling, (ep ? to : NO_SQUARE), 0);
			}
			if constexpr (Pt == KNIGHT) return Position(wp, wn ^ sqs, wb, wr, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, 0);
			if constexpr (Pt == BISHOP) return Position(wp, wn, wb ^ sqs, wr, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, 0);
			if constexpr (Pt == ROOK)   return Position(wp, wn, wb, wr ^ sqs, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling & ~sqs, NO_SQUARE, 0);
			if constexpr (Pt == QUEEN)  return Position(wp, wn, wb, wr, wq ^ sqs, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, 0);
			if constexpr (Pt == KING)   return Position(wp, wn, wb, wr, wq, wk ^ sqs, bp, bn, bb, br, bq, bk, BLACK, castling & Castling::B_CASTLES, NO_SQUARE, 0);
		}
		else // (turn == BLACK)
		{
			if constexpr (Pt == PAWN)
			{
				const bool ep = (SQUARE_DISTANCE[to][from] == 2 && wp & PAWN_ATTACKS[BLACK][from + pawn_step<BLACK>()]);
				return Position(wp, wn, wb, wr, wq, wk, bp ^ sqs, bn, bb, br, bq, bk, WHITE, castling, (ep ? to : NO_SQUARE), 0);
			}
			if constexpr (Pt == KNIGHT) return Position(wp, wn, wb, wr, wq, wk, bp, bn ^ sqs, bb, br, bq, bk, WHITE, castling, NO_SQUARE, 0);
			if constexpr (Pt == BISHOP) return Position(wp, wn, wb, wr, wq, wk, bp, bn, bb ^ sqs, br, bq, bk, WHITE, castling, NO_SQUARE, 0);
			if constexpr (Pt == ROOK)   return Position(wp, wn, wb, wr, wq, wk, bp, bn, bb, br ^ sqs, bq, bk, WHITE, castling & ~sqs, NO_SQUARE, 0);
			if constexpr (Pt == QUEEN)  return Position(wp, wn, wb, wr, wq, wk, bp, bn, bb, br, bq ^ sqs, bk, WHITE, castling, NO_SQUARE, 0);
			if constexpr (Pt == KING)   return Position(wp, wn, wb, wr, wq, wk, bp, bn, bb, br, bq, bk ^ sqs, WHITE, castling & Castling::W_CASTLES, NO_SQUARE, 0);
		}
	}
}

template<PieceType Pt>
inline Position Position::make_promotion(Square from, Square to) const
{
	const bool is_capture = pieces & to;

	if (is_capture)
	{
		const uint64_t rem = ~square_to_bb(to);

		if (turn == WHITE)
		{
			if constexpr (Pt == QUEEN)  return Position(wp ^ from, wn, wb, wr, wq ^ to, wk, bp, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == KNIGHT) return Position(wp ^ from, wn ^ to, wb, wr, wq, wk, bp, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == ROOK)   return Position(wp ^ from, wn, wb, wr ^ to, wq, wk, bp, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == BISHOP) return Position(wp ^ from, wn, wb ^ to, wr, wq, wk, bp, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0);
		}
		else // (turn == BLACK)
		{
			if constexpr (Pt == QUEEN)  return Position(wp, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ from, bn, bb, br, bq ^ to, wk, WHITE, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == KNIGHT) return Position(wp, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ from, bn ^ to, bb, br, bq, wk, WHITE, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == ROOK)   return Position(wp, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ from, bn, bb, br ^ to, bq, wk, WHITE, castling & rem, NO_SQUARE, 0);
			if constexpr (Pt == BISHOP) return Position(wp, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ from, bn, bb ^ to, br, bq, wk, WHITE, castling & rem, NO_SQUARE, 0);
		}
	}
	else // (!is_capture)
	{
		if (turn == WHITE)
		{
			if constexpr (Pt == QUEEN)  return Position(wp ^ from, wn, wb, wr, wq ^ to, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, 0);
			if constexpr (Pt == KNIGHT) return Position(wp ^ from, wn ^ to, wb, wr, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, 0);
			if constexpr (Pt == ROOK)   return Position(wp ^ from, wn, wb, wr ^ to, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, 0);
			if constexpr (Pt == BISHOP) return Position(wp ^ from, wn, wb ^ to, wr, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, 0);
		}
		else // (turn == BLACK)
		{
			if constexpr (Pt == QUEEN)  return Position(wp, wn, wb, wr, wq, wk, bp ^ from, bn, bb, br, bq ^ to, wk, WHITE, castling, NO_SQUARE, 0);
			if constexpr (Pt == KNIGHT) return Position(wp, wn, wb, wr, wq, wk, bp ^ from, bn ^ to, bb, br, bq, wk, WHITE, castling, NO_SQUARE, 0);
			if constexpr (Pt == ROOK)   return Position(wp, wn, wb, wr, wq, wk, bp ^ from, bn, bb, br ^ to, bq, wk, WHITE, castling, NO_SQUARE, 0);
			if constexpr (Pt == BISHOP) return Position(wp, wn, wb, wr, wq, wk, bp ^ from, bn, bb ^ to, br, bq, wk, WHITE, castling, NO_SQUARE, 0);
		}

	}
}

inline Piece Position::piece_on(Square s) const
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
force_inline uint64_t Position::bitboard() const
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
	static_assert(C == WHITE || C == BLACK);
	static_assert(Pt >= PAWN && Pt <= ALL_PIECE_TYPES);
}

template<Color C>
inline Square Position::king_sq() const
{
	if constexpr (C == WHITE)
		return lsb(wk);
	else
		return lsb(bk);
}

inline uint8_t Position::boardstate_pattern() const
{
	return ((turn == BLACK) << 5)
		 | ((ep_target != NO_SQUARE) << 4)
		 | (bool(castling & Castling::W_OOO) << 3)
		 | (bool(castling & Castling::W_OO)  << 2)
		 | (bool(castling & Castling::B_OOO) << 1)
		 |  bool(castling & Castling::B_OO);
}


#endif // POSITION_H_E76CAC7920F0
