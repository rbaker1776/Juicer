#include "engine.h"
#include "movegen.h"


Engine::Engine()
{
	states = std::unique_ptr<std::deque<Gamestate>>(new std::deque<Gamestate>(1));
	pos.seed(STARTING_POS, states->back());
}


void Engine::set_position(const std::string& fen)
{
	states = std::unique_ptr<std::deque<Gamestate>>(new std::deque<Gamestate>(1));
	pos.seed(fen, states->back());
}


uint64_t Engine::perft(int depth)
{
	uint64_t count = 0;
	uint64_t nodes = 0;
	const bool leaf = depth == 2;

	for (const ValuedMove& m: MoveList<LEGAL>(this->pos))
	{
		if (depth <= 1)
		{
			count = 1;
			++nodes;
		}
		else
		{
			states->emplace_back();
			pos.make_move(m, states->back());
			count = leaf ? MoveList<LEGAL>(pos).size() : perft(depth - 1);
			nodes += count;
			pos.undo_move(m);
			states->pop_back();
		}
	}
	return nodes;
}
