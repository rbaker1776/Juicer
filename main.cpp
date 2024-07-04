#include <iostream>
#include "position.h"


int main()
{
	Position pos = Position::startpos();
	Position pos2= pos.move<PAWN, WHITE, false>(square_to_bb(E2), square_to_bb(E4));
	Position pos3= pos2.move<PAWN, BLACK, false>(square_to_bb(E7), square_to_bb(E5));
	Position pos4= pos3.move<PAWN, WHITE, false>(square_to_bb(D2), square_to_bb(D4));
	Position pos5= pos4.move<PAWN, BLACK, false>(square_to_bb(D7), square_to_bb(D6));
	Position pos6= pos5.move<PAWN, WHITE, false>(square_to_bb(G2), square_to_bb(G3));
	std::cout << pos6.to_string() << std::endl;
	
	return 0;
}
