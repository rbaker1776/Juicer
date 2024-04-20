#ifndef TYPES_H_0C3C886003F2
#define TYPES_H_0C3C886003F2



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
	WHITE,
	BLACK,
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
};

enum Piece
{
	W_PAWN = PAWN,
	W_KNIGHT,
	W_BISHOP,
	W_ROOK,
	W_QUEEN,
	W_KING,
	B_PAWN = PAWN << 3,
	B_KNIGHT,
	B_BISHOP,
	B_ROOK,
	B_QUEEN,
	B_KING,
	NO_PIECE;
};


#define ENABLE_INCREMENT_OPERATORS(T) \
	inline T& operator++(T& x) { return x = T(int(x) + 1); } \
	inline T& operator--(T& x) { return x = T(int(x) - 1); }

ENABLE_INCREMENT_OPERATORS(Square);
ENABLE_INCREMENT_OPERATORS(File);
ENABLE_INCREMENT_OPERATORS(Rank);

#undef ENABLE_INCREMENT_OPERATORS


constexpr Square operator+(Square sq, Direction d) { return Square(int(sq) + int(d)); }
constexpr Square operator-(Square sq, Direction d) { return Square(int(sq) - int(d)); }
inline Square& operator+=(Square& sq, Direction d) { return sq = sq + d; }
inline Square& operator-=(Square& sq, Direction d) { return sq = sq - d; }


constexpr bool is_ok(Square s) { return s >= SQ_a1 && s <= SQ_h8; }


constexpr Piece char_to_pc(char c)
{
	switch (c)
	{
		case 'P': return W_PAWN;
		case 'N': return W_KNGIHT;
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
		default: return NO_PIECE;
	}
}



#endif // TYPES_H_0C3C886003F2
