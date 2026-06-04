// Verification gate for step A6 (make/unmake move + incremental Zobrist).
// Standalone test, no CTest wiring (deferred to step B2).
//
// Coverage:
//   1. Round-trip: makeMove then unmakeMove restores the board EXACTLY
//      (all bitboards, occupancies, side, castling, ep, clocks, AND hash)
//      for quiet, double push, capture, en passant, both castles (both
//      colors), promotions (N/B/R/Q), capture-promotions, rook-move /
//      rook-capture / king-move castling-right loss.
//   2. Incremental hash == fresh computeHash() after each makeMove.
//   3. Sequence: apply several moves, unmake in reverse, end == start.
//
// Equality uses the defaulted BBoard::operator== (member-wise over every
// field including hash).
//
// Build & run:
//   c++ -std=c++23 -Wall -Wextra -I include src/chess/bb/BBoard.cpp src/chess/bb/Zobrist.cpp tests/makemove_test.cpp -o /tmp/makemove_test && /tmp/makemove_test

#include "chess/bb/BBoard.h"
#include "chess/bb/BBMove.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
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

// Dump useful context on failure.
void dumpFailure(const std::string& fen, Move m, const BBoard& b) {
    std::cerr << "  FEN:   " << fen << "\n";
    std::cerr << "  move:  " << to_uci(m) << " (flag " << flag_of(m) << ")\n";
    std::cerr << "  board state:\n";
    b.printBoard(std::cerr);
}

// makeMove then unmakeMove must return the board to its exact prior state,
// and the incremental hash after makeMove must equal a fresh computeHash().
void roundTrip(const std::string& fen, Move m, const std::string& label) {
    BBoard board;
    check(board.setFromFEN(fen), label + ": FEN parses");

    const BBoard before = board;

    Undo u;
    board.makeMove(m, u);

    // Incremental hash must match a full recompute of the resulting position.
    BBoard afterCopy = board;
    const std::uint64_t recomputed = afterCopy.computeHash();
    bool hashOk = (board.hash == recomputed);
    check(hashOk, label + ": incremental hash == computeHash()");
    if (!hashOk) {
        std::cerr << "  incremental=0x" << std::hex << board.hash
                  << " recomputed=0x" << recomputed << std::dec << "\n";
        dumpFailure(fen, m, board);
    }

    board.unmakeMove(m, u);

    bool restored = (board == before);
    check(restored, label + ": round-trip restores board exactly");
    if (!restored) {
        std::cerr << "  --- expected (before) ---\n";
        before.printBoard(std::cerr);
        std::cerr << "  --- got (after unmake) ---\n";
        dumpFailure(fen, m, board);
    }
}

} // namespace

