#include <iostream>
#include <string>
#include <string_view>
#include "engine.h"
#include "juicer.h"


namespace CLI
{
	static constexpr std::string_view seperator = "========================================================================================================================";
	static constexpr std::string_view juicer_title = "Juicer V1 by Ryan Baker";
} // namespace CLI


int main(int argc, char* argv[])
{
	Engine juicer;

	std::string command = std::string();
	std::string arg;

	std::cout << std::endl << CLI::seperator << std::endl;
	std::cout << CLI::juicer_title << std::endl << std::endl;

	const bool clargs = argc > 1;
	for (int i = 1; i < argc; ++i)
		command += std::string(argv[i]) + " ";

	while (true)
	{
		if (command == "")
			std::getline(std::cin, command);
		std::istringstream ss(command);
		ss >> arg;

		if (arg == "show")
			std::cout << juicer.board().to_string() << std::endl;
		else if (arg == "move")
			while (ss >> arg)
				juicer.make_move(UCI::string_to_move(juicer.board(), juicer.boardstate(), juicer.gamestate(), arg));
		else if (arg == "perft")
		{
			if (!(ss >> arg)) arg = "4";
			uint64_t perft = juicer.perft<LEGAL, true>(std::stoi(arg));
			std::cout << std::endl << "Nodes searched: " << perft << std::endl << std::endl;
		}
		else if (arg == "captures")
		{
			for (const Move& capt: MoveList<CAPTURES>(juicer.board(), juicer.boardstate(), juicer.gamestate().ep_target))
				std::cout << UCI::move_to_string(capt) << std::endl;
		}

		command = "";

		if (clargs)
			return 0;
	}

	return 0;
}
