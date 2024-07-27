#ifndef MOVEGEN_H_6A20B59292CA
#define MOVEGEN_H_6A20B59292CA

#include <vector>
#include <iostream>
#include "position.h"
#include "movement.h"
#include "bitboard.h"
#include "types.h"
#include "juicer.h"


enum GenType: int
{
	LEGAL = 0,
	CAPTURES = 1,
	CHECKS = 2,
};

template<GenType Gt>
force_inline Move* enumerate(const Position& restrict pos, Move* moves);

template<GenType Gt>
struct MoveList
{
public:
	explicit MoveList(const Position& restrict pos): last(enumerate<Gt>(pos, moves)) {}

	const Move* begin() const { return moves; }
	const Move* end() const { return last; }
	size_t size() const { return last - moves; }

private:
	Move moves[218];
	Move* last;
}; // struct MoveList


// holds and updates variables used for move generation
namespace Movegen
{
	static constinit Square ep_target {NO_SQUARE};

	static constinit uint64_t rook_pins;
	static constinit uint64_t bishop_pins;

	static constinit uint64_t checkmask;
	static constinit uint64_t kingban;

	force_inline void register_slider_check(Square ksq, Square ssq);

	template<Color Us, bool EnPassant>
	force_inline void register_bishop_pin(const Position& restrict pos, Square ksq, Square bsq);

	template<Color Us>
	force_inline void register_rook_pin(const Position& restrict pos, Square ksq, Square rsq);

	template<Color Us>
	force_inline void register_ep_pin(const Position& restrict pos, Square ksq);
		
	template<GenType Gt, Color Us, bool EnPassant>
	force_inline uint64_t king_attacks(const Position& restrict pos);

	template<bool IsCheck>
	force_inline uint64_t const_checkmask(uint64_t checkmask);

	template<Color C, GenType Gt>
	force_inline uint64_t moveable_sqs(const Position& restrict pos, uint64_t checkmask);
} // namespace Movegen


