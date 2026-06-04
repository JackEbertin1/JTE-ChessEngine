#ifndef CHESS_BB_MOVEGEN_H
#define CHESS_BB_MOVEGEN_H

#include "chess/bb/BBoard.h"
#include "chess/bb/BBMove.h"

//
//  MoveGen: pseudo-legal move generation (step A7).
//
//  A pseudo-legal move is physically valid for the moving piece but does NOT
//  guarantee the king is not left in check after the move. Legality filtering
//  (isSquareAttacked + king-in-check test) is step A8.
//
//  Public API: one struct, one function.
//
//  MoveList capacity is fixed at 256 entries (stack-allocated, zero heap
//  activity). The theoretical maximum pseudo-legal count for any reachable
//  chess position is around 218; 256 gives safe headroom.
//
//  Callers own and clear the MoveList — generateMoves() only appends.

namespace chess::bb {

struct MoveList {
    static constexpr int MAX_MOVES = 256;
    Move moves[MAX_MOVES];
    int  count = 0;

    void        add(Move m)   { moves[count++] = m; }
    const Move* begin() const { return moves; }
    const Move* end()   const { return moves + count; }
};

// Generate all pseudo-legal moves for board.sideToMove into ml.
// Pseudo-legal: physically valid piece movement; does NOT filter for the
// king being left in check
// Callers own and clear the MoveList — do not clear it inside this function.
void generateMoves(const BBoard& board, MoveList& ml);

// ---------------------------------------------------------------------------
//  A8 — Legality filter + check detection
// ---------------------------------------------------------------------------

// Returns true if square `sq` is attacked by any piece of color `byColor`.
// Uses the reverse-lookup pattern: each attack function is applied FROM `sq`
// to see whether a piece of the appropriate type can reach it.
bool isSquareAttacked(int sq, Color byColor, const BBoard& b);

// Returns true if the king of color `c` is currently in check.
bool isInCheck(const BBoard& b, Color c);

// Populate `ml` with only the legal moves for board.sideToMove — i.e. the
// subset of pseudo-legal moves that do not leave the moving side's king in
// check. Uses make/unmake internally; generateMoves stays pseudo-legal.
// Callers own and clear the MoveList.
void generateLegalMoves(BBoard& board, MoveList& ml);

// Returns true if the side to move has no legal moves AND is in check.
bool isCheckmate(BBoard& b);

// Returns true if the side to move has no legal moves AND is NOT in check.
bool isStalemate(BBoard& b);

} // namespace chess::bb

#endif // CHESS_BB_MOVEGEN_H
