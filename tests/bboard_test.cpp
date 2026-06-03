// Verification gate for step A2 (bitboard Board: FEN, occupancy, Zobrist,
// printBoard). Standalone test, no CTest wiring (deferred to step B2).
//
// Build & run:
//   c++ -std=c++23 -Wall -Wextra -I include src/chess/bb/BBoard.cpp src/chess/bb/Zobrist.cpp tests/bboard_test.cpp -o /tmp/bboard_test && /tmp/bboard_test

#include "chess/bb/BBoard.h"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace chess::bb;

namespace {

int g_failures = 0;
int g_checks   = 0;

void check(bool cond, const std::string& what) {
    ++g_checks;
    if (!cond) {
        ++g_failures;
        std::cerr << "FAIL: " << what << "\n";
    }
}

// Count piece characters (letters) in a FEN placement field.
int countPiecesInFEN(const std::string& fen) {
    int n = 0;
    for (char c : fen) {
        if (c == ' ') break;       // placement field ends at first space
        if (std::isalpha(static_cast<unsigned char>(c))) ++n;
    }
    return n;
}

} // namespace

int main() {
    const std::vector<std::string> fens = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",         // Starting Position
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", //kiwipete
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
    };

    // 1. FEN round-trip.
    for (const auto& fen : fens) {
        BBoard b;
        bool ok = b.setFromFEN(fen);
        check(ok, "setFromFEN succeeds for: " + fen);
        std::string out = b.toFEN();
        check(out == fen, "round-trip mismatch:\n   in:  " + fen + "\n   out: " + out);
        b.printBoard(std::cout);
    }

    // 2. Occupancy invariants.
    for (const auto& fen : fens) {
        BBoard b;
        b.setFromFEN(fen);

        check(b.occupancyAll == (b.occupancy[WHITE] | b.occupancy[BLACK]),
              "occupancyAll == white | black for: " + fen);

        for (int c = 0; c < NUM_COLORS; ++c) {
            Bitboard acc = EMPTY;
            for (int t = 0; t < NUM_PIECE_TYPES; ++t) acc |= b.pieceBB[c][t];
            check(b.occupancy[c] == acc,
                  "occupancy[c] == OR of pieceBB[c][..] for: " + fen);
        }

        // No square set in two different piece bitboards.
        bool disjoint = true;
        Bitboard all[2 * NUM_PIECE_TYPES];
        int idx = 0;
        for (int c = 0; c < NUM_COLORS; ++c)
            for (int t = 0; t < NUM_PIECE_TYPES; ++t)
                all[idx++] = b.pieceBB[c][t];
        for (int i = 0; i < idx && disjoint; ++i)
            for (int j = i + 1; j < idx; ++j)
                if (all[i] & all[j]) { disjoint = false; break; }
        check(disjoint, "piece bitboards pairwise disjoint for: " + fen);

        check(popcount(b.occupancyAll) == countPiecesInFEN(fen),
              "popcount(occupancyAll) matches FEN piece count for: " + fen);
    }

    // Startpos has exactly 32 pieces.
    {
        BBoard b;
        b.setFromFEN(fens[0]);
        check(popcount(b.occupancyAll) == 32, "startpos has 32 pieces");
    }

    // 3. Hash determinism.
    {
        BBoard a, b;
        a.setFromFEN(fens[0]);
        b.setFromFEN(fens[0]);
        check(a.hash == b.hash, "same FEN -> same hash");

        BBoard c;
        c.setFromFEN(fens[1]);
        check(a.hash != c.hash, "different FEN -> different hash");

        // Side-to-move must affect the hash: startpos w vs b.
        BBoard w, blk;
        w.setFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        blk.setFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
        check(w.hash != blk.hash, "side-to-move changes the hash");

        // recomputeHash is idempotent.
        std::uint64_t h1 = a.hash;
        std::uint64_t h2 = a.computeHash();
        check(h1 == h2, "computeHash is idempotent");
    }

    // 4. printBoard spot check.
    {
        BBoard b;
        b.setFromFEN(fens[0]);
        std::ostringstream oss;
        b.printBoard(oss);
        std::string s = oss.str();
        check(s.find("R N B Q K B N R") != std::string::npos,
              "printBoard shows white back rank");
        check(s.find("r n b q k b n r") != std::string::npos,
              "printBoard shows black back rank");
        check(s.find("a b c d e f g h") != std::string::npos,
              "printBoard shows file legend");
        check(s.find("white") != std::string::npos,
              "printBoard shows side to move");
    }

    std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    if (g_failures == 0) {
        std::cout << "ALL TESTS PASSED\n";
        return EXIT_SUCCESS;
    }
    std::cout << g_failures << " CHECK(S) FAILED\n";
    return EXIT_FAILURE;
}