template<GenType Gt, Boardstate State, bool IsCheck>
force_inline Move* enumerate(const Position& restrict pos, uint64_t king_atk, Move* moves)
{
	constexpr Color Us = State.turn;
	constexpr Color Them = ~Us;

	const uint64_t rook_pins = Movegen::rook_pins;
	const uint64_t bishop_pins = Movegen::bishop_pins;
	const uint64_t checkmask = Movegen::const_checkmask<IsCheck>(Movegen::checkmask);
	const uint64_t moveable_sqs = Movegen::moveable_sqs<Us, Gt>(pos, checkmask);

	const Square ksq = pos.king_sq<Us>();

	{
		while (king_atk)
			*moves++ = Move(NORMAL, ksq, pop_lsb(king_atk), KING);

		if constexpr (!IsCheck && State.can_castle_queenside())
			if (State.can_castle_queenside(Movegen::kingban, pos.pieces, pos.bitboard<Us, ROOK>()))
				*moves++ = Move(CASTLING, ksq, ksq + 2 * Direction::W, KING);

		if constexpr (!IsCheck && State.can_castle_kingside())
			if (State.can_castle_kingside(Movegen::kingban, pos.pieces, pos.bitboard<Us, ROOK>()))
				*moves++ = Move(CASTLING, ksq, ksq + 2 * Direction::E, KING);
	}

	if constexpr (Gt == LEGAL)
	{
		const uint64_t vertical_pawns = pos.bitboard<Us, PAWN>() & ~bishop_pins;
		const uint64_t diagonal_pawns = pos.bitboard<Us, PAWN>() & ~rook_pins;

		uint64_t w_atk_pawns = diagonal_pawns & pawn_atk_east_bb<Them>(pos.bitboard<Them>() & checkmask);
		uint64_t e_atk_pawns = diagonal_pawns & pawn_atk_west_bb<Them>(pos.bitboard<Them>() & checkmask);
		uint64_t step_pawns = vertical_pawns & pawn_step_bb<Them>(~pos.pieces);
		uint64_t push_pawns = step_pawns & Bitboard::rank_2<Us>() & pawn_push_bb<Them>(~pos.pieces & checkmask);
		step_pawns &= pawn_step_bb<Them>(checkmask);

		w_atk_pawns &= pawn_atk_east_bb<Them>(bishop_pins) | ~bishop_pins;
		e_atk_pawns &= pawn_atk_west_bb<Them>(bishop_pins) | ~bishop_pins;
		step_pawns &= pawn_step_bb<Them>(rook_pins) | ~rook_pins;
		push_pawns &= pawn_push_bb<Them>(rook_pins) | ~rook_pins;

		if constexpr (State.has_ep_pawn)
		{
			if (Movegen::ep_target != NO_SQUARE)
			{
				uint64_t ep_atk_west = diagonal_pawns & shift<E>(checkmask & Movegen::ep_target);
				uint64_t ep_atk_east = diagonal_pawns & shift<W>(checkmask & Movegen::ep_target);

				#if (DEBUG)
					assert(Bitboard::rank_5<Us>() & Movegen::ep_target);
					assert(Bitboard::rank_6<Us>() & (Movegen::ep_target + pawn_step<Us>()));
				#endif

				if (ep_atk_west | ep_atk_east)
				{
					ep_atk_west &= pawn_atk_east_bb<Them>(bishop_pins) | ~bishop_pins;
					ep_atk_east &= pawn_atk_west_bb<Them>(bishop_pins) | ~bishop_pins;

					if (ep_atk_west)
						*moves++ = Move(EN_PASSANT, lsb(ep_atk_west), Movegen::ep_target + pawn_step<Us>(), PAWN);
					if (ep_atk_east)
						*moves++ = Move(EN_PASSANT, lsb(ep_atk_east), Movegen::ep_target + pawn_step<Us>(), PAWN);
				}
			}
		}

		if ((w_atk_pawns | e_atk_pawns | step_pawns) & Bitboard::rank_7<Us>())
		{
			for (uint64_t w_promote_pawns = w_atk_pawns & Bitboard::rank_7<Us>(); w_promote_pawns; )
			{
				const Square from = pop_lsb(w_promote_pawns);
				const Square to = from + pawn_atk_west<Us>();
				*moves++ = Move(PROMOTION, from, to, QUEEN); 
				*moves++ = Move(PROMOTION, from, to, KNIGHT); 
				*moves++ = Move(PROMOTION, from, to, ROOK); 
				*moves++ = Move(PROMOTION, from, to, BISHOP); 
			}

			for (uint64_t e_promote_pawns = e_atk_pawns & Bitboard::rank_7<Us>(); e_promote_pawns; )
			{
				const Square from = pop_lsb(e_promote_pawns);
				const Square to = from + pawn_atk_east<Us>();
				*moves++ = Move(PROMOTION, from, to, QUEEN); 
				*moves++ = Move(PROMOTION, from, to, KNIGHT);
				*moves++ = Move(PROMOTION, from, to, ROOK); 
				*moves++ = Move(PROMOTION, from, to, BISHOP); 
			}

			for (uint64_t s_promote_pawns = step_pawns & Bitboard::rank_7<Us>(); s_promote_pawns; )
			{
				const Square from = pop_lsb(s_promote_pawns);
				const Square to = from + pawn_step<Us>();
				*moves++ = Move(PROMOTION, from, to, QUEEN); 
				*moves++ = Move(PROMOTION, from, to, KNIGHT); 
				*moves++ = Move(PROMOTION, from, to, ROOK); 
				*moves++ = Move(PROMOTION, from, to, BISHOP); 
			}

			w_atk_pawns &= ~Bitboard::rank_7<Us>();
			e_atk_pawns &= ~Bitboard::rank_7<Us>();
			step_pawns &= ~Bitboard::rank_7<Us>();
		}

		while (w_atk_pawns)
		{
			const Square s = pop_lsb(w_atk_pawns);
			*moves++ = Move(NORMAL, s, s + pawn_atk_west<Us>(), PAWN);
		}

		while (e_atk_pawns)
		{
			const Square s = pop_lsb(e_atk_pawns);
			*moves++ = Move(NORMAL, s, s + pawn_atk_east<Us>(), PAWN);
		}

		while (step_pawns)
		{
			const Square s = pop_lsb(step_pawns);
			*moves++ = Move(NORMAL, s, s + pawn_step<Us>(), PAWN);
		}

		while (push_pawns)
		{
			const Square s = pop_lsb(push_pawns);
			*moves++ = Move(NORMAL, s, s + pawn_push<Us>(), PAWN);
		}
	}
	else if constexpr (Gt == CAPTURES)
	{
		const uint64_t diagonal_pawns = pos.bitboard<Us, PAWN>() & ~rook_pins;

		uint64_t w_atk_pawns = diagonal_pawns & pawn_atk_east_bb<Them>(pos.bitboard<Them>() & checkmask);
		uint64_t e_atk_pawns = diagonal_pawns & pawn_atk_west_bb<Them>(pos.bitboard<Them>() & checkmask);
		w_atk_pawns &= pawn_atk_east_bb<Them>(bishop_pins) | ~bishop_pins;
		e_atk_pawns &= pawn_atk_west_bb<Them>(bishop_pins) | ~bishop_pins;

		if constexpr (State.has_ep_pawn)
		{
			if (Movegen::ep_target != NO_SQUARE)
			{
				uint64_t ep_atk_west = diagonal_pawns & shift<E>(checkmask & Movegen::ep_target);
				uint64_t ep_atk_east = diagonal_pawns & shift<W>(checkmask & Movegen::ep_target);

				#if (DEBUG)
					assert(Bitboard::rank_5<Us>() & Movegen::ep_target);
					assert(Bitboard::rank_6<Us>() & (Movegen::ep_target + pawn_step<Us>()));
				#endif

				if (ep_atk_west | ep_atk_east)
				{
					ep_atk_west &= pawn_atk_east_bb<Them>(bishop_pins) | ~bishop_pins;
					ep_atk_east &= pawn_atk_west_bb<Them>(bishop_pins) | ~bishop_pins;

					if (ep_atk_west)
						*moves++ = Move(EN_PASSANT, lsb(ep_atk_west), Movegen::ep_target + pawn_step<Us>(), PAWN);
					if (ep_atk_east)
						*moves++ = Move(EN_PASSANT, lsb(ep_atk_east), Movegen::ep_target + pawn_step<Us>(), PAWN);
				}
			}
		}

		if ((w_atk_pawns | e_atk_pawns) & Bitboard::rank_7<Us>())
		{
			for (uint64_t w_promote_pawns = w_atk_pawns & Bitboard::rank_7<Us>(); w_promote_pawns; )
			{
				const Square from = pop_lsb(w_promote_pawns);
				const Square to = from + pawn_atk_west<Us>();
				*moves++ = Move(PROMOTION, from, to, QUEEN); 
				*moves++ = Move(PROMOTION, from, to, KNIGHT); 
				*moves++ = Move(PROMOTION, from, to, ROOK); 
				*moves++ = Move(PROMOTION, from, to, BISHOP); 
			}

			for (uint64_t e_promote_pawns = e_atk_pawns & Bitboard::rank_7<Us>(); e_promote_pawns; )
			{
				const Square from = pop_lsb(e_promote_pawns);
				const Square to = from + pawn_atk_east<Us>();
				*moves++ = Move(PROMOTION, from, to, QUEEN); 
				*moves++ = Move(PROMOTION, from, to, KNIGHT);
				*moves++ = Move(PROMOTION, from, to, ROOK); 
				*moves++ = Move(PROMOTION, from, to, BISHOP); 
			}

			w_atk_pawns &= ~Bitboard::rank_7<Us>();
			e_atk_pawns &= ~Bitboard::rank_7<Us>();
		}

		while (w_atk_pawns)
		{
			const Square s = pop_lsb(w_atk_pawns);
			*moves++ = Move(NORMAL, s, s + pawn_atk_west<Us>(), PAWN);
		}

		while (e_atk_pawns)
		{
			const Square s = pop_lsb(e_atk_pawns);
			*moves++ = Move(NORMAL, s, s + pawn_atk_east<Us>(), PAWN);
		}
	}

	{
		uint64_t knights = pos.bitboard<Us, KNIGHT>() & ~(rook_pins | bishop_pins);
		while (knights)
		{
			const Square from = pop_lsb(knights);
			for (uint64_t to = PIECE_ATTACKS[KNIGHT][from] & moveable_sqs; to; )
				*moves++ = Move(NORMAL, from, pop_lsb(to), KNIGHT);
		}
	}

	uint64_t queens = pos.bitboard<Us, QUEEN>();

	{
		uint64_t bishops = pos.bitboard<Us, BISHOP>() & ~rook_pins;
		uint64_t pinned_bishops = (bishops | queens) & bishop_pins;
		uint64_t unpinned_bishops = bishops & ~bishop_pins;

		while (pinned_bishops)
		{
			const Square from = pop_lsb(pinned_bishops);
			uint64_t to = BISHOP_MAGICS[from][pos.pieces] & moveable_sqs & bishop_pins;
			const PieceType slider = (queens & from ? QUEEN : BISHOP);

			while (to)
				*moves++ = Move(NORMAL, from, pop_lsb(to), slider);
		}

		while (unpinned_bishops)
		{
			const Square from = pop_lsb(unpinned_bishops);
			uint64_t to = BISHOP_MAGICS[from][pos.pieces] & moveable_sqs;
			
			while (to)
				*moves++ = Move(NORMAL, from, pop_lsb(to), BISHOP);
		}
	}

	{
		uint64_t rooks = pos.bitboard<Us, ROOK>() & ~bishop_pins;
		uint64_t pinned_rooks = (rooks | queens) & rook_pins;
		uint64_t unpinned_rooks = rooks & ~rook_pins;

		while (pinned_rooks)
		{
			const Square from = pop_lsb(pinned_rooks);
			uint64_t to = ROOK_MAGICS[from][pos.pieces] & moveable_sqs & rook_pins;
			const PieceType slider = (queens & from ? QUEEN : ROOK);

			while (to)
				*moves++ = Move(NORMAL, from, pop_lsb(to), slider);
		}

		while (unpinned_rooks)
		{
			const Square from = pop_lsb(unpinned_rooks);
			uint64_t to = ROOK_MAGICS[from][pos.pieces] & moveable_sqs;

			while (to)
				*moves++ = Move(NORMAL, from, pop_lsb(to), ROOK);
		}
	}

	{
		queens &= ~(rook_pins | bishop_pins);
		while (queens)
		{
			const Square from = pop_lsb(queens);
			uint64_t to = attacks_bb<QUEEN>(from, pos.pieces) & moveable_sqs;

			while (to)
				*moves++ = Move(NORMAL, from, pop_lsb(to), QUEEN);
		}
	}

	return moves;
}

