#ifndef POSITION_H_C668DBDA2B2D
#define POSITION_H_C668DBDA2B2D

#include "types.h"



class Position
{
private: 
	struct Gamestate
	{
		uint32_t castling_rights;
		Square ep_square;
		uint32_t half_clock;
		uint32_t full_clock;
	};

	uint64_t pieces[6];
	uint64_t armies[2];
	bool white_to_move;
	struct Gamestate gamestate;

public:
	Position& seed(const std::string& fen);
	inline void clear_board()
	{
		this->pieces[0] = this->pieces[1] = this->pieces[2] = this->pieces[3] = 
		this->pieces[4] = this->pieces[5] = this->armies[0] = this->armies[1] = 0ull;
	}
	inline void reset_gamestate()
	{
		this->gamestate.castling_rights = 0;
		this->gamestate.half_clock = this->gamestate.full_clock = 0;
		this->gamestate.ep_square = NO_SQUARE;
	}
	inline void place(char piece, Square s)
	{
		

	std::string& to_string() const;
};



#endif // POSITION_H_C668DBDA2B2D
