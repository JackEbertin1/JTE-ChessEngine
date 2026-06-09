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
constexpr int QSEARCH_DEPTH   = 4;
constexpr int LMR_FULL_DEPTH  = 4;  // moves before this index are always searched at full depth

int qsearch(chess::bb::BBoard& board, SearchState& state, int alpha, int beta, int ply, int qdepth)
{
    ++state.nodes;
    if (state.stop) return 0;

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

int negamax(chess::bb::BBoard& board, SearchState& state, int alpha, int beta, int depth, int ply, bool isNullNode)
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

    // Periodic hard-deadline check (every 2048 nodes avoids excessive clock calls).
    if ((state.nodes & 2047) == 0) {
        if (std::chrono::steady_clock::now() >= state.hardDeadline)
            state.stop = true;
    }
    if (state.stop) return 0;

    const bool inCheck = isInCheck(board, board.sideToMove);

    // Null Move Pruning: skip our turn and see if the opponent still fails to
    // beat beta. If so, the position is good enough that we can prune.
    // Guards: not in check (illegal to pass), no consecutive null moves,
    // minimum depth, non-trivial material (avoids zugzwang in K+P endings),
    // and beta not near a mate score (avoids corrupting mate-distance values).
    if (!isNullNode && !inCheck && depth >= 3
            && std::abs(beta) < MATE_SCORE - MAX_PLY) {
        const bool hasNonPawnMaterial =
            (board.pieceBB[board.sideToMove][chess::bb::KNIGHT] |
             board.pieceBB[board.sideToMove][chess::bb::BISHOP] |
             board.pieceBB[board.sideToMove][chess::bb::ROOK]   |
             board.pieceBB[board.sideToMove][chess::bb::QUEEN]) != 0;
        if (hasNonPawnMaterial) {
            const int R          = 3 + depth / 6;
            const int nullDepth  = std::max(0, depth - 1 - R);
            chess::bb::Undo nullUndo;
            board.makeNullMove(nullUndo);
            const int nullScore = -negamax(board, state, -beta, -beta + 1,
                                           nullDepth, ply + 1, true);
            board.unmakeNullMove(nullUndo);
            if (nullScore >= beta) return beta;
        }
    }

    chess::bb::MoveList ml;
    generateLegalMoves(board, ml);

    // If there are no legal moves, it is either checkmate or stalemate.
    if (ml.count == 0) {
        if (inCheck) {
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

        // Killers are quiet moves expected to be strong; don't reduce them.
        const bool isKiller = !isCapture &&
            (move == state.killers[ply][0] || move == state.killers[ply][1]);

        chess::bb::Undo u;
        board.makeMove(move, u);
        int score;
        if (i == 0) {
            // PVS: first move always gets a full window.
            score = -negamax(board, state, -beta, -alpha, depth - 1, ply + 1, false);
        } else if (i >= LMR_FULL_DEPTH && depth >= 3 && !isCapture && !inCheck && !isKiller) {
            // Late Move Reductions: later quiet moves are searched at reduced depth
            // with a null window. R scales with move index and depth.
            int R = std::max(1, 1 + (i >= 6 ? 1 : 0) + (depth >= 6 ? 1 : 0));
            R = std::min(R, depth - 2);  // never reduce into qsearch territory
            score = -negamax(board, state, -alpha - 1, -alpha, depth - 1 - R, ply + 1, false);
            // If the reduced search raised alpha, verify at full depth.
            if (!state.stop && score > alpha)
                score = -negamax(board, state, -beta, -alpha, depth - 1, ply + 1, false);
        } else {
            // PVS: non-first moves get a null-window probe at full depth.
            score = -negamax(board, state, -alpha - 1, -alpha, depth - 1, ply + 1, false);
            if (!state.stop && score > alpha && score < beta)
                score = -negamax(board, state, -beta, -alpha, depth - 1, ply + 1, false);
        }
        board.unmakeMove(move, u);
        if (state.stop) return 0;

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
    return negamax(board, state, -INF, INF, depth, 0, false);
}

int searchEx(chess::bb::BBoard& board, int depth, TT* tt, SearchState& stateOut) {
    stateOut = SearchState{};
    stateOut.tt = tt;
    return negamax(board, stateOut, -INF, INF, depth, 0, false);
}

SearchResult searchID(chess::bb::BBoard& board, TT& tt, const SearchLimits& limits)
{
    tt.newSearch();

    SearchState state;
    state.tt = &tt;

    // --- Time management: derive soft and hard deadlines from limits. ---
    {
        using Clock = std::chrono::steady_clock;
        const auto now = Clock::now();

        if (limits.movetime > 0) {
            // Fixed time per move: hard and soft are the same deadline.
            const auto dl = now + std::chrono::milliseconds(limits.movetime);
            state.softDeadline = dl;
            state.hardDeadline = dl;
        } else {
            const int myTime = (board.sideToMove == chess::bb::WHITE)
                               ? limits.wtime : limits.btime;
            const int myInc  = (board.sideToMove == chess::bb::WHITE)
                               ? limits.winc  : limits.binc;
            if (myTime > 0 || myInc > 0) {
                // Allocate a fraction of the remaining clock.
                int softMs = (limits.movestogo > 0)
                             ? myTime / (limits.movestogo + 2)
                             : myTime / 20 + myInc * 3 / 4;
                // Never spend more than half the remaining time on one move.
                softMs = std::max(1, std::min(softMs, myTime / 2));
                // Hard limit: 3x soft, capped at 75% of remaining time.
                const int hardMs = std::max(softMs,
                                            std::min(softMs * 3, myTime * 3 / 4));
                state.softDeadline = now + std::chrono::milliseconds(softMs);
                state.hardDeadline = now + std::chrono::milliseconds(hardMs);
            }
            // limits.depth only, or limits.infinite: deadlines stay at max.
        }
    }

    SearchResult result;
    int maxDepth = (limits.depth > 0 && limits.depth <= MAX_PLY) ? limits.depth : MAX_PLY;

    for (int depth = 1; depth <= maxDepth; ++depth) {
        int score;

        if (depth < 4 || isMateScore(result.score)) {
            // Full window for early depths or when the previous score is a mate.
            score = negamax(board, state, -INF, INF, depth, 0, false);
            if (state.stop) break;
        } else {
            // Aspiration window: center on the previous iteration's score.
            int delta      = 150;
            int aspirAlpha = result.score - delta;
            int aspirBeta  = result.score + delta;

            while (true) {
                score = negamax(board, state, aspirAlpha, aspirBeta, depth, 0, false);
                if (state.stop) break;

                if (score <= aspirAlpha) {
                    aspirAlpha = std::max(aspirAlpha - delta, -INF);
                    delta *= 2;
                } else if (score >= aspirBeta) {
                    aspirBeta = std::min(aspirBeta + delta, INF);
                    delta *= 2;
                } else {
                    break;  // score is within the window
                }
            }
            if (state.stop) break;
        }

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

        // Soft deadline: don't start a new depth that is unlikely to finish in time.
        if (std::chrono::steady_clock::now() >= state.softDeadline) break;
    }

    return result;
}

} // namespace chess::bbSearch
