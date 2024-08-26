#ifndef POSITION_H_E76CAC7920F0
#define POSITION_H_E76CAC7920F0

#include <cassert>
#include <string>
#include <sstream>
#include <string_view>
#include "movement.h"
#include "zobrist.h"
#include "bitboard.h"
#include "types.h"
#include "juicer.h"


class Boardstate
{
public:
	const Color turn;
	const bool has_ep_pawn;
	const bool w_castle_ooo;
	const bool w_castle_oo;
	const bool b_castle_ooo;
	const bool b_castle_oo;

	consteval Boardstate(uint8_t pattern): 
		turn((pattern & 0b100000) ? BLACK : WHITE), has_ep_pawn(bool(pattern & 0b010000)),
		w_castle_ooo(bool(pattern & 0b001000)), w_castle_oo(bool(pattern & 0b000100)),
		b_castle_ooo(bool(pattern & 0b000010)), b_castle_oo(bool(pattern & 0b000001))
	{}

	consteval bool can_castle_queenside() const { return turn == WHITE ? w_castle_ooo : b_castle_ooo; }
	consteval bool can_castle_kingside()  const { return turn == WHITE ? w_castle_oo : b_castle_oo; }

	inline bool can_castle_queenside(uint64_t seen, uint64_t occupied, uint64_t rook) const;
	inline bool can_castle_kingside(uint64_t seen, uint64_t occupied, uint64_t rook) const;

private:
	static constexpr uint64_t W_EMPTY_OOO = B1 | C1 | D1;
	static constexpr uint64_t B_EMPTY_OOO = B8 | C8 | D8;
	static constexpr uint64_t W_EMPTY_OO = F1 | G1;
	static constexpr uint64_t B_EMPTY_OO = F8 | G8;

	static constexpr uint64_t W_UNSEEN_OOO = E1 | D1 | C1;
	static constexpr uint64_t B_UNSEEN_OOO = E8 | D8 | C8;
	static constexpr uint64_t W_UNSEEN_OO = E1 | F1 | G1;
	static constexpr uint64_t B_UNSEEN_OO = E8 | F8 | G8;
}; // struct Boardstate


struct Position;


namespace FEN
{
	static constexpr std::string_view STARTPOS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	static constexpr std::string_view PIECES = " PNBRQK  pnbrqk";

	static constexpr uint64_t bitboard(std::string_view fen, Piece pc);
	static constexpr Color turn(std::string_view fen);
	static constexpr uint64_t castling_rights(std::string_view fen);
	static constexpr Square ep_target(std::string_view fen);
	static constexpr int halfmoves(std::string_view fen);
	static constexpr int gameply(std::string_view fen);

	static constexpr uint64_t zobrist_key(std::string_view fen);

	[[maybe_unused]] static std::string string(const Position& pos);
} // namespace FEN


namespace Zobrist
{
	static constexpr uint64_t hash(const Position& pos);
}


struct Position
{
	const uint64_t wp, wn, wb, wr, wq, wk;
	const uint64_t bp, bn, bb, br, bq, bk;
	const uint64_t w_pieces, b_pieces;
	const uint64_t pieces;

	const Color turn;
	const uint64_t castling;

	const Square ep_target;
	const int rule_50;
	const int ply;
	const uint64_t key;

	constexpr Position(
		uint64_t wp, uint64_t wn, uint64_t wb, uint64_t wr, uint64_t wq, uint64_t wk,
		uint64_t bp, uint64_t bn, uint64_t bb, uint64_t br, uint64_t bq, uint64_t bk,
		Color turn, uint64_t cr, Square ep, int r50, int ply, uint64_t key
	):
		wp(wp), wn(wn), wb(wb), wr(wr), wq(wq), wk(wk),
		bp(bp), bn(bn), bb(bb), br(br), bq(bq), bk(bk),
		w_pieces(wp | wn | wb | wr | wq | wk),
		b_pieces(bp | bn | bb | br | bq | bk),
		pieces(w_pieces | b_pieces),
		turn(turn), castling(cr), ep_target(ep), 
		rule_50(r50), ply(ply), key(key)
	{}

	constexpr Position(std::string_view fen): Position(
		FEN::bitboard(fen, W_PAWN), FEN::bitboard(fen, W_KNIGHT), FEN::bitboard(fen, W_BISHOP), FEN::bitboard(fen, W_ROOK), FEN::bitboard(fen, W_QUEEN), FEN::bitboard(fen, W_KING),
		FEN::bitboard(fen, B_PAWN), FEN::bitboard(fen, B_KNIGHT), FEN::bitboard(fen, B_BISHOP), FEN::bitboard(fen, B_ROOK), FEN::bitboard(fen, B_QUEEN), FEN::bitboard(fen, B_KING),
		FEN::turn(fen), FEN::castling_rights(fen), FEN::ep_target(fen), 
		FEN::halfmoves(fen), FEN::gameply(fen), FEN::zobrist_key(fen)
	) {}

	static constexpr Position startpos() { return Position(FEN::STARTPOS); }

	forceinline Position make_move(const Move& m) const;

	template<PieceType Pt>
	forceinline Position make_move(Square from, Square to) const;

	template<PieceType Pt>
	forceinline Position make_promotion(Square from, Square to) const;

	forceinline Piece piece_on(Square s) const;

	template<Color C>
	inline Piece piece_on(Square s) const;

	template<Color C, PieceType Pt = ALL_PIECE_TYPES>
	forceinline uint64_t bitboard() const;

	template<Color C, PieceType... Pts>
	inline uint64_t bitboards() const { return (bitboard<C, Pts>() | ...); }

	template<Color C>
	inline Square king_sq() const;

	inline uint8_t boardstate_pattern() const;

