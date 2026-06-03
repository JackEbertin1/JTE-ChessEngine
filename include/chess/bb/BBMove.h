#ifndef CHESS_BB_BBMOVE_H
#define CHESS_BB_BBMOVE_H

#include "chess/bb/Bitboard.h"   // Square, file_of/rank_of, square_name
#include "chess/bb/BBoard.h"     // PieceType (KNIGHT..QUEEN)

#include <cstdint>
#include <string>

//
//  BBMove: a packed 16-bit move.
//
//  A move is encoded in a single std::uint16_t, laid out as:
//
//      bit:  15 14 13 12 | 11 10 09 08 07 06 | 05 04 03 02 01 00
//            [  flags   ] [     to-square    ] [    from-square  ]
//
//      bits  0..5   : from-square (0..63, LERF; see Bitboard.h)
//      bits  6..11  : to-square   (0..63)
//      bits 12..15  : 4-bit flag
//
//  Flag values 4bit scheme tuned back into integer (0-15)
//
//       0  quiet
//       1  double pawn push
//       2  king-side castle  (OO)
//       3  queen-side castle (OOO)
//       4  capture
//       5  en-passant capture
//       6  (unused)
//       7  (unused)
//       8  knight-promotion
//       9  bishop-promotion
//      10  rook-promotion
//      11  queen-promotion
//      12  knight-promotion capture
//      13  bishop-promotion capture
//      14  rook-promotion capture
//      15  queen-promotion capture
//
//  Two bits of the flag are meaningful as predicates:
//      bit 2 (value 4)  -> capture flag   (set on 4,5,12,13,14,15)
//      bit 3 (value 8)  -> promotion flag (set on 8..15)
//  The low 2 bits of a promotion flag select the target piece type:
//      0->KNIGHT, 1->BISHOP, 2->ROOK, 3->QUEEN, matching PieceType - KNIGHT.
//
//  MOVE_NONE == 0 is a quiet a1->a1 move and serves as the null/none sentinel.
//
//  A6/A7 (make/unmake, move generation) rely on this exact layout.

