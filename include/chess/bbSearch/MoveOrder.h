#ifndef CHESS_BBSEARCH_MOVEORDER_H
#define CHESS_BBSEARCH_MOVEORDER_H

#include "chess/bb/BBoard.h"
#include "chess/bb/MoveGen.h"            // chess::bb::MoveList
#include "chess/bbSearch/SearchDefs.h"   // SearchState, MAX_PLY

namespace chess::bbSearch {

// Assign a score to every move in ml, written into scores[].
// TT move → 2'000'000 (tried first).
// Winning/equal capture (victim_val >= attacker_val): 10000 + victim_val*10 - attacker_val.
// Losing capture (victim_val < attacker_val): victim_val - attacker_val  (negative).
// Killer 1 → 9000, Killer 2 → 8999 (quiet moves only).
// Other quiet move → state.history[color][from][to].
void scoreMoves(const chess::bb::MoveList& ml,
                int scores[],
                const chess::bb::BBoard& board,
                const SearchState& state,
                chess::bb::Move ttMove,
                int ply);

// One step of selection sort: find the highest-scored move in [start, ml.count)
// and swap it (and its score) to position `start`.
void pickBestMove(chess::bb::MoveList& ml, int scores[], int start);

// On a beta cutoff by a quiet move: shift killers and store the new one.
// Never call with a capture.
void updateKillers(SearchState& state, chess::bb::Move move, int ply);

// Apply history bonus to a quiet move that caused a beta cutoff.
// bonus = depth * depth, capped at 8000 (below killer scores).
void applyHistoryBonus(SearchState& state, chess::bb::Move move,
                       chess::bb::Color color, int depth);

// Apply history malus to a quiet move that failed to cause a beta cutoff.
// malus = depth * depth, floored at -8000.
void applyHistoryMalus(SearchState& state, chess::bb::Move move,
                       chess::bb::Color color, int depth);

// Halve all history values (call after each completed iteration to preserve
// inter-iteration information without stale dominance).
void decayHistory(SearchState& state);

} // namespace chess::bbSearch

#endif // CHESS_BBSEARCH_MOVEORDER_H
