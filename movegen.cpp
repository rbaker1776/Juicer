#include "movegen.h"
#include <cassert>
#include "position.h"
#include "movement.h"


template<GenType Gt, Direction D, bool Enemy>
ValuedMove* make_promotions(ValuedMove* moves, Square to)
{
	constexpr bool All = Gt == EVASION || Gt == NON_EVASION;

	if constexpr (Gt == CAPTURE || All)
		*moves++ = Move::make(to - D, to, PROMOTION, QUEEN);

	if constexpr ((Gt == CAPTURE && Enemy) || (Gt == QUIET && !Enemy) || All)
	{	
		*moves++ = Move::make(to - D, to, PROMOTION, ROOK);
		*moves++ = Move::make(to - D, to, PROMOTION, BISHOP);
		*moves++ = Move::make(to - D, to, PROMOTION, KNIGHT);
	}

	return moves;
}


template<Color Us, GenType Gt>
ValuedMove* generate_pawn_moves(const Position& pos, ValuedMove* moves, uint64_t target)
{
	constexpr Color Them = ~Us;
	constexpr uint64_t Rank7 = (Us == WHITE ? RANK7_BB : RANK2_BB);
	constexpr uint64_t Rank3 = (Us == WHITE ? RANK3_BB : RANK6_BB);
	constexpr Direction Up = pawn_push(Us);
	constexpr Direction UpRight = (Us == WHITE ? Direction::NE : Direction::SW);
	constexpr Direction UpLeft = (Us == WHITE ? Direction::NW : Direction::SE);

	#if (DEBUG)
		assert(Us == pos.side_to_move());
		assert(Them == ~pos.side_to_move());
	#endif

	const uint64_t empty_sqs = ~pos.pieces();
	const uint64_t enemies = Gt == EVASION ? pos.checkers() : pos.pieces(Them);

	uint64_t pawns_on_7 = pos.pieces(Us, PAWN) & Rank7;
	uint64_t pawns_off_7 = pos.pieces(Us, PAWN) & ~Rank7;

	if constexpr (Gt != CAPTURE)
	{
		uint64_t b1 = shift<Up>(pawns_off_7) & empty_sqs;
		uint64_t b2 = shift<Up>(b1 & Rank3) & empty_sqs;

		if constexpr (Gt == EVASION)
		{
			b1 &= target;
			b2 &= target;
		}

		if constexpr (Gt == QUIET_CHECK)
		{
			Square ksq = pos.king_sq(Them);
			uint64_t discover_check_candidate_pawns = pos.kings_guards(Them) & ~file_bb(ksq);
			b1 &= pawn_attacks_bb(Them, ksq) | shift<Up+Up>(discover_check_candidate_pawns);
			b2 &= pawn_attacks_bb(Them, ksq) | shift<Up+Up>(discover_check_candidate_pawns);
		}

		while (b1)
		{
			Square to = pop_lsb(b1);
			*moves++ = Move(to - Up, to);
		}
		
		while (b2)
		{
			Square to = pop_lsb(b2);
			*moves++ = Move(to - Up - Up, to);
		}
	}

	if (pawns_on_7)
	{
		uint64_t b1 = shift<UpRight>(pawns_on_7) & enemies;
		uint64_t b2 = shift<UpLeft>(pawns_on_7) & enemies;
		uint64_t b3 = shift<Up>(pawns_on_7) & empty_sqs;

		if constexpr (Gt == EVASION)
			b3 &= target;

		while (b1) moves = make_promotions<Gt, UpRight, true>(moves, pop_lsb(b1));
		while (b2) moves = make_promotions<Gt, UpLeft, true>(moves, pop_lsb(b2));
		while (b3) moves = make_promotions<Gt, Up, false>(moves, pop_lsb(b3));
	}

	if constexpr (Gt == CAPTURE || Gt == EVASION || Gt == NON_EVASION)
	{
		uint64_t b1 = shift<UpRight>(pawns_off_7) & enemies;
		uint64_t b2 = shift<UpLeft>(pawns_off_7) & enemies;

		while (b1)
		{
			Square to = pop_lsb(b1);
			*moves++ = Move(to - UpRight, to);
		}

		while (b2)
		{
			Square to = pop_lsb(b2);
			*moves++ = Move(to - UpLeft, to);
		}

		if (pos.ep_square() != NO_SQUARE)
		{
			#if (DEBUG)
				assert(rank_of(pos.ep_square()) == (Us == WHITE ? RANK_6 : RANK_3));
			#endif

			if (Gt == EVASION && (target & (pos.ep_square() + Up))) return moves;

			b1 = pawns_off_7 & pawn_attacks_bb(Them, pos.ep_square());

			#if (DEBUG)
				assert(b1);
			#endif

			while (b1)
				*moves++ = Move::make(pop_lsb(b1), pos.ep_square(), EN_PASSANT);
		}
	}
	
	return moves;
}


