#ifndef CHESS_BB_ATTACKS_H
#define CHESS_BB_ATTACKS_H

#include "chess/bb/Bitboard.h"
#include "chess/bb/BBoard.h"   // for Color { WHITE, BLACK, NUM_COLORS }

#include <array>

//
//  Attacks: precomputed leaper (knight, king) attack tables, a per-square
//  pawn attack table, and bulk pawn push / pawn attack helpers.
//
//  Square convention is LERF (a1 = 0); see Bitboard.h. In this layout
//  <<8 shifts north, <<1 east, >>1 west, >>8 south.
//
//  The three lookup tables are built at COMPILE TIME by constexpr builder
//  functions and stored as `inline constexpr` variables (single shared
//  definition across translation units, zero runtime init, no access-time
//  guard in the hot move-generation loop). This is the faithful extension
//  of the constexpr primitives in Bitboard.h. 
//
//  Accessor style (be consistent): per-square lookups go through the thin
//  wrappers knight_attacks(sq) / king_attacks(sq) / pawn_attacks(c, sq).
//  The bulk pawn helpers (single_pawn_pushes / double_pawn_pushes /
//  pawn_attacks_bb) operate on a whole set of pawns at once. Note the
//  per-square `pawn_attacks` is distinct from the bulk `pawn_attacks_bb`.
//
//  Sliders (bishop/rook/queen), make/unmake, and move-list generation are
//  LATER steps (A5+) and are intentionally absent here.

namespace chess::bb {

// ---------------------------------------------------------------------
//  Table construction (compile time)
// ---------------------------------------------------------------------

// Knight attacks: the 8 (file,rank) L-offsets, each clamped to the board
// so attacks never wrap across files.
constexpr std::array<Bitboard, 64> make_knight_attacks() {
    std::array<Bitboard, 64> t{};
    constexpr int df[8] = {-2, -2, -1, -1,  1,  1,  2,  2};
    constexpr int dr[8] = {-1,  1, -2,  2, -2,  2, -1,  1};
    for (int sq = 0; sq < 64; ++sq) {
        const int f = file_of(sq), r = rank_of(sq);
        Bitboard bb = EMPTY;
        for (int i = 0; i < 8; ++i) {
            const int nf = f + df[i], nr = r + dr[i];
            if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8)
                bb |= square_bb(make_square(nf, nr));
        }
        t[sq] = bb;
    }
    return t;
}

// King attacks: the 8 surrounding squares, clamped to the board (no wrap).
constexpr std::array<Bitboard, 64> make_king_attacks() {
    std::array<Bitboard, 64> t{};
    constexpr int df[8] = {-1, -1, -1,  0,  0,  1,  1,  1};
    constexpr int dr[8] = {-1,  0,  1, -1,  1, -1,  0,  1};
    for (int sq = 0; sq < 64; ++sq) {
        const int f = file_of(sq), r = rank_of(sq);
        Bitboard bb = EMPTY;
        for (int i = 0; i < 8; ++i) {
            const int nf = f + df[i], nr = r + dr[i];
            if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8)
                bb |= square_bb(make_square(nf, nr));
        }
        t[sq] = bb;
    }
    return t;
}

// Pawn attacks, indexed [color][square]: the two diagonal capture targets.
// White attacks NW (+7) and NE (+9); black attacks SW (-9) and SE (-7).
// File wraps are masked via bounds-checked (file,rank) generation; a pawn
// with no forward rank (white on rank 8 / black on rank 1) yields EMPTY.
constexpr std::array<std::array<Bitboard, 64>, NUM_COLORS> make_pawn_attacks() {
    std::array<std::array<Bitboard, 64>, NUM_COLORS> t{};
    for (int sq = 0; sq < 64; ++sq) {
        const int f = file_of(sq), r = rank_of(sq);

        // White: forward rank r+1, files f-1 and f+1.
        Bitboard w = EMPTY;
        if (r + 1 < 8) {
            if (f - 1 >= 0) w |= square_bb(make_square(f - 1, r + 1));
            if (f + 1 < 8)  w |= square_bb(make_square(f + 1, r + 1));
        }
        t[WHITE][sq] = w;

        // Black: forward rank r-1, files f-1 and f+1.
        Bitboard b = EMPTY;
        if (r - 1 >= 0) {
            if (f - 1 >= 0) b |= square_bb(make_square(f - 1, r - 1));
            if (f + 1 < 8)  b |= square_bb(make_square(f + 1, r - 1));
        }
        t[BLACK][sq] = b;
    }
    return t;
}

// ---------------------------------------------------------------------
//  Lookup tables: Precomputed attack squares for each square on the board for given pieceType
//  Computed at compile time
// ---------------------------------------------------------------------

inline constexpr std::array<Bitboard, 64> KNIGHT_ATTACKS = make_knight_attacks();
inline constexpr std::array<Bitboard, 64> KING_ATTACKS   = make_king_attacks();
inline constexpr std::array<std::array<Bitboard, 64>, NUM_COLORS> PAWN_ATTACKS = make_pawn_attacks();



