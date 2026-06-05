#include "chess/bbSearch/Perft.h"

#include <cstdint>
#include <iostream>

namespace chess::bbSearch {

uint64_t perft(chess::bb::BBoard& board, int depth) {
    if (depth == 0) return 1;

    chess::bb::MoveList ml;
    generateLegalMoves(board, ml);

    // Bulk counting: at depth 1 return the move count directly without
    // making any moves. This is both faster and avoids a redundant layer
    // of recursion.
    if (depth == 1) return static_cast<uint64_t>(ml.count);

    uint64_t nodes = 0;
    for (int i = 0; i < ml.count; ++i) {
        chess::bb::Undo u;
        board.makeMove(ml.moves[i], u);
        nodes += perft(board, depth - 1);
        board.unmakeMove(ml.moves[i], u);
    }
    return nodes;
}

void perftDivide(chess::bb::BBoard& board, int depth) {
    chess::bb::MoveList ml;
    generateLegalMoves(board, ml);

    uint64_t total = 0;
    for (int i = 0; i < ml.count; ++i) {
        const chess::bb::Move m = ml.moves[i];
        chess::bb::Undo u;
        board.makeMove(m, u);

        uint64_t count;
        if (depth <= 1) {
            count = 1;
        } else {
            count = perft(board, depth - 1);
        }

        board.unmakeMove(m, u);

        std::cout << chess::bb::to_uci(m) << ": " << count << "\n";
        total += count;
    }
    std::cout << "\nTotal: " << total << "\n";
}

} // namespace chess::bbSearch
