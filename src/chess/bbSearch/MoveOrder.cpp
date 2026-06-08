#include "chess/bbSearch/MoveOrder.h"

#include "chess/bb/BBMove.h"
#include "chess/bb/BBoard.h"
#include "chess/bbSearch/Search.h"

namespace chess::bbSearch {

void scoreMoves(const chess::bb::MoveList& ml,
                int scores[],
                const chess::bb::BBoard& board,
                const SearchState& state,
                chess::bb::Move ttMove,
                int ply)
{
    using namespace chess::bb;

    for (int i = 0; i < ml.count; ++i) {
        Move move = ml.moves[i];

        // 1. TT move gets highest priority.
        if (move == ttMove) {
            scores[i] = 2'000'000;
            continue;
        }

        Move flag = flag_of(move);
        bool isCapture = (flag & FLAG_BIT_CAPTURE) != 0;

        if (isCapture) {
            // 2. Determine attacker and victim.
            int from = from_sq(move);
            int to   = to_sq(move);

            PieceType attacker = board.pieceTypeOn(board.sideToMove, from);
            PieceType victim;

            // En passant: the captured pawn is not on to_sq, so pieceTypeOn
            // would return NO_PIECE_TYPE. Force victim = PAWN.
            if (flag == FLAG_EP_CAPTURE) {
                victim = PAWN;
            } else {
                victim = board.pieceTypeOn(static_cast<Color>(~board.sideToMove & 1), to);
            }

            // King cannot be recaptured, so its capture risk is zero.
            // Using PIECE_VALUES[KING]=20000 would wrongly classify all king
            // captures as large losing moves; use 0 instead.
            static constexpr int MVV_LVA_ATTACKER[chess::bb::NUM_PIECE_TYPES] = {
                100,   // PAWN
                320,   // KNIGHT
                330,   // BISHOP
                500,   // ROOK
                900,   // QUEEN
                0      // KING — never recaptured
            };
            int av = MVV_LVA_ATTACKER[attacker];
            int vv = chess::bbEval::PIECE_VALUES[victim];

            // 3. MVV-LVA scoring.
            if (vv >= av) {
                scores[i] = 10000 + vv * 10 - av;
            } else {
                scores[i] = vv - av;   // negative for losing captures
            }
        } else {
            // 4. Quiet move ordering: killers then history.
            if (move == state.killers[ply][0]) {
                scores[i] = 9000;
            } else if (move == state.killers[ply][1]) {
                scores[i] = 8999;
            } else {
                int from = from_sq(move);
                int to   = to_sq(move);
                scores[i] = state.history[board.sideToMove][from][to];
            }
        }
    }
}

void pickBestMove(chess::bb::MoveList& ml, int scores[], int start)
{
    int best = start;
    for (int i = start + 1; i < ml.count; ++i) {
        if (scores[i] > scores[best]) {
            best = i;
        }
    }
    if (best != start) {
        // Swap move and its score to position start.
        chess::bb::Move tmpMove = ml.moves[start];
        ml.moves[start]  = ml.moves[best];
        ml.moves[best]   = tmpMove;

        int tmpScore  = scores[start];
        scores[start] = scores[best];
        scores[best]  = tmpScore;
    }
}

void updateKillers(SearchState& state, chess::bb::Move move, int ply)
{
    // Do not store duplicates.
    if (move != state.killers[ply][0]) {
        state.killers[ply][1] = state.killers[ply][0];
        state.killers[ply][0] = move;
    }
}

void applyHistoryBonus(SearchState& state, chess::bb::Move move,
                       chess::bb::Color color, int depth)
{
    int& h = state.history[color][chess::bb::from_sq(move)][chess::bb::to_sq(move)];
    h += depth * depth;
    if (h > 8000) h = 8000;
}

void applyHistoryMalus(SearchState& state, chess::bb::Move move,
                       chess::bb::Color color, int depth)
{
    int& h = state.history[color][chess::bb::from_sq(move)][chess::bb::to_sq(move)];
    h -= depth * depth;
    if (h < -8000) h = -8000;
}

void decayHistory(SearchState& state)
{
    for (int c = 0; c < 2; ++c)
        for (int f = 0; f < 64; ++f)
            for (int t = 0; t < 64; ++t)
                state.history[c][f][t] >>= 1;
}

} // namespace chess::bbSearch
