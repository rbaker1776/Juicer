#include <string_view>
#include <sstream>
#include <iostream>
#include <cassert>
#include "bitboard.h"
#include "position.h"


constexpr std::string_view PIECE_TO_CHAR(" PNBRQK  pnbrqk ");


bool Position::is_ok() const
{
	assert(popcount(bitboards[W_KING]) == 1);
	assert(popcount(bitboards[B_KING]) == 1);

	assert((pieces(WHITE) & pieces(BLACK)) == 0);
	assert((pieces(PAWN) & (RANK8_BB | RANK1_BB)) == 0);
	assert(popcount(bitboards[W_PAWN]) <= 8);
	assert(popcount(bitboards[B_PAWN]) <= 8);

	for (Square s = A1; s <= H8; ++s)
		assert(mailbox[s] == NO_PIECE || (bitboards[mailbox[s]] | s) > 0);

	uint64_t or_accumulation = 0;
	uint64_t xor_accumulation = 0;
	for (int i: { 1, 2, 3, 4, 5, 6, 9, 10, 11, 12, 13, 14 })
	{
		or_accumulation |= bitboards[i];
		xor_accumulation ^= bitboards[i];
	}
	assert(or_accumulation == xor_accumulation);

	if (state->castling_rights & WHITE_OO)  assert(mailbox[H1] == W_ROOK && mailbox[E1] == W_KING);
	if (state->castling_rights & WHITE_OOO) assert(mailbox[A1] == W_ROOK && mailbox[E1] == W_KING);
	if (state->castling_rights & BLACK_OO)  assert(mailbox[H8] == B_ROOK && mailbox[E8] == B_KING);
	if (state->castling_rights & BLACK_OOO) assert(mailbox[A8] == B_ROOK && mailbox[E8] == B_KING);

	if (state->ep_square != NO_SQUARE)
	{
		assert(rank_of(state->ep_square) == (turn == WHITE ? RANK_6 : RANK_3));
		assert(PAWN_ATTACKS[turn][state->ep_square] & pieces(~turn, PAWN));
	}

	return true;
}


Position& Position::seed(const std::string& fen, Gamestate& gs)
{
	// A FEN string represents a chess position using six fields seperated by a space:
	// Piece placement, Side to move, Castling rights, En passant target, Halfmoves, Fullmoves
	// For more information: https://en.wikipedia.org/wiki/Forsyth–Edwards_Notation

	this->state = &gs;
	this->clear_board();
	
	char token;
	std::istringstream ss(fen);
	ss >> std::noskipws;
	
	// 1) Piece placement
	for (Square s = A8; (ss >> token) && !isspace(token); )
	{
		if (isdigit(token)) // skip empty squares represented by digits
			s += (token - '0') * Direction::E;
		else if (token == '/') // newline fen character, must move two squares south
			s += Direction::SS;
		else // place a piece
		{
			this->place_piece(Piece(PIECE_TO_CHAR.find(token)), s);
			++s;
		}
	}

	// 2) Side to move
	ss >> token;
	this->turn = token == 'w' ? WHITE : BLACK;
	ss >> token;

	// 3) Castling rights
	for (this->state->castling_rights = 0; ss >> token && !isspace(token); )
	{
		switch (token)
		{
			case 'K': state->castling_rights |= WHITE_OO; break;
			case 'Q': state->castling_rights |= WHITE_OOO; break;
			case 'k': state->castling_rights |= BLACK_OO; break;
			case 'q': state->castling_rights |= BLACK_OOO; break;
			default: break;
		}
	}

	// 4) En passant target
	uint8_t row, col;
	bool en_passant = false;
	if (((ss >> col) && (col >= 'a' && col <= 'h')) && ((ss >> row) && (row == (turn == WHITE ? '6' : '3'))))
	{
		this->state->ep_square = make_square(File(col - 'a'), Rank(row - '1'));
		#if DEBUG == true
			assert(::is_ok(state->ep_square));
		#endif
		en_passant = !(this->pieces() & (state->ep_square | (state->ep_square + pawn_push(turn))))
				    && pieces(~turn, PAWN) & (state->ep_square + pawn_push(~turn))
					&& PAWN_ATTACKS[~turn][state->ep_square] & pieces(turn, PAWN);
	}
	if (!en_passant) state->ep_square = NO_SQUARE;

	// 5) Halfmoves and 6) Fullmoves
	ss >> std::skipws >> state->rule_50 >> gameply;
	gameply = 2 * (gameply - 1) + this->turn;
	
	return *this;
}

