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
	NO_RANK,
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
	NO_FILE,
};

constexpr Square& operator++(Square& s) { return s = Square(int(s) + 1); }
constexpr Square& operator--(Square& s) { return s = Square(int(s) - 1); }

constexpr Rank& operator++(Rank& r) { return r = Rank(int(r) + 1); }
constexpr Rank& operator--(Rank& r) { return r = Rank(int(r) - 1); }

constexpr File& operator++(File& f) { return f = File(int(f) + 1); }
constexpr File& operator--(File& f) { return f = File(int(f) - 1); }


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

constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }

constexpr Square operator+(Square s, int d) { return Square(int(s) + d); }
constexpr Square operator-(Square s, int d) { return Square(int(s) - d); }

constexpr Square& operator+=(Square& s, Direction d) { return s = s + d; }
constexpr Square& operator-=(Square& s, Direction d) { return s = s - d; }


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


// type bool instead of type int to ensure reduced template instantiations
enum Color: int
{
	WHITE = false,
	BLACK = true,
};

constexpr Color operator~(Color c) { return Color(!bool(c)); }

constexpr PieceType type_of(Piece pc) { return PieceType(pc & 7); }
constexpr Color color_of(Piece pc) { return Color(pc >> 3); }
constexpr Piece make_piece(Color c, PieceType pt) { return Piece((c << 3) | pt); }


enum Castling: int
{
	NO_CASTLES = 0,
	WHITE_OOO = 8,
	WHITE_OO  = 4,
	BLACK_OOO = 2,
	BLACK_OO  = 1,
	WHITE_CASTLES = WHITE_OOO | WHITE_OO,
	BLACK_CASTLES = BLACK_OOO | BLACK_OO,
	KING_SIDE = WHITE_OO | BLACK_OO,
	QUEEN_SIDE = WHITE_OOO | BLACK_OOO,
	ALL_CASTLES = KING_SIDE | QUEEN_SIDE,
};


enum MoveType
{
	NORMAL,
	CASTLING,
	EN_PASSANT,
	PROMOTION,
};

struct Move
{
public:
	MoveType type;
	Square from;
	Square to;
	PieceType piece;

	constexpr Move() = default;
	constexpr Move(MoveType mt, Square from, Square to, PieceType pt = KNIGHT): type(mt), from(from), to(to), piece(pt) {}
};


#endif // TYPES_H_8D2FD9177A41
