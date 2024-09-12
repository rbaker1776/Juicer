#ifndef TYPES_H_8D2FD9177A41
#define TYPES_H_8D2FD9177A41

#include <cstdint>
#include "juicer.h"


enum Square: int
{
	A1 = 0, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	NO_SQUARE,
};

enum Rank: int
{
	RANK_1 = 0,
	RANK_2,
	RANK_3,
	RANK_4,
	RANK_5,
	RANK_6,
	RANK_7,
	RANK_8,
};

enum File: int
{
	FILE_A = 0,
	FILE_B,
	FILE_C,
	FILE_D,
	FILE_E,
	FILE_F,
	FILE_G,
	FILE_H,
	NO_FILE = 8,
};

// value corresponds to the bitshift associated with a step in a given direction
// one step north (ex. f4->f5) is a bitshift right by 8
enum Direction: int
{
	N = 8,
	S = -N,
	E = 1,
	W = -E,
	NW = N + W,
	NE = N + E,
	SW = S + W,
	SE = S + E,
	NN = N + N,
	SS = S + S,
	NO_DIRECTION = 0,
};

enum PieceType: int
{
	NO_PIECE_TYPE = 0,
	PAWN = 1,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING,
	ALL_PIECE_TYPES,
};

enum Piece: int
{
	NO_PIECE = 0,
	W_PAWN = PAWN,
	W_KNIGHT,
	W_BISHOP,
	W_ROOK,
	W_QUEEN,
	W_KING,
	B_PAWN = PAWN + 8,
	B_KNIGHT,
	B_BISHOP,
	B_ROOK,
	B_QUEEN,
	B_KING,
};

enum MoveType
{
	NORMAL,
	CASTLING,
	EN_PASSANT,
	PROMOTION,
};

// type bool instead of type int to ensure reduced template instantiations
enum Color: int
{
	WHITE = false,
	BLACK = true,
};


constexpr Square& operator++(Square& s) { return s = Square(int(s) + 1); }
constexpr Square& operator--(Square& s) { return s = Square(int(s) - 1); }

constexpr Rank& operator++(Rank& r) { return r = Rank(int(r) + 1); }
constexpr Rank& operator--(Rank& r) { return r = Rank(int(r) - 1); }

constexpr File& operator++(File& f) { return f = File(int(f) + 1); }
constexpr File& operator--(File& f) { return f = File(int(f) - 1); }

constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }

constexpr Square operator+(Square s, int d) { return Square(int(s) + d); }
constexpr Square operator-(Square s, int d) { return Square(int(s) - d); }

constexpr Square& operator+=(Square& s, Direction d) { return s = s + d; }
constexpr Square& operator-=(Square& s, Direction d) { return s = s - d; }

constexpr Color operator~(Color c) { return Color(!bool(c)); }

constexpr PieceType type_of(Piece pc) { return PieceType(pc & 7); }
constexpr Color color_of(Piece pc) { return Color(pc >> 3); }
constexpr Piece make_piece(Color c, PieceType pt) { return Piece((c << 3) | pt); }


struct Move
{
	MoveType type;
	Square from;
	Square to;
	PieceType piece;
	int promise;

	constexpr Move() = default;
	constexpr Move(uint32_t data): type(MoveType(data >> 16)), from(Square(data & 63)), to(Square((data >> 6) & 63)), piece(PieceType((data >> 12) & 15)) {}
	constexpr Move(MoveType mt, Square from, Square to, PieceType pt = KNIGHT): type(mt), from(from), to(to), piece(pt) {}
}; // struct Move


namespace Castling
{
	static constexpr uint64_t W_OOO {1ull << A1};
	static constexpr uint64_t W_OO  {1ull << H1};
	static constexpr uint64_t B_OOO {1ull << A8};
	static constexpr uint64_t B_OO  {1ull << H8};

	static constexpr uint64_t W_CASTLES {W_OOO | W_OO};
	static constexpr uint64_t B_CASTLES {B_OOO | B_OO};
	
	static constexpr uint64_t QUEENSIDE {W_OOO | B_OOO};
	static constexpr uint64_t KINGSIDE  {W_OO | B_OO};
	
	static constexpr uint64_t ANY {KINGSIDE | QUEENSIDE};
}


using value_t = int;

namespace Value
{
	static constexpr value_t DRAW = 0;
	static constexpr value_t MATE = 0xffff;
}


#endif // TYPES_H_8D2FD9177A41
