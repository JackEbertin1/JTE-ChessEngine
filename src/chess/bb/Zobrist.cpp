#include "chess/bb/Zobrist.h"

#include <random>

namespace chess::bb {

namespace {

// Build the key table once, drawing random numbers in the exact same order
// as the original engine (src/chess/representation/Zobrist.cpp) so that the
// resulting hashes are bit-identical.
ZobristKeys buildKeys() {
    ZobristKeys k{};
    std::mt19937_64 rng(0xDEADBEEF);            // fixed seed for reproducibility
    std::uniform_int_distribution<std::uint64_t> dist;

    // color(0..1), type(0..5), square(0..63) reproduces the old engine's
    // i=0..11 / rank / file draw order exactly (square == rank*8 + file).
    for (int color = 0; color < 2; ++color)
        for (int type = 0; type < 6; ++type)
            for (int sq = 0; sq < 64; ++sq)
                k.pieceTable[color][type][sq] = dist(rng);

    for (int i = 0; i < 4; ++i) {
        k.castling[i]      = dist(rng);
        k.enPassant[i]     = dist(rng);
        k.enPassant[i + 4] = dist(rng);
    }

    k.sideToMove = dist(rng);
    return k;
}

} // namespace

const ZobristKeys& zobrist() {
    static const ZobristKeys keys = buildKeys();
    return keys;
}

} // namespace chess::bb
