#ifndef ENGINE_H_745972781DA8
#define ENGINE_H_745972781DA8

#include <deque>
#include "position.h"
#include "types.h"


class Engine
{
public:
	Engine();
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
	
	void set_position(const std::string& fen);

	uint64_t perft(int depth);

private:
	Position pos;
	std::unique_ptr<std::deque<Gamestate>> states;
};


#endif // ENGINE_H_745972781DA8
