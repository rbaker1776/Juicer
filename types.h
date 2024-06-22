#ifndef TYPES_H_E51343A1D293
#define TYPES_H_E51343A1D293


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
	NUM_SQUARES = 64,
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
};

#define ENABLE_INCREMENT_OPERATORS(T) \
	constexpr T& operator++(T& x) { return x = T(int(x) + 1); } \
	constexpr T& operator--(T& x) { return x = T(int(x) - 1); }

ENABLE_INCREMENT_OPERATORS(Square);
ENABLE_INCREMENT_OPERATORS(Rank);
ENABLE_INCREMENT_OPERATORS(File);

#undef ENABLE_INCREMENT_OPERATORS


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
	NN = 2 * N,
	SS = 2 * S,
};

enum PieceType: int
{
	NO_PIECE_TYPE,
	PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING,
	ALL_PIECE_TYPES,
};

enum Piece: int
{
	NO_PIECE,
	W_PAWN = PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
	B_PAWN = PAWN + 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
};

enum Color: int
{
	WHITE,
	BLACK,
};


constexpr Square operator+(Square sq, Direction d) { return Square(int(sq) + int(d)); }
constexpr Square operator+(Square sq, int d) { return Square(int(sq) + d); }
constexpr Square operator-(Square sq, Direction d) { return Square(int(sq) - int(d)); }
constexpr Square operator-(Square sq, int d) { return Square(int(sq) - d); }

constexpr Square& operator+=(Square& sq, Direction d) { return sq = sq + d; }
constexpr Square& operator+=(Square& sq, int d) { return sq = sq + d; }
constexpr Square& operator-=(Square& sq, Direction d) { return sq = sq - d; }
constexpr Square& operator-=(Square& sq, int d) { return sq = sq - d; }

constexpr Color operator~(Color c) { return Color(1 - int(c)); }
constexpr Color& operator^=(Color& c, int n) { return c = Color(int(c) ^ n); }

constexpr PieceType type_of(Piece pc) { return PieceType(pc & 7); }


enum CastlingRights: int
{
	WHITE_OO  = 1,
	WHITE_OOO = 2,
	BLACK_OO  = 4,
	BLACK_OOO = 8,
};

constexpr int CASTLING_RIGHTS[2] = { WHITE_OO | WHITE_OOO, BLACK_OO | BLACK_OOO, };


enum MoveType: int
{
	NORMAL = 0,
	PROMOTION = 1 << 14,
	EN_PASSANT = 2 << 14,
	CASTLES = 3 << 14,
};

class Move
{
protected:
	uint16_t data;

public:
	Move() = default;
	constexpr explicit Move(uint16_t d): data(d) {}
	constexpr Move(Square from, Square to): data((from << 6) + to) {}
	
	static constexpr Move make(Square from, Square to, MoveType mt = NORMAL, PieceType pt = KNIGHT)
	{ return Move(mt + ((pt - KNIGHT) << 12) + (from << 6) + to); }

	static constexpr Move none() { return Move(A1, A1); }
	static constexpr Move null() { return Move(A2, A2); }

	constexpr Square from() const { return Square((data >> 6) & 63); }
	constexpr Square to() const { return Square(data & 63); }
	constexpr int from_to() const { return data & 0xfff; }

	constexpr MoveType type() const { return MoveType(data & (3 << 14)); }
	constexpr PieceType promotion_type() const { return PieceType(((data >> 12) & 3) + KNIGHT); }

	constexpr bool operator==(const Move& m) const { return this->data == m.data; }
	constexpr bool operator!=(const Move& m) const { return this->data != m.data; }
	
	constexpr explicit operator bool() const { return data != 0; }
	constexpr uint16_t get_data() const { return data; }
};


#endif // TYPES_H_E51343A1D293
