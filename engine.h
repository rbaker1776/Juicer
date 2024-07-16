#ifndef ENGINE_H_8EC893250AFF
#define ENGINE_H_8EC893250AFF

#include <vector>
#include <iostream>
#include <string_view>
#include "movegen.h"
#include "uci.h"
#include "position.h"
#include "types.h"
#include "juicer.h"


class Engine
{
public:
	constexpr Engine();
	constexpr Engine(std::string_view fen);
	inline void seed(std::string_view fen);
	~Engine() = default;

	inline void make_move(const Move& m);
	inline void undo_move();
	
	template<bool IsRoot>
	uint64_t perft(int depth);

	inline const Board& board() const { return this->boards.back(); }
	inline const Boardstate& boardstate() const { return this->boardstates.back(); }
	inline const Gamestate& gamestate() const { return this->gamestates.back(); }

private:
	template<PieceType Pt> inline void make_move(Square from, Square to);
	template<PieceType Pt> inline void promote(Square from, Square to);
	template<Castling Cr>  inline void castle();
	inline void ep_capture(Square from, Square to);

	std::vector<Board> boards;
	std::vector<Boardstate> boardstates;
	std::vector<Gamestate> gamestates;
}; // class Engine


template<bool IsRoot>
uint64_t Engine::perft(int depth)
{
	uint64_t nodes = 0, count = 0;
	const bool leaf = (depth == 2);

	if (depth < 1)
		return 1;

	for (const Move& m: MoveList(this->boards.back(), this->boardstates.back(), this->gamestates.back().ep_target))
	{
		if (depth == 1)
			count = 1, nodes++;
		else
		{
			make_move(m);
			count = leaf ? MoveList(this->boards.back(), this->boardstates.back(), this->gamestates.back().ep_target).size() : perft<false>(depth - 1);
			nodes += count;
			undo_move();
		}
		if constexpr (IsRoot && VERBOSE)
			std::cout << UCI::move_to_string(m) << ": " << count << std::endl;
	}

	return nodes;
}

constexpr Engine::Engine()
{
	boards.reserve(256);
	boardstates.reserve(256);
	gamestates.reserve(256);
	boards.emplace_back(Board::startpos());
	boardstates.emplace_back(Boardstate::startpos());
	gamestates.emplace_back(Gamestate::startpos());
}

constexpr Engine::Engine(std::string_view fen)
{
	boards.reserve(256);
	boardstates.reserve(256);
	gamestates.reserve(256);
	this->boards.emplace_back(fen);
	this->boardstates.emplace_back(fen);
	this->gamestates.emplace_back(fen);
}

inline void Engine::seed(std::string_view fen)
{
	boards.clear();
	boardstates.clear();
	gamestates.clear();
	this->boards.emplace_back(fen);
	this->boardstates.emplace_back(fen);
	this->gamestates.emplace_back(fen);
}

inline void Engine::make_move(const Move& m)
{
	#if (DEBUG)
		assert(boardstates.size() == boards.size() && boards.size() == gamestates.size());
	#endif

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
				default:
					std::cerr << "Error, invalid PieceType: " << m.piece << ", passed into Engine::make_move." << std::endl;
			}
			break;
		case CASTLING:
			switch (m.from | m.to)
			{
				case (E1 | C1): this->castle<WHITE_OOO>(); break;
				case (E1 | G1): this->castle<WHITE_OO>(); break;
				case (E8 | C8): this->castle<BLACK_OOO>(); break;
				case (E8 | G8): this->castle<BLACK_OO>(); break;
				default:
					std::cerr << "Error, invalid squares for castling move: " << m.from << "->" << m.to << '.' << std::endl;
			}
			break;
		case EN_PASSANT:
			this->ep_capture(m.from, m.to);
			break;
		case PROMOTION:
			switch (m.piece)
			{
				case QUEEN:  this->promote<QUEEN>(m.from, m.to); break;
				case KNIGHT: this->promote<KNIGHT>(m.from, m.to); break;
				case ROOK:   this->promote<ROOK>(m.from, m.to); break;
				case BISHOP: this->promote<BISHOP>(m.from, m.to); break;
				default:
					std::cerr << "Error, invalid PieceType: " << m.piece << ", passed into Engine::make_move (PROMOTION)." << std::endl;
			}
			break;
	}
}

