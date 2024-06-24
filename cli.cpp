#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include "bitboard.h"
#include "position.h"
#include "types.h"
#include "juicer.h"


namespace CLI
{
static void print_help_message()
{
	std::cout << std::string(120, '=') << std::endl;
	std::cout << "Juicer V" << VERSION << " by Ryan Baker" << std::endl << std::endl;
	std::cout << "CLI Commands:" << std::endl;
	std::cout << '\t' << std::setw(11) << "seed (s)" << " - Set the board to the starting position or to a custom position with a fen string" << std::endl;
	std::cout << '\t' << std::setw(11) << "display (d)" << " - Show the current board position" << std::endl;
	std::cout << '\t' << std::setw(11) << "move (m)" << " - Make a move in the current position:" << std::endl;
	std::cout << "\t\t* Enter a starting square and an ending square (move e2 e4) // moves the piece on e2 to e4" << std::endl;
	std::cout << "\t\t* To castle, move the king to the square of the corresponding rook (move e1 h1) // castles kingside" << std::endl;
	std::cout << "\t\t* Use '=' when promoting (move f7 f8 =Q) // promotes the pawn on f7 to a queen on f8" << std::endl;
	std::cout << "\t\t* Use 'ep' when making an en-passant move (move e5 d6 ep) // makes an en-passant capture from e5 to d6" << std::endl;
	std::cout << '\t' << std::setw(11) << "help (?)" << " - It would be weird if you didn't already understand this command" << std::endl;
	std::cout << '\t' << std::setw(11) << "quit (q)" << " - Exit Juicer" << std::endl;
	std::cout << std::string(120, '=') << std::endl;
}

static void parse_move(std::istringstream& ss, std::string& word, Position& pos)
{
	const Square from = ((ss >> word) ? string_to_sq(word) : NO_SQUARE);
	const Square to = ((ss >> word) ? string_to_sq(word) : NO_SQUARE);
	if (from == NO_SQUARE || to == NO_SQUARE)
	{
		std::cout << "Error, invalid squares. Enter squares with algebraic notation (move e2 e4)" << std::endl;
		return;
	}
	
	MoveType mt = ((ss >> word) ? (word == "ep" ? EN_PASSANT : PROMOTION) : NORMAL);
	if (type_of(pos.piece_on(from)) == KING && SQUARE_DISTANCE[from][to] >= 3 && type_of(pos.piece_on(to)) == ROOK)
		mt = CASTLES;

	PieceType promotion = KNIGHT;
	if (mt == PROMOTION)
	{
		if (word.size() != 2 || word[1] != '=' || !(word[1] == 'Q' || word[1] == 'R' || word[1] == 'B' || word[1] == 'N'))
		{
			std::cout << "Error, invalid promotion flag. Enter '=' plus the promotion type (move f7 f8 =Q)" << std::endl;
			return;
		}
		promotion = word[1] == 'Q' ? QUEEN
				  : word[1] == 'R' ? ROOK
				  : word[1] == 'B' ? BISHOP
				  : word[1] == 'N' ? KNIGHT
				  : NO_PIECE_TYPE;
	}

	Gamestate newgs;
	pos.make_move(Move::make(from, to, mt, promotion), newgs);
}
} // namespace CLI


int main()
{
std::cout << "Juicer V" << VERSION << " by Ryan Baker" << std::endl;

init_bitboards();
Gamestate gs;
Position pos;

	while (true)
	{
		std::string line;
		std::getline(std::cin, line);
		std::istringstream ss(line);
		
		std::string word;
		ss >> word;

		if (word == "quit" || word == "q") break;
		else if (word == "help" || word == "?") CLI::print_help_message();
		else if (word == "seed" || word == "s")
		{
			std::string fen;
			if (!(ss >> fen)) fen = STARTING_POS;
			pos.seed(fen, gs);
		}
		else if (word == "display" || word == "d") std::cout << pos.to_string() << std::endl;
		else if (word == "move" || word == "m") CLI::parse_move(ss, word, pos);
		else std::cout << "Invalid command. Type \"help\" for more information." << std::endl;
	}

	return 0;
}
