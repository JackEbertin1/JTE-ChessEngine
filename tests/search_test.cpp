// Verification gate for step C2 (negamax alpha-beta search).
// Standalone test, no CTest wiring.
//
// Coverage:
//   1. Stalemate position returns 0.
//   2. Mate-in-1 is found at depth 2 (score == MATE_SCORE - 1).
//   3. Mate-in-1 is NOT visible at depth 1 (score is not a mate score).
//   4. Perspective flip: white up a queen scores > 0 for white, < 0 for black.
//
// Build & run:
//   c++ -std=c++23 -Wall -Wextra -I include \
//     src/chess/bb/BBoard.cpp src/chess/bb/Zobrist.cpp src/chess/bb/MoveGen.cpp \
//     src/chess/bbEval/Eval.cpp src/chess/bbSearch/Search.cpp \
//     tests/search_test.cpp -o /tmp/search_test && /tmp/search_test

#include "chess/bb/BBoard.h"
#include "chess/bbSearch/Search.h"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace chess::bb;
using namespace chess::bbSearch;

namespace {

int g_failures = 0;
int g_checks   = 0;

void check(bool cond, const std::string& what) {
    ++g_checks;
    if (!cond) {
        ++g_failures;
        std::cerr << "FAIL: " << what << "\n";
    } else {
        std::cout << "PASS: " << what << "\n";
    }
}

BBoard boardFromFEN(const std::string& fen) {
    BBoard board;
    if (!board.setFromFEN(fen)) {
        std::cerr << "ERROR: could not parse FEN: " << fen << "\n";
        std::exit(EXIT_FAILURE);
    }
    return board;
}

} // namespace

int main() {
    // Test 1 — Stalemate returns 0.
    // FEN: k7/8/1Q6/8/8/8/8/7K b - - 0 1
    // Black to move, no legal moves, not in check.
    {
        BBoard board = boardFromFEN("k7/8/1Q6/8/8/8/8/7K b - - 0 1");
        int score = search(board, 1);
        check(score == 0,
              "Test 1 — stalemate returns 0 (got " + std::to_string(score) + ")");
    }

    // Test 2 — Mate-in-1 found at depth 2.
    // FEN: 6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1
    // White has Ra8# available. At depth 2 the rook delivers checkmate.
    // Expected: MATE_SCORE - 1 == 29999
    {
        BBoard board = boardFromFEN("6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1");
        int score = search(board, 2);
        check(score == MATE_SCORE - 1,
              "Test 2 — mate-in-1 found at depth 2 (got " + std::to_string(score)
              + ", expected " + std::to_string(MATE_SCORE - 1) + ")");
    }

    // Test 3 — Mate not visible at depth 1.
    // Same FEN as Test 2.
    // At depth 1 the engine only sees evaluate() at leaves, not checkmate.
    {
        BBoard board = boardFromFEN("6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1");
        int score = search(board, 1);
        check(!isMateScore(score),
              "Test 3 — mate-in-1 not visible at depth 1 (got " + std::to_string(score) + ")");
    }

    // Test 4 — Perspective flip through the search.
    // FEN: rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    // White is up a queen. White to move should score > 0; black to move < 0.
    {
        const std::string fenWhite = "rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        const std::string fenBlack = "rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";

        BBoard boardW = boardFromFEN(fenWhite);
        int scoreW = search(boardW, 1);
        check(scoreW > 0,
              "Test 4a — white up queen, white to move scores > 0 (got " + std::to_string(scoreW) + ")");

        BBoard boardB = boardFromFEN(fenBlack);
        int scoreB = search(boardB, 1);
        check(scoreB < 0,
              "Test 4b — white up queen, black to move scores < 0 (got " + std::to_string(scoreB) + ")");
    }

    std::cout << "\n" << g_checks << " checks, " << g_failures << " failures.\n";
    if (g_failures == 0) {
        std::cout << "ALL TESTS PASSED\n";
        return 0;
    }
    return 1;
}