	template<Square S>
	inline uint64_t key_update_castles() const;

	template<Color C>
	inline uint64_t key_update_enpassant(Square from, Square to) const;

	template<Color C, PieceType Pt>
	inline uint64_t key_update_capture(Square from, Square to) const;

	template<Color C, PieceType Pt>
	inline uint64_t key_update(Square from, Square to) const;

	template<Color C, PieceType Pt>
	inline uint64_t key_update_promotion_capture(Square from, Square to) const;

	template<Color C, PieceType Pt>
	inline uint64_t key_update_promotion(Square from, Square to) const;
}; // struct Position


inline bool Boardstate::can_castle_queenside(uint64_t seen, uint64_t occupied, uint64_t rook) const
{
	if (turn == WHITE && w_castle_ooo)
	{
		if (occupied & W_EMPTY_OOO || seen & W_UNSEEN_OOO)
			return false;
		if (rook & Castling::W_OOO)
			return true;
	}
	else if (turn == BLACK && b_castle_ooo)
	{
		if (occupied & B_EMPTY_OOO || seen & B_UNSEEN_OOO)
			return false;
		if (rook & Castling::B_OOO)
			return true;
	}
	return false;
}

inline bool Boardstate::can_castle_kingside(uint64_t seen, uint64_t occupied, uint64_t rook) const
{
	if (turn == WHITE && w_castle_oo)
	{
		if (occupied & W_EMPTY_OO || seen & W_UNSEEN_OO) 
			return false;
		if (rook & Castling::W_OO)
			return true;
	}
	else if (turn == BLACK && b_castle_oo)
	{
		if (occupied & B_EMPTY_OO || seen & B_UNSEEN_OO) 
			return false;
		if (rook & Castling::B_OO)
			return true;
	}
	return false;
}


static constexpr uint64_t FEN::bitboard(std::string_view fen, Piece pc)
{
	const char* token = &fen[0];
	uint64_t bb = 0;

	for (Square s = A8; !(*token == ' '); ++token)
	{
		if (*token >= '1' && *token <= '8')
			s += Direction((*token - '0') * Direction::E);
		else if (*token == '/')
			s += Direction::SS;
		else if (FEN::PIECES[pc] == *token)
		{
			bb |= s;
			++s;
		}
		else
			++s;
	}

	return bb;
}

static constexpr Color FEN::turn(std::string_view fen)
{
	const char* token = &fen[0];
	while (!(*token++ == ' ')) {}
	return (*token == 'w' ? WHITE : BLACK);
}

static constexpr uint64_t FEN::castling_rights(std::string_view fen)
{
	const char* token = &fen[0];
	uint64_t cr = 0;

	for (int ws = 0; ws < 2; ws += (*token++ == ' ')) {}

	for (; !(*token == ' '); ++token)
	{
		switch (*token)
		{
			case 'K': cr |= Castling::W_OO;  break;
			case 'Q': cr |= Castling::W_OOO; break;
			case 'k': cr |= Castling::B_OO;  break;
			case 'q': cr |= Castling::B_OOO; break;
		}
	}

	return cr;
}

static constexpr Square FEN::ep_target(std::string_view fen)
{
	const char* token = &fen[0];

	while (!(*token++ == ' ')) {}

	Color turn = *token++ == 'w' ? WHITE : BLACK;

	++token;
	while (!(*token++ == ' ')) {}

	char rank, file;
	if (((file = *token++) && (file >= 'a' && file <= 'h')) && ((rank = *token++) && (rank >= '1' && rank <= '8')))
		return make_square(File(file - 'a'), Rank(rank - '1')) + (turn == WHITE ? Direction::S : Direction::N);

	return NO_SQUARE;
}

static constexpr int FEN::halfmoves(std::string_view fen)
{
	const char* token = &fen[0];
	int rule_50 = 0;

	for (int i = 0; i < 4; i += (*token++ == ' ')) {}

	while (*token >= '0' && *token <= '9')
	{
		rule_50 *= 10;
		rule_50 += *token - '0';
		++token;
	}

	return rule_50;
}

static constexpr int FEN::gameply(std::string_view fen)
{
	const char* token = &fen[0];
	int gameply = 0;

	while (!(*token++ == ' ')) {}

	Color turn = *token++ == 'w' ? WHITE : BLACK;

	++token;
	for (int i = 0; i < 3; i += (*token++ == ' ')) {}
	
	while (*token >= '0' && *token <= '9')
	{
		gameply *= 10;
		gameply += *token - '0';
		++token;
	}

	return std::max(2 * (gameply - 1), 0) + (turn == BLACK);
}

static constexpr uint64_t FEN::zobrist_key(std::string_view fen)
{
	uint64_t key = 0;

	for (Piece pc: { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING, B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING })
		for (uint64_t bb = FEN::bitboard(fen, pc); bb; )
			key ^= PIECE_SQUARE_KEYS[pc][pop_square(bb)];

	key ^= (FEN::turn(fen) == WHITE ? 0 : TURN_KEY);

	uint64_t cr = FEN::castling_rights(fen);
	if (cr & Castling::W_OOO)
		key ^= W_OOO_KEY;
	if (cr & Castling::W_OO)
		key ^= W_OO_KEY;
	if (cr & Castling::B_OOO)
		key ^= B_OOO_KEY;
	if (cr & Castling::B_OO)
		key ^= B_OO_KEY;

	const Square ep = FEN::ep_target(fen);
	if (ep != NO_SQUARE)
		key ^= EP_FILE_KEYS[file_of(ep)];

	return key;
}

