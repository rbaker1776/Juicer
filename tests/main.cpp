#include "test_bitboard_utils.cpp"
#include "test_magic_bitboards.cpp"
#include "test_movegen.cpp"
#include "test_fen_operations.cpp"
#include "minunit.h"
#include "../bitboard.h"


int main()
{
	init_bitboards();

	mu_suite(BitboardUtils::suite);
	mu_suite(MagicBitboards::suite);
	mu_suite(Movegen::suite);
	mu_suite(FENOperations::suite);

	return 0;
}
