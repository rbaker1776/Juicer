#ifndef ENGINE_H_745972781DA8
#define ENGINE_H_745972781DA8

#include <deque>
#include "position.h"
#include "types.h"
#include "uci.h"
#include "juicer.h"


class Engine
{
public:
	Engine();
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
	
	void set_position(const std::string& fen);

	inline void make_move(std::string move) { this->make_move(UCI::string_to_move(pos, move)); }
	inline void undo_move(std::string move) { this->undo_move(UCI::string_to_move(pos, move)); }
	void make_move(Move move);
	void undo_move(Move move);

	const Position& position() const { return this->pos; }

	uint64_t perft(int depth, bool isroot = true);

private:
	Position pos;
	std::unique_ptr<std::deque<Gamestate>> states;
};


#endif // ENGINE_H_745972781DA8
