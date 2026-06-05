#include "chess/bb/BBoard.h"
#include "chess/bbSearch/Perft.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>

// ---------------------------------------------------------------------------
//  Minimal assertion helper — prints the failing check and exits with 1.
// ---------------------------------------------------------------------------

static bool g_failed = false;

static void check(bool cond, const char* expr, uint64_t got, uint64_t expected) {
    if (!cond) {
        std::cout << "FAIL: " << expr
                  << "  got=" << got << "  expected=" << expected << "\n";
        g_failed = true;
    }
}

#define ASSERT_EQ(got, expected) \
    check((got) == (expected), #got " == " #expected, (got), (expected))

// ---------------------------------------------------------------------------
//  Tests
// ---------------------------------------------------------------------------

static void test_perft_startpos() {
    chess::bb::BBoard board;
    board.setFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    ASSERT_EQ(chess::bbSearch::perft(board, 1), UINT64_C(20));
    ASSERT_EQ(chess::bbSearch::perft(board, 2), UINT64_C(400));
    ASSERT_EQ(chess::bbSearch::perft(board, 3), UINT64_C(8902));
    ASSERT_EQ(chess::bbSearch::perft(board, 4), UINT64_C(197281));
}

static void test_perft_divide_total() {
    chess::bb::BBoard board;
    board.setFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Redirect stdout so we can inspect the "Total:" line.
    std::streambuf* original = std::cout.rdbuf();
    std::ostringstream captured;
    std::cout.rdbuf(captured.rdbuf());

    chess::bbSearch::perftDivide(board, 2);

    std::cout.rdbuf(original);  // restore

    const std::string output = captured.str();

    // Re-print to stdout so the user can read it.
    std::cout << output;

    // Parse the "Total: <n>" line.
    const std::string marker = "Total: ";
    const auto pos = output.rfind(marker);
    if (pos == std::string::npos) {
        std::cout << "FAIL: perftDivide output missing 'Total:' line\n";
        g_failed = true;
        return;
    }

    const uint64_t total = std::stoull(output.substr(pos + marker.size()));
    ASSERT_EQ(total, UINT64_C(400));
}

// ---------------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------------

int main() {
    test_perft_startpos();
    test_perft_divide_total();

    if (g_failed) {
        std::cout << "SOME TESTS FAILED\n";
        return 1;
    }
    std::cout << "ALL TESTS PASSED\n";
    return 0;
}