std::string Position::fen() const
{
	#if DEBUG == true
		assert(is_ok());
	#endif

	std::ostringstream  ss;

	// 1) Piece placement
	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			int empty_count = 0;
			for (; f <= FILE_H && mailbox[make_square(f, r)] == NO_PIECE; ++f) { ++empty_count; }
			if (empty_count) { ss << empty_count; }
			if (f <= FILE_H) { ss << PIECE_TO_CHAR[mailbox[make_square(f, r)]]; }
		}
		if (r > RANK_1) { ss << '/'; }
	}

	// 2) Side to move
	ss << (this->turn == WHITE ? " w " : " b ");

	// 3) Castling rights
	if (this->state->castling_rights & WHITE_OO)  { ss << 'K'; }
	if (this->state->castling_rights & WHITE_OOO) { ss << 'Q'; }
	if (this->state->castling_rights & BLACK_OO)  { ss << 'k'; }
	if (this->state->castling_rights & BLACK_OOO) { ss << 'q'; }
	if (this->state->castling_rights == 0) { ss << '-'; }

	// 4) En passant target
	ss << ' ' << sq_to_string(this->state->ep_square) << ' ';

	// 5) Halfmoves and 6) Fullmoves
	ss << state->rule_50 << ' ' << ((this->gameply - (turn == BLACK)) / 2 + 1); 

	return ss.str();
}

std::string Position::to_string() const
{
	const std::string newline = "+---+---+---+---+---+---+---+---+\n";
	std::string s = newline;

	for (Rank r = RANK_8; r >= RANK_1; --r)
	{
		for (File f = FILE_A; f <= FILE_H; ++f)
		{
			std::string newsquare = "|   ";
			newsquare[2] = PIECE_TO_CHAR[piece_on(make_square(f, r))];
			s += newsquare;
		}
		s += "| " + std::to_string(r + 1) + '\n' + newline;
	}

	return s + "  a   b   c   d   e   f   g   h";
}


// Makes a move and updates the gamestate accordingly
// Assumes the move is valid and legal
void Position::make_move(const Move move, Gamestate& gs, bool is_check)
{
	#if DEBUG == true
		assert(is_ok());
		assert(&gs != this->state);
	#endif

	std::memcpy(&gs, this->state, sizeof(Gamestate));
	gs.previous = this->state;
	this->state = &gs;

	const Square from = move.from();
	const Square to = move.to();
	const Piece pc = this->mailbox[from];
	const Piece captured_pc = move.type() == EN_PASSANT ? this->mailbox[to + pawn_push(~turn)] : this->mailbox[to];

	#if DEBUG == true
		assert(color_of(pc) == turn);
		assert(type_of(captured_pc) != KING);
		assert(captured_pc == NO_PIECE || color_of(captured_pc) == ~turn || (move.type() == CASTLES && color_of(captured_pc) == turn));
	#endif

	++this->gameply;
	++this->state->rule_50;
	this->state->captured_piece = NO_PIECE;
	
	if (move.type() == CASTLES)
	{
		const Direction d = from > to ? Direction::W : Direction::E;

		#if DEBUG == true
			assert(type_of(captured_pc) == ROOK);
			assert(type_of(pc) == KING);
			assert(mailbox[from + d] == NO_PIECE);
			assert(mailbox[from + 2 * d] == NO_PIECE);
		#endif

		remove_piece(from);
		remove_piece(to);
		place_piece(this->turn, ROOK, from + d);
		place_piece(this->turn, KING, from + 2 * d);
		this->state->castling_rights &= ~CASTLING_RIGHTS[this->turn];
		this->state->captured_piece = make_piece(turn, ROOK);
	}
	else if (captured_pc != NO_PIECE)
	{
		if (move.type() == EN_PASSANT)
		{
			#if DEBUG == true
				assert(type_of(pc) == PAWN);
				assert(to == state->ep_square);
				assert(rank_of(to) == (turn == WHITE ? RANK_6 : RANK_3));
				assert(piece_on(to) == NO_PIECE);
				assert(type_of(piece_on(to + pawn_push(~turn))) == PAWN);
				assert(color_of(piece_on(to + pawn_push(~turn))) == ~turn);
			#endif
			remove_piece(to + pawn_push(~this->turn));
		}
		else
			remove_piece(to);
		this->state->rule_50 = 0;
		this->state->captured_piece = captured_pc;
	}

	if (move.type() != CASTLES)
	{
		this->mailbox[to] = pc;
		this->mailbox[from] = NO_PIECE;
		this->bitboards[pc] ^= square_to_bb(from) | to;
	}

	this->state->ep_square = NO_SQUARE;

	if (type_of(pc) == PAWN)
	{
		if (SQUARE_DISTANCE[to][from] == 2 && PAWN_ATTACKS[turn][to-pawn_push(turn)] & pieces(~turn, PAWN))
			state->ep_square = to - pawn_push(turn);

		else if (move.type() == PROMOTION)
		{
			#if DEBUG == true	
				assert(rank_of(from) == (turn == WHITE ? RANK_7 : RANK_2));
				assert(rank_of(to) == (turn == WHITE ? RANK_8 : RANK_1));
				assert(type_of(pc) == PAWN);
			#endif
			remove_piece(to);
			place_piece(turn, move.promotion_type(), to);
		}
		this->state->rule_50 = 0;
	
	}
	switch (from)
	{
		case E1: state->castling_rights &= ~WHITE_CASTLES; break;
		case E8: state->castling_rights &= ~BLACK_CASTLES; break;
		case A1: state->castling_rights &= ~WHITE_OOO; break;
		case H1: state->castling_rights &= ~WHITE_OO; break;
		case A8: state->castling_rights &= ~BLACK_OOO; break;
		case H8: state->castling_rights &= ~BLACK_OO; break;
		default: break;
	}
	switch (to)
	{
		case A1: state->castling_rights &= ~WHITE_OOO; break;
		case H1: state->castling_rights &= ~WHITE_OO; break;
		case A8: state->castling_rights &= ~BLACK_OOO; break;
		case H8: state->castling_rights &= ~BLACK_OO; break;
		default: break;
	}

	this->turn ^= 1;

	#if DEBUG == true
		assert(is_ok());
	#endif
}