inline void Engine::undo_move()
{
	boards.pop_back();
	boardstates.pop_back();
	gamestates.pop_back();
}

template<PieceType Pt>
inline void Engine::make_move(Square from, Square to)
{
	const Board& board = this->boards.back();
	const Boardstate& boardstate = this->boardstates.back();
	const Gamestate& gamestate = this->gamestates.back();

	const bool is_capture = board.pieces & to;

	if (is_capture)
	{
		if (boardstate.turn == WHITE)
			boards.emplace_back(board.move<WHITE, Pt, true>(from, to));
		else 
			boards.emplace_back(board.move<BLACK, Pt, true>(from, to));
	}
	else // no capture
	{
		if (boardstate.turn == WHITE)
			boards.emplace_back(board.move<WHITE, Pt, false>(from, to));
		else
			boards.emplace_back(board.move<BLACK, Pt, false>(from, to));
	}

	if constexpr (Pt == PAWN)
	{
		if (SQUARE_DISTANCE[from][to] > 1 && ((boardstate.turn == WHITE ? board.bp : board.wp) & (shift<E>(square_to_bb(to)) | shift<W>(square_to_bb(to)))))
		{
			boardstates.emplace_back(boardstate.pawn_push());
			gamestates.emplace_back(gamestate.pawn_push(to));
		}
		else
		{
			boardstates.emplace_back(boardstate.quiet_move());
			gamestates.emplace_back(gamestate.pawn_step());
		}
	}
	else 
	{
		if constexpr (Pt == ROOK)
		{
			if ((A1 | A8) & from)
				boardstates.emplace_back(boardstate.rook_move_queenside());
			else if ((H1 | H8) & from)
				boardstates.emplace_back(boardstate.rook_move_kingside());
			else
				boardstates.emplace_back(boardstate.quiet_move());
		}
		else if constexpr (Pt == KING)
			boardstates.emplace_back(boardstate.king_move());
		else
			boardstates.emplace_back(boardstate.quiet_move());
		
		gamestates.emplace_back(is_capture ? gamestate.capture() : gamestate.quiet_move());
	}
}

template<Castling Cr>
inline void Engine::castle()
{
	this->boards.emplace_back(this->boards.back().castles<Cr>());
	this->boardstates.emplace_back(this->boardstates.back().king_move());
	this->gamestates.emplace_back(this->gamestates.back().quiet_move());
}

inline void Engine::ep_capture(Square from, Square to)
{
	if (this->boardstates.back().turn == WHITE)
		this->boards.emplace_back(this->boards.back().enpassant<WHITE>(from, to));
	else
		this->boards.emplace_back(this->boards.back().enpassant<BLACK>(from, to));

	this->boardstates.emplace_back(this->boardstates.back().quiet_move());
	this->gamestates.emplace_back(this->gamestates.back().capture());
}

template<PieceType Pt>
inline void Engine::promote(Square from, Square to)
{
	const Board& board = this->boards.back();
	const Boardstate& boardstate = this->boardstates.back();
	const Gamestate& gamestate = this->gamestates.back();

	const bool is_capture = board.pieces & to;

	if (is_capture)
		if (boardstate.turn == WHITE)
			this->boards.emplace_back(board.promote<WHITE, Pt, true>(from, to));
		else
			this->boards.emplace_back(board.promote<BLACK, Pt, true>(from, to));
	else
		if (boardstate.turn == WHITE)
			this->boards.emplace_back(board.promote<WHITE, Pt, false>(from, to));
		else
			this->boards.emplace_back(board.promote<BLACK, Pt, false>(from, to));

	this->boardstates.emplace_back(boardstate.quiet_move());
	this->gamestates.emplace_back((is_capture ? gamestate.capture() : gamestate.pawn_step()));
}


#endif // ENGINE_H_8EC893250AFF
