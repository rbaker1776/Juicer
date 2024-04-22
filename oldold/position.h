#ifndef POSITION_H_C668DBDA2B2D
#define POSITION_H_C668DBDA2B2D



class Position
{
private: 
	struct Gamestate
	{
		uint32_t castling_rights;
		Square ep_square;
		uint32_t half_clock;
		uint32_t full_clock;
	};

	uint64_t pieces[6];
	uint64_t armies[2];
	bool white_to_move;
	struct Gamestate gamestate;

public:
	Position& seed(const std::string& fen);
	std::string& to_string() const;
};



#endif // POSITION_H_C668DBDA2B2D
