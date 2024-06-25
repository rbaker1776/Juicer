#ifndef POSITION_H_F035A5484F9D
#define POSITION_H_F035A5484F9D

#include <string>
#include "types.h"
#include "bitboard.h"
#include "juicer.h"


const std::string STARTING_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


typedef struct Gamestate
{
	int rule_50;
	int castling_rights;
	Square ep_square;
	Piece captured_piece = NO_PIECE;
	Gamestate* previous;
	uint64_t kings_guards[2];
	uint64_t pinners[2];
	uint64_t checking_squares[7];
	uint64_t checkers;

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
	void undo_move(const Move m);

	// Board information
	inline Piece piece_on(Square s) const { return mailbox[s]; }
	inline uint64_t pieces(PieceType pt = ALL_PIECE_TYPES) const { return bitboards[pt] | bitboards[pt | 8]; }
	template<typename... PieceTypes>
	inline uint64_t pieces(PieceType pt, PieceTypes... pts) const { return pieces(pt) | pieces(pts...); }
	inline uint64_t pieces(Color c, PieceType pt = ALL_PIECE_TYPES) const { return bitboards[pt | (c << 3)]; }
	template<typename... PieceTypes>
	inline uint64_t pieces(Color c, PieceTypes... pts) const { return pieces(c) & pieces(pts...); }
	inline uint64_t pieces(Piece pc) const { return bitboards[pc]; }
	inline Square king_sq(Color c) const { return ::lsb(bitboards[(c << 3) | KING]); }

	uint64_t attackers_to(Square s, uint64_t occupied = 0ull) const;

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

	inline uint64_t kings_guards(Color c) const { return this->state->kings_guards[c]; }
	inline uint64_t checkers() const { return this->state->checkers; }
	inline uint64_t checking_squares(PieceType pt) const { return this->state->checking_squares[pt]; }

	// State manipulation
	void update_check_info() const;
	void update_guards(Color c) const;
};


#endif // POSITION_H_F035A5484F9D
