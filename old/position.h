#ifndef POSITION_H_7A8B21A194A6
#define POSITION_H_7A8B21A194A6

#include <string>
#include "types.h"



class Position
{
	struct Gamestate
	{
		int castling_rights;
		int rule_50;
		int full_move_clock;
		int half_move_clock;
		Square ep_square;
	};

	private: uint64_t pieces[6];
	private: uint64_t armies[2];
	private: bool white_to_move;

	public: Position& seed(const std::string& fen);
	private: inline bool place_pieces(const std::string& s);
	private: inline bool set_turn(char c);
	private: inline bool set_castling_right(const std::string& s);
	private: inline bool set_ep_square(const std::string& s)
	private: inline bool set_clocks(const std::string& s)

	private: inline void clear_board();
	private: inline void put(Piece p, Square s);
};
	



#endif // POSITION_H_7A8B21A194A6
