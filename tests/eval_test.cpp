#include "chess/bbEval/Eval.h"
#include "chess/bb/BBoard.h"

#include <cstdlib>
#include <iostream>
#include <string>

// Minimal assertion helper: prints the failed expression and returns false.
static bool check(bool condition, const char* expr, const char* file, int line) {
    if (!condition) {
        std::cerr << "FAILED: " << expr << "  (" << file << ":" << line << ")\n";
    }
    return condition;
}

#define CHECK(expr) check((expr), #expr, __FILE__, __LINE__)

// Load a FEN into a BBoard and return the board. Aborts on parse failure.
static chess::bb::BBoard load(const std::string& fen) {
    chess::bb::BBoard b;
    if (!b.setFromFEN(fen)) {
        std::cerr << "FEN parse failed: " << fen << "\n";
        std::exit(1);
    }
    return b;
}

int main() {
    bool all_ok = true;

    // ------------------------------------------------------------------
    // Test 1: Starting position is symmetric -> score == 0
    // ------------------------------------------------------------------
    {
        chess::bb::BBoard b = load("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        int score = chess::bbEval::evaluate(b);
        all_ok &= CHECK(score == 0);
    }

    // ------------------------------------------------------------------
    // Test 2: White up a queen, white to move -> score > 0 and ~900
    // ------------------------------------------------------------------
    {
        // Black is missing the queen (d8 empty).
        chess::bb::BBoard b = load("rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        int score = chess::bbEval::evaluate(b);
        all_ok &= CHECK(score > 0);
        all_ok &= CHECK(std::abs(score - 900) < 50);
    }

    // ------------------------------------------------------------------
    // Test 3: Same position but black to move -> score < 0
    // ------------------------------------------------------------------
    {
        chess::bb::BBoard b = load("rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
        int score = chess::bbEval::evaluate(b);
        all_ok &= CHECK(score < 0);
    }

    // ------------------------------------------------------------------
    // Test 4: White pawn on e4, white king on e1, black king on e8,
    //         white to move -> score > 0 (pawn on e4 has a positive PST bonus)
    // ------------------------------------------------------------------
    {
        chess::bb::BBoard b = load("4k3/8/8/8/4P3/8/8/4K3 w - - 0 1");
        int score = chess::bbEval::evaluate(b);
        all_ok &= CHECK(score > 0);
    }

    // ------------------------------------------------------------------
    if (all_ok) {
        std::cout << "ALL TESTS PASSED\n";
        return 0;
    } else {
        return 1;
    }
}
