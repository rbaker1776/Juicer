#ifndef ENGINE_H_8EC893250AFF
#define ENGINE_H_8EC893250AFF

#include <vector>
#include <iostream>
#include <string_view>
#include "movegen.h"
#include "uci.h"
#include "position.h"
#include "types.h"
#include "juicer.h"


class Engine
{
public:
	constexpr Engine();
	constexpr Engine(std::string_view fen);
	~Engine() = default;

	inline void seed(std::string_view fen);

	inline void make_move(const Move& m);
	inline void undo_move();
	
	template<bool IsRoot>
	inline uint64_t perft(int depth);

	inline const Position& position() const { return this->positions.back(); }

private:
	std::vector<Position> positions;
}; // class Engine


template<bool IsRoot>
inline uint64_t Engine::perft(int depth)
{
	uint64_t nodes = 0, count = 0;
	const bool leaf = (depth == 2);

	if (depth < 1)
		return 1;

	for (const Move& m: MoveList<LEGAL>(positions.back()))
	{
		if (depth == 1)
			count = 1, nodes++;
		else
		{
			make_move(m);
			count = leaf ? MoveList<LEGAL>(positions.back()).size() : perft<false>(depth - 1);
			nodes += count;
			undo_move();
		}
		if constexpr (IsRoot && VERBOSE)
			std::cout << UCI::move_to_string(m) << ": " << count << std::endl;
	}

	return nodes;
}

constexpr Engine::Engine()
{
	positions.reserve(256);
	positions.emplace_back(Position::startpos());
}

constexpr Engine::Engine(std::string_view fen)
{
	positions.reserve(256);
	positions.emplace_back(fen);
}

inline void Engine::seed(std::string_view fen)
{
	positions.clear();
	positions.emplace_back(Position(fen));
}

inline void Engine::make_move(const Move& m)
{
	positions.emplace_back(positions.back().make_move(m));
}

inline void Engine::undo_move()
{
	positions.pop_back();
}


#endif // ENGINE_H_8EC893250AFF*/
