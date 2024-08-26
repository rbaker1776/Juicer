#ifndef TRANSPOS_H_C4AE48BBF548
#define TRANSPOS_H_C4AE48BBF548

#include "types.h"


struct TTEntry
{
	Move move;
	int value;
	int eval;
	int depth;
	int bound;
	bool is_pv;
};

class TTWriter
{
public:
};


#endif // TRANSPOS_H_C4AE48BBF548