static std::string FEN::string(const Position& pos)
{
	std::ostringstream ss;

	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		int empty_cnt;
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			for (empty_cnt = 0; ~pos.pieces & make_square(f, r) && f <= FILE_H; ++f)
				++empty_cnt;

			if (empty_cnt)
				ss << empty_cnt;

			if (f <= FILE_H)
				ss << PIECES[pos.piece_on(make_square(f, r))];
		}

		if (r >= RANK_2)
			ss << '/';
	}

	ss << (pos.turn == WHITE ? " w " : " b ");

	if (pos.castling & Castling::W_OO)
		ss << 'K';
	if (pos.castling & Castling::W_OOO)
		ss << 'Q';
	if (pos.castling & Castling::B_OO)
		ss << 'k';
	if (pos.castling & Castling::B_OOO)
		ss << 'q';
	if (!(pos.castling & Castling::ANY))
		ss << '-';

	ss << ' ';
	if (pos.ep_target == NO_SQUARE)
		ss << '-';
	else
	{
		ss << char('a' + ((pos.ep_target + pawn_step(pos.turn)) & 7));
		ss << char('1' + ((pos.ep_target + pawn_step(pos.turn)) >> 3));
	}

	ss << ' ' << pos.rule_50 << ' ' << (1 + (pos.ply - (pos.turn == BLACK)) / 2);
	
	return ss.str();
}


static constexpr uint64_t Zobrist::hash(const Position& pos)
{
	uint64_t key = 0;

	for (uint64_t wp = pos.wp; wp; key ^= PIECE_SQUARE_KEYS[W_PAWN][pop_square(wp)]) {}
	for (uint64_t wn = pos.wn; wn; key ^= PIECE_SQUARE_KEYS[W_KNIGHT][pop_square(wn)]) {}
	for (uint64_t wb = pos.wb; wb; key ^= PIECE_SQUARE_KEYS[W_BISHOP][pop_square(wb)]) {}
	for (uint64_t wr = pos.wr; wr; key ^= PIECE_SQUARE_KEYS[W_ROOK][pop_square(wr)]) {}
	for (uint64_t wq = pos.wq; wq; key ^= PIECE_SQUARE_KEYS[W_QUEEN][pop_square(wq)]) {}

	for (uint64_t bp = pos.bp; bp; key ^= PIECE_SQUARE_KEYS[B_PAWN][pop_square(bp)]) {}
	for (uint64_t bn = pos.bn; bn; key ^= PIECE_SQUARE_KEYS[B_KNIGHT][pop_square(bn)]) {}
	for (uint64_t bb = pos.bb; bb; key ^= PIECE_SQUARE_KEYS[B_BISHOP][pop_square(bb)]) {}
	for (uint64_t br = pos.br; br; key ^= PIECE_SQUARE_KEYS[B_ROOK][pop_square(br)]) {}
	for (uint64_t bq = pos.bq; bq; key ^= PIECE_SQUARE_KEYS[B_QUEEN][pop_square(bq)]) {}

	key ^= PIECE_SQUARE_KEYS[W_KING][pos.king_sq<WHITE>()];
	key ^= PIECE_SQUARE_KEYS[B_KING][pos.king_sq<BLACK>()];

	key ^= (pos.turn == WHITE ? 0 : TURN_KEY);

	if (pos.castling & Castling::W_OOO)
		key ^= W_OOO_KEY;
	if (pos.castling & Castling::W_OO)
		key ^= W_OO_KEY;
	if (pos.castling & Castling::B_OOO)
		key ^= B_OOO_KEY;
	if (pos.castling & Castling::B_OO)
		key ^= B_OO_KEY;

	key ^= EP_FILE_KEYS[file_of(pos.ep_target)];

	return key;
}


// compiler estimates this function to be too costly to inline
// forceinlining make_move() eliminates ~400 missed gvn optimizations for +900'000 NPS perft
forceinline Position Position::make_move(const Move& m) const
{
	switch (m.type)
	{
		case NORMAL:
			switch (m.piece)
			{
				case PAWN:   return make_move<PAWN>(m.from, m.to);
				case KNIGHT: return make_move<KNIGHT>(m.from, m.to);
				case BISHOP: return make_move<BISHOP>(m.from, m.to);
				case ROOK:   return make_move<ROOK>(m.from, m.to);
				case QUEEN:  return make_move<QUEEN>(m.from, m.to);
				case KING:   return make_move<KING>(m.from, m.to);
				default:
					std::cerr << "Invalid PieceType: " << m.piece << std::endl;
			}
		case PROMOTION:
			switch (m.piece)
			{
				case QUEEN:  return make_promotion<QUEEN>(m.from, m.to);
				case KNIGHT: return make_promotion<KNIGHT>(m.from, m.to);
				case ROOK:   return make_promotion<ROOK>(m.from, m.to);
				case BISHOP: return make_promotion<BISHOP>(m.from, m.to);
				default:
					std::cerr << "Invalid PieceType for promotion: " << m.piece << std::endl;
			}
		case CASTLING:
			switch (m.to)
			{
				case C1: return Position(wp, wn, wb, wr ^ (A1 | D1), wq, wk ^ (E1 | C1), bp, bn, bb, br, bq, bk, BLACK, castling & Castling::B_CASTLES, NO_SQUARE, rule_50+1, ply+1, key_update_castles<C1>());
				case G1: return Position(wp, wn, wb, wr ^ (H1 | F1), wq, wk ^ (E1 | G1), bp, bn, bb, br, bq, bk, BLACK, castling & Castling::B_CASTLES, NO_SQUARE, rule_50+1, ply+1, key_update_castles<G1>());
				case C8: return Position(wp, wn, wb, wr, wq, wk, bp, bn, bb, br ^ (A8 | D8), bq, bk ^ (E8 | C8), WHITE, castling & Castling::W_CASTLES, NO_SQUARE, rule_50+1, ply+1, key_update_castles<C8>());
				case G8: return Position(wp, wn, wb, wr, wq, wk, bp, bn, bb, br ^ (H8 | F8), bq, bk ^ (E8 | G8), WHITE, castling & Castling::W_CASTLES, NO_SQUARE, rule_50+1, ply+1, key_update_castles<G8>());
				default:
					std::cerr << "Invalid castling move: " << m.to << "->" << m.from << std::endl;
			}
		case EN_PASSANT:
			const uint64_t sqs = m.from | m.to;
			if (turn == WHITE)
				return Position(wp ^ sqs, wn, wb, wr, wq, wk, bp & ~square_to_bb(ep_target), bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, 0, ply+1, key_update_enpassant<WHITE>(m.from, m.to));
			else
				return Position(wp & ~square_to_bb(ep_target), wn, wb, wr, wq, wk, bp ^ sqs, bn, bb, br, bq, bk, WHITE, castling, NO_SQUARE, 0, ply+1, key_update_enpassant<BLACK>(m.from, m.to));
	}
}

