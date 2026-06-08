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
#include "chess/bbSearch/TT.h"
#include "chess/bbSearch/MoveOrder.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

    // Test 3 — Mate-in-1 found at depth 1 with qsearch.
    // Same FEN as Test 2.
    // With qsearch, even depth 1 finds the mate: Ra8+ leads to checkmate in qsearch.
    {
        BBoard board = boardFromFEN("6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1");
        int score = search(board, 1);
        check(isMateScore(score),
              "Test 3 — mate-in-1 found at depth 1 via qsearch (got " + std::to_string(score) + ")");
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

    // Test 5 — Prefer shorter mate over longer one (ply-adjusted scoring).
    // FEN: 6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1
    // White has Ra8# (mate-in-1). At depth 3 the engine must still return
    // MATE_SCORE - 1 == 29999, proving it prefers the shortest forced mate.
    {
        BBoard board = boardFromFEN("6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1");
        int score = search(board, 3);
        check(score == MATE_SCORE - 1,
              "Test 5 — depth-3 prefers mate-in-1 (got " + std::to_string(score)
              + ", expected " + std::to_string(MATE_SCORE - 1) + ")");
    }

    // Test 6 — Node count drops with TT enabled at depth 7.
    // Without iterative deepening, the TT provides ~1.4x reduction on a fresh
    // search from startpos (transposition hits + move ordering). Require >=1.3x.
    {
        const std::string startFEN =
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

        BBoard boardNoTT = boardFromFEN(startFEN);
        SearchState stateNoTT;
        searchEx(boardNoTT, 7, nullptr, stateNoTT);
        uint64_t nodesWithout = stateNoTT.nodes;

        TT tt6(1);
        BBoard boardTT = boardFromFEN(startFEN);
        SearchState stateTT;
        searchEx(boardTT, 7, &tt6, stateTT);
        uint64_t nodesWith = stateTT.nodes;

        // Require at least 1.3x reduction: nodesWith * 13 <= nodesWithout * 10
        check(nodesWith * 13 <= nodesWithout * 10,
              "Test 6 — node count drops >=1.3x at depth 7 with TT (without="
              + std::to_string(nodesWithout) + ", with=" + std::to_string(nodesWith) + ")");
    }

    // Test 7 — Re-search uses TT (nodes collapse on second run).
    {
        const std::string startFEN =
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

        TT tt7(1);
        SearchState s1, s2;
        BBoard b1 = boardFromFEN(startFEN);
        searchEx(b1, 6, &tt7, s1);   // populates TT

        BBoard b2 = boardFromFEN(startFEN);
        searchEx(b2, 6, &tt7, s2);   // should hit TT heavily

        check(s2.nodes < s1.nodes / 2,
              "Test 7 — re-search node count < half of first search (first="
              + std::to_string(s1.nodes) + ", second=" + std::to_string(s2.nodes) + ")");
    }

    // Test 8 — Mate score survives TT round-trip without corruption.
    // FEN: "6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1" — white has Ra8#.
    {
        const std::string mateFEN = "6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1";
        TT tt8(1);

        BBoard board1 = boardFromFEN(mateFEN);
        int score1 = search(board1, 2, &tt8);

        BBoard board2 = boardFromFEN(mateFEN);
        int score2 = search(board2, 2, &tt8);  // second search uses stored TT entries

        check(score1 == MATE_SCORE - 1,
              "Test 8a — mate score correct on first search (got "
              + std::to_string(score1) + ", expected " + std::to_string(MATE_SCORE - 1) + ")");
        check(score2 == MATE_SCORE - 1,
              "Test 8b — TT did not corrupt mate score on re-search (got "
              + std::to_string(score2) + ", expected " + std::to_string(MATE_SCORE - 1) + ")");
    }

    // Test 9 — Move ordering reduces depth-7 node count vs C4 baseline.
    // C4 baseline with 1 MB TT (fresh search): 15,565,455 nodes (Test 6 output).
    // With TT + MoveOrder we require strictly fewer nodes.
    {
        TT tt9(1);
        BBoard b9 = boardFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        SearchState s9;
        searchEx(b9, 7, &tt9, s9);
        check(s9.nodes < 15'565'455,
              "Test 9 — depth-7 nodes with TT+ordering < C4 baseline (got " + std::to_string(s9.nodes) + ")");
    }

    // Test 10 — Depth-8 baseline (nodes/s). No assertion — records Phase 1 baseline.
    {
        TT tt10(64);
        BBoard b10 = boardFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        SearchState s10;
        auto t0 = std::chrono::steady_clock::now();
        searchEx(b10, 8, &tt10, s10);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
        long long nps = (ms > 0) ? (long long)s10.nodes * 1000 / ms : 0;
        std::cout << "  [C5 baseline] depth-8: " << s10.nodes
                  << " nodes in " << ms << " ms (" << nps << " nps)\n";
        check(true, "Test 10 — depth-8 baseline recorded");
    }

    // Test 11 — qsearch resolves a hanging recapture (horizon effect prevention).
    // FEN: k7/8/4b3/3r4/8/8/3Q4/K7 w - - 0 1
    // White: Ka1, Qd2. Black: Ka8, Rd5, Be6.
    // Without qsearch depth-1 naively plays Qxd5 seeing +565 cp (winning a rook).
    // With qsearch the engine sees the bishop recapture Bxd5 (losing the queen for a rook),
    // so the actual score should be much lower — require score < 300.
    {
        BBoard board = boardFromFEN("k7/8/4b3/3r4/8/8/3Q4/K7 w - - 0 1");
        int score = search(board, 1);
        check(score < 300,
              "Test 11 — qsearch prevents horizon Qxd5 blunder (got " + std::to_string(score) + ")");
    }

    // Test 12 — score stability across depths 3–8 on startpos.
    // Run search(startpos, d) for d = 3..8. Verify all scores are within ±50 cp of each other.
    {
        const std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        int scores[6];
        for (int d = 3; d <= 8; ++d) {
            BBoard b = boardFromFEN(startFEN);
            scores[d - 3] = search(b, d);
        }
        int mn = *std::min_element(scores, scores + 6);
        int mx = *std::max_element(scores, scores + 6);
        check(mx - mn <= 50,
              "Test 12 — startpos score stable within 50cp across depths 3-8 (spread="
              + std::to_string(mx - mn) + ", min=" + std::to_string(mn)
              + ", max=" + std::to_string(mx) + ")");
    }

    // Test 13 — check extension preserves mate score at depth 3.
    // FEN: 6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1
    // White has Ra8# (mate-in-1). At depth 3 with check extension, must still find it.
    {
        BBoard board = boardFromFEN("6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1");
        int score = search(board, 3);
        check(score == MATE_SCORE - 1,
              "Test 13 — check extension preserves mate score at depth 3 (got "
              + std::to_string(score) + ", expected " + std::to_string(MATE_SCORE - 1) + ")");
    }

    // Test 14 — searchID returns a legal move from startpos at depth 5
    {
        TT tt14(16);
        BBoard b14 = boardFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        SearchLimits lim14; lim14.depth = 5;
        SearchResult r14 = searchID(b14, tt14, lim14);

        // Best move must be legal
        MoveList ml14; generateLegalMoves(b14, ml14);
        bool legal14 = false;
        for (int i = 0; i < ml14.count; ++i)
            if (ml14.moves[i] == r14.bestMove) { legal14 = true; break; }
        check(legal14, "Test 14 — searchID returns a legal move (got "
              + (r14.bestMove == MOVE_NONE ? "NONE" : to_uci(r14.bestMove)) + ")");
        check(r14.depth == 5, "Test 14b — searchID completed to depth 5 (got "
              + std::to_string(r14.depth) + ")");
    }

    // Test 15 — all PV moves are legal from their respective positions
    {
        TT tt15(16);
        BBoard b15 = boardFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        SearchLimits lim15; lim15.depth = 5;
        SearchResult r15 = searchID(b15, tt15, lim15);

        check(!r15.pv.empty(), "Test 15a — PV is non-empty");

        BBoard pvBoard = boardFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        bool pvLegal = true;
        for (chess::bb::Move m : r15.pv) {
            MoveList pvml; generateLegalMoves(pvBoard, pvml);
            bool found = false;
            for (int i = 0; i < pvml.count; ++i)
                if (pvml.moves[i] == m) { found = true; break; }
            if (!found) { pvLegal = false; break; }
            Undo u; pvBoard.makeMove(m, u);
        }
        check(pvLegal, "Test 15b — all PV moves are legal");
    }

    // Test 16 — searchID finds mate-in-1 and returns correct score
    {
        TT tt16(1);
        BBoard b16 = boardFromFEN("6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1");
        SearchLimits lim16; lim16.depth = 4;
        SearchResult r16 = searchID(b16, tt16, lim16);

        check(r16.score == MATE_SCORE - 1,
              "Test 16 — searchID finds mate-in-1 (score=" + std::to_string(r16.score) + ")");
        check(!r16.pv.empty() && r16.pv[0] != MOVE_NONE,
              "Test 16b — mate PV is non-empty");
    }

    // Test 17 — scores across ID depths are stable (no wild oscillation)
    {
        TT tt17(16);
        std::vector<int> idScores;
        for (int d = 1; d <= 6; ++d) {
            BBoard b17 = boardFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            TT ttd(16);
            SearchLimits limd; limd.depth = d;
            SearchResult rd = searchID(b17, ttd, limd);
            idScores.push_back(rd.score);
        }
        int mn = *std::min_element(idScores.begin(), idScores.end());
        int mx = *std::max_element(idScores.begin(), idScores.end());
        check(mx - mn <= 100,
              "Test 17 — ID scores across depths 1-6 spread <= 100cp (spread="
              + std::to_string(mx - mn) + ")");
    }

    std::cout << "\n" << g_checks << " checks, " << g_failures << " failures.\n";
    if (g_failures == 0) {
        std::cout << "ALL TESTS PASSED\n";
        return 0;
    }
    return 1;
}
