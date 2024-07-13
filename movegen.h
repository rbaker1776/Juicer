#ifndef MOVEGEN_H_6A20B59292CA
#define MOVEGEN_H_6A20B59292CA

#include <vector>
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
};
/*
namespace Movestack
{
	static inline uint64_t king_moves[32];
	static inline uint64_t enemy_king_moves[32];
	static inline uint64_t check_status[32];
}*/

namespace Movegen
{
	static std::vector<Square> ep_squares([](){
		std::vector<Square> ep_sqs(1, NO_SQUARE);
		ep_sqs.reserve(256);
		return ep_sqs;
	}());
	static Square ep_square = NO_SQUARE;

	static uint64_t rook_pins;
	static uint64_t bishop_pins;

	static uint64_t checkmask;
	static uint64_t kingban;

	inline void register_slider_check(Square ksq, Square ssq)
	{
		if (Movegen::checkmask == BOARD_BB) // no checks yet
			Movegen::checkmask = BETWEEN_BB[ksq][ssq];
		else // already one check, hence there is a double check
			Movegen::checkmask = 0;

		Movegen::kingban |= CHECK_RAY_BB[ksq][ssq]; 
	}

	template<Gamestate State>
	inline void register_bishop_pin(const Board& board, Square ksq, Square bsq)
	{
		const uint64_t pinmask = BETWEEN_BB[ksq][bsq];

		if constexpr (State.has_ep_pawn)
			if (pinmask & Movegen::ep_square)
				Movegen::ep_square = NO_SQUARE;

		if (pinmask & board.pieces_bb<State.turn>()) // there lies a friendly piece in the enemy bishop's path
			Movegen::bishop_pins |= pinmask;
	}

	template<Gamestate State>
	inline void register_rook_pin(const Board& board, Square ksq, Square rsq)
	{
		const uint64_t pinmask = BETWEEN_BB[ksq][rsq];

		if (pinmask & board.pieces_bb<State.turn>()) // there lies a friendly piece in the enemy rook's path
			Movegen::rook_pins |= pinmask;
	}

	template<Gamestate State>
	inline void register_ep_pin(const Board& board, Square ksq)
	{
		constexpr Color Us = State.turn;
		constexpr Color Them = ~Us;

		const uint64_t pawns = board.pawn_bb<Us>();
		const uint64_t enemy_rq = board.hv_sliders_bb<Them>();

		if ((ep_rank_bb<Us>() & ksq) && (ep_rank_bb<Us>() & enemy_rq) && (ep_rank_bb<Us>() & pawns))
		{
			uint64_t ep_atk_west = pawns & shift<E>(square_to_bb(Movegen::ep_square));
			uint64_t ep_atk_east = pawns & shift<E>(square_to_bb(Movegen::ep_square));

			if ((ep_atk_west && (ROOK_MAGICS[ksq][board.pieces & ~(ep_atk_west | Movegen::ep_square)] & ep_rank_bb<Us>()) & enemy_rq)
			 || (ep_atk_east && (ROOK_MAGICS[ksq][board.pieces & ~(ep_atk_east | Movegen::ep_square)] & ep_rank_bb<Us>()) & enemy_rq))
				Movegen::ep_square = NO_SQUARE;
		}
	}
	
