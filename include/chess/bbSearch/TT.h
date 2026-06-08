#ifndef CHESS_BBSEARCH_TT_H
#define CHESS_BBSEARCH_TT_H

#include "chess/bb/BBoard.h"   // pulls in BBMove.h (Move, MOVE_NONE) in correct order
#include <cstdint>
#include <optional>
#include <vector>

namespace chess::bbSearch {

enum TTFlag : uint8_t { TT_EXACT = 0, TT_LOWER = 1, TT_UPPER = 2 };

struct TTEntry {
    uint64_t        key        = 0;
    int16_t         score      = 0;
    chess::bb::Move move       = chess::bb::MOVE_NONE;
    uint8_t         depth      = 0;
    TTFlag          flag       = TT_EXACT;
    uint8_t         generation = 0;  // wraps at 256; used for age-based replacement
};

class TT {
public:
    // sizeMb: desired size in megabytes; actual size rounded down to power-of-2 entries
    explicit TT(int sizeMb = 64);

    void clear();

    // Increment the generation counter. Call once at the start of each new root
    // search so stale entries from the previous position can be overwritten.
    void newSearch();

    // Returns the stored score (adjusted for current ply) if a cutoff is possible,
    // or std::nullopt. Sets outMove to the stored best move (may be MOVE_NONE).
    // If outExact is non-null it is set to true only when the entry is TT_EXACT
    // (i.e. the returned score is precise, not a bound). Callers that need to
    // build a PV from TT hits should check this flag.
    std::optional<int> probe(uint64_t key, int depth, int alpha, int beta,
                             int ply, chess::bb::Move& outMove,
                             bool* outExact = nullptr) const;

    // Store an entry. flag: TT_EXACT / TT_LOWER / TT_UPPER.
    // Replacement policy: always replace stale-generation entries; for same-generation
    // entries with the same key, keep the deeper one.
    void store(uint64_t key, int depth, int score, TTFlag flag,
               chess::bb::Move move, int ply);

private:
    std::vector<TTEntry> table_;
    uint64_t mask_;        // = size - 1, for fast indexing
    uint8_t  currentGen_ = 0;
};

} // namespace chess::bbSearch

#endif // CHESS_BBSEARCH_TT_H
