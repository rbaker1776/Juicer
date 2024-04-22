#ifndef POSITION_H_7A8B21A194A6
#define POSITION_H_7A8B21A194A6

#include <string>
#include "types.h"
#include "bitboard.h"



class Position
{
	struct Gamestate
	{
		int castling_rights;
		int rule_50;
		int full_clock;
		int half_clock;
		Square ep_square;
	};

	public: Position()
	{
		gamestate = new Gamestate;
	}

	public: ~Position()
	{
		delete this->gamestate;
	}

	private: uint64_t pieces[6];
	private: uint64_t armies[2];
	private: bool white_to_move;
	private: struct Gamestate* gamestate;

	private: Piece get_piece(Square s) const
	{
		if (this->armies[WHITE] & s)
		{
			if (this->pieces[PAWN] & s) { return W_PAWN; }
			if (this->pieces[KNIGHT] & s) { return W_KNIGHT; }
			if (this->pieces[BISHOP] & s) { return W_BISHOP; }
			if (this->pieces[ROOK] & s) { return W_ROOK; }
			if (this->pieces[QUEEN] & s) { return W_QUEEN; }
			if (this->pieces[KING] & s) { return W_KING; }
			else { return NO_PIECE; }
		}
		else if (this->armies[BLACK] & s)
		{
			if (this->pieces[PAWN] & s) { return B_PAWN; }
			if (this->pieces[KNIGHT] & s) { return B_KNIGHT; }
			if (this->pieces[BISHOP] & s) { return B_BISHOP; }
			if (this->pieces[ROOK] & s) { return B_ROOK; }
			if (this->pieces[QUEEN] & s) { return B_QUEEN; }
			if (this->pieces[KING] & s) { return B_KING; }
			else { return NO_PIECE; }
		}
		else
		{
			return NO_PIECE;
		}
	}

	public: Position& seed(const std::string& fen);
	public: std::string to_string() const;
	public: std::string to_fen() const;
};
	


#endif // POSITION_H_7A8B21A194A6