// clang deems this function too costly to inline
// forceinlining allows clang to complete an extra 84 slp-vectorizer optimizations for +3'800'000 NPS perft
template<PieceType Pt>
forceinline Position Position::make_move(Square from, Square to) const
{
	const uint64_t sqs = from | to;
	const bool is_capture = pieces & to;

	if (is_capture)
	{
		const uint64_t rem = ~square_to_bb(to);

		if (turn == WHITE)
		{
			if constexpr (Pt == PAWN)   return Position(wp ^ sqs, wn, wb, wr, wq, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0, ply+1, key_update_capture<WHITE, PAWN>(from, to));
			if constexpr (Pt == KNIGHT) return Position(wp, wn ^ sqs, wb, wr, wq, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0, ply+1, key_update_capture<WHITE, KNIGHT>(from, to));
			if constexpr (Pt == BISHOP) return Position(wp, wn, wb ^ sqs, wr, wq, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0, ply+1, key_update_capture<WHITE, BISHOP>(from, to));
			if constexpr (Pt == ROOK)   return Position(wp, wn, wb, wr ^ sqs, wq, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & ~sqs, NO_SQUARE, 0, ply+1, key_update_capture<WHITE, ROOK>(from, to));
			if constexpr (Pt == QUEEN)  return Position(wp, wn, wb, wr, wq ^ sqs, wk, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0, ply+1, key_update_capture<WHITE, QUEEN>(from, to));
			if constexpr (Pt == KING)   return Position(wp, wn, wb, wr, wq, wk ^ sqs, bp & rem, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem & Castling::B_CASTLES, NO_SQUARE, 0, ply+1, key_update_capture<WHITE, KING>(from, to));
		}
		else // (turn == BLACK)
		{
			if constexpr (Pt == PAWN)   return Position(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ sqs, bn, bb, br, bq, bk, WHITE, castling & rem, NO_SQUARE, 0, ply+1, key_update_capture<BLACK, PAWN>(from, to));
			if constexpr (Pt == KNIGHT) return Position(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn ^ sqs, bb, br, bq, bk, WHITE, castling & rem, NO_SQUARE, 0, ply+1, key_update_capture<BLACK, KNIGHT>(from, to));
			if constexpr (Pt == BISHOP) return Position(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn, bb ^ sqs, br, bq, bk, WHITE, castling & rem, NO_SQUARE, 0, ply+1, key_update_capture<BLACK, BISHOP>(from, to));
			if constexpr (Pt == ROOK)   return Position(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn, bb, br ^ sqs, bq, bk, WHITE, castling & ~sqs, NO_SQUARE, 0, ply+1, key_update_capture<BLACK, ROOK>(from, to));
			if constexpr (Pt == QUEEN)  return Position(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn, bb, br, bq ^ sqs, bk, WHITE, castling & rem, NO_SQUARE, 0, ply+1, key_update_capture<BLACK, QUEEN>(from, to));
			if constexpr (Pt == KING)   return Position(wp & rem, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp, bn, bb, br, bq, bk ^ sqs, WHITE, castling & rem & Castling::W_CASTLES, NO_SQUARE, 0, ply+1, key_update_capture<BLACK, KING>(from, to));
		}
	}
	else // (!is_capture)
	{
		if (turn == WHITE)
		{
			if constexpr (Pt == PAWN)
			{
				const Square ep = (SQUARE_DISTANCE[to][from] == 2 && bp & PAWN_ATTACKS[WHITE][from + pawn_step<WHITE>()]) ? to : NO_SQUARE;
				return Position(wp ^ sqs, wn, wb, wr, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling, ep, rule_50+1, ply+1, key_update<WHITE, Pt>(from, to) ^ EP_FILE_KEYS[file_of(ep)]);
			}
			if constexpr (Pt == KNIGHT) return Position(wp, wn ^ sqs, wb, wr, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, rule_50+1, ply+1, key_update<WHITE, Pt>(from, to));
			if constexpr (Pt == BISHOP) return Position(wp, wn, wb ^ sqs, wr, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, rule_50+1, ply+1, key_update<WHITE, Pt>(from, to));
			if constexpr (Pt == ROOK)   return Position(wp, wn, wb, wr ^ sqs, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling & ~sqs, NO_SQUARE, rule_50+1, ply+1, key_update<WHITE, Pt>(from, to));
			if constexpr (Pt == QUEEN)  return Position(wp, wn, wb, wr, wq ^ sqs, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, rule_50+1, ply+1, key_update<WHITE, Pt>(from, to));
			if constexpr (Pt == KING)   return Position(wp, wn, wb, wr, wq, wk ^ sqs, bp, bn, bb, br, bq, bk, BLACK, castling & Castling::B_CASTLES, NO_SQUARE, rule_50+1, ply+1, key_update<WHITE, Pt>(from, to));
		}
		else // (turn == BLACK)
		{
			if constexpr (Pt == PAWN)
			{
				const Square ep = (SQUARE_DISTANCE[to][from] == 2 && wp & PAWN_ATTACKS[BLACK][from + pawn_step<BLACK>()]) ? to : NO_SQUARE;
				return Position(wp, wn, wb, wr, wq, wk, bp ^ sqs, bn, bb, br, bq, bk, WHITE, castling, ep, rule_50+1, ply+1, key_update<BLACK, Pt>(from, to) ^ EP_FILE_KEYS[file_of(ep)]);
			}
			if constexpr (Pt == KNIGHT) return Position(wp, wn, wb, wr, wq, wk, bp, bn ^ sqs, bb, br, bq, bk, WHITE, castling, NO_SQUARE, rule_50+1, ply+1, key_update<BLACK, Pt>(from, to));
			if constexpr (Pt == BISHOP) return Position(wp, wn, wb, wr, wq, wk, bp, bn, bb ^ sqs, br, bq, bk, WHITE, castling, NO_SQUARE, rule_50+1, ply+1, key_update<BLACK, Pt>(from, to));
			if constexpr (Pt == ROOK)   return Position(wp, wn, wb, wr, wq, wk, bp, bn, bb, br ^ sqs, bq, bk, WHITE, castling & ~sqs, NO_SQUARE, rule_50+1, ply+1, key_update<BLACK, Pt>(from, to));
			if constexpr (Pt == QUEEN)  return Position(wp, wn, wb, wr, wq, wk, bp, bn, bb, br, bq ^ sqs, bk, WHITE, castling, NO_SQUARE, rule_50+1, ply+1, key_update<BLACK, Pt>(from, to));
			if constexpr (Pt == KING)   return Position(wp, wn, wb, wr, wq, wk, bp, bn, bb, br, bq, bk ^ sqs, WHITE, castling & Castling::W_CASTLES, NO_SQUARE, rule_50+1, ply+1, key_update<BLACK, Pt>(from, to));
		}
	}
}