	template<Gamestate State>
	inline uint64_t king_attacks(const Board& board)
	{
		constexpr Color Us = State.turn;
		constexpr Color Them = ~Us;

		Movegen::checkmask = BOARD_BB;
		Movegen::kingban = 0;
		Movegen::bishop_pins = Movegen::rook_pins = 0;

		const Square ksq = board.king_sq<Us>();
		const uint64_t king_bb = board.king_bb<Us>();

		// pawn checks
		{
			const uint64_t w_pawn_atk = pawn_attacks_west_bb<Them>(board.pawn_bb<Them>());
			const uint64_t e_pawn_atk = pawn_attacks_east_bb<Them>(board.pawn_bb<Them>());
			
			if (w_pawn_atk & king_bb)
				Movegen::checkmask = pawn_attacks_east_bb<Us>(king_bb);
			else if (e_pawn_atk & king_bb)
				Movegen::checkmask = pawn_attacks_west_bb<Us>(king_bb);
		}

		// knight checks
		{
			const uint64_t knight_atk = PIECE_ATTACKS[KNIGHT][ksq] & board.knight_bb<Them>();
			if (knight_atk)
				Movegen::checkmask = knight_atk;
		}

		// bishop checks and pins
		{
			if (PIECE_ATTACKS[BISHOP][ksq] & board.diag_sliders_bb<Them>())
			{
				uint64_t bishop_atk = BISHOP_MAGICS[ksq][board.pieces] & board.diag_sliders_bb<Them>();
				while (bishop_atk)
					Movegen::register_slider_check(ksq, pop_lsb(bishop_atk));

				uint64_t bishop_pin = BISHOP_XRAY_MAGICS[ksq][board.pieces] & board.diag_sliders_bb<Them>();
				while (bishop_pin)
					Movegen::register_bishop_pin<State>(board, ksq, pop_lsb(bishop_pin));
			}
		}

		// rook checks and pins
		{
			if (PIECE_ATTACKS[ROOK][ksq] & board.hv_sliders_bb<Them>())
			{
				uint64_t rook_atk = ROOK_MAGICS[ksq][board.pieces] & board.hv_sliders_bb<Them>();
				while (rook_atk)
					Movegen::register_slider_check(ksq, pop_lsb(rook_atk));

				uint64_t rook_pin = ROOK_XRAY_MAGICS[ksq][board.pieces] & board.hv_sliders_bb<Them>();
				while (rook_pin)
					Movegen::register_rook_pin<State>(board, ksq, pop_lsb(rook_pin));
			}
		}

		if constexpr (State.has_ep_pawn)
			register_ep_pin<State>(board, ksq);

		uint64_t king_moves = PIECE_ATTACKS[KING][ksq] & ~board.pieces_bb<Us>() & ~Movegen::kingban;
		if (king_moves == 0) // the king has no legal moves, no sense in updating kingban any further
			return 0;

		for (uint64_t enemy_knights = board.knight_bb<Them>(); enemy_knights; )
			Movegen::kingban |= PIECE_ATTACKS[KNIGHT][pop_lsb(enemy_knights)];

		Movegen::kingban |= pawn_attacks_bb<Them>(board.pawn_bb<Them>());

		for (uint64_t enemy_bishops = board.diag_sliders_bb<Them>(); enemy_bishops; )
			Movegen::kingban |= BISHOP_MAGICS[pop_lsb(enemy_bishops)][board.pieces];
		
		for (uint64_t enemy_rooks = board.hv_sliders_bb<Them>(); enemy_rooks; )
			Movegen::kingban |= ROOK_MAGICS[pop_lsb(enemy_rooks)][board.pieces];

		return king_moves & ~Movegen::kingban;
	}
}


