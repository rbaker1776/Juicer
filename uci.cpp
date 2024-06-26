#include <iostream>
#include "uci.h"
#include "bitboard.h"


std::string UCI::sq_to_string(Square s)
{
	if (s == NO_SQUARE) { return "-"; }
	std::string str = "a1";
	str[0] += s & 7;
	str[1] += s >> 3;
	return str;
}

std::string UCI::move_to_string(Move m)
{
	if (m == Move::none()) { return "(none)"; }
	if (m == Move::null()) { return "0000"; } // as per UCI spec

	Square from = m.from();
	Square to = m.to();

	if (m.type() == CASTLES) to = make_square(to > from ? FILE_G : FILE_C, rank_of(from));

	std::string move = UCI::sq_to_string(from) + UCI::sq_to_string(to);

	if (m.type() == PROMOTION) move += " pnbrqk"[m.promotion_type()];

	return move;
}

Move UCI::string_to_move(const Position& pos, const std::string& str)
{
	for (const ValuedMove& m: MoveList<LEGAL>(pos))
		if (str == UCI::move_to_string(m))
			return m;

	std::cout << str << std::endl;
	return Move::none();
}
