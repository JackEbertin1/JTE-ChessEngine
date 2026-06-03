// Verification gate for step A5 (classical ray-traced slider attacks:
// bishop, rook, queen). Standalone test, no CTest wiring (deferred to B2).
//
// Build & run:
//   c++ -std=c++23 -Wall -Wextra -I include src/chess/bb/BBoard.cpp \
//       src/chess/bb/Zobrist.cpp tests/slider_attacks_test.cpp \
//       -o /tmp/slider_test && /tmp/slider_test

#include "chess/bb/Attacks.h"
#include "chess/bb/BBoard.h"

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

} // namespace

int main() {
    // --- Empty board ----------------------------------------------------
    check(rook_attacks(A1, EMPTY) == ((FILE_A | RANK_1) & ~square_bb(A1)),
          "rook a1 empty == a-file | rank 1 minus a1");
    check(popcount(rook_attacks(A1, EMPTY)) == 14, "rook a1 empty count == 14");

    check(rook_attacks(D4, EMPTY) == ((FILE_D | RANK_4) & ~square_bb(D4)),
          "rook d4 empty == d-file | rank 4 minus d4");
    check(popcount(rook_attacks(D4, EMPTY)) == 14, "rook d4 empty count == 14");

    check(bishop_attacks(A1, EMPTY) ==
              (square_bb(B2) | square_bb(C3) | square_bb(D4) | square_bb(E5) |
               square_bb(F6) | square_bb(G7) | square_bb(H8)),
          "bishop a1 empty == main diagonal");
    check(popcount(bishop_attacks(A1, EMPTY)) == 7, "bishop a1 empty count == 7");

    check(bishop_attacks(D4, EMPTY) ==
              (square_bb(E5) | square_bb(F6) | square_bb(G7) | square_bb(H8) |
               square_bb(C5) | square_bb(B6) | square_bb(A7) |
               square_bb(E3) | square_bb(F2) | square_bb(G1) |
               square_bb(C3) | square_bb(B2) | square_bb(A1)),
          "bishop d4 empty == 4 diagonals");
    check(popcount(bishop_attacks(D4, EMPTY)) == 13, "bishop d4 empty count == 13");

    check(queen_attacks(D4, EMPTY) ==
              (bishop_attacks(D4, EMPTY) | rook_attacks(D4, EMPTY)),
          "queen d4 empty == bishop | rook");
    check(popcount(queen_attacks(D4, EMPTY)) == 27, "queen d4 empty count == 27");

    // --- Wrap guards ----------------------------------------------------
    check(bishop_attacks(H1, EMPTY) ==
              (square_bb(G2) | square_bb(F3) | square_bb(E4) | square_bb(D5) |
               square_bb(C6) | square_bb(B7) | square_bb(A8)),
          "bishop h1 empty == only NW diagonal (no east wrap)");
    check(popcount(bishop_attacks(H1, EMPTY)) == 7, "bishop h1 empty count == 7");

    {
        const Bitboard r = rook_attacks(H1, EMPTY);
        const Bitboard west = square_bb(A1) | square_bb(B1) | square_bb(C1) |
                              square_bb(D1) | square_bb(E1) | square_bb(F1) |
                              square_bb(G1);
        const Bitboard up = square_bb(H2) | square_bb(H3) | square_bb(H4) |
                            square_bb(H5) | square_bb(H6) | square_bb(H7) |
                            square_bb(H8);
        check(r == (west | up), "rook h1 empty == west rank + up file");
        check(popcount(r) == 14, "rook h1 empty count == 14");
        // No eastward wrap onto rank 2 (a2 is one east-shift away from h1).
        check(!test_bit(r, A2), "rook h1 no east wrap to a2");
    }

    // --- Blockers -------------------------------------------------------
    {
        const Bitboard r = rook_attacks(A1, square_bb(A4));
        const Bitboard expect = square_bb(A2) | square_bb(A3) | square_bb(A4) |
                                square_bb(B1) | square_bb(C1) | square_bb(D1) |
                                square_bb(E1) | square_bb(F1) | square_bb(G1) |
                                square_bb(H1);
        check(r == expect, "rook a1 blocker a4 == {a2,a3,a4,b1..h1}");
        check(popcount(r) == 10, "rook a1 blocker a4 count == 10");
        check(test_bit(r, A4), "rook a1 includes blocker a4");
        check(!test_bit(r, A5), "rook a1 stops before a5");
    }
    {
        // Bishop on c1, blocker on e3 along the NE diagonal.
        const Bitboard b = bishop_attacks(C1, square_bb(E3));
        check(test_bit(b, D2), "bishop c1 reaches d2 before blocker");
        check(test_bit(b, E3), "bishop c1 includes blocker e3");
        check(!test_bit(b, F4), "bishop c1 stops before f4");
        check(!test_bit(b, G5), "bishop c1 stops before g5");
        // Other-direction ray (SW->W edge / toward b2,a3) still walks freely.
        check(test_bit(b, B2), "bishop c1 reaches b2 on NW diagonal");
        check(test_bit(b, A3), "bishop c1 reaches a3 on NW diagonal");
    }

    // --- Differential invariants over all 64 squares --------------------
    for (int sq = 0; sq < NUM_SQUARES; ++sq) {
        check(queen_attacks(sq, EMPTY) ==
                  (bishop_attacks(sq, EMPTY) | rook_attacks(sq, EMPTY)),
              "queen == bishop|rook on EMPTY @ " + square_name(sq));
        check(queen_attacks(sq, FULL) ==
                  (bishop_attacks(sq, FULL) | rook_attacks(sq, FULL)),
              "queen == bishop|rook on FULL @ " + square_name(sq));
    }

    // FULL occupancy: each slider sees exactly its immediate neighbors.
    check(rook_attacks(D4, FULL) ==
              (square_bb(C4) | square_bb(E4) | square_bb(D3) | square_bb(D5)),
          "rook d4 FULL == 4 orthogonal neighbors");
    check(bishop_attacks(D4, FULL) ==
              (square_bb(C3) | square_bb(C5) | square_bb(E3) | square_bb(E5)),
          "bishop d4 FULL == 4 diagonal neighbors");

    // --- BBoard FEN cross-check (required) ------------------------------
    {
        BBoard board;
        const bool ok = board.setFromFEN("8/3p4/8/3R4/8/8/3P4/8 w - - 0 1");
        check(ok, "setFromFEN parsed lone-rook position");

        const Bitboard r = rook_attacks(D5, board.occupancyAll);
        const Bitboard expect =
            square_bb(D6) | square_bb(D7) |                       // up to enemy pawn d7
            square_bb(D4) | square_bb(D3) | square_bb(D2) |       // down to friendly pawn d2
            square_bb(E5) | square_bb(F5) | square_bb(G5) | square_bb(H5) |  // east
            square_bb(C5) | square_bb(B5) | square_bb(A5);        // west
        check(r == expect, "rook d5 (FEN occ) == expected 12 squares");
        check(popcount(r) == 12, "rook d5 (FEN occ) count == 12");
        check(test_bit(r, D7), "rook d5 includes enemy blocker d7");
        check(test_bit(r, D2), "rook d5 includes friendly blocker d2 (no color filter)");
        check(!test_bit(r, D8), "rook d5 stops before d8 (beyond enemy blocker)");
        check(!test_bit(r, D1), "rook d5 stops before d1 (beyond friendly blocker)");

        if (r != expect) {
            std::cerr << "  rook_attacks(D5, occ) actual:\n";
            print_bitboard(r, std::cerr);
            std::cerr << "  expected:\n";
            print_bitboard(expect, std::cerr);
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
