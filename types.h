#ifndef TYPES_H_0C3C886003F2
#define TYPES_H_0C3C886003F2

#include <string>



enum Square: int
{
	SQ_a1, SQ_b1, SQ_c1, SQ_d1, SQ_e1, SQ_f1, SQ_g1, SQ_h1,
	SQ_a2, SQ_b2, SQ_c2, SQ_d2, SQ_e2, SQ_f2, SQ_g2, SQ_h2,
	SQ_a3, SQ_b3, SQ_c3, SQ_d3, SQ_e3, SQ_f3, SQ_g3, SQ_h3,
	SQ_a4, SQ_b4, SQ_c4, SQ_d4, SQ_e4, SQ_f4, SQ_g4, SQ_h4,
	SQ_a5, SQ_b5, SQ_c5, SQ_d5, SQ_e5, SQ_f5, SQ_g5, SQ_h5,
	SQ_a6, SQ_b6, SQ_c6, SQ_d6, SQ_e6, SQ_f6, SQ_g6, SQ_h6,
	SQ_a7, SQ_b7, SQ_c7, SQ_d7, SQ_e7, SQ_f7, SQ_g7, SQ_h7,
	SQ_a8, SQ_b8, SQ_c8, SQ_d8, SQ_e8, SQ_f8, SQ_g8, SQ_h8,
	NUM_SQUARES = 64,
	NO_SQUARE,
};

enum Rank: int 
{
	RANK_1,
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
	FILE_A,
	FILE_B,
	FILE_C,
	FILE_D,
	FILE_E,
	FILE_F,
	FILE_G,
	FILE_H,
};

enum Direction 
{
	NORTH = 8,
	EAST = 1,
	SOUTH = -NORTH,
	WEST = -EAST,
	NORTH_EAST = NORTH + EAST,
	SOUTH_EAST = SOUTH + EAST,
	SOUTH_WEST = SOUTH + WEST,
	NORTH_WEST = NORTH + WEST,
	NORTH_2 = NORTH * 2,
	SOUTH_2 = SOUTH * 2,
};

enum Color
{
	WHITE = 0,
	BLACK = 1,
	NO_COLOR,
};

enum PieceType
{
	PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING,
	ALL_PIECES,
	NO_PIECE_TYPE,
};

enum Piece
{
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
	NO_PIECE,
};

enum Castle
{
	WHITE_SHORT = 1,
	WHITE_LONG = 2,
	BLACK_SHORT = 4,
	BLACK_LONG = 8,
	NO_CASTLE,
};


#define ENABLE_INCREMENT_OPERATORS(T) \
	inline T& operator++(T& x) { return x = T(int(x) + 1); } \
	inline T& operator--(T& x) { return x = T(int(x) - 1); }

ENABLE_INCREMENT_OPERATORS(Square);
ENABLE_INCREMENT_OPERATORS(File);
ENABLE_INCREMENT_OPERATORS(Rank);

#undef ENABLE_INCREMENT_OPERATORS


constexpr Square operator+(Square sq, Direction d) { return Square(int(sq) + int(d)); }
constexpr Square operator+(Square sq, int d) { return Square(int(sq) + d); }
constexpr Square operator-(Square sq, Direction d) { return Square(int(sq) - int(d)); }
constexpr Square operator-(Square sq, int d) { return Square(int(sq) - d); }
inline Square& operator+=(Square& sq, Direction d) { return sq = sq + d; }
inline Square& operator+=(Square& sq, int d) { return sq = sq + d; }
inline Square& operator-=(Square& sq, Direction d) { return sq = sq - d; }
inline Square& operator-=(Square& sq, int d) { return sq = sq - d; }

constexpr bool is_ok(Square s) { return s >= SQ_a1 && s <= SQ_h8; }

constexpr char file_to_char(File f) { return 'a' + int(f); }
constexpr char rank_to_char(Rank r) { return '1' + int(r); }

constexpr File char_to_file(char c) { return File(c - 'a'); }
constexpr Rank char_to_rank(char c) { return Rank(c - '1'); }


constexpr Piece char_to_piece(char c)
{
	switch (c)
	{
		case 'P': return W_PAWN;
		case 'N': return W_KNIGHT;
		case 'B': return W_BISHOP;
		case 'R': return W_ROOK;
		case 'Q': return W_QUEEN;
		case 'K': return W_KING;
		case 'p': return B_PAWN;
		case 'n': return B_KNIGHT;
		case 'b': return B_BISHOP;
		case 'r': return B_ROOK;
		case 'q': return B_QUEEN;
		case 'k': return B_KING;
		default: return Piece::NO_PIECE;
	}
}

constexpr PieceType char_to_pt(char c)
{
	switch(c)
	{
		case 'P': case 'p': return PAWN;
		case 'N': case 'n': return KNIGHT;
		case 'B': case 'b': return BISHOP;
		case 'R': case 'r': return ROOK;
		case 'Q': case 'q': return QUEEN;
		case 'K': case 'k': return KING;
		default: return PieceType::NO_PIECE_TYPE;
	}
}

constexpr Color char_to_color(char c)
{
	switch(c)
	{
		case 'P': case 'N': case 'B': case 'R': case 'Q': case 'K': return WHITE;
		case 'p': case 'n': case 'b': case 'r': case 'q': case 'k': return BLACK;
		default: return NO_COLOR;
	}
}

constexpr Castle char_to_castle(char c)
{
	switch (c)
	{
		case 'K': return Castle::WHITE_SHORT;
		case 'k': return Castle::BLACK_SHORT;
		case 'Q': return Castle::WHITE_LONG;
		case 'q': return Castle::BLACK_LONG;
		default: return Castle::NO_CASTLE;
	}
}

constexpr char castle_to_char(Castle c)
{
	switch (c)
	{
		case WHITE_SHORT: return 'K';
		case WHITE_LONG: return 'Q';
		case BLACK_SHORT: return 'k';
		case BLACK_LONG: return 'q';
		default: return '-';
	}
}

constexpr char piece_to_char(Piece p)
{
	switch(p)
	{
		case W_PAWN: return 'P';
		case W_KNIGHT: return 'N';
		case W_BISHOP: return 'B';
		case W_ROOK: return 'R';
		case W_QUEEN: return 'Q';
		case W_KING: return 'K';
		case B_PAWN: return 'p';
		case B_KNIGHT: return 'n';
		case B_BISHOP: return 'b';
		case B_ROOK: return 'r';
		case B_QUEEN: return 'q';
		case B_KING: return 'k';
		default: return ' ';
	}
}



#endif // TYPES_H_0C3C886003F2