template<GenType Gt, Boardstate State>
inline Move* enumerate(const Position& restrict pos, Move* moves)
{
	constexpr Color Us = State.turn;

	uint64_t king_atk = Movegen::king_attacks<Gt, Us, State.has_ep_pawn>(pos);

	if (Movegen::checkmask == Bitboard::BOARD) // not in check
		moves = enumerate<Gt, State, false>(pos, king_atk, moves);
	else if (Movegen::checkmask != 0) // single check
		moves = enumerate<Gt, State, true>(pos, king_atk, moves);
	else
	{
		const Square ksq = pos.king_sq<Us>();
		while (king_atk)
			*moves++ = Move(NORMAL, ksq, pop_lsb(king_atk), KING);
	}

	return moves;
}


force_inline void Movegen::register_slider_check(Square ksq, Square ssq)
{
	if (Movegen::checkmask == Bitboard::BOARD) // no checks yet
		Movegen::checkmask = BETWEEN_BB[ksq][ssq];
	else // already one check, hence there is a double check
		Movegen::checkmask = 0;

	Movegen::kingban |= CHECK_RAY_BB[ksq][ssq]; 
}

template<Color Us, bool EnPassant>
force_inline void Movegen::register_bishop_pin(const Position& restrict pos, Square ksq, Square bsq)
{
	const uint64_t pinmask = BETWEEN_BB[ksq][bsq];

	if constexpr (EnPassant)
		if (pinmask & (Movegen::ep_target))
			Movegen::ep_target = NO_SQUARE;

	if (pinmask & pos.bitboard<Us>()) // there lies a friendly piece in the enemy bishop's path
		Movegen::bishop_pins |= pinmask;
}

