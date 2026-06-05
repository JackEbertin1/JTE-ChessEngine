#ifndef CHESS_BBSEARCH_PERFT_H
#define CHESS_BBSEARCH_PERFT_H

#include "chess/bb/BBoard.h"
#include "chess/bb/BBMove.h"
#include "chess/bb/MoveGen.h"

#include <cstdint>

//
//  Perft: performance test / move-path enumerator.
//
//  Used to validate move generation correctness by counting the number of
//  leaf nodes at a given depth from a position. Reference values for the
//  starting position and common test positions are published online and
//  can be used to bisect move-generation bugs.
//
//  Both functions take a non-const BBoard& because generateLegalMoves
//  requires a mutable board (it calls make/unmakeMove internally for
//  legality filtering).

namespace chess::bbSearch {

// Returns the total leaf-node count reachable from `board` at `depth`.
// Uses bulk counting: at depth == 1 returns the move count directly without
// recursing. At depth == 0 returns 1 (the node itself).
uint64_t perft(chess::bb::BBoard& board, int depth);

// Prints per-root-move leaf counts to stdout, then a grand total.
// Format per move: "<uci>: <count>\n"
// Format for total: "\nTotal: <total>\n"
// Useful for bisecting perft mismatches against a reference engine.
void perftDivide(chess::bb::BBoard& board, int depth);

} // namespace chess::bbSearch

#endif // CHESS_BBSEARCH_PERFT_H
