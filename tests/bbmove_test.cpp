// Verification gate for step A3 (packed BBMove: encode/decode, predicates,
// UCI formatting). Standalone test, no CTest wiring (deferred to step B2).
//
// Build & run:
//   c++ -std=c++23 -Wall -Wextra -I include tests/bbmove_test.cpp -o /tmp/bbmove_test && /tmp/bbmove_test

#include "chess/bb/BBMove.h"

#include <cstdint>
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
    // Move must be exactly 16 bits wide.
    check(sizeof(Move) == 2, "sizeof(Move) == 2");

    // Null move.
    check(MOVE_NONE == 0, "MOVE_NONE == 0");
    check(to_uci(MOVE_NONE) == "0000", "null move -> 0000");

    // --- quiet move: b1c3 (knight develop) ------------------------------
    {
        Move m = make_move(B1, C3, FLAG_QUIET);
        check(from_sq(m) == B1, "quiet from == b1");
        check(to_sq(m) == C3, "quiet to == c3");
        check(!is_capture(m), "quiet not capture");
        check(!is_promotion(m), "quiet not promotion");
        check(!is_en_passant(m), "quiet not ep");
        check(!is_castle(m), "quiet not castle");
        check(!is_double_push(m), "quiet not double push");
        check(to_uci(m) == "b1c3", "quiet uci b1c3");
    }

    // --- double pawn push: e2e4 -----------------------------------------
    {
        Move m = make_move(E2, E4, FLAG_DOUBLE_PUSH);
        check(from_sq(m) == E2, "double from == e2");
        check(to_sq(m) == E4, "double to == e4");
        check(is_double_push(m), "double push predicate");
        check(!is_capture(m), "double push not capture");
        check(!is_promotion(m), "double push not promotion");
        check(to_uci(m) == "e2e4", "double push uci e2e4");
    }

    // --- plain capture: e4d5 --------------------------------------------
    {
        Move m = make_move(E4, D5, FLAG_CAPTURE);
        check(from_sq(m) == E4, "capture from == e4");
        check(to_sq(m) == D5, "capture to == d5");
        check(is_capture(m), "capture predicate");
        check(!is_en_passant(m), "capture not ep");
        check(!is_promotion(m), "capture not promotion");
        check(to_uci(m) == "e4d5", "capture uci e4d5");
    }

    // --- en-passant capture: e5d6 ---------------------------------------
    {
        Move m = make_move(E5, D6, FLAG_EP_CAPTURE);
        check(from_sq(m) == E5, "ep from == e5");
        check(to_sq(m) == D6, "ep to == d6");
        check(is_en_passant(m), "ep predicate");
        check(is_capture(m), "ep counts as capture");
        check(!is_promotion(m), "ep not promotion");
        check(to_uci(m) == "e5d6", "ep uci e5d6");
    }

    // --- king-side castle: e1g1 -----------------------------------------
    {
        Move m = make_move(E1, G1, FLAG_OO);
        check(from_sq(m) == E1, "OO from == e1");
        check(to_sq(m) == G1, "OO to == g1");
        check(is_king_castle(m), "OO king-castle predicate");
        check(is_castle(m), "OO is castle");
        check(!is_queen_castle(m), "OO not queen-castle");
        check(!is_capture(m), "OO not capture");
        check(to_uci(m) == "e1g1", "OO uci e1g1");
    }

    // --- queen-side castle: e1c1 ----------------------------------------
    {
        Move m = make_move(E1, C1, FLAG_OOO);
        check(from_sq(m) == E1, "OOO from == e1");
        check(to_sq(m) == C1, "OOO to == c1");
        check(is_queen_castle(m), "OOO queen-castle predicate");
        check(is_castle(m), "OOO is castle");
        check(!is_king_castle(m), "OOO not king-castle");
        check(to_uci(m) == "e1c1", "OOO uci e1c1");
    }

    // --- non-capturing promotions: e7e8 = Q/R/B/N -----------------------
    {
        struct Case { PieceType pt; const char* uci; };
        const Case cases[] = {
            {QUEEN,  "e7e8q"},
            {ROOK,   "e7e8r"},
            {BISHOP, "e7e8b"},
            {KNIGHT, "e7e8n"},
        };
        for (const auto& c : cases) {
            Move m = make_promotion(E7, E8, c.pt, /*capture=*/false);
            check(from_sq(m) == E7, std::string("promo from == e7 for ") + c.uci);
            check(to_sq(m) == E8, std::string("promo to == e8 for ") + c.uci);
            check(is_promotion(m), std::string("promo predicate for ") + c.uci);
            check(!is_capture(m), std::string("quiet promo not capture for ") + c.uci);
            check(promotion_type(m) == c.pt, std::string("promo type for ") + c.uci);
            check(to_uci(m) == c.uci, std::string("promo uci ") + c.uci);
        }
    }

    // --- capturing promotions: e7d8 = Q/R/B/N ---------------------------
    {
        struct Case { PieceType pt; const char* uci; };
        const Case cases[] = {
            {QUEEN,  "e7d8q"},
            {ROOK,   "e7d8r"},
            {BISHOP, "e7d8b"},
            {KNIGHT, "e7d8n"},
        };
        for (const auto& c : cases) {
            Move m = make_promotion(E7, D8, c.pt, /*capture=*/true);
            check(from_sq(m) == E7, std::string("capt-promo from == e7 for ") + c.uci);
            check(to_sq(m) == D8, std::string("capt-promo to == d8 for ") + c.uci);
            check(is_promotion(m), std::string("capt-promo predicate for ") + c.uci);
            check(is_capture(m), std::string("capt-promo is capture for ") + c.uci);
            check(promotion_type(m) == c.pt, std::string("capt-promo type for ") + c.uci);
            check(to_uci(m) == c.uci, std::string("capt-promo uci ") + c.uci);
        }
    }

    std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    if (g_failures == 0) {
        std::cout << "ALL TESTS PASSED\n";
        return EXIT_SUCCESS;
    }
    std::cout << g_failures << " CHECK(S) FAILED\n";
    return EXIT_FAILURE;
}