template<Color Us>
force_inline void Movegen::register_rook_pin(const Position& restrict pos, Square ksq, Square rsq)
{
	const uint64_t pinmask = BETWEEN_BB[ksq][rsq];

	if (pinmask & pos.bitboard<Us>()) // there lies a friendly piece in the enemy rook's path
		Movegen::rook_pins |= pinmask;
}

template<Color Us>
force_inline void Movegen::register_ep_pin(const Position& restrict pos, Square ksq)
{
	constexpr Color Them = ~Us;

	const uint64_t pawns = pos.bitboard<Us, PAWN>();
	const uint64_t enemy_rq = pos.bitboards<Them, ROOK, QUEEN>();

	if ((Bitboard::rank_5<Us>() & ksq) && (Bitboard::rank_5<Us>() & enemy_rq) && (Bitboard::rank_5<Us>() & pawns))
	{
		uint64_t ep_atk_west = pawns & shift<E>(square_to_bb(Movegen::ep_target));
		uint64_t ep_atk_east = pawns & shift<W>(square_to_bb(Movegen::ep_target));

		if ((ep_atk_west && (ROOK_MAGICS[ksq][pos.pieces & ~(ep_atk_west | Movegen::ep_target)] & Bitboard::rank_5<Us>()) & enemy_rq)
		 || (ep_atk_east && (ROOK_MAGICS[ksq][pos.pieces & ~(ep_atk_east | Movegen::ep_target)] & Bitboard::rank_5<Us>()) & enemy_rq))
			Movegen::ep_target = NO_SQUARE;
	}
}

