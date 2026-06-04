// Verification gate for step A8 (legality filter + check detection).
// Standalone test, no CTest wiring (deferred to step B2).
//
// Coverage:
//   1. Starting position: exactly 20 legal moves (white).
//   2. Kiwipete: exactly 48 legal moves (white).
//   3. King in check, one escape: isInCheck true; 4 legal king moves remain.
//   4. Fool's mate: isCheckmate true; 0 legal moves.
//   5. Stalemate: isStalemate true; 0 legal moves.
//   6. isSquareAttacked spot check: white pawn on d3 attacks c4 and e4, not d4.
//
// Build & run:
//   c++ -std=c++23 -Wall -Wextra -I include \
//       src/chess/bb/BBoard.cpp src/chess/bb/Zobrist.cpp src/chess/bb/MoveGen.cpp \
//       tests/legality_test.cpp -o /tmp/legality_test && /tmp/legality_test

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

// Parse a FEN into a BBoard, abort on failure.
BBoard boardFrom(const std::string& fen) {
    BBoard b;
    if (!b.setFromFEN(fen)) {
        std::cerr << "ERROR: could not parse FEN: " << fen << "\n";
        std::exit(EXIT_FAILURE);
    }
    return b;
}

// Generate legal moves from a FEN and return the list.
MoveList legalFrom(const std::string& fen) {
    BBoard b = boardFrom(fen);
    MoveList ml;
    generateLegalMoves(b, ml);
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
    // Test 1: Starting position — white has exactly 20 legal moves.
    //   16 pawn moves + 4 knight moves = 20 (same as pseudo-legal since
    //   no king can be left in check from the starting position).
    // ------------------------------------------------------------------
    {
        const std::string fen =
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        MoveList ml = legalFrom(fen);

        bool ok = (ml.count == 20);
        check(ok, "startpos: legal count == 20");
        if (!ok) {
            std::cerr << "  got " << ml.count << " legal moves:\n";
            dumpMoveList(ml);
        }
    }

    // ------------------------------------------------------------------
    // Test 2: Kiwipete — white has exactly 48 legal moves.
    //   r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1
    // ------------------------------------------------------------------
    {
        const std::string fen =
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
        MoveList ml = legalFrom(fen);

        bool ok = (ml.count == 48);
        check(ok, "Kiwipete: legal count == 48");
        if (!ok) {
            std::cerr << "  got " << ml.count << " legal moves:\n";
            dumpMoveList(ml);
        }
    }

    // ------------------------------------------------------------------
    // Test 3: King in check, one escape.
    //   "8/8/8/8/4r3/8/8/4K3 w - - 0 1"
    //   White king on e1, black rook on e4.  The rook controls the entire
    //   e-file, so e2 is not a legal king destination.  Remaining squares:
    //   d1, d2, f1, f2 — 4 legal moves.
    // ------------------------------------------------------------------
    {
        const std::string fen = "8/8/8/8/4r3/8/8/4K3 w - - 0 1";
        BBoard b = boardFrom(fen);

        bool inCheck = isInCheck(b, WHITE);
        check(inCheck, "king in check position: isInCheck(WHITE) == true");

        MoveList ml;
        generateLegalMoves(b, ml);
        bool ok = (ml.count == 4);
        check(ok, "king in check position: legal count == 4");
        if (!ok) {
            std::cerr << "  got " << ml.count << " legal moves:\n";
            dumpMoveList(ml);
        }
    }

    // ------------------------------------------------------------------
    // Test 4: Fool's mate (white is checkmated).
    //   "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3"
    //   After 1.f3 e5 2.g4 Qh4#.  White king on e1 is in check from Qh4
    //   with no legal escape.
    // ------------------------------------------------------------------
    {
        const std::string fen =
            "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3";
        BBoard b = boardFrom(fen);

        bool cm = isCheckmate(b);
        check(cm, "Fool's mate: isCheckmate == true");

        MoveList ml;
        generateLegalMoves(b, ml);
        bool ok = (ml.count == 0);
        check(ok, "Fool's mate: legal count == 0");
        if (!ok) {
            std::cerr << "  got " << ml.count << " legal moves:\n";
            dumpMoveList(ml);
        }
    }

    // ------------------------------------------------------------------
    // Test 5: Stalemate.
    //   "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1"
    //   Black king on h8 has no legal move; white queen covers all
    //   escape squares, but the black king is NOT currently in check.
    // ------------------------------------------------------------------
    {
        const std::string fen = "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1";
        BBoard b = boardFrom(fen);

        bool sm = isStalemate(b);
        check(sm, "stalemate: isStalemate == true");

        MoveList ml;
        generateLegalMoves(b, ml);
        bool ok = (ml.count == 0);
        check(ok, "stalemate: legal count == 0");
        if (!ok) {
            std::cerr << "  got " << ml.count << " legal moves:\n";
            dumpMoveList(ml);
        }
    }

    // ------------------------------------------------------------------
    // Test 6: isSquareAttacked spot check.
    //   "8/8/8/8/8/3P4/8/8 w - - 0 1"
    //   White pawn on d3.  It attacks c4 (C4) and e4 (E4), but NOT d4 (D4).
    //   (No kings in this FEN — we only call isSquareAttacked, not isInCheck.)
    // ------------------------------------------------------------------
    {
        const std::string fen = "8/8/8/8/8/3P4/8/8 w - - 0 1";
        BBoard b = boardFrom(fen);

        bool attacksC4 = isSquareAttacked(C4, WHITE, b);
        bool attacksE4 = isSquareAttacked(E4, WHITE, b);
        bool attacksD4 = isSquareAttacked(D4, WHITE, b);

        check(attacksC4, "pawn on d3: isSquareAttacked(C4, WHITE) == true");
        check(attacksE4, "pawn on d3: isSquareAttacked(E4, WHITE) == true");
        check(!attacksD4, "pawn on d3: isSquareAttacked(D4, WHITE) == false");
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
