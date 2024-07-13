#ifndef ENGINE_H_8EC893250AFF
#define ENGINE_H_8EC893250AFF

#include <vector>
#include <iostream>
#include "movegen.h"
#include "uci.h"
#include "position.h"
#include "types.h"
#include "juicer.h"


class Engine
{
public:
	Engine()
	{
		boards.reserve(256);
		states.reserve(256);
		boards.push_back(Board::DEFAULT());
		states.push_back(Gamestate::DEFAULT());
	};
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
	~Engine() = default;

	void make_move(const Move& m);
	inline void undo_move()
	{
		this->boards.pop_back();
		this->states.pop_back();
		Movegen::ep_squares.pop_back();
	}

	template<bool IsRoot>
	uint64_t perft(int depth);

	const Board& board() const { return this->boards.back(); }
	const Gamestate& state() const { return this->states.back(); }

private:
	template<PieceType Pt> void make_move(Square from, Square to);
	template<Castling Cr> void castle();
	template<PieceType Pt> void promote(Square from, Square to);
	void ep_capture(Square from, Square to);

	std::vector<Board> boards;
	std::vector<Gamestate> states;
};


template<bool IsRoot>
uint64_t Engine::perft(int depth)
{
	uint64_t nodes = 0, count = 0;
	const bool leaf = (depth == 2);

	if (depth < 1)
		return 1;

	for (const Move& m: MoveList(this->boards.back(), this->states.back()))
	{
		if (depth == 1)
			count = 1, nodes++;
		else
		{
			make_move(m);
			count = leaf ? MoveList(this->boards.back(), this->states.back()).size() : perft<false>(depth - 1);
			nodes += count;
			undo_move();
		}
		if constexpr (IsRoot && VERBOSE)
			std::cout << UCI::move_to_string(m) << ": " << count << std::endl;
	}

	return nodes;
}


inline void Engine::make_move(const Move& m)
{
	#if (DEBUG)
		assert(states.size() == boards.size() && boards.size() == Movegen::ep_squares.size());
	#endif
	Movegen::ep_squares.push_back(NO_SQUARE);

	switch (m.type)
	{
		case NORMAL:
			switch (m.piece)
			{
				case PAWN:   this->make_move<PAWN>(m.from, m.to); break;
				case KNIGHT: this->make_move<KNIGHT>(m.from, m.to); break;
				case BISHOP: this->make_move<BISHOP>(m.from, m.to); break;
				case ROOK:   this->make_move<ROOK>(m.from, m.to); break;
				case QUEEN:  this->make_move<QUEEN>(m.from, m.to); break;
				case KING:   this->make_move<KING>(m.from, m.to); break;
			}
			break;
		case CASTLING:
			switch (m.from | m.to)
			{
				case (E1 | C1): this->castle<WHITE_OOO>(); break;
				case (E1 | G1): this->castle<WHITE_OO>(); break;
				case (E8 | C8): this->castle<BLACK_OOO>(); break;
				case (E8 | G8): this->castle<BLACK_OO>(); break;
			}
			break;
		case EN_PASSANT: this->ep_capture(m.from, m.to); break;
		case PROMOTION:
			switch (m.piece)
			{
				case QUEEN:  this->promote<QUEEN>(m.from, m.to); break;
				case KNIGHT: this->promote<KNIGHT>(m.from, m.to); break;
				case ROOK:   this->promote<ROOK>(m.from, m.to); break;
				case BISHOP: this->promote<BISHOP>(m.from, m.to); break;
			}
			break;
	}
}

template<PieceType Pt>
inline void Engine::make_move(Square from, Square to)
{
	const Board& board = this->boards.back();
	const Gamestate& state = this->states.back();

	const bool is_capture = board.pieces & to;

	if (is_capture)
		if (state.turn == WHITE)
			boards.push_back(board.move<WHITE, Pt, true>(from, to));
		else 
			boards.push_back(board.move<BLACK, Pt, true>(from, to));
	else // no capture
		if (state.turn == WHITE)
			boards.push_back(board.move<WHITE, Pt, false>(from, to));
		else
			boards.push_back(board.move<BLACK, Pt, false>(from, to));

	if constexpr (Pt == PAWN)
	{
		if (SQUARE_DISTANCE[from][to] > 1 && ((state.turn == WHITE ? board.bp : board.wp) & (shift<E>(square_to_bb(to)) | shift<W>(square_to_bb(to)))))
		{
			Movegen::ep_squares.back() = Square((int(to) + int(from)) >> 1);
			states.push_back(state.pawn_push());
		}
		else
			states.push_back(state.quiet_move());
	}
	else if constexpr (Pt == ROOK)
		if ((A1 | A8) & from)
			states.push_back(state.rook_move_queenside());
		else if ((H1 | H8) & from)
			states.push_back(state.rook_move_kingside());
		else
			states.push_back(state.quiet_move());
	else if constexpr (Pt == KING)
		states.push_back(state.king_move());
	else
		states.push_back(state.quiet_move());
}

template<Castling Cr>
void Engine::castle()
{
	if constexpr (Cr == WHITE_OOO)
		this->boards.push_back(this->boards.back().castles<WHITE>(E1 | C1, A1 | D1));
	else if constexpr (Cr == WHITE_OO)
		this->boards.push_back(this->boards.back().castles<WHITE>(E1 | G1, H1 | F1));
	else if constexpr (Cr == BLACK_OOO)
		this->boards.push_back(this->boards.back().castles<BLACK>(E8 | C8, A8 | D8));
	else if constexpr (Cr == BLACK_OO)
		this->boards.push_back(this->boards.back().castles<BLACK>(E8 | G8, H8 | F8));

	this->states.push_back(this->states.back().king_move());
}

void Engine::ep_capture(Square from, Square to)
{
	if (this->states.back().turn == WHITE)
		this->boards.push_back(this->boards.back().enpassant<WHITE>(from, to));
	else
		this->boards.push_back(this->boards.back().enpassant<BLACK>(from, to));

	this->states.push_back(this->states.back().quiet_move());
}

template<PieceType Pt>
void Engine::promote(Square from, Square to)
{
	const Board& board = this->boards.back();
	const Gamestate& state = this->states.back();

	const bool is_capture = board.pieces & to;

	if (is_capture)
		if (state.turn == WHITE)
			this->boards.push_back(board.promote<WHITE, Pt, true>(from, to));
		else
			this->boards.push_back(board.promote<BLACK, Pt, true>(from, to));
	else
		if (state.turn == WHITE)
			this->boards.push_back(board.promote<WHITE, Pt, false>(from, to));
		else
			this->boards.push_back(board.promote<BLACK, Pt, false>(from, to));

	this->states.push_back(state.quiet_move());
}


#endif // ENGINE_H_8EC893250AFF
