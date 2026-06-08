#ifndef CHESS_BB_BBOARD_H
#define CHESS_BB_BBOARD_H

#include "chess/bb/Bitboard.h"

#include <cstdint>
#include <ostream>
#include <string>

//
//  BBoard: the bitboard board representation 
//
//  Holds 12 piece bitboards (indexed [color][pieceType]), per-color and
//  combined occupancies (derived, never independent state), side-to-move,
//  castling rights, en-passant TARGET square, halfmove clock, fullmove
//  number, and a Zobrist hash.
//
//  Square convention is LERF (a1 = 0); see Bitboard.h.
//
//  This step provides FEN parse/print, full-hash recomputation, and a
//  debug printer. Move generation, attacks, and make/unmake are LATER
//  steps (A3-A8) and are intentionally absent here.

namespace chess::bb {

enum Color : int { WHITE = 0, BLACK = 1, NUM_COLORS = 2 };

// Order matters: it indexes piece bitboards and the Zobrist table.
enum PieceType : int {
    PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5,
    NUM_PIECE_TYPES = 6,
    NO_PIECE_TYPE = 6
};

// Castling rights as 4 bitflags, OR-combined in BBoard::castling.
enum CastlingRight : std::uint8_t {
    WHITE_OO  = 1,  // white king-side  (K)
    WHITE_OOO = 2,  // white queen-side (Q)
    BLACK_OO  = 4,  // black king-side  (k)
    BLACK_OOO = 8   // black queen-side (q)
};

} // namespace chess::bb

// Move is needed in the make/unmake signatures below. BBMove.h re-includes
// this header for PieceType, but that nested include is a guarded no-op and
// the enums it needs (Color/PieceType/CastlingRight) are already defined
// above, so this cycle resolves cleanly. Placed here (after the enums,
// before struct Undo/BBoard).
#include "chess/bb/BBMove.h"

namespace chess::bb {

// Information saved on makeMove so unmakeMove can exactly reverse it.
// sideToMove and fullmoveNumber are NOT stored: they are restored
// deterministically (sideToMove flips back to the mover; fullmoveNumber is
// decremented when the unmade move was Black's). Everything else that a move
// can mutate irreversibly is captured here.
struct Undo {
    PieceType     captured      = NO_PIECE_TYPE; // captured piece type, or NO_PIECE_TYPE
    std::uint8_t  castling      = 0;             // castling rights before the move
    int           epSquare      = NO_SQUARE;     // en-passant target before the move
    int           halfmoveClock = 0;             // halfmove clock before the move
    std::uint64_t hash          = 0;             // Zobrist hash before the move
};

// This struct holds all fields needed to actually track a bitboard and board state.
struct BBoard {
    Bitboard pieceBB[NUM_COLORS][NUM_PIECE_TYPES] = {}; // 12 piece bitboards, for example white pawns are located at pieceBB[WHITE][PAWN]
    Bitboard occupancy[NUM_COLORS] = {};                // per-color occupancy
    Bitboard occupancyAll = 0;                          // both colors

    PieceType mailbox[NUM_COLORS][NUM_SQUARES] = {};  // zero-init; properly set by clear()/setFromFEN

    Color         sideToMove     = WHITE;
    std::uint8_t  castling       = 0;          // OR of CastlingRight flags
    int           epSquare       = NO_SQUARE;  // en-passant TARGET square
    int           halfmoveClock  = 0;          // for the 50-move rule
    int           fullmoveNumber = 1;

    std::uint64_t hash = 0;

    //Construction of positions / state trcking

    // Clear all state back to an empty board (white to move, no rights).
    void clear();

    // Replace state from a FEN string. Rebuilds occupancies and the hash.
    // Returns false on a malformed FEN (state is left cleared on failure).
    bool setFromFEN(const std::string& fen);

    // Serialize the current state to a FEN string. Exact inverse of
    // setFromFEN for well-formed input.
    std::string toFEN() const;

    // --- derived data ----------------------------------------------------

    // Rebuild occupancy[] and occupancyAll from pieceBB[][].
    void rebuildOccupancies();
    void rebuildMailbox();

    // Recompute the full Zobrist hash from scratch (no incremental update;
    // that is step A6). Also assigns it to `hash` and returns it.
    std::uint64_t computeHash();


    // Returns the piece type on `sq` for `color`, or NO_PIECE_TYPE if none.
    PieceType pieceTypeOn(Color color, int sq) const;

    // --- make / unmake ---------------------------------------------------

    // Apply `m` to the board, saving reversal info into `u`. Maintains the
    // Zobrist `hash` incrementally (matching computeHash() bit-for-bit).
    void makeMove(Move m, Undo& u);

    // Reverse a previously-applied `m`, restoring the state saved in `u`.
    void unmakeMove(Move m, const Undo& u);

    // --- debug -----------------------------------------------------------

    // Pretty-print the board (rank 8 on top, a-file on left) plus the
    // side-to-move / castling / ep / clocks summary.
    void printBoard(std::ostream& os) const;

    // Member-wise equality over every field (all 12 bitboards, occupancies,
    // side, castling, ep, both clocks, and hash). Used by the make/unmake
    // round-trip test (and later the transposition table).
    friend bool operator==(const BBoard&, const BBoard&) = default;
};

} // namespace chess::bb

#endif // CHESS_BB_BBOARD_H
