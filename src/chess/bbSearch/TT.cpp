#include "chess/bbSearch/TT.h"
#include "chess/bbSearch/SearchDefs.h"

#include <algorithm>  // std::fill
#include <cstddef>    // std::size_t

namespace chess::bbSearch {

// -----------------------------------------------------------------------
//  Helpers
// -----------------------------------------------------------------------

// Round n down to the largest power of 2 that is <= n. Returns 1 for n<=1.
static uint64_t floorPow2(uint64_t n) {
    if (n <= 1) return 1;
    // Highest set bit trick.
    uint64_t p = 1;
    while (p * 2 <= n) p *= 2;
    return p;
}

// -----------------------------------------------------------------------
//  TT
// -----------------------------------------------------------------------

TT::TT(int sizeMb) {
    uint64_t bytes   = static_cast<uint64_t>(sizeMb) * 1024ULL * 1024ULL;
    uint64_t entries = bytes / sizeof(TTEntry);
    if (entries < 1) entries = 1;
    entries = floorPow2(entries);
    table_.assign(entries, TTEntry{});
    mask_ = entries - 1;
}

void TT::clear() {
    std::fill(table_.begin(), table_.end(), TTEntry{});
    currentGen_ = 0;
}

void TT::newSearch() {
    ++currentGen_;
}

std::optional<int> TT::probe(uint64_t key, int depth, int alpha, int beta,
                              int ply, chess::bb::Move& outMove, bool* outExact) const
{
    const TTEntry& entry = table_[key & mask_];

    // Step 1 – key mismatch (hash collision).
    if (entry.key != key) {
        outMove = chess::bb::MOVE_NONE;
        return std::nullopt;
    }

    // Step 2 – set outMove regardless of depth check (useful for move ordering).
    outMove = entry.move;

    // Step 3 – insufficient depth: can't use for a cutoff.
    if (entry.depth < depth) {
        return std::nullopt;
    }

    // Step 4 – recover score with mate-distance adjustment.
    int score = entry.score;
    if (score > MATE_SCORE - MAX_PLY) {
        score -= ply;   // stored with +ply → recover by subtracting ply
    } else if (score < -(MATE_SCORE - MAX_PLY)) {
        score += ply;   // stored with -ply → recover by adding ply
    }

    // Step 5 – apply bound.
    switch (entry.flag) {
        case TT_EXACT:
            if (outExact) *outExact = true;
            return score;
        case TT_LOWER:   // lower bound — caused beta cutoff
            if (score >= beta) return score;
            break;
        case TT_UPPER:   // upper bound — all moves failed low
            if (score <= alpha) return score;
            break;
    }

    return std::nullopt;
}

void TT::store(uint64_t key, int depth, int score, TTFlag flag,
               chess::bb::Move move, int ply)
{
    TTEntry& entry = table_[key & mask_];

    // Age-aware replacement policy:
    //   - Always replace entries from a previous generation (stale).
    //   - For same-generation entries with the same key:
    //       * Never overwrite an EXACT entry with a bound (LOWER/UPPER).
    //       * Otherwise keep the deeper entry.
    if (entry.key == key && entry.generation == currentGen_) {
        if (entry.flag == TT_EXACT && flag != TT_EXACT) return;
        if (entry.depth > static_cast<uint8_t>(depth)) return;
    }

    // Adjust mate scores for storage: add ply to positive, subtract from negative.
    int stored = score;
    if (score > MATE_SCORE - MAX_PLY) {
        stored = score + ply;
    } else if (score < -(MATE_SCORE - MAX_PLY)) {
        stored = score - ply;
    }

    entry.key        = key;
    entry.score      = static_cast<int16_t>(stored);
    entry.move       = move;
    entry.depth      = static_cast<uint8_t>(depth);
    entry.flag       = flag;
    entry.generation = currentGen_;
}

} // namespace chess::bbSearch
