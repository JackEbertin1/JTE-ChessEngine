// Verification gate for step A4 (leaper + pawn attack tables, pawn push
// helpers). Standalone test, no CTest wiring (deferred to step B2).
//
// Build & run:
//   c++ -std=c++23 -Wall -Wextra -I include tests/attacks_test.cpp -o /tmp/attacks_test && /tmp/attacks_test

#include "chess/bb/Attacks.h"

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
    // --- Knight: corner, center, corner ---------------------------------
    check(knight_attacks(A1) == 0x0000000000020400ULL, "knight a1 bitboard");
    check(knight_attacks(A1) == (square_bb(B3) | square_bb(C2)), "knight a1 == {b3,c2}");
    check(popcount(knight_attacks(A1)) == 2, "knight a1 count == 2");

    check(knight_attacks(E4) ==
              (square_bb(D2) | square_bb(F2) | square_bb(C3) | square_bb(G3) |
               square_bb(C5) | square_bb(G5) | square_bb(D6) | square_bb(F6)),
          "knight e4 == 8 targets");
    check(popcount(knight_attacks(E4)) == 8, "knight e4 count == 8");

    check(knight_attacks(H1) == (square_bb(F2) | square_bb(G3)), "knight h1 == {f2,g3}");
    check(popcount(knight_attacks(H1)) == 2, "knight h1 count == 2");

    // --- King: corner, center, corner -----------------------------------
    check(king_attacks(A1) == 0x0000000000000302ULL, "king a1 bitboard");
    check(king_attacks(A1) == (square_bb(A2) | square_bb(B1) | square_bb(B2)),
          "king a1 == {a2,b1,b2}");
    check(popcount(king_attacks(A1)) == 3, "king a1 count == 3");

    check(king_attacks(E4) ==
              (square_bb(D3) | square_bb(E3) | square_bb(F3) |
               square_bb(D4) | square_bb(F4) |
               square_bb(D5) | square_bb(E5) | square_bb(F5)),
          "king e4 == 8 targets");
    check(popcount(king_attacks(E4)) == 8, "king e4 count == 8");

    check(king_attacks(H8) == (square_bb(G8) | square_bb(G7) | square_bb(H7)),
          "king h8 == {g8,g7,h7}");
    check(popcount(king_attacks(H8)) == 3, "king h8 count == 3");

    // --- Pawns: center + file edges -------------------------------------
    check(pawn_attacks(WHITE, E4) == (square_bb(D5) | square_bb(F5)),
          "white pawn e4 == {d5,f5}");
    check(popcount(pawn_attacks(WHITE, E4)) == 2, "white pawn e4 count == 2");

    check(pawn_attacks(WHITE, A2) == square_bb(B3), "white pawn a2 == {b3} only");
    check(popcount(pawn_attacks(WHITE, A2)) == 1, "white pawn a2 count == 1");

    check(pawn_attacks(BLACK, E5) == (square_bb(D4) | square_bb(F4)),
          "black pawn e5 == {d4,f4}");
    check(popcount(pawn_attacks(BLACK, E5)) == 2, "black pawn e5 count == 2");

    check(pawn_attacks(BLACK, H7) == square_bb(G6), "black pawn h7 == {g6} only");
    check(popcount(pawn_attacks(BLACK, H7)) == 1, "black pawn h7 count == 1");

    // --- General invariants over all squares ----------------------------
    for (int sq = 0; sq < NUM_SQUARES; ++sq) {
        check(popcount(knight_attacks(sq)) <= 8, "knight count <= 8 @ " + square_name(sq));
        check(popcount(king_attacks(sq)) <= 8, "king count <= 8 @ " + square_name(sq));

        const int f = file_of(sq);
        // File-wrap guard: no attack may land two or more files away.
        Bitboard farFiles = EMPTY;
        for (int ff = 0; ff < 8; ++ff)
            if (ff < f - 1 || ff > f + 1) farFiles |= file_mask(ff);

        // Knights reach at most two files away; guard against >2 wrap.
        Bitboard knightFar = EMPTY;
        for (int ff = 0; ff < 8; ++ff)
            if (ff < f - 2 || ff > f + 2) knightFar |= file_mask(ff);

        check((knight_attacks(sq) & knightFar) == EMPTY,
              "knight no >2-file wrap @ " + square_name(sq));
        check((king_attacks(sq) & farFiles) == EMPTY,
              "king no >1-file wrap @ " + square_name(sq));
        check((pawn_attacks(WHITE, sq) & farFiles) == EMPTY,
              "white pawn no >1-file wrap @ " + square_name(sq));
        check((pawn_attacks(BLACK, sq) & farFiles) == EMPTY,
              "black pawn no >1-file wrap @ " + square_name(sq));
    }

    // --- Pawn pushes ----------------------------------------------------
    // White pawns on rank 2, everything ahead empty.
    {
        const Bitboard pawns = RANK_2;
        const Bitboard empty = ~RANK_2;
        check(single_pawn_pushes(pawns, empty, WHITE) == RANK_3,
              "white single push RANK_2 -> RANK_3");
        check(double_pawn_pushes(pawns, empty, WHITE) == RANK_4,
              "white double push RANK_2 -> RANK_4");
    }
    // Black pawns on rank 7, everything ahead empty.
    {
        const Bitboard pawns = RANK_7;
        const Bitboard empty = ~RANK_7;
        check(single_pawn_pushes(pawns, empty, BLACK) == RANK_6,
              "black single push RANK_7 -> RANK_6");
        check(double_pawn_pushes(pawns, empty, BLACK) == RANK_5,
              "black double push RANK_7 -> RANK_5");
    }
    // Blocked double push: a blocker on the intermediate square stops it.
    {
        const Bitboard pawns = RANK_2;
        const Bitboard empty = ~(RANK_2 | square_bb(E3)); // e3 occupied
        check(single_pawn_pushes(pawns, empty, WHITE) == (RANK_3 & ~square_bb(E3)),
              "white push blocked at e3");
        check(double_pawn_pushes(pawns, empty, WHITE) == (RANK_4 & ~square_bb(E4)),
              "white double push blocked at e3");
    }

    // --- Bulk pawn attacks ----------------------------------------------
    {
        // Single white pawn matches the per-square table.
        check(pawn_attacks_bb(square_bb(E4), WHITE) == pawn_attacks(WHITE, E4),
              "bulk white pawn e4 matches table");
        check(pawn_attacks_bb(square_bb(A2), WHITE) == pawn_attacks(WHITE, A2),
              "bulk white pawn a2 (edge) matches table");
        check(pawn_attacks_bb(square_bb(H7), BLACK) == pawn_attacks(BLACK, H7),
              "bulk black pawn h7 (edge) matches table");
        // All starting white pawns attack all of rank 3.
        check(pawn_attacks_bb(RANK_2, WHITE) == RANK_3,
              "bulk white RANK_2 attacks RANK_3");
        check(pawn_attacks_bb(RANK_7, BLACK) == RANK_6,
              "bulk black RANK_7 attacks RANK_6");
    }

    std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    if (g_failures == 0) {
        std::cout << "ALL TESTS PASSED\n";
        return EXIT_SUCCESS;
    }
    std::cout << g_failures << " CHECK(S) FAILED\n";
    return EXIT_FAILURE;
}