template<PieceType Pt>
forceinline Position Position::make_promotion(Square from, Square to) const
{
	static_assert(Pt >= KNIGHT && Pt <= QUEEN);

	const bool is_capture = pieces & to;

	if (is_capture)
	{
		const uint64_t rem = ~square_to_bb(to);

		if (turn == WHITE)
		{
			if constexpr (Pt == QUEEN)  return Position(wp ^ from, wn, wb, wr, wq ^ to, wk, bp, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0, ply+1, key_update_promotion_capture<WHITE, QUEEN>(from, to));
			if constexpr (Pt == KNIGHT) return Position(wp ^ from, wn ^ to, wb, wr, wq, wk, bp, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0, ply+1, key_update_promotion_capture<WHITE, KNIGHT>(from, to));
			if constexpr (Pt == ROOK)   return Position(wp ^ from, wn, wb, wr ^ to, wq, wk, bp, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0, ply+1, key_update_promotion_capture<WHITE, ROOK>(from, to));
			if constexpr (Pt == BISHOP) return Position(wp ^ from, wn, wb ^ to, wr, wq, wk, bp, bn & rem, bb & rem, br & rem, bq & rem, bk, BLACK, castling & rem, NO_SQUARE, 0, ply+1, key_update_promotion_capture<WHITE, BISHOP>(from, to));
		}
		else // (turn == BLACK)
		{
			if constexpr (Pt == QUEEN)  return Position(wp, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ from, bn, bb, br, bq ^ to, bk, WHITE, castling & rem, NO_SQUARE, 0, ply+1, key_update_promotion_capture<BLACK, QUEEN>(from, to));
			if constexpr (Pt == KNIGHT) return Position(wp, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ from, bn ^ to, bb, br, bq, bk, WHITE, castling & rem, NO_SQUARE, 0, ply+1, key_update_promotion_capture<BLACK, KNIGHT>(from, to));
			if constexpr (Pt == ROOK)   return Position(wp, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ from, bn, bb, br ^ to, bq, bk, WHITE, castling & rem, NO_SQUARE, 0, ply+1, key_update_promotion_capture<BLACK, ROOK>(from, to));
			if constexpr (Pt == BISHOP) return Position(wp, wn & rem, wb & rem, wr & rem, wq & rem, wk, bp ^ from, bn, bb ^ to, br, bq, bk, WHITE, castling & rem, NO_SQUARE, 0, ply+1, key_update_promotion_capture<BLACK, BISHOP>(from, to));
		}
	}
	else // (!is_capture)
	{
		if (turn == WHITE)
		{
			if constexpr (Pt == QUEEN)  return Position(wp ^ from, wn, wb, wr, wq ^ to, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, rule_50+1, ply+1, key_update_promotion<WHITE, QUEEN>(from, to));
			if constexpr (Pt == KNIGHT) return Position(wp ^ from, wn ^ to, wb, wr, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, rule_50+1, ply+1, key_update_promotion<WHITE, KNIGHT>(from, to));
			if constexpr (Pt == ROOK)   return Position(wp ^ from, wn, wb, wr ^ to, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, rule_50+1, ply+1, key_update_promotion<WHITE, ROOK>(from, to));
			if constexpr (Pt == BISHOP) return Position(wp ^ from, wn, wb ^ to, wr, wq, wk, bp, bn, bb, br, bq, bk, BLACK, castling, NO_SQUARE, rule_50+1, ply+1, key_update_promotion<WHITE, BISHOP>(from, to));
		}
		else // (turn == BLACK)
		{
			if constexpr (Pt == QUEEN)  return Position(wp, wn, wb, wr, wq, wk, bp ^ from, bn, bb, br, bq ^ to, bk, WHITE, castling, NO_SQUARE, rule_50+1, ply+1, key_update_promotion<BLACK, QUEEN>(from, to));
			if constexpr (Pt == KNIGHT) return Position(wp, wn, wb, wr, wq, wk, bp ^ from, bn ^ to, bb, br, bq, bk, WHITE, castling, NO_SQUARE, rule_50+1, ply+1, key_update_promotion<BLACK, KNIGHT>(from, to));
			if constexpr (Pt == ROOK)   return Position(wp, wn, wb, wr, wq, wk, bp ^ from, bn, bb, br ^ to, bq, bk, WHITE, castling, NO_SQUARE, rule_50+1, ply+1, key_update_promotion<BLACK, ROOK>(from, to));
			if constexpr (Pt == BISHOP) return Position(wp, wn, wb, wr, wq, wk, bp ^ from, bn, bb ^ to, br, bq, bk, WHITE, castling, NO_SQUARE, rule_50+1, ply+1, key_update_promotion<BLACK, BISHOP>(from, to));
		}
	}
}