template<Color Us, PieceType Pt, bool Checks>
ValuedMove* generate_moves(const Position& pos, ValuedMove* moves, uint64_t target)
{
	#if (DEBUG)
		assert(Pt != KING && Pt != PAWN);
		assert(Us == pos.side_to_move());
	#endif

	uint64_t from_pieces = pos.pieces(Us, Pt);
	
	while (from_pieces)
	{
		Square from = pop_lsb(from_pieces);
		uint64_t to_bb = attacks_bb<Pt>(from, pos.pieces()) & target;

		if (Checks && (Pt == QUEEN || !(pos.kings_guards(~Us) & from)))
			to_bb &= pos.checking_squares(Pt);

		while (to_bb)
			*moves++ = Move(from, pop_lsb(to_bb));
	}

	return moves;
}


template<Color Us, GenType Gt>
ValuedMove* generate_all(const Position& pos, ValuedMove* moves)
{
	#if (DEBUG)
		assert(Gt != LEGAL);
		assert(Us == pos.side_to_move());
	#endif

	constexpr bool Checks = Gt == QUIET_CHECK;
	const Square ksq = pos.king_sq(Us);
	uint64_t target;

	if (Gt != EVASION || popcount(pos.checkers()) <= 1)
	{
		target = Gt == EVASION ? BETWEEN_BB[ksq][square_of(pos.checkers())]
			   : Gt == NON_EVASION ? ~pos.pieces(Us)
			   : Gt == CAPTURE ? pos.pieces(~Us)
			   : ~pos.pieces();

		moves = generate_pawn_moves<Us, Gt>(pos, moves, target);
		moves = generate_moves<Us, KNIGHT, Checks>(pos, moves, target);
		moves = generate_moves<Us, ROOK, Checks>(pos, moves, target);
		moves = generate_moves<Us, BISHOP, Checks>(pos, moves, target);
		moves = generate_moves<Us, QUEEN, Checks>(pos, moves, target);
	}

	if (!Checks || pos.kings_guards(~Us) & ksq)
	{
		uint64_t bb = attacks_bb<KING>(ksq) & (Gt == EVASION ? ~pos.pieces(Us) : target);
		if (Checks) bb &= ~attacks_bb<QUEEN>(pos.king_sq(~Us));

		while (bb)
			*moves++ = Move(ksq, pop_lsb(bb));

		if ((Gt == QUIET || Gt == NON_EVASION) && pos.can_castle(Us & ANY_CASTLES))
			for (CastlingRights cr: { Us & KING_SIDE, Us & QUEEN_SIDE })
				if (!(pos.pieces() & CASTLING_BB[cr]) && pos.can_castle(cr))
					*moves++ = Move::make(ksq, CASTLING_TO[cr], CASTLES);
	}

	return moves;
}


template<GenType Gt>
ValuedMove* generate(const Position& pos, ValuedMove* moves)
{
	#if (DEBUG)
		assert(Gt != LEGAL);
		assert((Gt == EVASION) == bool(pos.checkers()));
	#endif

	return pos.side_to_move() == WHITE ? generate_all<WHITE, Gt>(pos, moves) : generate_all<BLACK, Gt>(pos, moves);
}


template<>
ValuedMove* generate<LEGAL>(const Position& pos, ValuedMove* moves)
{
	const Color us = pos.side_to_move();
	const uint64_t pinned = pos.kings_guards(us) & pos.pieces(us);
	const Square ksq = pos.king_sq(us);
	ValuedMove* curr = moves;

	moves = pos.checkers() ? generate<EVASION>(pos, moves) : generate<NON_EVASION>(pos, moves);

	while (curr != moves)
	{
		if (((pinned & curr->from()) || curr->from() == ksq || curr->type() == EN_PASSANT) && !pos.is_legal(*curr))
			*curr = *(--moves);
		else
			++curr;
	}

	return moves;	
}
