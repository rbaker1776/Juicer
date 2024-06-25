#include <cassert>
#include "movegen.h"
#include "position.h"


template<>
ValuedMove* generate<LEGAL>(const Position& pos, ValuedMove* moves)
{
	//const Color us = pos.side_to_move();
	//const uint64_t pinned = pos.blockers_for_king(us) & pos.pieces(us);
	//Square friendly_king_sq = pos.king_sq(us);
	//ValuedMove* cur = moves;

return nullptr;	
}
