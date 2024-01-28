#ifndef POSITION_H_INCLUDED
#define POSITION_H_INCLUDED



struct StateInfo {
	 
	 Key 	material_key;
	 Key 	pawn_key;
	 Value 	non_pawn_material[2];
	 int	castling_rights;
	 int	rule50;
	 int 	plies_from_null;
	 Square ep_square;

}


class Position {
	public:
	 static void init();

	 Position() 						  = default;
	 Position(const Position&)            = delete;
	 Position& operator=(const Position&) = delete;

	 Position& seed(const std::string& fen);
	 Position& seed(const std::string& code, Color c);
	 std::string fen() const;

	 Bitboard pieces(PieceType pt = ALL_PIECES) const;
	 
	 template<typename... PieceTypes>
	 Bitboard pieces(PieceType pt, PieceTypes... pts) const;
	 
	 Bitbaord pieces(Color c) const;

	 template<typename... PieceTypes>
	 Bitboard pieces(Color c, PieceTypes... pts) const;
	
	 Piece 	  piece_on(Square s) const;
	 Square   ep_square() const;
	 bool	  empty(Square s) const;
	 
	 template<PieceType Pt>
	 int count() const;

	 template<PieceType Pt>
	 Square square(Color c) const;

	 
	 CastlingRights castling_rights(Color c) const;
	 bool			can_bastle(CastlingRights cr) const;
	 bool			castling_impeded(CastlingRights cr) const;
	 Square 		castling_rook_square(CastlingRights cr) const;


	 Bitboard checlers() const;
	 Bitboard blockers_for_king(Color c) const;
	 Bitboard check_squares(PieceType pt) const;
	 Bitboard pinners(Color c) const;


	 Bitboard attackers_to(Square s) const;
	 Bitboard attackers_to(Square s, Bitboard occupied) const;
	 void	  update_slider_blockers(Color c) const;
	 template<PieceType Pt>
	 Bitboard attacks_by(Color c) const;


	 bool  legal(Move m) const;
	 bool  pseudo_legal(Move m) const;
	 bool  capture(Move m) const;
	 bool  capture_state(Move m) const;
	 bool  gives_check(Move m) const;
	 Piece moved_piece(Move m) const;
	 Piece captured_piece() const;


	 void make_move(Move m, StateInfo& newSt);
	 void make_move(Move m, StateInfo& newSt, bool givesCheck);
	 void make_null_move(StateInfo& newSt. TranspositionTable& tt);

	 void undo_move(Move m);
	 void undo_null_move();


	private:
		 
}


#endif // ifndef POSITION_H_INCLUDED
