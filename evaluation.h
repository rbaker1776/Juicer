#ifndef EVALUATION_H_B8F5C92DCB57
#define EVALUATION_H_B8F5C92DCB57


namespace Evaluation
{
	static constexpr int PAWN_VALUE   = 100;
	static constexpr int KNIGHT_VALUE = 325;
	static constexpr int BISHOP_VALUE = 340;
	static constexpr int ROOK_VALUE   = 500;
	static constexpr int QUEEN_VALUE  = 1050;

	inline int negamax_crude(Color c, const Board& board);
} // namespace Evaluation


// returns a strictly materialistic evaluation from the perspective of Color c
// if c == WHITE and evaluation is positve then white is better
inline int Evaluation::negamax_crude(Color c, const Board& board)
{
	return (PAWN_VALUE * (board.wp - board.bp)
	    + KNIGHT_VALUE * (board.wn - board.bn)
	    + BISHOP_VALUE * (board.wb - board.bb)
	      + ROOK_VALUE * (board.wr - board.br)
	     + QUEEN_VALUE * (board.wq - board.bq)) * (c == WHITE ? 1 : -1);
}


#endif // EVALUATION_H_B8F5C92DCB57