void Position::undo_move(const Move move)
{
	#if DEBUG == true
		assert(is_ok());
	#endif

	this->turn ^= 1;

	const Square from = move.from();
	const Square to = move.to();
	Piece pc = mailbox[to];

	#if DEBUG == true
		assert(mailbox[from] == NO_PIECE || move.type() == CASTLES);
		assert(type_of(state->captured_piece) != KING);
	#endif

	if (move.type() == PROMOTION)
	{
		#if DEBUG == true
			assert(rank_of(to) == (turn == WHITE ? RANK_8 : RANK_1));
			assert(rank_of(from) == (turn == WHITE ? RANK_7 : RANK_2));
			assert(type_of(pc) >= KNIGHT && type_of(pc) <= QUEEN);
		#endif
		remove_piece(to);
		pc = make_piece(turn, PAWN);
		place_piece(turn, PAWN, to);
	}
	
	if (move.type() == CASTLES)
	{
		const Direction d = from > to ? Direction::W : Direction::E;

		#if DEBUG == true
			assert(mailbox[to] == NO_PIECE);
			assert(mailbox[from] == NO_PIECE);
		#endif

		remove_piece(from + d);
		remove_piece(from + 2 * d);
		place_piece(turn, ROOK, to);
		place_piece(turn, KING, from);
	}
	else
	{
		remove_piece(to);
		place_piece(pc, from);

		if (state->captured_piece)
		{
			if (move.type() == EN_PASSANT)
			{
				#if DEBUG == true
					assert(type_of(pc) == PAWN);
					assert(to == state->previous->ep_square);
					assert(rank_of(to) == (turn == WHITE ? RANK_6 : RANK_3));
					assert(mailbox[to - pawn_push(turn)] == NO_PIECE);
					assert(type_of(state->captured_piece) == PAWN);
					assert(color_of(state->captured_piece) == ~turn);
				#endif
				place_piece(state->captured_piece, to - pawn_push(turn));
			}
			else
			{
				place_piece(state->captured_piece, to);
			}
		}
	}

	this->state = this->state->previous;
	--gameply;

	#if DEBUG == true
	 	assert(is_ok());
	#endif
}


inline void Position::place_piece(Piece pc, Square s)
{
	this->mailbox[s] = pc;
	this->bitboards[pc | ALL_PIECE_TYPES] |= this->bitboards[pc] |= s;
}

inline void Position::remove_piece(Square s)
{
	this->bitboards[this->mailbox[s]] &= this->bitboards[this->mailbox[s] | ALL_PIECE_TYPES] &= ~square_to_bb(s);
	this->mailbox[s] = NO_PIECE;
}

inline void Position::clear_board()
{
	for (Square s = A1; s <= H8; ++s) { mailbox[s] = NO_PIECE; }
	for (uint64_t& bb: bitboards) { bb = 0ull; }
}


void Position::update_guards(Color c) const
{
	Square ksq = this->king_sq(c);

	state->kings_guards[c] = 0ull;
	state->pinners[~c] = 0ull;

	uint64_t snipers = (attacks_bb<ROOK>(ksq) & pieces(~c, QUEEN, ROOK)) | (attacks_bb<BISHOP>(ksq) & pieces(~c, QUEEN, BISHOP));
	uint64_t occupied = this->pieces() ^ snipers;

	while (snipers)
	{
		Square s = pop_lsb(snipers);
	}
}