template<GenType Gt, Color Us, bool EnPassant>
force_inline uint64_t Movegen::king_attacks(const Position& restrict pos)
{
	constexpr Color Them = ~Us;

	Movegen::checkmask = Bitboard::BOARD;
	Movegen::kingban = 0;
	Movegen::bishop_pins = Movegen::rook_pins = 0;

	const Square ksq = pos.king_sq<Us>();
	const uint64_t king_bb = pos.bitboard<Us, KING>();

	// pawn checks
	{
		const uint64_t w_pawn_atk = pawn_atk_west_bb<Them>(pos.bitboard<Them, PAWN>());
		const uint64_t e_pawn_atk = pawn_atk_east_bb<Them>(pos.bitboard<Them, PAWN>());
		
		if (w_pawn_atk & king_bb)
			Movegen::checkmask = pawn_atk_east_bb<Us>(king_bb);
		else if (e_pawn_atk & king_bb)
			Movegen::checkmask = pawn_atk_west_bb<Us>(king_bb);
	}

	// knight checks
	{
		const uint64_t knight_atk = PIECE_ATTACKS[KNIGHT][ksq] & pos.bitboard<Them, KNIGHT>();
		if (knight_atk)
			Movegen::checkmask = knight_atk;
	}

	// bishop checks and pins
	{
		if (PIECE_ATTACKS[BISHOP][ksq] & pos.bitboards<Them, BISHOP, QUEEN>())
		{
			uint64_t bishop_atk = BISHOP_MAGICS[ksq][pos.pieces] & pos.bitboards<Them, BISHOP, QUEEN>();
			while (bishop_atk)
				Movegen::register_slider_check(ksq, pop_lsb(bishop_atk));

			uint64_t bishop_pin = BISHOP_XRAY_MAGICS[ksq][pos.pieces] & pos.bitboards<Them, BISHOP, QUEEN>();
			while (bishop_pin)
				Movegen::register_bishop_pin<Us, EnPassant>(pos, ksq, pop_lsb(bishop_pin));
		}
	}

	// rook checks and pins
	{
		if (PIECE_ATTACKS[ROOK][ksq] & pos.bitboards<Them, ROOK, QUEEN>())
		{
			uint64_t rook_atk = ROOK_MAGICS[ksq][pos.pieces] & pos.bitboards<Them, ROOK, QUEEN>();
			while (rook_atk)
				Movegen::register_slider_check(ksq, pop_lsb(rook_atk));

			uint64_t rook_pin = ROOK_XRAY_MAGICS[ksq][pos.pieces] & pos.bitboards<Them, ROOK, QUEEN>();
			while (rook_pin)
				Movegen::register_rook_pin<Us>(pos, ksq, pop_lsb(rook_pin));
		}
	}

	if constexpr (EnPassant)
		register_ep_pin<Us>(pos, ksq);

	uint64_t king_moves = PIECE_ATTACKS[KING][ksq] & ~pos.bitboard<Us>() & ~Movegen::kingban;
	if (king_moves == 0) // the king has no legal moves, no sense in updating kingban any further
		return 0;

	for (uint64_t enemy_knights = pos.bitboard<Them, KNIGHT>(); enemy_knights; )
		Movegen::kingban |= PIECE_ATTACKS[KNIGHT][pop_lsb(enemy_knights)];

	Movegen::kingban |= pawn_atk_bb<Them>(pos.bitboard<Them, PAWN>());

	for (uint64_t enemy_bishops = pos.bitboards<Them, BISHOP, QUEEN>(); enemy_bishops; )
		Movegen::kingban |= BISHOP_MAGICS[pop_lsb(enemy_bishops)][pos.pieces];
	
	for (uint64_t enemy_rooks = pos.bitboards<Them, ROOK, QUEEN>(); enemy_rooks; )
		Movegen::kingban |= ROOK_MAGICS[pop_lsb(enemy_rooks)][pos.pieces];

	Movegen::kingban |= PIECE_ATTACKS[KING][pos.king_sq<Them>()];

	if constexpr (Gt == CAPTURES)
		Movegen::kingban |= ~pos.bitboard<Them>();

	return king_moves & ~Movegen::kingban;
}

