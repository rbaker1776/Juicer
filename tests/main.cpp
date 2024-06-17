#include "test_bitboard_utils.cpp"
#include "test_magic_bitboards.cpp"
#include "minunit.h"
#include "../bitboard.h"


int main()
{
	init_bitboards();

	mu_suite(BitboardUtils::suite);
	mu_suite(MagicBitboards::suite);

	return 0;
}
