#include "chess/bb/MoveGen.h"

#include "chess/bb/Attacks.h"
#include "chess/bb/Bitboard.h"

namespace chess::bb {

// ---------------------------------------------------------------------------
//  Internal helpers
// ---------------------------------------------------------------------------

namespace {

// Emit one move per promotion piece type (N/B/R/Q), quiet or capture.
inline void addPromotions(MoveList& ml, int from, int to, bool capture) {
    ml.add(make_promotion(from, to, KNIGHT, capture));
    ml.add(make_promotion(from, to, BISHOP, capture));
    ml.add(make_promotion(from, to, ROOK,   capture));
    ml.add(make_promotion(from, to, QUEEN,  capture));
}

// Expand a bitboard of destination squares into individual moves from `from`.
// Each destination is classified as quiet or capture based on enemy occupancy.
inline void emitMoves(MoveList& ml, int from, Bitboard targets, Bitboard enemy) {
    while (targets) {
        const int to = pop_lsb(targets);
        if (test_bit(enemy, to))
            ml.add(make_move(from, to, FLAG_CAPTURE));
        else
            ml.add(make_move(from, to, FLAG_QUIET));
    }
}

// ---------------------------------------------------------------------------
//  Per-piece-type generators
// ---------------------------------------------------------------------------

void genKnights(const BBoard& b, Color us, Color them, MoveList& ml) {
    Bitboard pieces = b.pieceBB[us][KNIGHT];
    while (pieces) {
        const int from = pop_lsb(pieces);
        Bitboard targets = knight_attacks(from) & ~b.occupancy[us];
        emitMoves(ml, from, targets, b.occupancy[them]);
    }
}

void genBishops(const BBoard& b, Color us, Color them, MoveList& ml) {
    Bitboard pieces = b.pieceBB[us][BISHOP];
    while (pieces) {
        const int from = pop_lsb(pieces);
        Bitboard targets = bishop_attacks(from, b.occupancyAll) & ~b.occupancy[us];
        emitMoves(ml, from, targets, b.occupancy[them]);
    }
}

void genRooks(const BBoard& b, Color us, Color them, MoveList& ml) {
    Bitboard pieces = b.pieceBB[us][ROOK];
    while (pieces) {
        const int from = pop_lsb(pieces);
        Bitboard targets = rook_attacks(from, b.occupancyAll) & ~b.occupancy[us];
        emitMoves(ml, from, targets, b.occupancy[them]);
    }
}

void genQueens(const BBoard& b, Color us, Color them, MoveList& ml) {
    Bitboard pieces = b.pieceBB[us][QUEEN];
    while (pieces) {
        const int from = pop_lsb(pieces);
        Bitboard targets = queen_attacks(from, b.occupancyAll) & ~b.occupancy[us];
        emitMoves(ml, from, targets, b.occupancy[them]);
    }
}

void genKing(const BBoard& b, Color us, Color them, MoveList& ml) {
    Bitboard pieces = b.pieceBB[us][KING];
    while (pieces) {
        const int from = pop_lsb(pieces);
        Bitboard targets = king_attacks(from) & ~b.occupancy[us];
        emitMoves(ml, from, targets, b.occupancy[them]);
    }
}

// Castling: check that the right is set and all intermediate squares are empty.
// Does NOT check for attacks on intermediate or destination squares (that is A8).
void genCastling(const BBoard& b, Color us, MoveList& ml) {
    const Bitboard occ = b.occupancyAll;

    if (us == WHITE) {
        // White king-side: F1, G1 must be empty.
        if ((b.castling & WHITE_OO) &&
            !(occ & (square_bb(F1) | square_bb(G1)))) {
            ml.add(make_move(E1, G1, FLAG_OO));
        }
        // White queen-side: B1, C1, D1 must be empty.
        if ((b.castling & WHITE_OOO) &&
            !(occ & (square_bb(B1) | square_bb(C1) | square_bb(D1)))) {
            ml.add(make_move(E1, C1, FLAG_OOO));
        }
    } else {
        // Black king-side: F8, G8 must be empty.
        if ((b.castling & BLACK_OO) &&
            !(occ & (square_bb(F8) | square_bb(G8)))) {
            ml.add(make_move(E8, G8, FLAG_OO));
        }
        // Black queen-side: B8, C8, D8 must be empty.
        if ((b.castling & BLACK_OOO) &&
            !(occ & (square_bb(B8) | square_bb(C8) | square_bb(D8)))) {
            ml.add(make_move(E8, C8, FLAG_OOO));
        }
    }
}

// Pawn generation: single pushes, double pushes, captures, and en passant.
// Promotions (back-rank arrivals) each expand into four separate moves.
void genPawns(const BBoard& b, Color us, Color them, MoveList& ml) {
    const Bitboard pawns  = b.pieceBB[us][PAWN];
    const Bitboard empty  = ~b.occupancyAll;
    const Bitboard enemy  = b.occupancy[them];

    // The back rank from the perspective of `us`: rank 7 (index 7) for white,
    // rank 0 (index 0) for black.
    const int backRank = (us == WHITE) ? 7 : 0;

    // --- 1. Single pushes -----------------------------------------------
    {
        Bitboard dests = single_pawn_pushes(pawns, empty, us);
        Bitboard tmp = dests;
        while (tmp) {
            const int to   = pop_lsb(tmp);
            const int from = (us == WHITE) ? (to - 8) : (to + 8);
            if (rank_of(to) == backRank)
                addPromotions(ml, from, to, false);
            else
                ml.add(make_move(from, to, FLAG_QUIET));
        }
    }

    // --- 2. Double pushes -----------------------------------------------
    {
        Bitboard dests = double_pawn_pushes(pawns, empty, us);
        Bitboard tmp = dests;
        while (tmp) {
            const int to   = pop_lsb(tmp);
            const int from = (us == WHITE) ? (to - 16) : (to + 16);
            ml.add(make_move(from, to, FLAG_DOUBLE_PUSH));
        }
    }

    // --- 3. Pawn captures (including promotion-captures) ----------------
    {
        Bitboard tmp = pawns;
        while (tmp) {
            const int from    = pop_lsb(tmp);
            Bitboard  attacks = PAWN_ATTACKS[us][from] & enemy;
            while (attacks) {
                const int to = pop_lsb(attacks);
                if (rank_of(to) == backRank)
                    addPromotions(ml, from, to, true);
                else
                    ml.add(make_move(from, to, FLAG_CAPTURE));
            }
        }
    }

    // --- 4. En passant --------------------------------------------------
    if (b.epSquare != NO_SQUARE) {
        // PAWN_ATTACKS[them][epSquare] gives the squares from which an
        // opponent pawn would attack from the ep square — these are exactly the
        // squares one of our pawns must occupy to make the capture.
        Bitboard epPawns = PAWN_ATTACKS[them][b.epSquare] & pawns;
        while (epPawns) {
            const int from = pop_lsb(epPawns);
            ml.add(make_move(from, b.epSquare, FLAG_EP_CAPTURE));
        }
    }
}

} // namespace

// ---------------------------------------------------------------------------
//  Public entry point
// ---------------------------------------------------------------------------

void generateMoves(const BBoard& board, MoveList& ml) {
    const Color us   = board.sideToMove;
    const Color them = static_cast<Color>(us ^ 1);

    genPawns(board, us, them, ml);
    genKnights(board, us, them, ml);
    genBishops(board, us, them, ml);
    genRooks(board, us, them, ml);
    genQueens(board, us, them, ml);
    genKing(board, us, them, ml);
    genCastling(board, us, ml);
}

// ---------------------------------------------------------------------------
//  A8 — Legality filter + check detection
// ---------------------------------------------------------------------------

bool isSquareAttacked(int sq, Color byColor, const BBoard& b) {
    // Pawns: a pawn of `byColor` attacks `sq` iff a pawn of the opposite color
    // placed on `sq` would attack back into `byColor`'s pawn set.
    if (PAWN_ATTACKS[byColor ^ 1][sq] & b.pieceBB[byColor][PAWN])
        return true;

    // Knights
    if (knight_attacks(sq) & b.pieceBB[byColor][KNIGHT])
        return true;

    // Bishops and diagonal-moving queens
    if (bishop_attacks(sq, b.occupancyAll) &
        (b.pieceBB[byColor][BISHOP] | b.pieceBB[byColor][QUEEN]))
        return true;

    // Rooks and rank/file-moving queens
    if (rook_attacks(sq, b.occupancyAll) &
        (b.pieceBB[byColor][ROOK] | b.pieceBB[byColor][QUEEN]))
        return true;

    // King
    if (king_attacks(sq) & b.pieceBB[byColor][KING])
        return true;

    return false;
}

bool isInCheck(const BBoard& b, Color c) {
    const int kingSq = lsb(b.pieceBB[c][KING]);
    return isSquareAttacked(kingSq, static_cast<Color>(c ^ 1), b);
}

void generateLegalMoves(BBoard& board, MoveList& ml) {
    const Color us   = board.sideToMove;
    const Color them = static_cast<Color>(us ^ 1);

    MoveList pseudo;
    generateMoves(board, pseudo);

    for (int i = 0; i < pseudo.count; ++i) {
        const Move m = pseudo.moves[i];

        // Castling has two extra legality conditions that must be tested in
        // the PRE-move position (makeMove loses this information):
        //   (a) The king may not be in check before castling.
        //   (b) The king may not pass through an attacked square.
        // The destination square is already covered by the post-move isInCheck
        // below, so only the starting square and the single transit square need
        // to be tested here.
        if (is_castle(m)) {
            const int kingSq  = lsb(board.pieceBB[us][KING]);
            if (isSquareAttacked(kingSq, them, board)) continue;

            const int transitSq = is_king_castle(m) ? (us == WHITE ? F1 : F8)
                                                     : (us == WHITE ? D1 : D8);
            if (isSquareAttacked(transitSq, them, board)) continue;
        }

        Undo u;
        board.makeMove(m, u);
        if (!isInCheck(board, us))
            ml.add(m);
        board.unmakeMove(m, u);
    }
}

bool isCheckmate(BBoard& b) {
    MoveList ml;
    generateLegalMoves(b, ml);
    return ml.count == 0 && isInCheck(b, b.sideToMove);
}

bool isStalemate(BBoard& b) {
    MoveList ml;
    generateLegalMoves(b, ml);
    return ml.count == 0 && !isInCheck(b, b.sideToMove);
}

} // namespace chess::bb