template<bool IsCheck>
force_inline uint64_t Movegen::const_checkmask(uint64_t checkmask)
{
	if constexpr (IsCheck)
		return checkmask;
	else
		return Bitboard::BOARD;
}

template<Color C, GenType Gt>
force_inline uint64_t Movegen::moveable_sqs(const Position& restrict pos, uint64_t checkmask)
{
	static_assert(Gt == LEGAL || Gt == CAPTURES);
	if constexpr (Gt == LEGAL)
		return ~pos.bitboard<C>() & checkmask;
	if constexpr (Gt == CAPTURES)
		return pos.bitboard<~C>() & checkmask;
}



template<GenType Gt>
force_inline Move* enumerate(const Position& restrict pos, Move* moves)
{
	Movegen::ep_target = pos.ep_target;
	switch (pos.boardstate_pattern())
	{
		case 0:  return enumerate<Gt, 0>(pos, moves);
		case 1:  return enumerate<Gt, 1>(pos, moves);
		case 2:  return enumerate<Gt, 2>(pos, moves);
		case 3:  return enumerate<Gt, 3>(pos, moves);
		case 4:  return enumerate<Gt, 4>(pos, moves);
		case 5:  return enumerate<Gt, 5>(pos, moves);
		case 6:  return enumerate<Gt, 6>(pos, moves);
		case 7:  return enumerate<Gt, 7>(pos, moves);
		case 8:  return enumerate<Gt, 8>(pos, moves);
		case 9:  return enumerate<Gt, 9>(pos, moves);
		case 10: return enumerate<Gt, 10>(pos, moves);
		case 11: return enumerate<Gt, 11>(pos, moves);
		case 12: return enumerate<Gt, 12>(pos, moves);
		case 13: return enumerate<Gt, 13>(pos, moves);
		case 14: return enumerate<Gt, 14>(pos, moves);
		case 15: return enumerate<Gt, 15>(pos, moves);
		case 16: return enumerate<Gt, 16>(pos, moves);
		case 17: return enumerate<Gt, 17>(pos, moves);
		case 18: return enumerate<Gt, 18>(pos, moves);
		case 19: return enumerate<Gt, 19>(pos, moves);
		case 20: return enumerate<Gt, 20>(pos, moves);
		case 21: return enumerate<Gt, 21>(pos, moves);
		case 22: return enumerate<Gt, 22>(pos, moves);
		case 23: return enumerate<Gt, 23>(pos, moves);
		case 24: return enumerate<Gt, 24>(pos, moves);
		case 25: return enumerate<Gt, 25>(pos, moves);
		case 26: return enumerate<Gt, 26>(pos, moves);
		case 27: return enumerate<Gt, 27>(pos, moves);
		case 28: return enumerate<Gt, 28>(pos, moves);
		case 29: return enumerate<Gt, 29>(pos, moves);
		case 30: return enumerate<Gt, 30>(pos, moves);
		case 31: return enumerate<Gt, 31>(pos, moves);
		case 32: return enumerate<Gt, 32>(pos, moves);
		case 33: return enumerate<Gt, 33>(pos, moves);
		case 34: return enumerate<Gt, 34>(pos, moves);
		case 35: return enumerate<Gt, 35>(pos, moves);
		case 36: return enumerate<Gt, 36>(pos, moves);
		case 37: return enumerate<Gt, 37>(pos, moves);
		case 38: return enumerate<Gt, 38>(pos, moves);
		case 39: return enumerate<Gt, 39>(pos, moves);
		case 40: return enumerate<Gt, 40>(pos, moves);
		case 41: return enumerate<Gt, 41>(pos, moves);
		case 42: return enumerate<Gt, 42>(pos, moves);
		case 43: return enumerate<Gt, 43>(pos, moves);
		case 44: return enumerate<Gt, 44>(pos, moves);
		case 45: return enumerate<Gt, 45>(pos, moves);
		case 46: return enumerate<Gt, 46>(pos, moves);
		case 47: return enumerate<Gt, 47>(pos, moves);
		case 48: return enumerate<Gt, 48>(pos, moves);
		case 49: return enumerate<Gt, 49>(pos, moves);
		case 50: return enumerate<Gt, 50>(pos, moves);
		case 51: return enumerate<Gt, 51>(pos, moves);
		case 52: return enumerate<Gt, 52>(pos, moves);
		case 53: return enumerate<Gt, 53>(pos, moves);
		case 54: return enumerate<Gt, 54>(pos, moves);
		case 55: return enumerate<Gt, 55>(pos, moves);
		case 56: return enumerate<Gt, 56>(pos, moves);
		case 57: return enumerate<Gt, 57>(pos, moves);
		case 58: return enumerate<Gt, 58>(pos, moves);
		case 59: return enumerate<Gt, 59>(pos, moves);
		case 60: return enumerate<Gt, 60>(pos, moves);
		case 61: return enumerate<Gt, 61>(pos, moves);
		case 62: return enumerate<Gt, 62>(pos, moves);
		case 63: return enumerate<Gt, 63>(pos, moves);
		default:
			std::cerr << "Error, unrecognizable Boardstate pattern: " << pos.boardstate_pattern() << '.' << std::endl;
			return nullptr;
	}
}


#endif // MOVEGEN_H_6A20B59292CA
