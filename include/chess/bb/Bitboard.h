#ifndef CHESS_BB_BITBOARD_H
#define CHESS_BB_BITBOARD_H

#include <bit>
#include <cstdint>
#include <ostream>
#include <string>


//
//  A Bitboard is a 64-bit integer in which each bit corresponds to one
//  square of the chessboard. A 1 represents the existence of a piece on a square, a 0 represents an empty square. 
//
//  Bit index = rank * 8 + file, where file 0 = a-file, rank 0 = rank 1.
//
//      square index = 8 * rank + file
//
//      a1 =  0, b1 =  1, ..., h1 =  7
//      a2 =  8, b2 =  9, ..., h2 = 15
//      ...
//      a8 = 56, b8 = 57, ..., h8 = 63
//
//  Visually (bit indices), with rank 8 on top as a board is drawn:
//
//      8 | 56 57 58 59 60 61 62 63
//      7 | 48 49 50 51 52 53 54 55
//      6 | 40 41 42 43 44 45 46 47
//      5 | 32 33 34 35 36 37 38 39
//      4 | 24 25 26 27 28 29 30 31
//      3 | 16 17 18 19 20 21 22 23
//      2 |  8  9 10 11 12 13 14 15
//      1 |  0  1  2  3  4  5  6  7
//        +-------------------------
//          a  b  c  d  e  f  g  h
//
//  All move generation in later steps assumes this layout.

namespace chess::bb {

using Bitboard = std::uint64_t; //Bitboard definition (64bit int)

constexpr Bitboard EMPTY = 0ULL;
constexpr Bitboard FULL  = ~0ULL;

constexpr int NUM_SQUARES = 64;

// Named squares (LERF). Useful for tests and readable masks.
enum Square : int {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQUARE = 64
};

/*
    Below is a series of helpers that convert square (ie A1) into the individual rank/file that represent this square on the chessboard
*/

// File of a square: 0 = a-file ... 7 = h-file.
constexpr int file_of(int sq) { return sq & 7; }

// Rank of a square: 0 = rank 1 ... 7 = rank 8.
//Logially, >> 3 removes offset from 
constexpr int rank_of(int sq) { return sq >> 3; }

// Build a square index from a (file, rank) pair, both 0-based.
constexpr int make_square(int file, int rank) { return rank * 8 + file; }

// A single-bit bitboard with only `sq` set.
constexpr Bitboard square_bb(int sq) { return 1ULL << sq; }

/*
    Below is a series of one bit operations used mostly for testing and bit update operations
*/

// Test whether bit `sq` is set.
constexpr bool test_bit(Bitboard b, int sq) { return (b >> sq) & 1ULL; }

// Return a copy of `b` with bit `sq` set.
constexpr Bitboard set_bit(Bitboard b, int sq) { return b | square_bb(sq); }

// Return a copy of `b` with bit `sq` cleared.
constexpr Bitboard clear_bit(Bitboard b, int sq) { return b & ~square_bb(sq); }

// In-place variants (mutate the referenced bitboard).
constexpr void set_bit_ip(Bitboard& b, int sq)   { b |= square_bb(sq); } //Sets bit at sq, leaves all other bits unchanged as square_bb is 64bit int with single 1
constexpr void clear_bit_ip(Bitboard& b, int sq) { b &= ~square_bb(sq); } //Clears bit at sq, leaves all other bits unchanged as sqaure_bb is 64bit int with single 0

/*
    Set of fucntions used to scan the board.
*/

// Number of set bits.
constexpr int popcount(Bitboard b) { return std::popcount(b); }

// Index of the least-significant set bit (0..63). Undefined if b == 0.
constexpr int lsb(Bitboard b) { return std::countr_zero(b); }

// Index of the most-significant set bit (0..63). Undefined if b == 0.
constexpr int msb(Bitboard b) { return 63 - std::countl_zero(b); }

// Pop (return and clear) the least-significant set bit. Undefined if b == 0.
constexpr int pop_lsb(Bitboard& b) {
    const int s = lsb(b);
    b &= b - 1;            // clears the lowest set bit
    return s;
}

// ---------------------------------------------------------------------
//  File and rank masks
// ---------------------------------------------------------------------

constexpr Bitboard FILE_A = 0x0101010101010101ULL; //Represents indices 0, 8, 16 etc in hexadecimal
constexpr Bitboard FILE_B = FILE_A << 1;
constexpr Bitboard FILE_C = FILE_A << 2;
constexpr Bitboard FILE_D = FILE_A << 3;
constexpr Bitboard FILE_E = FILE_A << 4;
constexpr Bitboard FILE_F = FILE_A << 5;
constexpr Bitboard FILE_G = FILE_A << 6;
constexpr Bitboard FILE_H = FILE_A << 7;

constexpr Bitboard RANK_1 = 0x00000000000000FFULL;
constexpr Bitboard RANK_2 = RANK_1 << (8 * 1);
constexpr Bitboard RANK_3 = RANK_1 << (8 * 2);
constexpr Bitboard RANK_4 = RANK_1 << (8 * 3);
constexpr Bitboard RANK_5 = RANK_1 << (8 * 4);
constexpr Bitboard RANK_6 = RANK_1 << (8 * 5);
constexpr Bitboard RANK_7 = RANK_1 << (8 * 6);
constexpr Bitboard RANK_8 = RANK_1 << (8 * 7);

// Indexed access: file_mask(0) == FILE_A, rank_mask(0) == RANK_1.
constexpr Bitboard file_mask(int file) { return FILE_A << file; }
constexpr Bitboard rank_mask(int rank) { return RANK_1 << (8 * rank); }

// ---------------------------------------------------------------------
//  Debug helper
// ---------------------------------------------------------------------

// Convert a square index to algebraic coordinates, e.g. 0 -> "a1".
inline std::string square_name(int sq) {
    if (sq < 0 || sq >= NUM_SQUARES) return "--";
    return std::string{static_cast<char>('a' + file_of(sq)),
                       static_cast<char>('1' + rank_of(sq))};
}

// Pretty-print a bitboard as an 8x8 grid (rank 8 on top, a-file on left).
// 'X' marks a set bit, '.' an empty square.
inline void print_bitboard(Bitboard b, std::ostream& os) {
    for (int rank = 7; rank >= 0; --rank) {
        os << (rank + 1) << " |";
        for (int file = 0; file < 8; ++file) {
            os << ' ' << (test_bit(b, make_square(file, rank)) ? 'X' : '.');
        }
        os << '\n';
    }
    os << "    a b c d e f g h\n";
    os << "  bitboard = 0x" << std::hex << b << std::dec << '\n';
}

} // namespace chess::bb

#endif // CHESS_BB_BITBOARD_H
