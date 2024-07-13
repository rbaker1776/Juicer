#include "position.h"
#include <string>
#include <sstream>


std::string Board::to_string() const
{
	std::ostringstream ss;
	ss << "+---+---+---+---+---+---+---+---+\n";

	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			ss << '|' << ' ' << FEN::PIECES[this->piece_on(make_square(f, r))] << ' ';
		}
		ss << '|' << ' ' << std::to_string(r + 1) << '\n';
		ss << "+---+---+---+---+---+---+---+---+\n";
	}

	ss << "  a   b   c   d   e   f   g   h";
	return ss.str();
}
