#include "chess/bbSearch/Search.h"

#include "chess/bb/BBoard.h"
#include "chess/bb/BBMove.h"
#include "chess/bb/MoveGen.h"
#include "chess/bbEval/Eval.h"

namespace chess::bbSearch {

namespace {

int negamax(chess::bb::BBoard& board, SearchState& state, int alpha, int beta, int depth, int ply)
{
    ++state.nodes;

    if (depth == 0) {
        return chess::bbEval::evaluate(board);
    }

    chess::bb::MoveList ml;
    generateLegalMoves(board, ml);

    //If there are no legal moves generated, it is either checkmate or stalemate 
    if (ml.count == 0) {
        if (isInCheck(board, board.sideToMove)) {
            return -(MATE_SCORE - ply);  // checkmate — sooner mate is worse for the losing side
        }
        return 0;  // stalemate
    }

    for (int i = 0; i < ml.count; ++i) {
        chess::bb::Undo u;
        board.makeMove(ml.moves[i], u);
        int score = -negamax(board, state, -beta, -alpha, depth - 1, ply + 1);
        board.unmakeMove(ml.moves[i], u);

        if (score >= beta) {
            return beta;  // beta cutoff
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

} // anonymous namespace

int search(chess::bb::BBoard& board, int depth) {
    SearchState state;
    return negamax(board, state, -INF, INF, depth, 0);
}

} // namespace chess::bbSearch