template<Gamestate State, bool IsCheck>
inline Move* enumerate(const Board& board, uint64_t king_atk, Move* moves)
{
	constexpr Color Us = State.turn;
	constexpr Color Them = ~Us;

	if constexpr (!IsCheck)
		Movegen::checkmask = BOARD_BB;

	const uint64_t moveable_sqs = ~board.pieces_bb<Us>() & Movegen::checkmask;
	const Square ksq = board.king_sq<Us>();

	{
		while (king_atk)
			*moves++ = Move(NORMAL, ksq, pop_lsb(king_atk), KING);

		if constexpr (!IsCheck && State.can_castle_queenside())
			if (State.can_castle_queenside(Movegen::kingban, board.pieces, board.rook_bb<Us>()))
				*moves++ = Move(CASTLING, ksq, ksq + 2 * Direction::W, KING);

		if constexpr (!IsCheck && State.can_castle_kingside())
			if (State.can_castle_kingside(Movegen::kingban, board.pieces, board.rook_bb<Us>()))
				*moves++ = Move(CASTLING, ksq, ksq + 2 * Direction::E, KING);
	}

	{
		const uint64_t vertical_pawns = board.pawn_bb<Us>() & ~Movegen::bishop_pins;
		const uint64_t diagonal_pawns = board.pawn_bb<Us>() & ~Movegen::rook_pins;

		uint64_t w_atk_pawns = diagonal_pawns & pawn_attacks_east_bb<Them>(board.pieces_bb<Them>() & Movegen::checkmask);
		uint64_t e_atk_pawns = diagonal_pawns & pawn_attacks_west_bb<Them>(board.pieces_bb<Them>() & Movegen::checkmask);
		uint64_t step_pawns = vertical_pawns & pawn_step_bb<Them>(board.empty());
		uint64_t push_pawns = step_pawns & rank_2_bb<Us>() & pawn_push_bb<Them>(board.empty() & Movegen::checkmask);
		step_pawns &= pawn_step_bb<Them>(Movegen::checkmask);

		w_atk_pawns &= pawn_attacks_east_bb<Them>(Movegen::bishop_pins) | ~Movegen::bishop_pins;
		e_atk_pawns &= pawn_attacks_west_bb<Them>(Movegen::bishop_pins) | ~Movegen::bishop_pins;
		step_pawns &= pawn_step_bb<Them>(Movegen::rook_pins) | ~Movegen::rook_pins;
		push_pawns &= pawn_push_bb<Them>(Movegen::rook_pins) | ~Movegen::rook_pins;

		if constexpr (State.has_ep_pawn)
		{
			uint64_t ep_atk_west = diagonal_pawns & shift<pawn_step<Them>()>(shift<E>(Movegen::checkmask & Movegen::ep_square));
			uint64_t ep_atk_east = diagonal_pawns & shift<pawn_step<Them>()>(shift<W>(Movegen::checkmask & Movegen::ep_square));

			#if (DEBUG)
				assert((rank_6_bb<Us>() & Movegen::ep_square) || Movegen::ep_square == NO_SQUARE);
			#endif

			if (ep_atk_west | ep_atk_east)
			{
				ep_atk_west &= pawn_attacks_east_bb<Them>(Movegen::bishop_pins) | ~Movegen::bishop_pins;
				ep_atk_east &= pawn_attacks_west_bb<Them>(Movegen::bishop_pins) | ~Movegen::bishop_pins;

				if (ep_atk_west)
					*moves++ = Move(EN_PASSANT, lsb(ep_atk_west), Movegen::ep_square, PAWN);
				if (ep_atk_east)
					*moves++ = Move(EN_PASSANT, lsb(ep_atk_east), Movegen::ep_square, PAWN);
			}
		}

		if ((w_atk_pawns | e_atk_pawns | step_pawns) & rank_7_bb<Us>())
		{
			for (uint64_t w_promote_pawns = w_atk_pawns & rank_7_bb<Us>(); w_promote_pawns; )
			{
				const Square from = pop_lsb(w_promote_pawns);
				const Square to = from + pawn_step<Us>() + W;
				*moves++ = Move(PROMOTION, from, to, QUEEN); 
				*moves++ = Move(PROMOTION, from, to, KNIGHT); 
				*moves++ = Move(PROMOTION, from, to, ROOK); 
				*moves++ = Move(PROMOTION, from, to, BISHOP); 
			}

			for (uint64_t e_promote_pawns = e_atk_pawns & rank_7_bb<Us>(); e_promote_pawns; )
			{
				const Square from = pop_lsb(e_promote_pawns);
				const Square to = from + pawn_step<Us>() + E;
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
			*moves++ = Move(NORMAL, s, s + pawn_step<Us>() + W, PAWN);
		}

		while (e_atk_pawns)
		{
			const Square s = pop_lsb(e_atk_pawns);
			*moves++ = Move(NORMAL, s, s + pawn_step<Us>() + E, PAWN);
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
		uint64_t knights = board.knight_bb<Us>() & ~(Movegen::rook_pins | Movegen::bishop_pins);
		while (knights)
		{
			const Square from = pop_lsb(knights);
			for (uint64_t to = PIECE_ATTACKS[KNIGHT][from] & moveable_sqs; to; )
				*moves++ = Move(NORMAL, from, pop_lsb(to), KNIGHT);
		}
	}

	uint64_t queens = board.queen_bb<Us>();

	{
		uint64_t bishops = board.bishop_bb<Us>() & ~Movegen::rook_pins;
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
		uint64_t rooks = board.rook_bb<Us>() & ~Movegen::bishop_pins;
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
	constexpr Color Them = ~Us;

	Movegen::ep_square = Movegen::ep_squares.back();
	uint64_t king_atk = Movegen::king_attacks<State>(board);

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
	}
}


/*
namespace Movelist
{
	Square ep_target = NO_SQUARE; // only valid if Gamestate.has_ep_pawn == true

	uint64_t rook_pin_bb = {};
	uint64_t bishop_pin_bb = {};

	template<Gamestate State, int Depth>
	inline void init_stack(const Board& board)
	{
		constexpr Color Us = State.turn;
		constexpr Color Them = ~Us;

		Movestack::king_moves[Depth] = PIECE_ATTACKS[KING][board.king_sq<Us>()];
		Movestack::enemy_king_moves[Depth] = PIECE_ATTACKS[KING][board.king_sq<Them>()];

		{
			const uint64_t pawn_west = pawn_attacks_west_bb<Them>(board.pawn_bb<Them>() & ~FILEA_BB);
			const uint64_t pawn_east = pawn_attacks_east_bb<Them>(board.pawn_bb<Them>() & ~FILEH_BB);

			if (pawn_west & board.king_bb<Us>())
				Movestack::check_status[Depth] = pawn_attacks_east_bb<Us>(board.king_bb<Us>());
			else if (pawn_east & board.king_bb<Us>())
				Movestack::check_status[Depth] = pawn_attacks_west_bb<Us>(board.king_bb<Us>());
			else
				Movestack::check_status[Depth] = BOARD_BB;
		}

		{
			const uint64_t knight_check = PIECE_ATTACKS[KNIGHT][board.king_sq<Us>()] & board.knight_bb<Them>();
			if (knight_check)
				Movestack::check_status[Depth] = knight_check;
		}
	}

	template<Gamestate State>
	inline void register_diag_pin(Square ksq, Square enemy_sq, const Board& board, Square& ep_square)
{
		const uint64_t pinmask = BETWEEN_BB[ksq][enemy_sq];
		if constexpr (State.has_ep_pawn)
			if (pinmask & ep_square)
				ep_square = NO_SQUARE;

		if (pinmask & board.pieces_bb<State.turn>())
			Movelist::bishop_pin_bb |= pinmask;
	}

	template<Gamestate State>
	inline void register_hv_pin(Square ksq, Square enemy_sq, const Board& board)
	{
		const uint64_t pinmask = BETWEEN_BB[ksq][enemy_sq];
		if (pinmask & board.pieces_bb<State.turn>())
			Movelist::rook_pin_bb |= pinmask;
	}

	template<Gamestate State>
	inline void register_ep_pin(Square ksq,  uint64_t enemy_rq, const Board& board)
	{
		constexpr Color Us = State.turn;
		constexpr Color Them = ~Us;

		const uint64_t pawns = board.pawn_bb<Us>();

		if ((ep_rank_bb<Us>() & ksq) && (ep_rank_bb<Us>() & enemy_rq) && (ep_rank_bb<Us> & pawns))
		{
			uint64_t ep_west = pawns & shift<W>(ep_target & ~FILEA_BB);
			uint64_t ep_east = pawns & shift<E>(ep_target & ~FILEH_BB);

			if (ep_west && (ROOK_MAGICS[ksq][board.pieces & ~(ep_west | ep_target)] & ep_rank_bb<Us>()) & enemy_rq)
				ep_target = NO_SQUARE;
			if (ep_east && (ROOK_MAGICS[ksq][board.pieces & ~(ep_east | ep_target)] & ep_rank_bb<Us>()) & enemy_rq)
				ep_target = NO_SQUARE;
		}
	}

	inline void check_by_slider(Square ksq, Square enemy_sq, uint64_t king_ban, uint64_t& check_mask)
	{
		if (check_mask == BOARD_BB)
			check_mask = BETWEEN_BB[ksq][enemy_sq];
		else
			check_mask = 0;

		king_ban |= CHECK_RAY_BB[ksq][enemy_sq];
	}

	template<Gamestate State, int Depth>
	inline uint64_t refresh(const Board& board, uint64_t& king_ban, uint64_t& check_mask)
	{
		constexpr Color Us = State.turn;
		constexpr Color Them = ~Us;

		const Square ksq = board.king_sq<Us>();

		{
			rook_pin_bb = 0;
			bishop_pin_bb = 0;

			if (PIECE_ATTACKS[ROOK][ksq] & board.hv_sliders_bb<Them>())
			{
				uint64_t checkers_hv = ROOK_MAGICS[ksq][board.pieces] & board.hv_sliders_bb<Them>();
				while (checkers_hv)
					check_by_slider(ksq, pop_lsb(checkers_hv), king_ban, check_mask);

				uint64_t pinners_hv = {};
			}
			
			if (PIECE_ATTACKS[BISHOP][ksq] & board.diag_sliders_bb<Them>())
			{
				uint64_t checkers_diag = BISHOP_MAGICS[ksq][board.pieces] & board.diag_sliders_bb<Them>();
				while (checkers_diag)
					check_by_slider(ksq, pop_lsb(checkers_diag), king_ban, check_mask);

				uint64_t pinners_diag = {};
			}

			if constexpr (State.has_ep_pawn) register_ep_pin(ksq, board.hv_sliders_bb<Them>(), board);
		}

		Movestack::check_status[Depth - 1] = BOARD_BB;

		uint64_t king_attacks = Movestack::king_moves[Depth] & ~board.pieces_bb<Us>() & ~king_ban;
		if (king_attacks == 0)
			return 0;

		{
			uint64_t enemy_knight_bb = board.knight_bb<Them>();
			while (enemy_knight_bb)
				king_ban |= PIECE_ATTACKS[KNIGHT][pop_lsb(enemy_knight_bb)];
		}

		{	
		
		}

		{
			uint64_t enemy_diag_sliders_bb = board.diag_sliders_bb<Them>();
			while (enemy_diag_sliders_bb)
				king_ban |= BISHOP_MAGICS[pop_lsb(enemy_diag_sliders_bb)][board.pieces];
		}

		{
			uint64_t enemy_hv_sliders_bb = board.hv_sliders_bb<Them>();
			while (enemy_hv_sliders_bb)
				king_ban |= ROOK_MAGICS[pop_lsb(enemy_hv_sliders_bb)][board.pieces];
		}

		return king_attacks & ~king_ban;
	}

	class VoidClass{};

	template<Gamestate State, class CallbackMove, int Depth, bool InCheck>
	inline auto enumerate(const Board& board, uint64_t king_attack, uint64_t king_ban, uint64_t checkmask)
	{
		constexpr Color Us = State.turn;
		constexpr Color Them = ~Us;

		const uint64_t hv_pins = Movelist::rook_pin_bb;
		const uint64_t diag_pins = Movelist::bishop_pin_bb;
		const uint64_t check_mask = const_check_mask<InCheck>(checkmask);
		const uint64_t moveable_squares_bb = moveable_squares<Us, InCheck>(board, check_mask);
		const uint64_t ep_square = square_to_bb(Movelist::ep_target);

		{
			while (king_attack)
			{
				const Square s = pop_lsb(king_attack);
				Movestack::enemy_king_moves[Depth - 1] = PIECE_ATTACKS[KING][s];
				CallbackMove::template KingMove<State, Depth>(board, board.king_bb<Us>(), square_to_bb(s));
			}

			if constexpr (!InCheck && State.can_castle_queenside())
				if (State.can_castle_queenside(king_ban, board.pieces, board.rook_bb<Us>()))
				{
					Movestack::enemy_king_moves[Depth - 1] = PIECE_ATTACKS[KING][lsb(board.king_bb<Us>() << 2)];
					CallbackMove::template KingCastle<State, Depth>(board, (board.king_bb<Us>() | board.king_bb<Us>() << 2), State.rookswitch_queenside());
				}
		
			if constexpr (!InCheck && State.can_castle_kingside())
				if (State.can_castle_kingside(king_ban, board.pieces, board.rook_bb<Us>()))
				{
					Movestack::enemy_king_moves[Depth - 1] = PIECE_ATTACKS[KING][lsb(board.king_bb<Us>() >> 2)];
					CallbackMove::template KingCastle<State, Depth>(board, (board.king_bb<Us>() | board.king_bb<Us>() >> 2), State.rookswitch_kingside());
				}

			Movestack::enemy_king_moves[Depth - 1] = Movestack::king_moves[Depth];
		}

		{
			const uint64_t pawns_h = board.pawn_bb<Us>() & ~hv_pins;
			const uint64_t pawns_v = board.pawn_bb<Us>() & ~diag_pins;

			uint64_t east_pawns = pawns_h & pawn_attacks_west<Them>(board.pieces_bb<Them>());
			uint64_t west_pawns = pawns_h & pawn_attacks_east<Them>(board.pieces_bb<Them>());
			uint64_t step_pawns = pawns_v & pawn_step_bb<Them>(board.empty());
			uint64_t push_pawns = step_pawns & rank_2_bb<Us>() & pawn_push_bb<Them>(board.empty());
			step_pawns &= pawn_step_bb<Us>(check_mask);

			{
				const uint64_t pinned = east_pawns & pawn_attacks_west<Them>(diag_pins & ~FILEH_BB);
				const uint64_t unpinned = east_pawns & ~diag_pins;
				east_pawns = pinned | unpinned;
			}

			{
				const uint64_t pinned = west_pawns & pawn_attacks_east<Them>(diag_pins & ~FILEA_BB);
				const uint64_t unpinned = west_pawns & ~diag_pins;
				west_pawns = pinned | unpinned;
			}

			{
				const uint64_t pinned = step_pawns & pawn_step_bb<Them>(diag_pins);
				const uint64_t unpinned = step_pawns & ~diag_pins;
				step_pawns = pinned | unpinned;
			}

			{
				const uint64_t pinned = push_pawns & pawn_push_bb<Them>(diag_pins);
				const uint64_t unpinned = push_pawns & ~diag_pins;
				push_pawns = pinned | unpinned;
			}

			if constexpr (State.has_ep_pawn)
			{	
				uint64_t ep_west_pawn = pawns_h & shift<E>(check_mask & ep_target);
				uint64_t ep_east_pawn = pawns_h & shift<W>(check_mask & ep_target);

				if (ep_west_pawn | ep_east_pawn)
				{
					{
						const uint64_t pinned = ep_west_pawn & pawn_attacks_east_bb<Them>(diag_pins);
						const uint64_t unpinned = ep_west_pawn & ~diag_pins;
						ep_west_pawn = pinned | unpinned;
					}

					{
						const uint64_t pinned = ep_east_pawn & pawn_attacks_west_bb<Them>(diag_pins);
						const uint64_t unpinned = ep_east_pawn & ~diag_pins;
						ep_east_pawn = pinned | unpinned;
					}

					if (ep_west_pawn)
						CallbackMove::template pawn_ep_capture<State, Depth>(board, ep_east_pawn);
					if (ep_east_pawn)
						CallbackMove::template pawn_ep_capture<State, Depth>(board, ep_west_pawn);
				}
			}

			if ((east_pawns | west_pawns | step_pawns) & rank_7_bb<Us>())
			{
				uint64_t west_promote = west_pawns & rank_7_bb<Us>();
				uint64_t east_promote = east_pawns & rank_7_bb<Us>();
				uint64_t step_promote = step_pawns & rank_7_bb<Us>();

				while (west_promote)
				{
					const Square s = pop_lsb(west_promote);
					CallbackMove::template pawn_promote<State, Depth>(board, s, pawn_attacks_west_bb<Us>(square_to_bb(s)));
				}

				while (east_promote)
				{
					const Square s = pop_lsb(east_promote);
					CallbackMove::template pawn_promote<State, Depth>(board, s, pawn_attacks_east_bb<Us>(square_to_bb(s)));
				}

				while (step_promote)
				{
					const Square s = pop_lsb(step_promote);
					CallbackMove::template pawn_promote<State, Depth>(board, s, pawn_step_bb<Us>(square_to_bb(s)));
				}

				west_pawns &= ~rank_7_bb<Us>();
				east_pawns &= ~rank_7_bb<Us>();
				step_pawns &= ~rank_7_bb<Us>();
			}

			while (west_pawns)
			{
				const Square s = pop_lsb(west_pawns);
				CallbackMove::template pawn_capture<State, Depth>(board, s, pawn_attack_west_bb<Us>(square_to_bb(s)));
			}

			while (east_pawns)
			{
				const Square s = pop_lsb(east_pawns);
				CallbackMove::template pawn_capture<State, Depth>(board, s, pawn_attack_east_bb<Us>(square_to_bb(s)));
			}

			while (step_pawns)
			{
				const Square s = pop_lsb(step_pawns);
				CallbackMove::template pawn_step<State, Depth>(board, s, pawn_step_bb<Us>(square_to_bb(s)));
			}

			while (push_pawns)
			{
				const Square s = pop_lsb(push_pawns);
				CallbackMove::template pawn_push<State, Depth>(board, s, pawn_push_bb<Us>(square_to_bb(s)));
			}
		}

		{
			uint64_t knight_bb = board.knight_bb<Us>() & ~(diag_pins | hv_pins);

			while (knight_bb)
			{
				const Square s = pop_lsb(knight_bb);
				uint64_t moves = PIECE_ATTACKS[KNIGHT][s] & moveable_squares_bb;
				while (moves)
				{
					const Square to = pop_lsb(moves);
					CallbackMove::template knight_move<State, Depth>(board, square_to_bb(s), square_to_bb(to));
				}
			}
		}

		const uint64_t queen_bb = board.queen_bb<Us>();

		{
			uint64_t bishop_bb = board.bishop_bb<Us>() & ~hv_pins;
			uint64_t pinned_bishop_bb = (bishop_bb | queen_bb) & diag_pins;
			uint64_t unpinned_bishop_bb = bishop_bb & ~diag_pins;

			while (pinned_bishop_bb)
			{
				const Square s = pop_lsb(pinned_bishop_bb);
				uint64_t moves = BISHOP_MAGICS[s][board.pieces] & moveable_squares_bb & diag_pins;
				uint64_t s_bb = square_to_bb(s);
				if (s_bb & queen_bb)
					while (moves)
					{
						const Square to = pop_lsb(moves);
						CallbackMove::template queen_move<State, Depth>(board, s_bb, square_to_bb(to));
					}
				else
					while (moves)
					{
						const Square to = pop_lsb(moves);
						CallbackMove::template bishop_move<State, Depth>(board, s_bb, square_to_bb(to));
					}
			}

			while (unpinned_bishop_bb)
			{
				const Square s = pop_lsb(unpinned_bishop_bb);
				uint64_t moves = BISHOP_MAGICS[s][board.pieces] & moveable_squares_bb;
				while (moves)
				{
					const Square to = pop_lsb(moves);
					CallbackMove::template bishop_move<State, Depth>(board, square_to_bb(s), square_to_bb(to));
				}
			}
		}

		{
			uint64_t rook_bb = board.rook_bb<Us>() & ~diag_pins;
			uint64_t pinned_rook_bb = (rook_bb | queen_bb) & hv_pins;
			uint64_t unpinned_rook_bb = rook_bb & ~hv_pins;

			while (pinned_rook_bb)
			{
				const Square s = pop_lsb(pinned_rook_bb);
				uint64_t moves = ROOK_MAGICS[s][board.pieces] & moveable_squares_bb & hv_pins;
				uint64_t s_bb = square_to_bb(s);
				if (s_bb & queen_bb)
					while (moves)
					{
						const Square to = pop_lsb(moves);
						CallbackMove::template queen_move<State, Depth>(board, s_bb, square_to_bb(to));
					}
				else
					while (moves)
					{
						const Square to = pop_lsb(moves);
						CallbackMove::template rook_move<State, Depth>(board, s_bb, square_to_bb(to));
					}
			}

			while (unpinned_rook_bb)
			{
				const Square s = pop_lsb(unpinned_rook_bb);
				uint64_t moves = ROOK_MAGICS[s][board.pieces] & moveable_squares_bb;
				while (moves)
				{
					const Square to = pop_lsb(moves);
					CallbackMove::template rook_move<State, Depth>(board, square_to_bb(s), square_to_bb(to));
				}
			}
		}

		{
			uint64_t queen_bb = board.queen_bb<Us>() & ~(diag_pins | hv_pins);
			while (queen_bb)
			{
				const Square s = pop_lsb(queen_bb);
				uint64_t attacks = attacks_bb<QUEEN>(s, board.pieces);

				uint64_t moves = attacks & moveable_squares_bb;

				while (moves)
				{
					const Square to = pop_lsb(moves);
					CallbackMove::template queen_move<State, Depth>(board, square_to_bb(s), square_to_bb(to));
				}
			}
		}
	}

	template<Gamestate State, class CallbackMove, int Depth>
	void enumerate_moves(const Board& board)
	{
		uint64_t check_mask = Movestack::check_status[Depth];
		uint64_t king_ban = Movestack::king_moves[Depth - 1] = Movestack::enemy_king_moves[Depth];
		uint64_t king_attack = Refresh<State, Depth>(board, king_ban, check_mask);

		if (check_mask == BOARD_BB)
			enumerate<State, CallbackMove, Depth, false>(board, king_attack, king_ban, check_mask);
		else if (check_mask != 0)
			enumerate<State, CallbackMove, Depth, true>(board, king_attack, king_ban, check_mask);
		else
			while (king_attack)
			{
				const Square s = pop_lsb(king_attack);
				Movestack::enemy_king_moves[Depth - 1] = PIECE_ATTACKS[KING][s];
				CallbackMove::template king_move<State, Depth>(board, board.king_bb<State.turn>(), square_to_bb(s));
			}
	}
} // namespace Movelist


class MoveReciever
{
public:
	static inline uint64_t nodes;

	static inline void init(const Board& board, Square ep_square)
	{
		MoveReciever::nodes = 0;
		Movelist::ep_target = ep_square;
	}

	template<Gamestate State, int Depth>
	static inline void perft(const Board& board) 
	{ 
		if constexpr (Depth == 0)
		{
			++MoveReciever::nodes;
			return;
		}
		Movelist::enumerate_moves<State, MoveReciever, Depth>(board); 
	}

	template<Gamestate State, int Depth>
	static inline void register_move(const Board& board)
	{
		MoveReciever::perft<State, Depth - 1>(board);
	}

	template<Gamestate State, int Depth>
	static inline void king_move(const Board& board, Square from, Square to)
	{
		Board next = board.move<KING, State.turn>(from, to, board.pieces_bb<~State.turn>() & to);
		register_move<State.king_move(), Depth>(next);
	}

	template<Gamestate State, int Depth>
	static inline void king_castle(const Board& board, uint64_t kingswitch, uint64_t rookswitch)
	{
		Board next = board.castles<State.turn>(kingswitch, rookswitch);
		register_move<State.king_move(), Depth>(next);
	}

	template<Gamestate State, int Depth>
	static inline void pawn_check(uint64_t enemy_king_bb, uint64_t to)
	{
		constexpr Color Us = State.turn;
		uint64_t west_pawns = pawn_attacks_west_bb<Us>(to);
		uint64_t east_pawns = pawn_attacks_east_bb<Us>(to);
		if (enemy_king_bb & (west_pawns | east_pawns)) Movestack::check_status[Depth - 1] = to;
	}

	template<Gamestate State, int Depth>
	static inline void knight_check(uint64_t enemy_king_bb, uint64_t to)
	{
		constexpr Color Us = State.turn;
		if (PIECE_ATTACKS[KNIGHT][lsb(enemy_king_bb)] & to) Movestack::check_status[Depth - 1] = to;
	}

	template<Gamestate State, int Depth>
	static inline void pawn_move(const Board& board, Square from, Square to)
	{
		
	}
};*/


#endif // MOVEGEN_H_6A20B59292CA
