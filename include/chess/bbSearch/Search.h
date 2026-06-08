#ifndef CHESS_BBSEARCH_SEARCH_H
#define CHESS_BBSEARCH_SEARCH_H

#include "chess/bb/BBoard.h"
#include "chess/bb/BBMove.h"
#include "chess/bb/MoveGen.h"
#include "chess/bbEval/Eval.h"
#include "chess/bbSearch/TT.h"
#include "chess/bbSearch/SearchDefs.h"

#include <cstdlib>  // std::abs
#include <vector>

namespace chess::bbSearch {

// Returns true if |score| indicates a forced mate.
inline bool isMateScore(int score) { return std::abs(score) > MATE_SCORE - MAX_PLY; }

struct SearchLimits {
    int  wtime     = 0, btime    = 0;
    int  winc      = 0, binc     = 0;
    int  movestogo = 0;
    int  movetime  = 0;
    int  depth     = MAX_PLY;
    bool infinite  = false;
};

struct SearchResult {
    chess::bb::Move              bestMove = chess::bb::MOVE_NONE;
    int                          score    = 0;
    int                          depth    = 0;
    std::vector<chess::bb::Move> pv;       // full PV from root, main-search moves only
};

// Entry point: initialises state and calls negamax with full window.
// tt may be nullptr (no TT used).
int search(chess::bb::BBoard& board, int depth, TT* tt = nullptr);

// Runs search and writes the final SearchState to *stateOut (for node-count testing).
int searchEx(chess::bb::BBoard& board, int depth, TT* tt, SearchState& stateOut);

// Iterative deepening search: runs negamax from depth 1 up to limits.depth,
// printing a UCI info line after each depth. Returns the best result found.
SearchResult searchID(chess::bb::BBoard& board, TT& tt, const SearchLimits& limits);

} // namespace chess::bbSearch

#endif // CHESS_BBSEARCH_SEARCH_H
