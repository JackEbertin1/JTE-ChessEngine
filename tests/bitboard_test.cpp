// Trivial assertions for the bitboard primitives
//
// Build & run standalone:
//   c++ -std=c++23 -I include tests/bitboard_test.cpp -o /tmp/bitboard_test && /tmp/bitboard_test
//
// CTest wiring is intentionally deferred to step B2.

#include "chess/bb/Bitboard.h"

#include <cassert>
#include <iostream>
#include <sstream>

using namespace chess::bb;

int main() {
    // --- Square indexing convention: LERF, a1 = 0 -----------------------
    static_assert(A1 == 0,  "a1 must be index 0");
    static_assert(H1 == 7,  "h1 must be index 7");
    static_assert(A8 == 56, "a8 must be index 56");
    static_assert(H8 == 63, "h8 must be index 63");
    static_assert(make_square(file_of(E4), rank_of(E4)) == E4,
                  "square <-> file/rank round-trip");
    static_assert(file_of(E4) == 4 && rank_of(E4) == 3, "e4 = file e, rank 4");

    // --- Single-bit operations -----------------------------------------
    Bitboard b = EMPTY;
    set_bit_ip(b, E4);
    assert(test_bit(b, E4));
    assert(!test_bit(b, E5));
    assert(b == square_bb(E4));
    clear_bit_ip(b, E4);
    assert(b == EMPTY);

    // const variants
    assert(set_bit(EMPTY, D5) == square_bb(D5));
    assert(clear_bit(FULL, A1) == (FULL & ~square_bb(A1)));

    // --- popcount / lsb / msb / pop_lsb --------------------------------
    Bitboard corners = square_bb(A1) | square_bb(H1) | square_bb(A8) | square_bb(H8);
    assert(popcount(corners) == 4);
    assert(popcount(EMPTY) == 0);
    assert(popcount(FULL) == 64);
    assert(lsb(corners) == A1);
    assert(msb(corners) == H8);

    // pop_lsb walks the set bits in ascending index order.
    Bitboard scan = corners;
    assert(pop_lsb(scan) == A1);
    assert(pop_lsb(scan) == H1);
    assert(pop_lsb(scan) == A8);
    assert(pop_lsb(scan) == H8);
    assert(scan == EMPTY);

    // --- File / rank masks ---------------------------------------------
    static_assert(FILE_A == 0x0101010101010101ULL, "FILE_A literal");
    static_assert(RANK_1 == 0x00000000000000FFULL, "RANK_1 literal");
    assert(popcount(FILE_A) == 8 && popcount(RANK_1) == 8);
    assert(test_bit(FILE_A, A1) && test_bit(FILE_A, A8) && !test_bit(FILE_A, B1));
    assert(test_bit(RANK_8, A8) && test_bit(RANK_8, H8) && !test_bit(RANK_8, A7));
    assert(file_mask(4) == FILE_E && rank_mask(3) == RANK_4);
    // a1 is the unique intersection of the a-file and rank 1.
    assert((FILE_A & RANK_1) == square_bb(A1));

    // --- Debug helpers --------------------------------------------------
    assert(square_name(A1) == "a1");
    assert(square_name(E4) == "e4");
    assert(square_name(H8) == "h8");

    std::ostringstream oss;
    print_bitboard(corners, oss);
    const std::string out = oss.str();
    // 8 board rows + coordinate row + hex row.
    assert(out.find("a b c d e f g h") != std::string::npos);
    assert(out.find('X') != std::string::npos);

    std::cout << "Sample print_bitboard(corners):\n";
    print_bitboard(corners, std::cout);

    std::cout << "\nAll bitboard primitive assertions passed.\n";
    return 0;
}
