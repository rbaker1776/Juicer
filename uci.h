#ifndef UCI_H_5CD6E7B92F59
#define UCI_H_5CD6E7B92F59

#include <string>
#include "position.h"
#include "types.h"


namespace UCI
{
std::string sq_to_string(Square s);
std::string move_to_string(Move m);
Move string_to_move(const Position& pos, const std::string& str);
}


#endif // UCI_H_5CD6E7B92F59