// ---------------------------------------------------------------------
//  Per-square accessors (thin wrappers over the tables)
// ---------------------------------------------------------------------

// All squares a knight on `sq` attacks.
constexpr Bitboard knight_attacks(int sq) { return KNIGHT_ATTACKS[sq]; }

// All squares a king on `sq` attacks.
constexpr Bitboard king_attacks(int sq) { return KING_ATTACKS[sq]; }

// Diagonal capture targets of a pawn of color `c` on `sq`.
constexpr Bitboard pawn_attacks(Color c, int sq) { return PAWN_ATTACKS[c][sq]; }





// ---------------------------------------------------------------------
//  Bulk pawn helpers (bitboard-in -> bitboard-out)
//
//  These take runtime bitboards and so are plain constexpr functions,
//  independent of how the lookup tables above were built. They are the
//  building blocks the move generator (A7) will use.
//
//  "empty" has 1s for squares that have nothing on them allowing & check to work
// ---------------------------------------------------------------------

// Single pushes: each pawn advanced one square forward, kept only where the
// destination square is empty. 
constexpr Bitboard single_pawn_pushes(Bitboard pawns, Bitboard empty, Color c) {
    return (c == WHITE) ? ((pawns << 8) & empty)
                        : ((pawns >> 8) & empty);
}

// Double pushes: pawns that can advance two squares. A pawn must first be
// able to single-push (intermediate square empty), then push again onto an
// empty square; the result is constrained to the relevant landing rank
// (RANK_4 for white, RANK_5 for black) which also enforces the start rank.
constexpr Bitboard double_pawn_pushes(Bitboard pawns, Bitboard empty, Color c) {
    const Bitboard single = single_pawn_pushes(pawns, empty, c);
    return (c == WHITE) ? ((single << 8) & empty & RANK_4)
                        : ((single >> 8) & empty & RANK_5);
}

// All squares attacked by a set of pawns of color `c` (shift + file-wrap
// mask). Used later by isSquareAttacked.
constexpr Bitboard pawn_attacks_bb(Bitboard pawns, Color c) {
    if (c == WHITE) {
        const Bitboard nw = (pawns << 7) & ~FILE_H;  // NW: not from a-file wrap
        const Bitboard ne = (pawns << 9) & ~FILE_A;  // NE
        return nw | ne;
    } else {
        const Bitboard sw = (pawns >> 9) & ~FILE_H;  // SW
        const Bitboard se = (pawns >> 7) & ~FILE_A;  // SE
        return sw | se;
    }
}

// ---------------------------------------------------------------------
//  Slider attacks (bishop / rook / queen) — classical ray tracing
//
//  Unlike the leapers above there is no precomputed table: a slider's
//  reach depends on the runtime `occupancy` (the set of ALL occupied
//  squares, both colors). For each direction we step outward one square
//  at a time, adding every empty square; the FIRST occupied square is
//  included (the slider "sees" the blocker — own vs. enemy is filtered
//  later in move generation, not here) and the ray stops there.
//
//  Bounds are checked on (file, rank) every step — the same wrap-safe
//  technique the leaper tables use — so rays never wrap across files.
//  Raw bit-shifts are deliberately NOT used for sliders because they
//  wrap around the board edges. The origin square's own bit in
//  `occupancy` is irrelevant since we start from its neighbors.
//
//  When magic/PEXT bitboards arrive (a later step) these functions
//  become both the table generator and the differential test oracle.
// ---------------------------------------------------------------------

// Walk the four (df, dr) directions in `dirs`, accumulating attacked
// squares until each ray leaves the board or hits an occupied square.
constexpr Bitboard ray_attacks(int sq, Bitboard occupancy,
                               const int (&df)[4], const int (&dr)[4]) {
    const int f = file_of(sq), r = rank_of(sq);
    Bitboard bb = EMPTY;
    for (int i = 0; i < 4; ++i) {
        int nf = f + df[i], nr = r + dr[i];
        while (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
            const int target = make_square(nf, nr);
            bb |= square_bb(target);
            if (test_bit(occupancy, target)) break;  // include blocker, then stop
            nf += df[i];
            nr += dr[i];
        }
    }
    return bb;
}

// All squares a bishop on `sq` attacks given `occupancy` (diagonals).
constexpr Bitboard bishop_attacks(int sq, Bitboard occupancy) {
    constexpr int df[4] = { 1,  1, -1, -1};
    constexpr int dr[4] = { 1, -1,  1, -1};
    return ray_attacks(sq, occupancy, df, dr);
}

// All squares a rook on `sq` attacks given `occupancy` (file + rank).
constexpr Bitboard rook_attacks(int sq, Bitboard occupancy) {
    constexpr int df[4] = {0,  0, 1, -1};
    constexpr int dr[4] = {1, -1, 0,  0};
    return ray_attacks(sq, occupancy, df, dr);
}

// All squares a queen on `sq` attacks: union of bishop and rook rays.
constexpr Bitboard queen_attacks(int sq, Bitboard occupancy) {
    return bishop_attacks(sq, occupancy) | rook_attacks(sq, occupancy);
}

} // namespace chess::bb

#endif // CHESS_BB_ATTACKS_H
