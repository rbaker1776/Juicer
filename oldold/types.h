#ifndef TYPES_H_4EE98B6A35E4
#define TYPES_H_4EE98B6A35E4

#include <cstdint>



enum class Square: uint8_t
{
	A1 = 0,  B1, C1, D1, E1, F1, G1, H1,
	A2 = 8,  B2, C2, D2, E2, F2, G2, H2,
	A3 = 16, B3, C3, D3, E3, F3, G3, H3,
	A4 = 24, B4, C4, D4, E4, F4, G4, H4,
	A5 = 32, B5, C5, D5, E5, F5, G5, H5,
	A6 = 40, B6, C6, D6, E6, F6, G6, H6,
	A7 = 48, B7, C7, D7, E7, F7, G7, H7,
	A8 = 56, B8, C8, D8, E8, F8, G8, H8,
	NUM_SQUARES = 64,
	NO_SQUARE,
};

enum class PieceType: uint8_t
{
	PAWN = 0,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING,
	NUM_PIECES = 6,
	NO_PIECE,
};

enum class Color: uint8_t
{
	WHITE = 0,
	BLACK = 1,
	NO_COLOR,
};

enum class Rank: uint8_t
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

enum class File: uint8_t
{
	FILE_A = 0,
	FILE_B,
	FILE_C,
	FILE_D,
	FILE_E,
	FILE_F,
	FILE_G,
	FILE_H,
};

enum class Castle: uint8_t
{
	WHITE_SHORT = 1,
	WHITE_LONG = 2,
	BLACK_SHORT = 4,
	BLACK_LONG = 8,
	NO_CASTLE,
};


constexpr Square make_square(File f, Rank r) { return Square((r << 3) + f); }


constexpr PieceType char_to_pt(char c)
{
	switch(c)
	{
		case 'P': case 'p': return PieceType::PAWN;
		case 'N': case 'n': return PieceType::KNIGHT;
		case 'B': case 'b': return PieceType::BISHOP;
		case 'R': case 'r': return PieceType::ROOK;
		case 'Q': case 'q': return PieceType::QUEEN;
		case 'K': case 'k': return PieceType::KING;
		default: return PieceType::NO_PIECE;
	}
}

constexpr Color char_to_color(char c)
{
	switch(c)
	{
		case 'W': case 'w': return Color::WHITE;
		case 'B': case 'b': return Color::BLACK;
		default: return Color::NO_COLOR;
	}
}

constexpr Castle char_to_castle(char c)
{
	switch (c)
	{
		case 'K': return Castle::WHITE_SHORT;
		case 'k': return Castle::WHITE_LONG;
		case 'Q': return Castle::BLACK_SHORT;
		case 'q': return Castle::BLACK_LONG;
		default: return Castle::NO_CASTLE;
	}
}



#endif // TYPES_H_4EE98B6A35E4
