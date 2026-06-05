#ifndef CHESS_BBSEARCH_SEARCH_H
#define CHESS_BBSEARCH_SEARCH_H

#include "chess/bb/BBoard.h"
#include "chess/bb/BBMove.h"
#include "chess/bb/MoveGen.h"
#include "chess/bbEval/Eval.h"

#include <cstdlib>  // std::abs

namespace chess::bbSearch {

constexpr int MATE_SCORE = 30000;
constexpr int MAX_PLY    = 128;
constexpr int INF        = 1'000'000;

// Returns true if |score| indicates a forced mate.
inline bool isMateScore(int score) { return std::abs(score) > MATE_SCORE - MAX_PLY; }

struct SearchState {
    int  nodes = 0;
    bool stop  = false;
};

// Entry point: initialises state and calls negamax with full window.
int search(chess::bb::BBoard& board, int depth);

} // namespace chess::bbSearch

#endif // CHESS_BBSEARCH_SEARCH_H
