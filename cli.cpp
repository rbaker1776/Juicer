#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "juicer.h"
#include "engine.h"
#include "position.h"


namespace CLI
{
std::string seperator(120, '=');
std::string juicer = "Juicer V" + std::to_string(VERSION) + " by Ryan Baker";
void print_help_msg()
{
	std::cout << CLI::seperator << std::endl;
	std::cout << CLI::juicer << std::endl << std::endl;
	std::cout << "Commands:" << std::endl;
	std::cout << std::setw(12) << "\"seed\"" << " - Initialize the chess engine with a FEN string (default is startpos)" << std::endl;
	std::cout << std::setw(12) << "\"move\"" << " - Make a move using UCI notation:" << std::endl;
		std::cout << std::string(10, ' ') << "* move e2e4  // moves piece from e2 to e4" << std::endl;
		std::cout << std::string(10, ' ') << "* move e8g8  // moves piece from e8 to g8, performs castling if e8 is the king's square" << std::endl;
		std::cout << std::string(10, ' ') << "* move a7a8q // promotes the pawn on a7 to a queen on a8" << std::endl;
		std::cout << std::string(10, ' ') << "* move e2e4 c7c5 g1f3 d7d6 d2d4 // plays multiple moves" << std::endl;;
	std::cout << std::setw(12) << "\"show\"" << " - Show the current position" << std::endl;
	std::cout << std::setw(12) << "\"perft\"" << " - Get perft data for a given depth (default is perft 6)" << std::endl;
	std::cout << std::setw(12) << "\"help\"" << " - It would be weird if you didn't already understand this command" << std::endl;
	std::cout << std::setw(12) << "\"quit\"" << " - Quit Juicer" << std::endl;

	std::cout << CLI::seperator << std::endl;
}
}


int main(int argc, char* argv[])
{
	init_bitboards();

	Engine juicer;
	std::string command = std::string();
	std::string arg;
	const bool clargs = argc > 1;

	for (int i = 1; i < argc; ++i)
	{
		command += argv[i];
		if (i < argc - 1) command += " ";
	}

	while (true)
	{
		if (command == "") std::getline(std::cin, command);
		std::istringstream ss(command);
		ss >> arg;
		
		if 		(arg == "help") CLI::print_help_msg();
		else if (arg == "quit") break;
		else if (arg == "show") std::cout << juicer.position().to_string() << std::endl;
		else if (arg == "seed")
		{
			if (!(ss >> arg) || arg == "startpos") arg = STARTING_POS;
			juicer.set_position(arg);
		}
		else if (arg == "move")
		{
			while (ss >> arg)
				juicer.make_move(UCI::string_to_move(juicer.position(), arg));
		}
		else if (arg == "perft") 
		{
			if (!(ss >> arg)) arg = "6";
			uint64_t perft = juicer.perft(std::stoi(arg));
			std::cout << std::endl << "Nodes searched: " << perft << std::endl;
		}
		else std::cout << "Invalid command. Try \"help\" for valid commands." << std::endl;

		command = "";
		if (clargs) break;
	}

	return 0;
}
