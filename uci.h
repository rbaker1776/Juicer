#ifndef UCI_H_205B7875C4E2
#define UCI_H_205B7875C4E2

#include <string>
#include "position.h"
#include "types.h"
#include "juicer.h"


namespace UCI
{
	inline std::string sq_to_string(Square s);
	inline std::string move_to_string(const Move& m);
	inline Move string_to_move(const Board& board, const Boardstate& boardstate, const Gamestate& gamestate, const std::string& str);
} // namespace UCI


inline std::string UCI::sq_to_string(Square s)
{
	if (s == NO_SQUARE) { return "-"; }
	std::string str = "a1";
	str[0] += s & 7;
	str[1] += s >> 3;
	return str;
}

inline std::string UCI::move_to_string(const Move& m)
{
	Square from = m.from;
	Square to = m.to;

	if (m.type == CASTLING) to = make_square(to > from ? FILE_G : FILE_C, rank_of(from));

	std::string move = UCI::sq_to_string(from) + UCI::sq_to_string(to);

	if (m.type == PROMOTION) move += " pnbrqk"[m.piece];

	return move;
}

inline Move UCI::string_to_move(const Board& board, const Boardstate& boardstate, const Gamestate& gamestate, const std::string& str)
{
	for (const Move& m: MoveList<LEGAL>(board, boardstate, gamestate.ep_target))
		if (str == UCI::move_to_string(m))
			return m;

	return Move();
}


#endif // UCI_H_205B7875C4E2
