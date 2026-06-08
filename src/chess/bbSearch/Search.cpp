#include "chess/bbSearch/Search.h"

#include "chess/bb/BBoard.h"
#include "chess/bb/BBMove.h"
#include "chess/bb/MoveGen.h"
#include "chess/bbEval/Eval.h"
#include "chess/bbSearch/TT.h"
#include "chess/bbSearch/MoveOrder.h"

#include <algorithm>
#include <chrono>
#include <iostream>

namespace chess::bbSearch {

namespace {

// qdepth counts down from a small budget; when it hits 0 we stop recursing.
// This bounds qsearch to at most QSEARCH_DEPTH extra plies beyond the main search,
// preventing explosion in long capture/check chains.
constexpr int QSEARCH_DEPTH = 4;

int qsearch(chess::bb::BBoard& board, SearchState& state, int alpha, int beta, int ply, int qdepth)
{
    ++state.nodes;

    bool inCheck = isInCheck(board, board.sideToMove);

    if (!inCheck) {
        int standPat = chess::bbEval::evaluate(board);
        if (standPat >= beta)  return standPat;
        if (standPat + 1100 < alpha) return standPat;
        if (standPat > alpha) alpha = standPat;
    }

    // Budget exhausted.  Not in check: return the best lower bound we accumulated
    // (stand-pat or a previous capture's score).  In check: evaluate() does not
    // detect checkmate, so we must verify legal moves exist before returning eval.
    if (qdepth <= 0) {
        if (!inCheck) return alpha;
        chess::bb::MoveList ml;
        generateLegalMoves(board, ml);
        if (ml.count == 0) return -(MATE_SCORE - ply);
        return chess::bbEval::evaluate(board);
    }

    chess::bb::MoveList ml;
    generateLegalMoves(board, ml);

    if (ml.count == 0) {
        if (inCheck) return -(MATE_SCORE - ply);
        return 0;
    }

    int scores[256];
    // killers is sized [MAX_PLY + 1], so ply up to MAX_PLY is safe.
    // Clamp defensively in case qsearch is ever called beyond that.
    int safePly = (ply <= MAX_PLY) ? ply : MAX_PLY;
    scoreMoves(ml, scores, board, state, chess::bb::MOVE_NONE, safePly);

    for (int i = 0; i < ml.count; ++i) {
        pickBestMove(ml, scores, i);
        chess::bb::Move move = ml.moves[i];

        if (!inCheck && !(chess::bb::flag_of(move) & chess::bb::FLAG_BIT_CAPTURE)) continue;

        chess::bb::Undo u;
        board.makeMove(move, u);
        int score = -qsearch(board, state, -beta, -alpha, ply + 1, qdepth - 1);
        board.unmakeMove(move, u);

        if (score >= beta) return score;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

int negamax(chess::bb::BBoard& board, SearchState& state, int alpha, int beta, int depth, int ply)
{
    state.pvLength[ply] = 0;

   

    // Check if current position exists in the Transposition table
    chess::bb::Move ttMove = chess::bb::MOVE_NONE;
    if (state.tt) {
        bool ttExact = false;
        auto hit = state.tt->probe(board.hash, depth, alpha, beta, ply, ttMove, &ttExact);
        if (hit) {
            // For exact hits, record the best move so the PV is not left empty.
            // Bound hits (TT_LOWER / TT_UPPER) must not touch the PV table.
            if (ttExact && ttMove != chess::bb::MOVE_NONE) {
                state.pvTable[ply][0] = ttMove;
                state.pvLength[ply]   = 1;
            }
            return *hit;
        }
    }

    // Base Case
    if (depth == 0) {
        return qsearch(board, state, alpha, beta, ply, QSEARCH_DEPTH);
    }

     ++state.nodes;

    chess::bb::MoveList ml;
    generateLegalMoves(board, ml);

    // If there are no legal moves, it is either checkmate or stalemate.
    if (ml.count == 0) {
        if (isInCheck(board, board.sideToMove)) {
            return -(MATE_SCORE - ply);  // checkmate — sooner mate is worse for the losing side
        }
        return 0;  // stalemate
    }

    // Score and sort moves using MVV-LVA, killers, history, and TT move.
    int scores[256];
    scoreMoves(ml, scores, board, state, ttMove, ply);

    int originalAlpha = alpha;
    chess::bb::Move bestMove = chess::bb::MOVE_NONE;

    chess::bb::Move quietsTried[256];
    int quietsCount = 0;

    for (int i = 0; i < ml.count; ++i) {
        pickBestMove(ml, scores, i);   // selection-sort step
        chess::bb::Move move = ml.moves[i];
        bool isCapture = (chess::bb::flag_of(move) & chess::bb::FLAG_BIT_CAPTURE) != 0;

        chess::bb::Undo u;
        board.makeMove(move, u);
        int score = -negamax(board, state, -beta, -alpha, depth - 1, ply + 1);
        board.unmakeMove(move, u);

        if (score >= beta) {
            if (!isCapture) {
                updateKillers(state, move, ply);
                // board.sideToMove is already restored after unmakeMove
                applyHistoryBonus(state, move, board.sideToMove, depth);
                for (int q = 0; q < quietsCount; ++q)
                    applyHistoryMalus(state, quietsTried[q], board.sideToMove, depth);
            }
            // --- TT store (LOWER bound — caused a beta cutoff) ---
            if (state.tt)
                state.tt->store(board.hash, depth, score, TT_LOWER, move, ply);
            return score;  // fail-soft: return actual score, not the window bound
        }

        // Track quiet moves that did not cause a cutoff for potential malus application.
        if (!isCapture)
            quietsTried[quietsCount++] = move;

        if (score > alpha) {
            alpha = score;
            bestMove = move;

            // Triangular PV update
            state.pvTable[ply][0] = move;
            std::copy(state.pvTable[ply + 1],
                      state.pvTable[ply + 1] + state.pvLength[ply + 1],
                      state.pvTable[ply] + 1);
            state.pvLength[ply] = 1 + state.pvLength[ply + 1];
        }
    }

    // When all moves fail low, preserve the TT move from the probe as the best ordering
    // hint for future searches rather than overwriting with MOVE_NONE.
    if (state.tt) {
        TTFlag flag = (alpha > originalAlpha) ? TT_EXACT : TT_UPPER;
        chess::bb::Move moveToStore = (bestMove != chess::bb::MOVE_NONE) ? bestMove : ttMove;
        state.tt->store(board.hash, depth, alpha, flag, moveToStore, ply);
    }

    return alpha;
}

} // anonymous namespace

int search(chess::bb::BBoard& board, int depth, TT* tt) {
    SearchState state;
    state.tt = tt;
    return negamax(board, state, -INF, INF, depth, 0);
}

int searchEx(chess::bb::BBoard& board, int depth, TT* tt, SearchState& stateOut) {
    stateOut = SearchState{};
    stateOut.tt = tt;
    return negamax(board, stateOut, -INF, INF, depth, 0);
}

SearchResult searchID(chess::bb::BBoard& board, TT& tt, const SearchLimits& limits)
{
    tt.newSearch();

    SearchState state;
    state.tt = &tt;

    SearchResult result;
    int maxDepth = (limits.depth > 0 && limits.depth <= MAX_PLY) ? limits.depth : MAX_PLY;

    for (int depth = 1; depth <= maxDepth; ++depth) {
        int score = negamax(board, state, -INF, INF, depth, 0);

        if (state.stop) break;   // C8 will set this; ignored for now

        // Commit this depth's result
        result.score = score;
        result.depth = depth;

        if (state.pvLength[0] > 0) {
            result.bestMove = state.pvTable[0][0];
            result.pv.assign(state.pvTable[0],
                             state.pvTable[0] + state.pvLength[0]);
        }

        std::cout << "Finished depth " << depth << ". Eval: ";
        if (isMateScore(score)) {
            int plies = MATE_SCORE - std::abs(score);
            int moves  = (plies + 1) / 2;
            std::cout << (score > 0 ? "mate in " : "mated in ") << moves;
        } else {
            std::cout << (score > 0 ? "+" : "") << score << " cp";
        }
        std::cout << ", BestMove: " << chess::bb::to_uci(result.bestMove) << '\n';
        std::cout.flush();

        // Decay after committing so each new iteration starts with history
        // from the completed depth, halved to prevent stale dominance.
        decayHistory(state);
    }

    return result;
}

} // namespace chess::bbSearch
