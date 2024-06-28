#ifndef MOVEGEN_H_DBC6611E298C
#define MOVEGEN_H_DBC6611E298C

#include "juicer.h"
#include "types.h"
#include "juicer.h"


class Position;


// Maximum number of legal moves in a chess position
// Arises from the incredibly unlikely: 1B1KNNBk/4Q1pp/2Q4Q/Q4Q2/3Q4/1Q4Q1/4Q3/R6R w - - 0 1
constexpr int MAX_LEGAL_MOVES = 218;


enum GenType
{
	LEGAL,
	EVASION,
	NON_EVASION,
	CAPTURE,
	QUIET_CHECK,
	QUIET,
};


typedef struct ValuedMove: public Move
{
	int value;
	inline void operator=(Move m) { data = m.raw(); }
	operator float() const = delete;
} ValuedMove;


template<GenType>
ValuedMove* generate(const Position& pos, ValuedMove* move);


template<GenType Gt>
struct MoveList
{
	explicit MoveList(const Position& pos): last(generate<Gt>(pos, moves)) {}
	const ValuedMove* begin() const { return moves; }
	const ValuedMove* end()   const { return last; }
	int size() const { return last - moves; }

private:
	ValuedMove* last;
	ValuedMove moves[MAX_LEGAL_MOVES];
};
	

#endif // MOVEGEN_H_DBC6611E298C