namespace chess::bb {

// A packed move. Thin typedef keeps it trivially copyable / usable as a key.
using Move = std::uint16_t;

// Bit shifts to grab different aspects of a 16 bit integer packed move
constexpr int MOVE_FROM_SHIFT = 0;      //How to shift 16 bit value to grab moveFrom square
constexpr int MOVE_TO_SHIFT   = 6;      //How to shift 16 bit value to grab moveTo square
constexpr int MOVE_FLAG_SHIFT = 12;     //How to shift 16 bit value to grab flags info
constexpr Move MOVE_SQ_MASK   = 0x3F;  // 6 bit mask
constexpr Move MOVE_FLAG_MASK = 0x0F;  // 4 bit mask

// The 4-bit move flags.
enum MoveFlag : Move {
    FLAG_QUIET            = 0,
    FLAG_DOUBLE_PUSH      = 1,
    FLAG_OO               = 2,   // king-side castle
    FLAG_OOO              = 3,   // queen-side castle
    FLAG_CAPTURE          = 4,
    FLAG_EP_CAPTURE       = 5,
    FLAG_PROMO_N          = 8,
    FLAG_PROMO_B          = 9,
    FLAG_PROMO_R          = 10,
    FLAG_PROMO_Q          = 11,
    FLAG_PROMO_N_CAPTURE  = 12,
    FLAG_PROMO_B_CAPTURE  = 13,
    FLAG_PROMO_R_CAPTURE  = 14,
    FLAG_PROMO_Q_CAPTURE  = 15
};

// Single-bit meanings within the flag nibble.
constexpr Move FLAG_BIT_CAPTURE   = 4;  // bit 2, this holds logically as 4(0100), 5(0101), 12(1100), 13(1101), 14(1110), and 15(1111) are the capture flags 
constexpr Move FLAG_BIT_PROMOTION = 8;  // bit 3, same reasoning as above but for MSB

// The null / none move.
constexpr Move MOVE_NONE = 0;




/*
    Construction of different move types (Regular move, Promotion)
*/

// Pack a from-square, to-square and flag into a 16-bit Move.
// Constructs the "Move" object
constexpr Move make_move(int from, int to, Move flag = FLAG_QUIET) {
    return static_cast<Move>((flag & MOVE_FLAG_MASK) << MOVE_FLAG_SHIFT |           //Grab the 4 bit flag, shift it left 12 bits
                             (to   & MOVE_SQ_MASK)   << MOVE_TO_SHIFT   |           //Grab the 6bit TO square, shift it left 6 bits
                             (from & MOVE_SQ_MASK)   << MOVE_FROM_SHIFT);           //Grab the 6bit FROM square, shift it left 0 bits
}

// Map a promotion target PieceType (KNIGHT..QUEEN) to its 2-bit code.

constexpr Move promo_code(PieceType pt) {
    return static_cast<Move>(pt - KNIGHT);  // KNIGHT->0 .. QUEEN->3
}

// Build a promotion move. `pt` must be one of KNIGHT, BISHOP, ROOK, QUEEN.
// If `capture` is true, the capturing promotion flag is produced.
constexpr Move make_promotion(int from, int to, PieceType pt, bool capture = false) {
    const Move base = capture ? FLAG_PROMO_N_CAPTURE : FLAG_PROMO_N;
    return make_move(from, to, static_cast<Move>(base + promo_code(pt)));
}

// ---------------------------------------------------------------------
//  Decoding
// ---------------------------------------------------------------------

//Returns the square that the piece moved from by grabbing lower 6 bits and turning into integer.
constexpr int from_sq(Move m) {
    return (m >> MOVE_FROM_SHIFT) & MOVE_SQ_MASK;
}

//Returns the square the piece moved to by grabbing bits 6-11 and turning into integer
constexpr int to_sq(Move m) {
    return (m >> MOVE_TO_SHIFT) & MOVE_SQ_MASK;
}

// Gets the flag value by grabbing bits 12-15 and turning into integer.
constexpr Move flag_of(Move m) {
    return (m >> MOVE_FLAG_SHIFT) & MOVE_FLAG_MASK;
}

// ---------------------------------------------------------------------
//  Flag predicates
// ---------------------------------------------------------------------

constexpr bool is_capture(Move m) {
    return (flag_of(m) & FLAG_BIT_CAPTURE) != 0;
}

constexpr bool is_promotion(Move m) {
    return (flag_of(m) & FLAG_BIT_PROMOTION) != 0;
}

constexpr bool is_en_passant(Move m) {
    return flag_of(m) == FLAG_EP_CAPTURE;
}

constexpr bool is_double_push(Move m) {
    return flag_of(m) == FLAG_DOUBLE_PUSH;
}

constexpr bool is_king_castle(Move m) {
    return flag_of(m) == FLAG_OO;
}

constexpr bool is_queen_castle(Move m) {
    return flag_of(m) == FLAG_OOO;
}

constexpr bool is_castle(Move m) {
    return is_king_castle(m) || is_queen_castle(m);
}

// The promotion target piece type. Only meaningful when is_promotion(m).
constexpr PieceType promotion_type(Move m) {
    return static_cast<PieceType>(KNIGHT + (flag_of(m) & 0x3));
}

// ---------------------------------------------------------------------
//  Formatting
// ---------------------------------------------------------------------

// Long-algebraic (UCI) string: "e2e4", "e7e8q", or "0000" for the null move.
inline std::string to_uci(Move m) {
    if (m == MOVE_NONE) return "0000";
    std::string s = square_name(from_sq(m)) + square_name(to_sq(m));
    if (is_promotion(m)) {
        // KNIGHT->n, BISHOP->b, ROOK->r, QUEEN->q (UCI lowercases promo).
        static constexpr char kPromoChar[4] = {'n', 'b', 'r', 'q'};
        s += kPromoChar[flag_of(m) & 0x3];
    }
    return s;
}

} // namespace chess::bb

#endif // CHESS_BB_BBMOVE_H
