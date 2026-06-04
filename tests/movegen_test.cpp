// Verification gate for step A7 (pseudo-legal move generation).
// Standalone test, no CTest wiring (deferred to step B2).
//
// Coverage:
//   1. Starting position: exactly 20 pseudo-legal moves (white).
//   2. After 1.e4 (black to move): exactly 20 pseudo-legal moves, ep square set.
//   3. Pawn-only promotion position: exactly 4 promotion moves in list.
//   4. En-passant capture: the move "e5d6" with FLAG_EP_CAPTURE is present.
//   5. Both castling moves (OO and OOO) generated from a position with clear paths.
//
// Build & run:
//   c++ -std=c++23 -Wall -Wextra -I include \
//       src/chess/bb/BBoard.cpp src/chess/bb/Zobrist.cpp src/chess/bb/MoveGen.cpp \
//       tests/movegen_test.cpp -o /tmp/movegen_test && /tmp/movegen_test

#include "chess/bb/BBoard.h"
#include "chess/bb/BBMove.h"
#include "chess/bb/MoveGen.h"

#include <cstdlib>
#include <iostream>
#include <string>

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

// Parse a FEN, generate moves, and return the list. Aborts on bad FEN.
MoveList genFrom(const std::string& fen) {
    BBoard board;
    if (!board.setFromFEN(fen)) {
        std::cerr << "ERROR: could not parse FEN: " << fen << "\n";
        std::exit(EXIT_FAILURE);
    }
    MoveList ml;
    generateMoves(board, ml);
    return ml;
}

// Print all moves in the list to stderr (UCI notation), one per line.
void dumpMoveList(const MoveList& ml) {
    for (int i = 0; i < ml.count; ++i)
        std::cerr << "  [" << i << "] " << to_uci(ml.moves[i])
                  << "  flag=" << flag_of(ml.moves[i]) << "\n";
}

} // namespace

int main() {
    // ------------------------------------------------------------------
    // Test 1: Starting position — white has exactly 20 pseudo-legal moves.
    //   16 pawn moves (8 single pushes + 8 double pushes)
    //   +4 knight moves (Nb1-a3/c3 and Ng1-f3/h3)
    //   = 20
    // ------------------------------------------------------------------
    {
        const std::string fen =
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        MoveList ml = genFrom(fen);

        bool ok = (ml.count == 20);
        check(ok, "startpos: count == 20");
        if (!ok) {
            std::cerr << "  got " << ml.count << " moves:\n";
            dumpMoveList(ml);
        }
    }

    // ------------------------------------------------------------------
    // Test 2: After 1.e4 (black to move) — black has exactly 20 moves.
    //   Same reasoning as test 1 by symmetry.
    // ------------------------------------------------------------------
    {
        const std::string fen =
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
        MoveList ml = genFrom(fen);

        bool ok = (ml.count == 20);
        check(ok, "after 1.e4 (black): count == 20");
        if (!ok) {
            std::cerr << "  got " << ml.count << " moves:\n";
            dumpMoveList(ml);
        }
    }

    // ------------------------------------------------------------------
    // Test 3: Promotion position — white pawn on a7, kings far apart.
    //   "8/P7/8/8/8/8/8/4k2K w - - 0 1"
    //   The pawn on a7 can push to a8 → 4 promotion moves.
    //   The king on h1 has 2 moves (g1, g2).
    //   Total: 6 moves, of which exactly 4 are is_promotion().
    // ------------------------------------------------------------------
    {
        const std::string fen = "8/P7/8/8/8/8/8/4k2K w - - 0 1";
        MoveList ml = genFrom(fen);

        int promoCount = 0;
        for (int i = 0; i < ml.count; ++i)
            if (is_promotion(ml.moves[i]))
                ++promoCount;

        bool ok = (promoCount == 4);
        check(ok, "promotion position: exactly 4 promotion moves");
        if (!ok) {
            std::cerr << "  got " << promoCount << " promotion moves (total=" << ml.count << "):\n";
            dumpMoveList(ml);
        }
    }

    // ------------------------------------------------------------------
    // Test 4: En-passant capture available.
    //   "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3"
    //   White pawn on e5, black pawn just double-pushed to d5, ep target d6.
    //   Expect a move with is_en_passant() whose UCI string is "e5d6".
    // ------------------------------------------------------------------
    {
        const std::string fen =
            "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3";
        MoveList ml = genFrom(fen);

        bool found = false;
        for (int i = 0; i < ml.count; ++i) {
            if (is_en_passant(ml.moves[i]) && to_uci(ml.moves[i]) == "e5d6") {
                found = true;
                break;
            }
        }

        check(found, "en passant: FLAG_EP_CAPTURE move e5d6 present");
        if (!found) {
            std::cerr << "  full move list (" << ml.count << " moves):\n";
            dumpMoveList(ml);
        }
    }

    // ------------------------------------------------------------------
    // Test 5: Both castling directions for white — clear paths on both sides.
    //   "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"
    //   White king on e1, rooks on a1 and h1. B1/C1/D1 and F1/G1 all empty.
    //   Expect both FLAG_OO (e1g1) and FLAG_OOO (e1c1) to be in the list.
    // ------------------------------------------------------------------
    {
        const std::string fen = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
        MoveList ml = genFrom(fen);

        bool hasOO  = false;
        bool hasOOO = false;
        for (int i = 0; i < ml.count; ++i) {
            if (is_king_castle(ml.moves[i]))   hasOO  = true;
            if (is_queen_castle(ml.moves[i]))  hasOOO = true;
        }

        check(hasOO,  "castling: FLAG_OO move present");
        check(hasOOO, "castling: FLAG_OOO move present");
        if (!hasOO || !hasOOO) {
            std::cerr << "  full move list (" << ml.count << " moves):\n";
            dumpMoveList(ml);
        }
    }

    // ------------------------------------------------------------------
    // Summary
    // ------------------------------------------------------------------
    std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    if (g_failures == 0) {
        std::cout << "ALL TESTS PASSED\n";
        return EXIT_SUCCESS;
    }
    std::cout << g_failures << " CHECK(S) FAILED\n";
    return EXIT_FAILURE;
}