forceinline Piece Position::piece_on(Square s) const
{
	if (w_pieces & s)
	{
		if (wp & s) return W_PAWN;
		if (wn & s) return W_KNIGHT;
		if (wb & s) return W_BISHOP;
		if (wr & s) return W_ROOK;
		if (wq & s) return W_QUEEN;
		return W_KING;
	}
	else if (b_pieces & s)
	{
		if (bp & s) return B_PAWN;
		if (bn & s) return B_KNIGHT;
		if (bb & s) return B_BISHOP;
		if (br & s) return B_ROOK;
		if (bq & s) return B_QUEEN;
		return B_KING;
	}
	else
		return NO_PIECE;
}

template<Color C>
inline Piece Position::piece_on(Square s) const
{
	if constexpr (C == WHITE)
	{
		if (wp & s) return W_PAWN;
		if (wn & s) return W_KNIGHT;
		if (wb & s) return W_BISHOP;
		if (wr & s) return W_ROOK;
		if (wq & s) return W_QUEEN;
		if (wk & s) return W_KING;
	}
	else // (C == BLACK)
	{
		if (bp & s) return B_PAWN;
		if (bn & s) return B_KNIGHT;
		if (bb & s) return B_BISHOP;
		if (br & s) return B_ROOK;
		if (bq & s) return B_QUEEN;
		if (bk & s) return B_KING;
	}
	return NO_PIECE;
}

// forceinlining Position::bitboard() removes ~10,000 missed gvn optimizations for +1'000'000 NPS
template<Color C, PieceType Pt>
forceinline uint64_t Position::bitboard() const
{
	static_assert(C == WHITE || C == BLACK);
	static_assert(Pt >= PAWN && Pt <= ALL_PIECE_TYPES);

	if constexpr (C == WHITE)
	{
		if constexpr (Pt == PAWN)   return wp;
		if constexpr (Pt == KNIGHT) return wn;
		if constexpr (Pt == BISHOP) return wb;
		if constexpr (Pt == ROOK)   return wr;
		if constexpr (Pt == QUEEN)  return wq;
		if constexpr (Pt == KING)   return wk;
		if constexpr (Pt == ALL_PIECE_TYPES)
			return w_pieces;
	}
	else
	{
		if constexpr (Pt == PAWN)   return bp;
		if constexpr (Pt == KNIGHT) return bn;
		if constexpr (Pt == BISHOP) return bb;
		if constexpr (Pt == ROOK)   return br;
		if constexpr (Pt == QUEEN)  return bq;
		if constexpr (Pt == KING)   return bk;
		if constexpr (Pt == ALL_PIECE_TYPES)
			return b_pieces;
	}
}

template<Color C>
inline Square Position::king_sq() const
{
	if constexpr (C == WHITE)
		return square_of(wk);
	else
		return square_of(bk);
}

inline uint8_t Position::boardstate_pattern() const
{
	return ((turn == BLACK) << 5)
		 | ((ep_target != NO_SQUARE) << 4)
		 | (bool(castling & Castling::W_OOO) << 3)
		 | (bool(castling & Castling::W_OO)  << 2)
		 | (bool(castling & Castling::B_OOO) << 1)
		 |  bool(castling & Castling::B_OO);
}

#define NEW_KEY key ^ TURN_KEY ^ EP_FILE_KEYS[file_of(ep_target)]

template<Square S>
inline uint64_t Position::key_update_castles() const
{
	if constexpr (S == C1)
		return NEW_KEY ^ W_OOO_KEY ^ (castling &  Castling::W_OO ?  W_OO_KEY : 0) ^ W_KING_KEYS[E1][C1] ^ W_ROOK_KEYS[A1][D1];
	if constexpr (S == G1)
		return NEW_KEY ^  W_OO_KEY ^ (castling & Castling::W_OOO ? W_OOO_KEY : 0) ^ W_KING_KEYS[E1][G1] ^ W_ROOK_KEYS[H1][F1];
	if constexpr (S == C8)
		return NEW_KEY ^ B_OOO_KEY ^ (castling &  Castling::B_OO ?  B_OO_KEY : 0) ^ B_KING_KEYS[E8][C8] ^ B_ROOK_KEYS[A8][D8];
	if constexpr (S == G8)
		return NEW_KEY ^  B_OO_KEY ^ (castling & Castling::B_OOO ? B_OOO_KEY : 0) ^ B_KING_KEYS[E8][G8] ^ B_ROOK_KEYS[H8][F8];
}

