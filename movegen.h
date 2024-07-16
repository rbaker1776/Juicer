#ifndef MOVEGEN_H_6A20B59292CA
#define MOVEGEN_H_6A20B59292CA

#include <vector>
#include <iostream>
#include "position.h"
#include "movement.h"
#include "bitboard.h"
#include "types.h"
#include "juicer.h"


inline Move* enumerate(const Board& board, const Gamestate& state, Move* moves);


struct MoveList
{
public:
	explicit MoveList(const Board& board, const Gamestate& state): last(enumerate(board, state, moves)) {}

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
	static std::vector<Square> ep_targets([](){
		std::vector<Square> eps(1, NO_SQUARE);
		eps.reserve(256);
		return eps;
	}());
	static Square ep_target = NO_SQUARE;

	static uint64_t rook_pins;
	static uint64_t bishop_pins;

	static uint64_t checkmask;
	static uint64_t kingban;

	inline void register_slider_check(Square ksq, Square ssq);

	template<Color Us, bool EnPassant>
	inline void register_bishop_pin(const Board& board, Square ksq, Square bsq);

	template<Color Us>
	inline void register_rook_pin(const Board& board, Square ksq, Square rsq);

	template<Color Us>
	inline void register_ep_pin(const Board& board, Square ksq);
		
	template<Color Us, bool EnPassant>
	inline uint64_t king_attacks(const Board& board);
} // namespace Movegen


