#include "chess/bb/BBoard.h"

#include "chess/bb/Zobrist.h"

#include <cctype>
#include <sstream>

namespace chess::bb {

namespace {

// Map a FEN piece letter to (color, type). Returns false if not a piece char.
bool decodePieceChar(char c, Color& color, PieceType& type) {
    color = std::isupper(static_cast<unsigned char>(c)) ? WHITE : BLACK;
    switch (std::toupper(static_cast<unsigned char>(c))) {
        case 'P': type = PAWN;   return true;
        case 'N': type = KNIGHT; return true;
        case 'B': type = BISHOP; return true;
        case 'R': type = ROOK;   return true;
        case 'Q': type = QUEEN;  return true;
        case 'K': type = KING;   return true;
        default:  return false;
    }
}

// Map (color, type) to the FEN letter (upper for white, lower for black).
char encodePieceChar(Color color, PieceType type) {
    static const char letters[NUM_PIECE_TYPES] = {'P', 'N', 'B', 'R', 'Q', 'K'};
    char c = letters[type];
    return (color == WHITE) ? c
                            : static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

} // namespace

void BBoard::clear() {
    for (int c = 0; c < NUM_COLORS; ++c)
        for (int t = 0; t < NUM_PIECE_TYPES; ++t)
            pieceBB[c][t] = EMPTY; //Clears all bitboards to be empty
    occupancy[WHITE] = occupancy[BLACK] = EMPTY;
    occupancyAll     = EMPTY;
    sideToMove       = WHITE;
    castling         = 0;
    epSquare         = NO_SQUARE;
    halfmoveClock    = 0;
    fullmoveNumber   = 1;
    hash             = 0;  //Zobrist Hash of current Bitboard position
}

void BBoard::rebuildOccupancies() {
    occupancy[WHITE] = occupancy[BLACK] = EMPTY;
    for (int t = 0; t < NUM_PIECE_TYPES; ++t) {
        occupancy[WHITE] |= pieceBB[WHITE][t];
        occupancy[BLACK] |= pieceBB[BLACK][t];
    }
    occupancyAll = occupancy[WHITE] | occupancy[BLACK];
}

PieceType BBoard::pieceTypeOn(Color color, int sq) const {
    for (int t = 0; t < NUM_PIECE_TYPES; ++t)
        if (test_bit(pieceBB[color][t], sq))
            return static_cast<PieceType>(t);
    return NO_PIECE_TYPE;
}

bool BBoard::setFromFEN(const std::string& fen) {
    clear();

    std::istringstream iss(fen);
    std::string placement, activeColor, castleField, epField;
    if (!(iss >> placement >> activeColor >> castleField >> epField)) {
        clear();
        return false;
    }

    // --- piece placement -------------------------------------------------
    // FEN lists rank 8 first; LERF has rank 8 at the high indices. Start at
    // fenRank = 7 (rank 8) and decrement on '/'.
    int fenRank = 7;
    int file    = 0;
    for (char c : placement) {
        if (c == '/') {
            --fenRank;
            file = 0;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            file += c - '0';
        } else {
            Color color;
            PieceType type;
            if (!decodePieceChar(c, color, type) ||
                fenRank < 0 || fenRank > 7 || file < 0 || file > 7) {
                clear();
                return false;
            }
            set_bit_ip(pieceBB[color][type], make_square(file, fenRank));
            ++file;
        }
    }

    // --- active color ----------------------------------------------------
    if (activeColor == "w")      sideToMove = WHITE;
    else if (activeColor == "b") sideToMove = BLACK;
    else { clear(); return false; }

    // --- castling rights -------------------------------------------------
    if (castleField != "-") {
        for (char c : castleField) {
            switch (c) {
                case 'K': castling |= WHITE_OO;  break;
                case 'Q': castling |= WHITE_OOO; break;
                case 'k': castling |= BLACK_OO;  break;
                case 'q': castling |= BLACK_OOO; break;
                default:  clear(); return false;
            }
        }
    }

    // --- en passant target ----------------------------------------------
    if (epField == "-") {
        epSquare = NO_SQUARE;
    } else if (epField.size() == 2 &&
               epField[0] >= 'a' && epField[0] <= 'h' &&
               epField[1] >= '1' && epField[1] <= '8') {
        epSquare = make_square(epField[0] - 'a', epField[1] - '1');
    } else {
        clear();
        return false;
    }

    // --- halfmove / fullmove (default if absent) -------------------------
    int halfmove = 0;
    int fullmove = 1;
    if (iss >> halfmove) {
        if (!(iss >> fullmove)) fullmove = 1;
    }
    halfmoveClock  = halfmove;
    fullmoveNumber = fullmove;

    rebuildOccupancies();
    computeHash();
    return true;
}

std::string BBoard::toFEN() const {
    std::ostringstream out;

    // --- piece placement: walk ranks 8 -> 1, files a -> h ---------------
    for (int rank = 7; rank >= 0; --rank) {
        int empty = 0;
        for (int file = 0; file < 8; ++file) {
            const int sq = make_square(file, rank);
            PieceType wt = pieceTypeOn(WHITE, sq);
            PieceType bt = pieceTypeOn(BLACK, sq);
            if (wt == NO_PIECE_TYPE && bt == NO_PIECE_TYPE) {
                ++empty;
            } else {
                if (empty > 0) { out << empty; empty = 0; }
                if (wt != NO_PIECE_TYPE) out << encodePieceChar(WHITE, wt);
                else                     out << encodePieceChar(BLACK, bt);
            }
        }
        if (empty > 0) out << empty;
        if (rank > 0) out << '/';
    }

    // --- active color ----------------------------------------------------
    out << ' ' << (sideToMove == WHITE ? 'w' : 'b');

    // --- castling: exact KQkq order, '-' if none ------------------------
    out << ' ';
    if (castling == 0) {
        out << '-';
    } else {
        if (castling & WHITE_OO)  out << 'K';
        if (castling & WHITE_OOO) out << 'Q';
        if (castling & BLACK_OO)  out << 'k';
        if (castling & BLACK_OOO) out << 'q';
    }

    // --- en passant ------------------------------------------------------
    out << ' ' << (epSquare == NO_SQUARE ? std::string("-") : square_name(epSquare));

    // --- clocks ----------------------------------------------------------
    out << ' ' << halfmoveClock << ' ' << fullmoveNumber;

    return out.str();
}

std::uint64_t BBoard::computeHash() {
    const ZobristKeys& z = zobrist();
    std::uint64_t h = 0;

    for (int c = 0; c < NUM_COLORS; ++c) {
        for (int t = 0; t < NUM_PIECE_TYPES; ++t) {
            Bitboard bb = pieceBB[c][t];
            while (bb) {
                const int sq = pop_lsb(bb);
                h ^= z.pieceTable[c][t][sq];
            }
        }
    }

    if (castling & WHITE_OO)  h ^= z.castling[0];
    if (castling & WHITE_OOO) h ^= z.castling[1];
    if (castling & BLACK_OO)  h ^= z.castling[2];
    if (castling & BLACK_OOO) h ^= z.castling[3];

    if (epSquare != NO_SQUARE)
        h ^= z.enPassant[file_of(epSquare)];

    if (sideToMove == BLACK)
        h ^= z.sideToMove;

    hash = h;
    return hash;
}

namespace {

// XOR in/out all castling keys whose bit is set in `rights`.
inline std::uint64_t castlingHash(const ZobristKeys& z, std::uint8_t rights) {
    std::uint64_t h = 0;
    if (rights & WHITE_OO)  h ^= z.castling[0];
    if (rights & WHITE_OOO) h ^= z.castling[1];
    if (rights & BLACK_OO)  h ^= z.castling[2];
    if (rights & BLACK_OOO) h ^= z.castling[3];
    return h;
}

// Castling rights lost when a piece leaves OR is captured on a given square.
// (A rook leaving a1 loses WHITE_OOO; a rook captured on a1 loses WHITE_OOO;
// the king's own square is handled separately by clearing both of its rights.)
inline std::uint8_t castlingRightOnSquare(int sq) {
    switch (sq) {
        case A1: return WHITE_OOO;
        case H1: return WHITE_OO;
        case A8: return BLACK_OOO;
        case H8: return BLACK_OO;
        default: return 0;
    }
}

} // namespace

void BBoard::makeMove(Move m, Undo& u) {
    const ZobristKeys& z = zobrist();

    const Color us   = sideToMove;
    const Color them = static_cast<Color>(us ^ 1);
    const int   from = from_sq(m);
    const int   to   = to_sq(m);
    const PieceType moving = pieceTypeOn(us, from);

    // 1. Save reversal info.
    u.captured      = NO_PIECE_TYPE;
    u.castling      = castling;
    u.epSquare      = epSquare;
    u.halfmoveClock = halfmoveClock;
    u.hash          = hash;

    // 2. XOR out the current ep key and current castling keys; the new ones
    //    are XORed back in once those fields are updated.
    if (epSquare != NO_SQUARE)
        hash ^= z.enPassant[file_of(epSquare)];
    hash ^= castlingHash(z, castling);

    // 3. Captures.
    if (is_en_passant(m)) {
        // Captured pawn sits behind `to` (south for white capturer, north for black).
        const int capSq = (us == WHITE) ? (to - 8) : (to + 8);
        clear_bit_ip(pieceBB[them][PAWN], capSq);
        hash ^= z.pieceTable[them][PAWN][capSq];
        u.captured = PAWN;
    } else if (is_capture(m)) {
        const PieceType captured = pieceTypeOn(them, to);
        clear_bit_ip(pieceBB[them][captured], to);
        hash ^= z.pieceTable[them][captured][to];
        u.captured = captured;
    }

    // 4. Move the moving piece off `from`.
    clear_bit_ip(pieceBB[us][moving], from);
    hash ^= z.pieceTable[us][moving][from];
    if (is_promotion(m)) {
        const PieceType promo = promotion_type(m);
        set_bit_ip(pieceBB[us][promo], to);
        hash ^= z.pieceTable[us][promo][to];
    } else {
        set_bit_ip(pieceBB[us][moving], to);
        hash ^= z.pieceTable[us][moving][to];
    }

    // 5. Castling: relocate the rook (king move is handled in step 4).
    if (is_castle(m)) {
        int rookFrom = NO_SQUARE, rookTo = NO_SQUARE;
        if (is_king_castle(m)) {                 // O-O
            rookFrom = (us == WHITE) ? H1 : H8;
            rookTo   = (us == WHITE) ? F1 : F8;
        } else {                                 // O-O-O
            rookFrom = (us == WHITE) ? A1 : A8;
            rookTo   = (us == WHITE) ? D1 : D8;
        }
        clear_bit_ip(pieceBB[us][ROOK], rookFrom);
        set_bit_ip(pieceBB[us][ROOK], rookTo);
        hash ^= z.pieceTable[us][ROOK][rookFrom];
        hash ^= z.pieceTable[us][ROOK][rookTo];
    }

    // 6. Update castling rights.
    if (moving == KING) {
        // King moved (including a castle): lose both of its rights.
        castling &= (us == WHITE) ? ~(WHITE_OO | WHITE_OOO)
                                  : ~(BLACK_OO | BLACK_OOO);
    }
    // A rook that left its home square loses the matching right.
    castling &= ~castlingRightOnSquare(from);
    // A rook captured on its home square removes the opponent's matching right
    // (en passant never captures on a rook home square, so `to` is correct).
    castling &= ~castlingRightOnSquare(to);
    hash ^= castlingHash(z, castling);

    // 7. En-passant target: only a double push sets it.
    if (is_double_push(m)) {
        epSquare = (us == WHITE) ? (to - 8) : (to + 8);
        hash ^= z.enPassant[file_of(epSquare)];
    } else {
        epSquare = NO_SQUARE;
    }

    // 8. Clocks.
    if (moving == PAWN || is_capture(m)) halfmoveClock = 0;
    else                                 ++halfmoveClock;
    if (us == BLACK) ++fullmoveNumber;

    // 9. Flip side to move.
    sideToMove = them;
    hash ^= z.sideToMove;

    // 10. Occupancies (full rebuild is acceptable for this step; incremental
    //     occupancy is a later optimization).
    rebuildOccupancies();
}

void BBoard::unmakeMove(Move m, const Undo& u) {
    // The side that made the move is the opposite of the current side to move.
    const Color us   = static_cast<Color>(sideToMove ^ 1);
    const Color them = sideToMove;
    const int   from = from_sq(m);
    const int   to   = to_sq(m);

    // Restore side to move first so `us` semantics match makeMove.
    sideToMove = us;
    if (us == BLACK) --fullmoveNumber;

    // Undo the moved piece: promotions resolve to a pawn back on `from`.
    if (is_promotion(m)) {
        const PieceType promo = promotion_type(m);
        clear_bit_ip(pieceBB[us][promo], to);
        set_bit_ip(pieceBB[us][PAWN], from);
    } else {
        const PieceType moving = pieceTypeOn(us, to);
        clear_bit_ip(pieceBB[us][moving], to);
        set_bit_ip(pieceBB[us][moving], from);
    }

    // Undo the castling rook move.
    if (is_castle(m)) {
        int rookFrom = NO_SQUARE, rookTo = NO_SQUARE;
        if (is_king_castle(m)) {                 // O-O
            rookFrom = (us == WHITE) ? H1 : H8;
            rookTo   = (us == WHITE) ? F1 : F8;
        } else {                                 // O-O-O
            rookFrom = (us == WHITE) ? A1 : A8;
            rookTo   = (us == WHITE) ? D1 : D8;
        }
        clear_bit_ip(pieceBB[us][ROOK], rookTo);
        set_bit_ip(pieceBB[us][ROOK], rookFrom);
    }

    // Restore any captured piece on its correct square.
    if (u.captured != NO_PIECE_TYPE) {
        if (is_en_passant(m)) {
            const int capSq = (us == WHITE) ? (to - 8) : (to + 8);
            set_bit_ip(pieceBB[them][PAWN], capSq);
        } else {
            set_bit_ip(pieceBB[them][u.captured], to);
        }
    }

    // Restore scalar state and the saved hash directly.
    castling      = u.castling;
    epSquare      = u.epSquare;
    halfmoveClock = u.halfmoveClock;
    hash          = u.hash;

    rebuildOccupancies();
}

void BBoard::printBoard(std::ostream& os) const {
    for (int rank = 7; rank >= 0; --rank) {
        os << (rank + 1) << " |";
        for (int file = 0; file < 8; ++file) {
            const int sq = make_square(file, rank);
            PieceType wt = pieceTypeOn(WHITE, sq);
            PieceType bt = pieceTypeOn(BLACK, sq);
            char glyph = '.';
            if (wt != NO_PIECE_TYPE)      glyph = encodePieceChar(WHITE, wt);
            else if (bt != NO_PIECE_TYPE) glyph = encodePieceChar(BLACK, bt);
            os << ' ' << glyph;
        }
        os << '\n';
    }
    os << "-------------------\n";
    os << "    a b c d e f g h\n";
    os << "  side to move: " << (sideToMove == WHITE ? "white" : "black") << '\n';
    os << "  castling: ";
    if (castling == 0) {
        os << '-';
    } else {
        if (castling & WHITE_OO)  os << 'K';
        if (castling & WHITE_OOO) os << 'Q';
        if (castling & BLACK_OO)  os << 'k';
        if (castling & BLACK_OOO) os << 'q';
    }
    os << '\n';
    os << "  en passant: "
       << (epSquare == NO_SQUARE ? std::string("-") : square_name(epSquare)) << '\n';
    os << "  halfmove: " << halfmoveClock
       << "  fullmove: " << fullmoveNumber << '\n';
    os << "  hash: 0x" << std::hex << hash << std::dec << '\n';
}

} // namespace chess::bb
