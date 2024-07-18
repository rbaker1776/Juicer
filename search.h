#ifndef SEARCH_H_5C45E3BCBCFC
#define SEARCH_H_5C45E3BCBCFC

#include "evaluation.h"
#include "position.h"
#include "types.h"
#include "juicer.h"


namespace Search
{
	Move pv_move;
} // namespace Search


int negamax_search(int depth, color c, const Board& board, const Boardstate& boardstate, const Gamestate& gamestate)
{
	if (depth == 0)
		return Evaluation::negamax_crude(c, board);

	int max = INT_MIN;

	for (const Move& m: MoveList<LEGAL>(board, boardstate, gamestate.ep_target))
	{
		int score = -negamax_search
	}
}


#endif // SEARCH_H_5C45E3BCBCFC
