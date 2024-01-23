#include <movegen.h>
#include <vector>
#include <string>


class Bitboard {
	public:
	 Bitboard(std::vector<int> squares);
	 Bitboard(std::vector<std::string> squares);
	 Bitboard();
	 	
	private:
	 uint64_t bits;
}


