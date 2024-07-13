#include "engine.h"
#include "position.h"
#include "types.h"
#include "juicer.h"


inline void Engine::make_move(const Move& m)
{
	const Board& board = this->boards.back();
	const Gamestate& state = this->states.back();

	switch (m.type())
	{
		case NORMAL:
			switch (m.piece())
			{
				case PAWN:
					if (board.pieces & m.to()) // capture
					{
						if (state.turn == WHITE)
						{
							boards.push_back(board.move<WHITE, PAWN, true>(m.from(), m.to());
							if (SQUARE_DISTANCE[m.from()][m.to()] == 2)
								states.pus
						}
						else
						{
							boards.push_back(board.move<BLACK, PAWN, true>(m.from(), m.to());
							state.push_back(state.quiet_move());
						}
					}
					else // no capture
					{
						if (state.turn == WHITE)
						{
							boards.push_back(board.move<WHITE, PAWN, false>(m.from(), m.to());
							state.push_back(state.quiet_move());
						}
						else
						{
							boards.push_back(board.move<BLACK, PAWN, false>(m.from(), m.to());
							state.push_back(state.quiet_move());
						}
					}

					if (SQUARE_DISTANCE[m.from()][m.to()] == 2)
						
				case KNIGHT:
					if (board.pieces & m.to()) // capture
					{
						if (state.turn == WHITE)
						{
							boards.push_back(board.move<WHITE, KNIGHT, true>(m.from(), m.to());
							states.push_back(state.quiet_move());
						}
						else
						{
							boards.push_back(board.move<BLACK, KNIGHT, true>(m.from(), m.to());
							state.push_back(state.quiet_move());
						}
					}
					else // no capture
					{
						if (state.turn == WHITE)
						{
							boards.push_back(board.move<WHITE, KNIGHT, false>(m.from(), m.to());
							state.push_back(state.quiet_move());
						}
						else
						{
							boards.push_back(board.move<BLACK, KNIGHT, false>(m.from(), m.to());
							state.push_back(state.quiet_move());
						}
					}

				case BISHOP:
					if (board.pieces & m.to()) // capture
					{
						if (state.turn == WHITE)
						{
							boards.push_back(board.move<WHITE, BISHOP, true>(m.from(), m.to());
							states.push_back(state.quiet_move());
						}
						else
						{
							boards.push_back(board.move<BLACK, BISHOP, true>(m.from(), m.to());
							state.push_back(state.quiet_move());
						}
					}
					else // no capture
					{
						if (state.turn == WHITE)
						{
							boards.push_back(board.move<WHITE, BISHOP, false>(m.from(), m.to());
							state.push_back(state.quiet_move());
						}
						else
						{
							boards.push_back(board.move<BLACK, BISHOP, false>(m.from(), m.to());
							state.push_back(state.quiet_move());
						}
					}

				case ROOK:
					if (board.pieces & m.to()) // capture
					{
						if (state.turn == WHITE)
						{
							boards.push_back(board.move<WHITE, ROOK, true>(m.from(), m.to());
							if ((m.from() | m.to()) & A1)
								states.push_back(state.rook_move_queenside());
							else if ((m.from() | m.to()) & H1)
								states.push_back(state.rook_move_kingside());
							else
								states.push_back(state.quiet_move());
						}
						else
						{
							boards.push_back(board.move<BLACK, ROOK, true>(m.from(), m.to());
							if ((m.from() | m.to()) & A8)
								states.push_back(state.rook_move_queenside());
							else if ((m.from() | m.to()) & H8)
								states.push_back(state.rook_move_kingside());
							else
								states.push_back(state.quiet_move());
						}
					}
					else // no capture
					{
						if (state.turn == WHITE)
						{
							boards.push_back(board.move<WHITE, ROOK, false>(m.from(), m.to());
							if ((A8 | A1) & (m.from() | m.to()))
								states.push_back(state.rook_move_queenside());
							else if ((H8 | H1) & (m.from() | m.to()))
								states.push_back(state.rook_move_kingside());
							else
								states.push_back(state.quiet_move());
						}
						else
						{
							boards.push_back(board.move<BLACK, ROOK, false>(m.from(), m.to());
							if ((A8 | A1) & (m.from() | m.to()))
								states.push_back(state.rook_move_queenside());
							else if ((H8 | H1) & (m.from() | m.to()))
								states.push_back(state.rook_move_kingside());
							else
								states.push_back(state.quiet_move());
						}
					}

				case QUEEN:
					if (board.pieces & m.to()) // capture
					{
						if (state.turn == WHITE)
						{
							boards.push_back(board.move<WHITE, QUEEN, true>(m.from(), m.to());
							states.push_back(state.quiet_move());
						}
						else
						{
							boards.push_back(board.move<BLACK, QUEEN, true>(m.from(), m.to());
							state.push_back(state.quiet_move());
						}
					}
					else // no capture
					{
						if (state.turn == WHITE)
						{
							boards.push_back(board.move<WHITE, QUEEN, false>(m.from(), m.to());
							state.push_back(state.quiet_move());
						}
						else
						{
							boards.push_back(board.move<BLACK, QUEEN, false>(m.from(), m.to());
							state.push_back(state.quiet_move());
						}
					}

				case KING:
			}
		case CASTLING:
		case EN_PASSANT:
		case PROMOTION:
	}
}