template<Gamestate State, bool IsCheck>
inline Move* enumerate(const Board& board, uint64_t king_atk, Move* moves)
{
	constexpr Color Us = State.turn;
	constexpr Color Them = ~Us;

	if constexpr (!IsCheck)
		Movegen::checkmask = BOARD_BB;

	const uint64_t moveable_sqs = ~board.bitboard<Us>() & Movegen::checkmask;
	const Square ksq = board.king_sq<Us>();

	{
		while (king_atk)
			*moves++ = Move(NORMAL, ksq, pop_lsb(king_atk), KING);

		if constexpr (!IsCheck && State.can_castle_queenside())
			if (State.can_castle_queenside(Movegen::kingban, board.pieces, board.bitboard<Us, ROOK>()))
				*moves++ = Move(CASTLING, ksq, ksq + 2 * Direction::W, KING);

		if constexpr (!IsCheck && State.can_castle_kingside())
			if (State.can_castle_kingside(Movegen::kingban, board.pieces, board.bitboard<Us, ROOK>()))
				*moves++ = Move(CASTLING, ksq, ksq + 2 * Direction::E, KING);
	}

	{
		const uint64_t vertical_pawns = board.bitboard<Us, PAWN>() & ~Movegen::bishop_pins;
		const uint64_t diagonal_pawns = board.bitboard<Us, PAWN>() & ~Movegen::rook_pins;

		uint64_t w_atk_pawns = diagonal_pawns & pawn_atk_east_bb<Them>(board.bitboard<Them>() & Movegen::checkmask);
		uint64_t e_atk_pawns = diagonal_pawns & pawn_atk_west_bb<Them>(board.bitboard<Them>() & Movegen::checkmask);
		uint64_t step_pawns = vertical_pawns & pawn_step_bb<Them>(board.empty());
		uint64_t push_pawns = step_pawns & rank_2_bb<Us>() & pawn_push_bb<Them>(board.empty() & Movegen::checkmask);
		step_pawns &= pawn_step_bb<Them>(Movegen::checkmask);

		w_atk_pawns &= pawn_atk_east_bb<Them>(Movegen::bishop_pins) | ~Movegen::bishop_pins;
		e_atk_pawns &= pawn_atk_west_bb<Them>(Movegen::bishop_pins) | ~Movegen::bishop_pins;
		step_pawns &= pawn_step_bb<Them>(Movegen::rook_pins) | ~Movegen::rook_pins;
		push_pawns &= pawn_push_bb<Them>(Movegen::rook_pins) | ~Movegen::rook_pins;

		if constexpr (State.has_ep_pawn)
		{
			if (Movegen::ep_target != NO_SQUARE)
			{
				uint64_t ep_atk_west = diagonal_pawns & shift<E>(Movegen::checkmask & Movegen::ep_target);
				uint64_t ep_atk_east = diagonal_pawns & shift<W>(Movegen::checkmask & Movegen::ep_target);

				#if (DEBUG)
					assert(ep_rank_bb<Us>() & Movegen::ep_target);
					assert(rank_6_bb<Us>() & (Movegen::ep_target + pawn_step<Us>()));
				#endif

				if (ep_atk_west | ep_atk_east)
				{
					ep_atk_west &= pawn_atk_east_bb<Them>(Movegen::bishop_pins) | ~Movegen::bishop_pins;
					ep_atk_east &= pawn_atk_west_bb<Them>(Movegen::bishop_pins) | ~Movegen::bishop_pins;

					if (ep_atk_west)
						*moves++ = Move(EN_PASSANT, lsb(ep_atk_west), Movegen::ep_target + pawn_step<Us>(), PAWN);
					if (ep_atk_east)
						*moves++ = Move(EN_PASSANT, lsb(ep_atk_east), Movegen::ep_target + pawn_step<Us>(), PAWN);
				}
			}
		}

		if ((w_atk_pawns | e_atk_pawns | step_pawns) & rank_7_bb<Us>())
		{
			for (uint64_t w_promote_pawns = w_atk_pawns & rank_7_bb<Us>(); w_promote_pawns; )
			{
				const Square from = pop_lsb(w_promote_pawns);
				const Square to = from + pawn_atk_west<Us>();
				*moves++ = Move(PROMOTION, from, to, QUEEN); 
				*moves++ = Move(PROMOTION, from, to, KNIGHT); 
				*moves++ = Move(PROMOTION, from, to, ROOK); 
				*moves++ = Move(PROMOTION, from, to, BISHOP); 
			}

			for (uint64_t e_promote_pawns = e_atk_pawns & rank_7_bb<Us>(); e_promote_pawns; )
			{
				const Square from = pop_lsb(e_promote_pawns);
				const Square to = from + pawn_atk_east<Us>();
				*moves++ = Move(PROMOTION, from, to, QUEEN); 
				*moves++ = Move(PROMOTION, from, to, KNIGHT);
				*moves++ = Move(PROMOTION, from, to, ROOK); 
				*moves++ = Move(PROMOTION, from, to, BISHOP); 
			}

			for (uint64_t s_promote_pawns = step_pawns & rank_7_bb<Us>(); s_promote_pawns; )
			{
				const Square from = pop_lsb(s_promote_pawns);
				const Square to = from + pawn_step<Us>();
				*moves++ = Move(PROMOTION, from, to, QUEEN); 
				*moves++ = Move(PROMOTION, from, to, KNIGHT); 
				*moves++ = Move(PROMOTION, from, to, ROOK); 
				*moves++ = Move(PROMOTION, from, to, BISHOP); 
			}

			w_atk_pawns &= ~rank_7_bb<Us>();
			e_atk_pawns &= ~rank_7_bb<Us>();
			step_pawns &= ~rank_7_bb<Us>();
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

	{
		uint64_t knights = board.bitboard<Us, KNIGHT>() & ~(Movegen::rook_pins | Movegen::bishop_pins);
		while (knights)
		{
			const Square from = pop_lsb(knights);
			for (uint64_t to = PIECE_ATTACKS[KNIGHT][from] & moveable_sqs; to; )
				*moves++ = Move(NORMAL, from, pop_lsb(to), KNIGHT);
		}
	}

	uint64_t queens = board.bitboard<Us, QUEEN>();

	{
		uint64_t bishops = board.bitboard<Us, BISHOP>() & ~Movegen::rook_pins;
		uint64_t pinned_bishops = (bishops | queens) & Movegen::bishop_pins;
		uint64_t unpinned_bishops = bishops & ~Movegen::bishop_pins;

		while (pinned_bishops)
		{
			const Square from = pop_lsb(pinned_bishops);
			uint64_t to = BISHOP_MAGICS[from][board.pieces] & moveable_sqs & Movegen::bishop_pins;
			const PieceType slider = (queens & from ? QUEEN : BISHOP);

			while (to)
				*moves++ = Move(NORMAL, from, pop_lsb(to), slider);
		}

		while (unpinned_bishops)
		{
			const Square from = pop_lsb(unpinned_bishops);
			uint64_t to = BISHOP_MAGICS[from][board.pieces] & moveable_sqs;
			
			while (to)
				*moves++ = Move(NORMAL, from, pop_lsb(to), BISHOP);
		}
	}

	{
		uint64_t rooks = board.bitboard<Us, ROOK>() & ~Movegen::bishop_pins;
		uint64_t pinned_rooks = (rooks | queens) & Movegen::rook_pins;
		uint64_t unpinned_rooks = rooks & ~Movegen::rook_pins;

		while (pinned_rooks)
		{
			const Square from = pop_lsb(pinned_rooks);
			uint64_t to = ROOK_MAGICS[from][board.pieces] & moveable_sqs & Movegen::rook_pins;
			const PieceType slider = (queens & from ? QUEEN : ROOK);

			while (to)
				*moves++ = Move(NORMAL, from, pop_lsb(to), slider);
		}

		while (unpinned_rooks)
		{
			const Square from = pop_lsb(unpinned_rooks);
			uint64_t to = ROOK_MAGICS[from][board.pieces] & moveable_sqs;

			while (to)
				*moves++ = Move(NORMAL, from, pop_lsb(to), ROOK);
		}
	}

	{
		queens &= ~(Movegen::rook_pins | Movegen::bishop_pins);
		while (queens)
		{
			const Square from = pop_lsb(queens);
			uint64_t to = attacks_bb<QUEEN>(from, board.pieces) & moveable_sqs;

			while (to)
				*moves++ = Move(NORMAL, from, pop_lsb(to), QUEEN);
		}
	}

	return moves;
}

template<Gamestate State>
inline Move* enumerate(const Board& board, Move* moves)
{
	constexpr Color Us = State.turn;

	Movegen::ep_target = Movegen::ep_targets.back();
	uint64_t king_atk = Movegen::king_attacks<Us, State.has_ep_pawn>(board);

	if (Movegen::checkmask == BOARD_BB) // not in check
		moves = enumerate<State, false>(board, king_atk, moves);
	else if (Movegen::checkmask != 0) // single check
		moves = enumerate<State, true>(board, king_atk, moves);
	else
	{
		const Square ksq = board.king_sq<Us>();
		while (king_atk)
			*moves++ = Move(NORMAL, ksq, pop_lsb(king_atk), KING);
	}

	return moves;
}


inline void Movegen::register_slider_check(Square ksq, Square ssq)
{
	if (Movegen::checkmask == BOARD_BB) // no checks yet
		Movegen::checkmask = BETWEEN_BB[ksq][ssq];
	else // already one check, hence there is a double check
		Movegen::checkmask = 0;

	Movegen::kingban |= CHECK_RAY_BB[ksq][ssq]; 
}

template<Color Us, bool EnPassant>
inline void Movegen::register_bishop_pin(const Board& board, Square ksq, Square bsq)
{
	const uint64_t pinmask = BETWEEN_BB[ksq][bsq];

	if constexpr (EnPassant)
		if (pinmask & (Movegen::ep_target))
			Movegen::ep_target = NO_SQUARE;

	if (pinmask & board.bitboard<Us>()) // there lies a friendly piece in the enemy bishop's path
		Movegen::bishop_pins |= pinmask;
}

template<Color Us>
inline void Movegen::register_rook_pin(const Board& board, Square ksq, Square rsq)
{
	const uint64_t pinmask = BETWEEN_BB[ksq][rsq];

	if (pinmask & board.bitboard<Us>()) // there lies a friendly piece in the enemy rook's path
		Movegen::rook_pins |= pinmask;
}

template<Color Us>
inline void Movegen::register_ep_pin(const Board& board, Square ksq)
{
	constexpr Color Them = ~Us;

	const uint64_t pawns = board.bitboard<Us, PAWN>();
	const uint64_t enemy_rq = board.bitboards<Them, ROOK, QUEEN>();

	if ((ep_rank_bb<Us>() & ksq) && (ep_rank_bb<Us>() & enemy_rq) && (ep_rank_bb<Us>() & pawns))
	{
		uint64_t ep_atk_west = pawns & shift<E>(square_to_bb(Movegen::ep_target));
		uint64_t ep_atk_east = pawns & shift<W>(square_to_bb(Movegen::ep_target));

		if ((ep_atk_west && (ROOK_MAGICS[ksq][board.pieces & ~(ep_atk_west | Movegen::ep_target)] & ep_rank_bb<Us>()) & enemy_rq)
		 || (ep_atk_east && (ROOK_MAGICS[ksq][board.pieces & ~(ep_atk_east | Movegen::ep_target)] & ep_rank_bb<Us>()) & enemy_rq))
			Movegen::ep_target = NO_SQUARE;
	}
}

template<Color Us, bool EnPassant>
inline uint64_t Movegen::king_attacks(const Board& board)
{
	constexpr Color Them = ~Us;

	Movegen::checkmask = BOARD_BB;
	Movegen::kingban = 0;
	Movegen::bishop_pins = Movegen::rook_pins = 0;

	const Square ksq = board.king_sq<Us>();
	const uint64_t king_bb = board.bitboard<Us, KING>();

	// pawn checks
	{
		const uint64_t w_pawn_atk = pawn_atk_west_bb<Them>(board.bitboard<Them, PAWN>());
		const uint64_t e_pawn_atk = pawn_atk_east_bb<Them>(board.bitboard<Them, PAWN>());
		
		if (w_pawn_atk & king_bb)
			Movegen::checkmask = pawn_atk_east_bb<Us>(king_bb);
		else if (e_pawn_atk & king_bb)
			Movegen::checkmask = pawn_atk_west_bb<Us>(king_bb);
	}

	// knight checks
	{
		const uint64_t knight_atk = PIECE_ATTACKS[KNIGHT][ksq] & board.bitboard<Them, KNIGHT>();
		if (knight_atk)
			Movegen::checkmask = knight_atk;
	}

	// bishop checks and pins
	{
		if (PIECE_ATTACKS[BISHOP][ksq] & board.bitboards<Them, BISHOP, QUEEN>())
		{
			uint64_t bishop_atk = BISHOP_MAGICS[ksq][board.pieces] & board.bitboards<Them, BISHOP, QUEEN>();
			while (bishop_atk)
				Movegen::register_slider_check(ksq, pop_lsb(bishop_atk));

			uint64_t bishop_pin = BISHOP_XRAY_MAGICS[ksq][board.pieces] & board.bitboards<Them, BISHOP, QUEEN>();
			while (bishop_pin)
				Movegen::register_bishop_pin<Us, EnPassant>(board, ksq, pop_lsb(bishop_pin));
		}
	}

	// rook checks and pins
	{
		if (PIECE_ATTACKS[ROOK][ksq] & board.bitboards<Them, ROOK, QUEEN>())
		{
			uint64_t rook_atk = ROOK_MAGICS[ksq][board.pieces] & board.bitboards<Them, ROOK, QUEEN>();
			while (rook_atk)
				Movegen::register_slider_check(ksq, pop_lsb(rook_atk));

			uint64_t rook_pin = ROOK_XRAY_MAGICS[ksq][board.pieces] & board.bitboards<Them, ROOK, QUEEN>();
			while (rook_pin)
				Movegen::register_rook_pin<Us>(board, ksq, pop_lsb(rook_pin));
		}
	}

	if constexpr (EnPassant)
		register_ep_pin<Us>(board, ksq);

	uint64_t king_moves = PIECE_ATTACKS[KING][ksq] & ~board.bitboard<Us>() & ~Movegen::kingban;
	if (king_moves == 0) // the king has no legal moves, no sense in updating kingban any further
		return 0;

	for (uint64_t enemy_knights = board.bitboard<Them, KNIGHT>(); enemy_knights; )
		Movegen::kingban |= PIECE_ATTACKS[KNIGHT][pop_lsb(enemy_knights)];

	Movegen::kingban |= pawn_atk_bb<Them>(board.bitboard<Them, PAWN>());

	for (uint64_t enemy_bishops = board.bitboards<Them, BISHOP, QUEEN>(); enemy_bishops; )
		Movegen::kingban |= BISHOP_MAGICS[pop_lsb(enemy_bishops)][board.pieces];
	
	for (uint64_t enemy_rooks = board.bitboards<Them, ROOK, QUEEN>(); enemy_rooks; )
		Movegen::kingban |= ROOK_MAGICS[pop_lsb(enemy_rooks)][board.pieces];

	Movegen::kingban |= PIECE_ATTACKS[KING][board.king_sq<Them>()];

	return king_moves & ~Movegen::kingban;
}


inline Move* enumerate(const Board& board, const Gamestate& state, Move* moves)
{
	switch (state.pattern())
	{
		case 0:  return enumerate<0>(board, moves);
		case 1:  return enumerate<1>(board, moves);
		case 2:  return enumerate<2>(board, moves);
		case 3:  return enumerate<3>(board, moves);
		case 4:  return enumerate<4>(board, moves);
		case 5:  return enumerate<5>(board, moves);
		case 6:  return enumerate<6>(board, moves);
		case 7:  return enumerate<7>(board, moves);
		case 8:  return enumerate<8>(board, moves);
		case 9:  return enumerate<9>(board, moves);
		case 10: return enumerate<10>(board, moves);
		case 11: return enumerate<11>(board, moves);
		case 12: return enumerate<12>(board, moves);
		case 13: return enumerate<13>(board, moves);
		case 14: return enumerate<14>(board, moves);
		case 15: return enumerate<15>(board, moves);
		case 16: return enumerate<16>(board, moves);
		case 17: return enumerate<17>(board, moves);
		case 18: return enumerate<18>(board, moves);
		case 19: return enumerate<19>(board, moves);
		case 20: return enumerate<20>(board, moves);
		case 21: return enumerate<21>(board, moves);
		case 22: return enumerate<22>(board, moves);
		case 23: return enumerate<23>(board, moves);
		case 24: return enumerate<24>(board, moves);
		case 25: return enumerate<25>(board, moves);
		case 26: return enumerate<26>(board, moves);
		case 27: return enumerate<27>(board, moves);
		case 28: return enumerate<28>(board, moves);
		case 29: return enumerate<29>(board, moves);
		case 30: return enumerate<30>(board, moves);
		case 31: return enumerate<31>(board, moves);
		case 32: return enumerate<32>(board, moves);
		case 33: return enumerate<33>(board, moves);
		case 34: return enumerate<34>(board, moves);
		case 35: return enumerate<35>(board, moves);
		case 36: return enumerate<36>(board, moves);
		case 37: return enumerate<37>(board, moves);
		case 38: return enumerate<38>(board, moves);
		case 39: return enumerate<39>(board, moves);
		case 40: return enumerate<40>(board, moves);
		case 41: return enumerate<41>(board, moves);
		case 42: return enumerate<42>(board, moves);
		case 43: return enumerate<43>(board, moves);
		case 44: return enumerate<44>(board, moves);
		case 45: return enumerate<45>(board, moves);
		case 46: return enumerate<46>(board, moves);
		case 47: return enumerate<47>(board, moves);
		case 48: return enumerate<48>(board, moves);
		case 49: return enumerate<49>(board, moves);
		case 50: return enumerate<50>(board, moves);
		case 51: return enumerate<51>(board, moves);
		case 52: return enumerate<52>(board, moves);
		case 53: return enumerate<53>(board, moves);
		case 54: return enumerate<54>(board, moves);
		case 55: return enumerate<55>(board, moves);
		case 56: return enumerate<56>(board, moves);
		case 57: return enumerate<57>(board, moves);
		case 58: return enumerate<58>(board, moves);
		case 59: return enumerate<59>(board, moves);
		case 60: return enumerate<60>(board, moves);
		case 61: return enumerate<61>(board, moves);
		case 62: return enumerate<62>(board, moves);
		case 63: return enumerate<63>(board, moves);
		default:
			std::cerr << "Error, unrecognizable Gamestate pattern: " << state.pattern() << '.' << std::endl;
			return nullptr;
	}
}


#endif // MOVEGEN_H_6A20B59292CA