int main() {
    // ----------------------------------------------------------------------
    // 1. Round-trip + incremental-hash checks, one crafted FEN per case.
    // ----------------------------------------------------------------------

    // Quiet move: knight b1->c3 from the start position.
    roundTrip("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
              make_move(B1, C3, FLAG_QUIET), "quiet");

    // Double pawn push: e2->e4 (must set ep target + ep hash).
    roundTrip("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
              make_move(E2, E4, FLAG_DOUBLE_PUSH), "double push");

    // Plain capture: white pawn e4 takes black pawn d5.
    roundTrip("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
              make_move(E4, D5, FLAG_CAPTURE), "capture");

    // En-passant capture: white pawn e5 takes black pawn that just pushed to d5,
    // ep target d6. Captured pawn sits behind `to` (d5 = d6 - 8).
    roundTrip("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3",
              make_move(E5, D6, FLAG_EP_CAPTURE), "en passant (white)");

    // En-passant capture, black: black pawn d4 takes white pawn that pushed to e4,
    // ep target e3. Captured pawn at e4 = e3 + 8.
    roundTrip("rnbqkbnr/pppp1ppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 3",
              make_move(D4, E3, FLAG_EP_CAPTURE), "en passant (black)");

    // King-side castle, white: e1->g1, rook h1->f1.
    roundTrip("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
              make_move(E1, G1, FLAG_OO), "white O-O");

    // Queen-side castle, white: e1->c1, rook a1->d1.
    roundTrip("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
              make_move(E1, C1, FLAG_OOO), "white O-O-O");

    // King-side castle, black: e8->g8, rook h8->f8.
    roundTrip("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1",
              make_move(E8, G8, FLAG_OO), "black O-O");

    // Queen-side castle, black: e8->c8, rook a8->d8.
    roundTrip("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1",
              make_move(E8, C8, FLAG_OOO), "black O-O-O");

    // Promotions (quiet) to N/B/R/Q: white pawn a7->a8 on an open file.
    {
        const std::string fen = "8/P7/8/8/8/8/8/4k2K w - - 0 1";
        roundTrip(fen, make_promotion(A7, A8, KNIGHT), "promo N");
        roundTrip(fen, make_promotion(A7, A8, BISHOP), "promo B");
        roundTrip(fen, make_promotion(A7, A8, ROOK),   "promo R");
        roundTrip(fen, make_promotion(A7, A8, QUEEN),  "promo Q");
    }

    // Capture-promotions to N/B/R/Q: white pawn b7 takes rook a8, promoting.
    // Capturing the rook on a8 also clears BLACK_OOO.
    {
        const std::string fen = "r3k3/1P6/8/8/8/8/8/4K3 w q - 0 1";
        roundTrip(fen, make_promotion(B7, A8, KNIGHT, true), "capt-promo N");
        roundTrip(fen, make_promotion(B7, A8, BISHOP, true), "capt-promo B");
        roundTrip(fen, make_promotion(B7, A8, ROOK,   true), "capt-promo R");
        roundTrip(fen, make_promotion(B7, A8, QUEEN,  true), "capt-promo Q");
    }

    // Rook move removing one castling right: white h1 rook moves, loses WHITE_OO.
    roundTrip("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
              make_move(H1, H5, FLAG_QUIET), "rook move drops WHITE_OO");

    // Rook capture removing opponent's castling right: white rook a1 captures
    // black rook a8, clearing BLACK_OOO (and white a1 rook leaving clears WHITE_OOO).
    roundTrip("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
              make_move(A1, A8, FLAG_CAPTURE), "rook capture drops BLACK_OOO");

    // King move removing both rights: white king e1->e2 loses WHITE_OO+WHITE_OOO.
    roundTrip("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
              make_move(E1, E2, FLAG_QUIET), "king move drops both white rights");

    // ----------------------------------------------------------------------
    // 3. Sequence test: apply several varied moves, unmake in reverse.
    // ----------------------------------------------------------------------
    {
        const std::string startFen =
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        BBoard board;
        check(board.setFromFEN(startFen), "sequence: FEN parses");

        const BBoard start = board;

        struct Step { Move m; std::string label; };
        std::vector<Step> seq = {
            { make_move(E2, E4, FLAG_DOUBLE_PUSH), "1. e4"  },   // white double push (ep set)
            { make_move(C7, C5, FLAG_DOUBLE_PUSH), "1... c5" },  // black double push (ep set)
            { make_move(G1, F3, FLAG_QUIET),       "2. Nf3" },   // white quiet
            { make_move(D7, D6, FLAG_QUIET),       "2... d6" },  // black quiet
            { make_move(F1, C4, FLAG_QUIET),       "3. Bc4" },   // white quiet
            { make_move(B8, C6, FLAG_QUIET),       "3... Nc6" }, // black quiet
        };

        std::vector<Undo> undos;
        for (const auto& s : seq) {
            Undo u;
            board.makeMove(s.m, u);
            undos.push_back(u);

            // Each forward step's incremental hash must equal a fresh recompute.
            BBoard copy = board;
            check(board.hash == copy.computeHash(),
                  std::string("sequence: incremental hash ") + s.label);
        }

        // Unmake in reverse order.
        for (int i = static_cast<int>(seq.size()) - 1; i >= 0; --i) {
            board.unmakeMove(seq[i].m, undos[i]);
        }

        bool back = (board == start);
        check(back, "sequence: returns exactly to start (incl. hash)");
        if (!back) {
            std::cerr << "  --- expected start ---\n";
            start.printBoard(std::cerr);
            std::cerr << "  --- got ---\n";
            board.printBoard(std::cerr);
        }
    }

    std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    if (g_failures == 0) {
        std::cout << "ALL TESTS PASSED\n";
        return EXIT_SUCCESS;
    }
    std::cout << g_failures << " CHECK(S) FAILED\n";
    return EXIT_FAILURE;
}
