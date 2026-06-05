#include "chess/bbEval/Eval.h"

#include "chess/bb/BBoard.h"
#include "chess/bb/Bitboard.h"

namespace chess::bbEval {

namespace {

// Material values in centipawns.
constexpr int MATERIAL[chess::bb::NUM_PIECE_TYPES] = {
    100,    // PAWN
    320,    // KNIGHT
    330,    // BISHOP
    500,    // ROOK
    900,    // QUEEN
    20000   // KING
};

// Piece-square tables from White's perspective, LERF (a1=0).
// Indexed [0..63]: index 0 = a1, index 7 = h1, index 56 = a8, index 63 = h8.

constexpr int PST_PAWN[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,   // rank 1
     5, 10, 10,-20,-20, 10, 10,  5,   // rank 2
     5, -5,-10,  0,  0,-10, -5,  5,   // rank 3
     0,  0,  0, 20, 20,  0,  0,  0,   // rank 4
     5,  5, 10, 25, 25, 10,  5,  5,   // rank 5
    10, 10, 20, 30, 30, 20, 10, 10,   // rank 6
    50, 50, 50, 50, 50, 50, 50, 50,   // rank 7
     0,  0,  0,  0,  0,  0,  0,  0    // rank 8
};

constexpr int PST_KNIGHT[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,   // rank 1
    -40,-20,  0,  5,  5,  0,-20,-40,   // rank 2
    -30,  5, 10, 15, 15, 10,  5,-30,   // rank 3
    -30,  0, 15, 20, 20, 15,  0,-30,   // rank 4
    -30,  5, 15, 20, 20, 15,  5,-30,   // rank 5
    -30,  0, 10, 15, 15, 10,  0,-30,   // rank 6
    -40,-20,  0,  0,  0,  0,-20,-40,   // rank 7
    -50,-40,-30,-30,-30,-30,-40,-50    // rank 8
};

constexpr int PST_BISHOP[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,   // rank 1
    -10,  5,  0,  0,  0,  0,  5,-10,   // rank 2
    -10, 10, 10, 10, 10, 10, 10,-10,   // rank 3
    -10,  0, 10, 10, 10, 10,  0,-10,   // rank 4
    -10,  5,  5, 10, 10,  5,  5,-10,   // rank 5
    -10,  0,  5, 10, 10,  5,  0,-10,   // rank 6
    -10,  0,  0,  0,  0,  0,  0,-10,   // rank 7
    -20,-10,-10,-10,-10,-10,-10,-20    // rank 8
};

constexpr int PST_ROOK[64] = {
     0,  0,  0,  5,  5,  0,  0,  0,   // rank 1
    -5,  0,  0,  0,  0,  0,  0, -5,   // rank 2
    -5,  0,  0,  0,  0,  0,  0, -5,   // rank 3
    -5,  0,  0,  0,  0,  0,  0, -5,   // rank 4
    -5,  0,  0,  0,  0,  0,  0, -5,   // rank 5
    -5,  0,  0,  0,  0,  0,  0, -5,   // rank 6
     5, 10, 10, 10, 10, 10, 10,  5,   // rank 7
     0,  0,  0,  0,  0,  0,  0,  0    // rank 8
};

constexpr int PST_QUEEN[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,   // rank 1
    -10,  0,  5,  0,  0,  0,  0,-10,   // rank 2
    -10,  5,  5,  5,  5,  5,  0,-10,   // rank 3
      0,  0,  5,  5,  5,  5,  0, -5,   // rank 4
     -5,  0,  5,  5,  5,  5,  0, -5,   // rank 5
    -10,  0,  5,  5,  5,  5,  0,-10,   // rank 6
    -10,  0,  0,  0,  0,  0,  0,-10,   // rank 7
    -20,-10,-10, -5, -5,-10,-10,-20    // rank 8
};

constexpr int PST_KING[64] = {
     20, 30, 10,  0,  0, 10, 30, 20,   // rank 1
     20, 20,  0,  0,  0,  0, 20, 20,   // rank 2
    -10,-20,-20,-20,-20,-20,-20,-10,   // rank 3
    -20,-30,-30,-40,-40,-30,-30,-20,   // rank 4
    -30,-40,-40,-50,-50,-40,-40,-30,   // rank 5
    -30,-40,-40,-50,-50,-40,-40,-30,   // rank 6
    -30,-40,-40,-50,-50,-40,-40,-30,   // rank 7
    -30,-40,-40,-50,-50,-40,-40,-30    // rank 8
};

// Dispatch PST lookup by piece type.
constexpr int pst_value(chess::bb::PieceType pt, int sq) {
    switch (pt) {
        case chess::bb::PAWN:   return PST_PAWN[sq];
        case chess::bb::KNIGHT: return PST_KNIGHT[sq];
        case chess::bb::BISHOP: return PST_BISHOP[sq];
        case chess::bb::ROOK:   return PST_ROOK[sq];
        case chess::bb::QUEEN:  return PST_QUEEN[sq];
        case chess::bb::KING:   return PST_KING[sq];
        default:                return 0;
    }
}

} // anonymous namespace

int evaluate(const chess::bb::BBoard& board) {
    int white_total = 0;
    int black_total = 0;

    for (int pt_int = 0; pt_int < chess::bb::NUM_PIECE_TYPES; ++pt_int) {
        const auto pt = static_cast<chess::bb::PieceType>(pt_int);
        const int mat = MATERIAL[pt_int];

        // White pieces
        chess::bb::Bitboard wb = board.pieceBB[chess::bb::WHITE][pt];
        while (wb) {
            int sq = chess::bb::pop_lsb(wb);
            white_total += mat + pst_value(pt, sq);
        }

        // Black pieces — mirror square vertically for PST lookup
        chess::bb::Bitboard bb = board.pieceBB[chess::bb::BLACK][pt];
        while (bb) {
            int sq = chess::bb::pop_lsb(bb);
            black_total += mat + pst_value(pt, sq ^ 56);
        }
    }

    int score = white_total - black_total;
    return score * (board.sideToMove == chess::bb::WHITE ? 1 : -1);
}

} // namespace chess::bbEval
