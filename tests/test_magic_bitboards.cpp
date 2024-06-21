#include <iostream>
#include "../bitboard.h"
#include "../xorshiftstar64.h"
#include "minunit.h"





namespace MagicBitboards
{
	static void rook_attacks()
	{
		}

	static void bishop_attacks()
	{
			}

	static void suite()
	{
		mu_run(MagicBitboards::rook_attacks);
		mu_run(MagicBitboards::bishop_attacks);
	}
}
