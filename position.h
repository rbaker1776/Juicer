#ifndef POSITION_H_F035A5484F9D
#define POSITION_H_F035A5484F9D

#include <string>
#include "types.h"
#include "juicer.h"


// Maximum number of legal moves in a chess position
// Arises from the incredibly unlikely: 1B1KNNBk/4Q1pp/2Q4Q/Q4Q2/3Q4/1Q4Q1/4Q3/R6R w - - 0 1
constexpr int MAX_LEGAL_MOVES = 218;


typedef struct Gamestate
{
	int rule_50;
	int castling_rights;
	Square ep_square;

	Gamestate* previous;
} Gamestate;


class Position
{
private:
	uint64_t bitboards[16];
	Piece mailbox[64];
	Color turn;
	Gamestate* state;
	int gameply;

	bool is_ok() const;

public:
	Position() = default;
	~Position() = default;

	Position& seed(const std::string& fen, Gamestate& gs);
	std::string fen() const;
	std::string to_string() const;

	void make_move(const Move m, Gamestate& gs, bool is_check = false);

	// Board information
	inline Piece piece_on(Square s) const { return mailbox[s]; }
	inline uint64_t pieces(Color c, PieceType pt = ALL_PIECE_TYPES) const { return bitboards[pt | (c << 3)]; }
	inline uint64_t pieces(Piece pc) const { return bitboards[pc]; }
	inline uint64_t pieces(PieceType pt = ALL_PIECE_TYPES) const { return bitboards[pt] | bitboards[pt | 8]; }

	// Board manipulation
	void place_piece(Piece pc, Square s);
	inline void place_piece(Color c, PieceType pt, Square s) { place_piece(Piece(pt | (c << 3)), s); }
	void remove_piece(Square s);
	void clear_board();

	// State information
	inline Color side_to_move() const { return this->turn; }
	inline int castling_rights() const { return this->state->castling_rights; }
	inline int rule_50() const { return this->state->rule_50; }
	inline Square ep_square() const { return this->state->ep_square; }
	inline int ply() const { return this->gameply; }
};


#endif // POSITION_H_F035A5484F9D