template<Color C>
inline uint64_t Position::key_update_enpassant(Square from, Square to) const
{
	if constexpr (C == WHITE)
		return NEW_KEY ^ W_PAWN_KEYS[from][to] ^ PIECE_SQUARE_KEYS[B_PAWN][ep_target];
	else
		return NEW_KEY ^ B_PAWN_KEYS[from][to] ^ PIECE_SQUARE_KEYS[W_PAWN][ep_target];
}

template<Color C, PieceType Pt>
inline uint64_t Position::key_update_capture(Square from, Square to) const
{
	if constexpr (C == WHITE)
	{
		const Piece captured = piece_on<BLACK>(to);
		if constexpr (Pt == PAWN)   return key_update<C, Pt>(from, to) ^ PIECE_SQUARE_KEYS[captured][to];
		if constexpr (Pt == KNIGHT) return key_update<C, Pt>(from, to) ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A8 && castling & Castling::B_OOO ? B_OOO_KEY : 0) ^ (to == H8 && castling & Castling::B_OO ? B_OO_KEY : 0);
		if constexpr (Pt == BISHOP) return key_update<C, Pt>(from, to) ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A8 && castling & Castling::B_OOO ? B_OOO_KEY : 0) ^ (to == H8 && castling & Castling::B_OO ? B_OO_KEY : 0);
		if constexpr (Pt == ROOK)   return key_update<C, Pt>(from, to) ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A8 && castling & Castling::B_OOO ? B_OOO_KEY : 0) ^ (to == H8 && castling & Castling::B_OO ? B_OO_KEY : 0);
		if constexpr (Pt == QUEEN)  return key_update<C, Pt>(from, to) ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A8 && castling & Castling::B_OOO ? B_OOO_KEY : 0) ^ (to == H8 && castling & Castling::B_OO ? B_OO_KEY : 0);
		if constexpr (Pt == KING)   return key_update<C, Pt>(from, to) ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A8 && castling & Castling::B_OOO ? B_OOO_KEY : 0) ^ (to == H8 && castling & Castling::B_OO ? B_OO_KEY : 0);
	}
	else // (C == BLACK)
	{
		const Piece captured = piece_on<WHITE>(to);
		if constexpr (Pt == PAWN)   return key_update<C, Pt>(from, to) ^ PIECE_SQUARE_KEYS[captured][to];
		if constexpr (Pt == KNIGHT) return key_update<C, Pt>(from, to) ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A1 && castling & Castling::W_OOO ? W_OOO_KEY : 0) ^ (to == H1 && castling & Castling::W_OO ? W_OO_KEY : 0);
		if constexpr (Pt == BISHOP) return key_update<C, Pt>(from, to) ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A1 && castling & Castling::W_OOO ? W_OOO_KEY : 0) ^ (to == H1 && castling & Castling::W_OO ? W_OO_KEY : 0);
		if constexpr (Pt == ROOK)   return key_update<C, Pt>(from, to) ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A1 && castling & Castling::W_OOO ? W_OOO_KEY : 0) ^ (to == H1 && castling & Castling::W_OO ? W_OO_KEY : 0);
		if constexpr (Pt == QUEEN)  return key_update<C, Pt>(from, to) ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A1 && castling & Castling::W_OOO ? W_OOO_KEY : 0) ^ (to == H1 && castling & Castling::W_OO ? W_OO_KEY : 0);
		if constexpr (Pt == KING)   return key_update<C, Pt>(from, to) ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A1 && castling & Castling::W_OOO ? W_OOO_KEY : 0) ^ (to == H1 && castling & Castling::W_OO ? W_OO_KEY : 0);
	}
}

template<Color C, PieceType Pt>
inline uint64_t Position::key_update(Square from, Square to) const
{
	if constexpr (C == WHITE)
	{
		if constexpr (Pt == PAWN)   return NEW_KEY ^   W_PAWN_KEYS[from][to];
		if constexpr (Pt == KNIGHT) return NEW_KEY ^ W_KNIGHT_KEYS[from][to];
		if constexpr (Pt == BISHOP) return NEW_KEY ^ W_BISHOP_KEYS[from][to];
		if constexpr (Pt == ROOK)   return NEW_KEY ^   W_ROOK_KEYS[from][to] ^ (from == A1 && castling & Castling::W_OOO ? W_OOO_KEY : 0) ^ (from == H1 && castling & Castling::W_OO ? W_OO_KEY : 0);
		if constexpr (Pt == QUEEN)  return NEW_KEY ^  W_QUEEN_KEYS[from][to];
		if constexpr (Pt == KING)   return NEW_KEY ^   W_KING_KEYS[from][to] ^ (castling & Castling::W_OOO ? W_OOO_KEY : 0) ^ (castling & Castling::W_OO ? W_OO_KEY : 0);
	}
	else // (C == BLACK)
	{
		if constexpr (Pt == PAWN)   return NEW_KEY ^   B_PAWN_KEYS[from][to];
		if constexpr (Pt == KNIGHT) return NEW_KEY ^ B_KNIGHT_KEYS[from][to];
		if constexpr (Pt == BISHOP) return NEW_KEY ^ B_BISHOP_KEYS[from][to];
		if constexpr (Pt == ROOK)   return NEW_KEY ^   B_ROOK_KEYS[from][to] ^ (from == A8 && castling & Castling::B_OOO ? B_OOO_KEY : 0) ^ (from == H8 && castling & Castling::B_OO ? B_OO_KEY : 0);
		if constexpr (Pt == QUEEN)  return NEW_KEY ^  B_QUEEN_KEYS[from][to];
		if constexpr (Pt == KING)   return NEW_KEY ^   B_KING_KEYS[from][to] ^ (castling & Castling::B_OOO ? B_OOO_KEY : 0) ^ (castling & Castling::B_OO ? B_OO_KEY : 0);
	}
}

