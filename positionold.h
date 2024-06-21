#ifndef POSITION_H_8FE3B408B44C
#define POSITION_H_8FE3B408B44C

#include <string>
#include "types.h"
#include "bitboard.h"


typedef struct Gamestate
{
	int castling_rights;
	int halfmoves;
	Square ep_square;

	Gamestate* previous;
} Gamestate;


class Position
{
private:
	Piece mailbox[64] = { NO_PIECE };
	uint64_t bitboards[16] = { 0ull };
	Color side_to_move;
	Gamestate* gamestate;
	int gameply;

public:
	Position() = default;
	Position(const std::string& fen, Gamestate& gs) { this->seed(fen, gs); }
	Position& seed(const std::string& fen, Gamestate& gs);
	~Position()	= default;

	inline Piece piece_on(Square s) const { return mailbox[s]; }
	void place_piece(Piece p, Square s);
	void reset();

	inline uint64_t pieces(PieceType pt = ALL_PIECE_TYPES) const { return bitboards[pt] | bitboards[pt + 8]; }
	inline uint64_t pieces(Color c, PieceType pt) const { return bitboards[8 * c + pt]; }
	inline uint64_t pieces(Color c) const { return bitboards[8 * c + ALL_PIECE_TYPES]; }

	inline Color get_turn() const { return this->side_to_move; }
	inline uint16_t get_castling_rights() const { return this->gamestate->castling_rights; }
	inline Square get_ep_square() const { return this->gamestate->ep_square; }
	inline uint16_t get_halfmoves() const { return this->gamestate->halfmoves; }
	inline int get_gameply() const { return this->gameply; }

	void make_move(Move m, Gamestate& new_gd, bool check);
};


inline void Position::place_piece(Piece p, Square s)
{
	mailbox[s] = p;
	bitboards[p | ALL_PIECE_TYPES] |= bitboards[p] |= s;
}

inline void Position::reset()
{
	for (uint64_t& bb: bitboards) { bb = 0ull; }
	for (Piece& pc: mailbox) { pc = NO_PIECE; }
	gameply = 0;
	gamestate->ep_square = NO_SQUARE;
	gamestate->halfmoves = 0;
	gamestate->castling_rights = 0;
}

std::string pos_to_string(const Position& pos);


#endif // POSITION_H_8FE3B408B44C
