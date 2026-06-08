#ifndef CHESS_BBEVAL_EVAL_H
#define CHESS_BBEVAL_EVAL_H

#include "chess/bb/BBoard.h"
#include "chess/bb/Bitboard.h"

namespace chess::bbEval {

inline constexpr int PIECE_VALUES[chess::bb::NUM_PIECE_TYPES] = {
    100,    // PAWN
    320,    // KNIGHT
    330,    // BISHOP
    500,    // ROOK
    900,    // QUEEN
    20000   // KING
};

// Returns a centipawn score from the side-to-move's perspective.
// Positive = good for the side to move.
int evaluate(const chess::bb::BBoard& board);

} // namespace chess::bbEval

#endif // CHESS_BBEVAL_EVAL_H
