#ifndef CHESS_BB_ZOBRIST_H
#define CHESS_BB_ZOBRIST_H

#include <cstdint>

//
//  Zobrist hashing tables for the bitboard board representation.
//
//  The random numbers are drawn from std::mt19937_64 seeded with 0xDEADBEEF, in the exact same order,
//  so that hashes computed here cross-validate bit-for-bit against the old engine.
//
//  Layout / indexing conventions:
//
//    pieceTable[2][6][64]   -- [color][pieceType][square]
//        color:     WHITE = 0, BLACK = 1   (matches enum Color)
//        pieceType: PAWN..KING = 0..5      (matches enum PieceType)
//        square:    LERF index 0..63 (a1 = 0 ... h8 = 63)
//
//    castling[4]            -- 0 = white-K (OO), 1 = white-Q (OOO),
//                              2 = black-K (oo), 3 = black-Q (ooo)
//    enPassant[8]           -- keyed by file (a..h = 0..7)
//    sideToMove             -- XORed in when it is BLACK to move
//
//  The keys are bit-identical to the old engine. Because the LERF square index is
//  exactly rank*8 + file, drawing in order color(0..1), type(0..5),
//  square(0..63) reproduces the identical stream and lands each key on the
//  same square -- just indexed directly by square instead of (rank, file).
//  After the piece keys, for i=0..3: castling[i], enPassant[i],
//  enPassant[i+4]; then sideToMove.

namespace chess::bb {

struct ZobristKeys {
    std::uint64_t pieceTable[2][6][64];   // [color][pieceType][square]
    std::uint64_t castling[4];
    std::uint64_t enPassant[8];
    std::uint64_t sideToMove;
};

// Returns the lazily-initialized, deterministic Zobrist key table.
const ZobristKeys& zobrist();

} // namespace chess::bb

#endif // CHESS_BB_ZOBRIST_H
