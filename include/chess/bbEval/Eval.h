#ifndef CHESS_BBEVAL_EVAL_H
#define CHESS_BBEVAL_EVAL_H

#include "chess/bb/BBoard.h"
#include "chess/bb/Bitboard.h"

namespace chess::bbEval {

// Returns a centipawn score from the side-to-move's perspective.
// Positive = good for the side to move.
int evaluate(const chess::bb::BBoard& board);

} // namespace chess::bbEval

#endif // CHESS_BBEVAL_EVAL_H
