#ifndef CHESS_BBSEARCH_SEARCHDEFS_H
#define CHESS_BBSEARCH_SEARCHDEFS_H

#include "chess/bb/BBoard.h"   // chess::bb::Move (via BBMove.h), Color
#include <cstdint>

namespace chess::bbSearch {

constexpr int MATE_SCORE = 30000;
constexpr int MAX_PLY    = 128;
constexpr int INF        = 1'000'000;

// Forward declaration so SearchState can hold a TT pointer without pulling in
// the full TT definition here (TT.h includes BBoard.h — already included above,
// but the forward decl keeps the dependency graph clean).
class TT;

// All per-search mutable data: node counters, killers, history, PV table, and
// a non-owning TT pointer.
//
// Arrays are sized MAX_PLY + 1 (not MAX_PLY).  When searching at depth D, negamax
// is called recursively until ply == D and depth == 0.  The depth-0 call still
// executes "pvLength[ply] = 0" before entering qsearch.  If D == MAX_PLY that
// write lands at pvLength[MAX_PLY], which is one past the end of a [MAX_PLY]
// array — undefined behaviour.  The +1 eliminates this off-by-one.
struct SearchState {
    uint64_t nodes = 0;
    bool stop  = false;
    TT*  tt    = nullptr;   // non-owning; null = no TT

    chess::bb::Move killers[MAX_PLY + 1][2]           = {};
    int             history[2][64][64]                 = {};
    chess::bb::Move pvTable[MAX_PLY + 1][MAX_PLY + 1] = {};
    int             pvLength[MAX_PLY + 1]              = {};
};

} // namespace chess::bbSearch

#endif // CHESS_BBSEARCH_SEARCHDEFS_H