template<Color C, PieceType Pt>
inline uint64_t Position::key_update_promotion_capture(Square from, Square to) const
{
	if constexpr (C == WHITE)
	{
		const Piece captured = piece_on<BLACK>(to);
		if constexpr (Pt == QUEEN)  return NEW_KEY ^ PIECE_SQUARE_KEYS[W_PAWN][from] ^  PIECE_SQUARE_KEYS[W_QUEEN][to] ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A8 && castling & Castling::B_OOO ? B_OOO_KEY : 0) ^ (to == H8 && castling & Castling::B_OO ? B_OO_KEY : 0);
		if constexpr (Pt == KNIGHT) return NEW_KEY ^ PIECE_SQUARE_KEYS[W_PAWN][from] ^ PIECE_SQUARE_KEYS[W_KNIGHT][to] ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A8 && castling & Castling::B_OOO ? B_OOO_KEY : 0) ^ (to == H8 && castling & Castling::B_OO ? B_OO_KEY : 0);
		if constexpr (Pt == ROOK)   return NEW_KEY ^ PIECE_SQUARE_KEYS[W_PAWN][from] ^   PIECE_SQUARE_KEYS[W_ROOK][to] ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A8 && castling & Castling::B_OOO ? B_OOO_KEY : 0) ^ (to == H8 && castling & Castling::B_OO ? B_OO_KEY : 0);
		if constexpr (Pt == BISHOP) return NEW_KEY ^ PIECE_SQUARE_KEYS[W_PAWN][from] ^ PIECE_SQUARE_KEYS[W_BISHOP][to] ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A8 && castling & Castling::B_OOO ? B_OOO_KEY : 0) ^ (to == H8 && castling & Castling::B_OO ? B_OO_KEY : 0);

	}
	else // (C == BLACK)
	{
		const Piece captured = piece_on<WHITE>(to);
		if constexpr (Pt == QUEEN)  return NEW_KEY ^ PIECE_SQUARE_KEYS[B_PAWN][from] ^  PIECE_SQUARE_KEYS[B_QUEEN][to] ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A1 && castling & Castling::W_OOO ? W_OOO_KEY : 0) ^ (to == H1 && castling & Castling::W_OO ? W_OO_KEY : 0);
		if constexpr (Pt == KNIGHT) return NEW_KEY ^ PIECE_SQUARE_KEYS[B_PAWN][from] ^ PIECE_SQUARE_KEYS[B_KNIGHT][to] ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A1 && castling & Castling::W_OOO ? W_OOO_KEY : 0) ^ (to == H1 && castling & Castling::W_OO ? W_OO_KEY : 0);
		if constexpr (Pt == ROOK)   return NEW_KEY ^ PIECE_SQUARE_KEYS[B_PAWN][from] ^   PIECE_SQUARE_KEYS[B_ROOK][to] ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A1 && castling & Castling::W_OOO ? W_OOO_KEY : 0) ^ (to == H1 && castling & Castling::W_OO ? W_OO_KEY : 0);
		if constexpr (Pt == BISHOP) return NEW_KEY ^ PIECE_SQUARE_KEYS[B_PAWN][from] ^ PIECE_SQUARE_KEYS[B_BISHOP][to] ^ PIECE_SQUARE_KEYS[captured][to] ^ (to == A1 && castling & Castling::W_OOO ? W_OOO_KEY : 0) ^ (to == H1 && castling & Castling::W_OO ? W_OO_KEY : 0);
	}

}

template<Color C, PieceType Pt>
inline uint64_t Position::key_update_promotion(Square from, Square to) const
{
	if constexpr (C == WHITE)
	{
		if constexpr (Pt == QUEEN)  return NEW_KEY ^ PIECE_SQUARE_KEYS[W_PAWN][from] ^  PIECE_SQUARE_KEYS[W_QUEEN][to];
		if constexpr (Pt == KNIGHT) return NEW_KEY ^ PIECE_SQUARE_KEYS[W_PAWN][from] ^ PIECE_SQUARE_KEYS[W_KNIGHT][to];
		if constexpr (Pt == ROOK)   return NEW_KEY ^ PIECE_SQUARE_KEYS[W_PAWN][from] ^   PIECE_SQUARE_KEYS[W_ROOK][to];
		if constexpr (Pt == BISHOP) return NEW_KEY ^ PIECE_SQUARE_KEYS[W_PAWN][from] ^ PIECE_SQUARE_KEYS[W_BISHOP][to];

	}
	else // (C == BLACK)
	{
		if constexpr (Pt == QUEEN)  return NEW_KEY ^ PIECE_SQUARE_KEYS[B_PAWN][from] ^  PIECE_SQUARE_KEYS[B_QUEEN][to];
		if constexpr (Pt == KNIGHT) return NEW_KEY ^ PIECE_SQUARE_KEYS[B_PAWN][from] ^ PIECE_SQUARE_KEYS[B_KNIGHT][to];
		if constexpr (Pt == ROOK)   return NEW_KEY ^ PIECE_SQUARE_KEYS[B_PAWN][from] ^   PIECE_SQUARE_KEYS[B_ROOK][to];
		if constexpr (Pt == BISHOP) return NEW_KEY ^ PIECE_SQUARE_KEYS[B_PAWN][from] ^ PIECE_SQUARE_KEYS[B_BISHOP][to];
	}
}

#undef NEW_KEY


#endif // POSITION_H_E76CAC7920F0
