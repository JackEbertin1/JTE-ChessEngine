// Interactive observability REPL for the bitboard core (chess::bb).
//
// Displays the board, side to move, and en-passant square. Moves are
// validated against the full legal move list — only legal moves are applied.
//
// Build & run:
//   c++ -std=c++23 -Wall -Wextra -I include src/chess/bb/BBoard.cpp src/chess/bb/Zobrist.cpp src/chess/bb/MoveGen.cpp src/chess/bbSearch/Perft.cpp src/chess/bbSearch/TT.cpp src/chess/bbSearch/MoveOrder.cpp src/chess/bbSearch/Search.cpp src/chess/bbEval/Eval.cpp tools/observe.cpp -o /tmp/observe && /tmp/observe
//
// Commands:
//   <move>         e.g. a2a3, e1g1 (castle), e7e8q (promotion: q/r/b/n)
//   moves | m      list all legal moves for the current position
//   search depth <n>   run iterative deepening to depth n; report score and best move
//   search time <s>    search for s seconds; report eval, best move, and achieved depth
//   perft <n>      count leaf nodes to depth n from the current position
//   divide <n>     per-root-move leaf counts to depth n (bisect perft bugs)
//   undo  | u      take back the last move
//   help  | h      show this help
//   quit  | q      exit

#include "chess/bb/BBoard.h"
#include "chess/bb/BBMove.h"
#include "chess/bb/Bitboard.h"
#include "chess/bb/MoveGen.h"
#include "chess/bbSearch/Perft.h"
#include "chess/bbSearch/Search.h"
#include "chess/bbSearch/TT.h"

#include <cctype>
#include <chrono>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace chess::bb;

namespace {

constexpr const char* START_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// Map (color, pieceType) to a display glyph: uppercase = white, lowercase = black.
char pieceGlyph(Color c, PieceType t) {
    static constexpr char upper[NUM_PIECE_TYPES] = {'P', 'N', 'B', 'R', 'Q', 'K'};
    return (c == WHITE) ? upper[t] : static_cast<char>(std::tolower(upper[t]));
}

// Print the board grid, side to move, and en-passant target (or "--").
void printPosition(const BBoard& b) {
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << (rank + 1) << " |";
        for (int file = 0; file < 8; ++file) {
            const int sq = make_square(file, rank);
            const PieceType wt = b.pieceTypeOn(WHITE, sq);
            const PieceType bt = b.pieceTypeOn(BLACK, sq);
            char g = '.';
            if      (wt != NO_PIECE_TYPE) g = pieceGlyph(WHITE, wt);
            else if (bt != NO_PIECE_TYPE) g = pieceGlyph(BLACK, bt);
            std::cout << ' ' << g;
        }
        std::cout << '\n';
    }
    std::cout << "-------------------\n";
    std::cout << "    a b c d e f g h\n";
    std::cout << "  side: " << (b.sideToMove == WHITE ? "white" : "black") << '\n';
    std::cout << "  ep:   "
              << (b.epSquare == NO_SQUARE ? "--" : square_name(b.epSquare)) << '\n';
}

// List every legal move for the current position, numbered.
void printMoves(BBoard& b) {
    MoveList ml;
    generateLegalMoves(b, ml);
    const char* side = (b.sideToMove == WHITE) ? "White" : "Black";
    std::cout << "  " << side << " — " << ml.count << " legal move(s):\n";
    for (int i = 0; i < ml.count; ++i)
        std::cout << "  " << (i + 1) << ". " << to_uci(ml.moves[i]) << '\n';
}

void printHelp() {
    std::cout <<
        "  commands:\n"
        "    <move>         e.g. a2a3, e1g1 (castle), e7e8q (promotion: q/r/b/n)\n"
        "    moves | m      list all legal moves\n"
        "    search depth <n>   iterative deepening to depth n; shows eval and best move\n"
        "    search time <s>    search for s seconds; shows eval, best move, and depth\n"
        "    perft <n>      count leaf nodes to depth n\n"
        "    divide <n>     per-root-move leaf counts to depth n\n"
        "    undo  | u      take back the last move\n"
        "    help  | h      show this help\n"
        "    quit  | q      exit\n";
}

// Try to play `tok` as a UCI move. Generates the full legal move list and
// looks for an exact match. Reports an error and leaves the board unchanged
// if no match is found.
void tryPlay(BBoard& board, const std::string& tok,
             std::vector<std::pair<Move, Undo>>& history) {
    MoveList ml;
    generateLegalMoves(board, ml);

    Move found = MOVE_NONE;
    for (int i = 0; i < ml.count; ++i) {
        if (to_uci(ml.moves[i]) == tok) { found = ml.moves[i]; break; }
    }

    if (found == MOVE_NONE) {
        // If the 4-char prefix matches a promotion move, hint the piece suffix.
        if (tok.size() == 4) {
            bool promoAvail = false;
            for (int i = 0; i < ml.count; ++i) {
                const std::string uci = to_uci(ml.moves[i]);
                if (uci.size() == 5 && uci.substr(0, 4) == tok) {
                    promoAvail = true; break;
                }
            }
            if (promoAvail) {
                std::cout << "  promotion — specify piece: "
                          << tok << "q / " << tok << "r / "
                          << tok << "b / " << tok << "n\n";
                return;
            }
        }
        std::cout << "  illegal move: " << tok << '\n';
        return;
    }

    Undo u;
    board.makeMove(found, u);
    history.emplace_back(found, u);
    std::cout << "  played " << to_uci(found) << '\n';
    printPosition(board);
}

} // namespace

int main() {
    BBoard board;

    std::cout << "Enter a FEN, or press Enter for the starting position:\n> ";
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) { board.setFromFEN(START_FEN); break; }
        if (board.setFromFEN(line)) break;
        std::cout << "  ? malformed FEN; try again, or press Enter for start:\n> ";
    }

    chess::bbSearch::TT tt(64);
    std::vector<std::pair<Move, Undo>> history;

    std::cout << '\n';
    printPosition(board);
    printHelp();

    std::string cmd;
    std::cout << "\n> ";
    while (std::cin >> cmd) {
        if (cmd == "quit" || cmd == "q" || cmd == "exit") break;

        if (cmd == "help" || cmd == "h") {
            printHelp();
        } else if (cmd == "moves" || cmd == "m") {
            printMoves(board);
        } else if (cmd == "search") {
            std::string mode;
            if (!(std::cin >> mode) || (mode != "depth" && mode != "time")) {
                std::cout << "  usage: search depth <n> | search time <seconds>\n";
            } else if (mode == "depth") {
                int depth = 0;
                if (!(std::cin >> depth) || depth < 1) {
                    std::cout << "  usage: search depth <n>\n";
                } else {
                    chess::bbSearch::SearchLimits lim;
                    lim.depth = depth;
                    const auto t0 = std::chrono::steady_clock::now();
                    chess::bbSearch::searchID(board, tt, lim);
                    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - t0).count();
                    std::cout << "  Time: " << ms << " ms\n";
                }
            } else {
                // mode == "time"
                int seconds = 0;
                if (!(std::cin >> seconds) || seconds < 1) {
                    std::cout << "  usage: search time <seconds>\n";
                } else {
                    const auto timeLimit = std::chrono::seconds(seconds);
                    const auto start     = std::chrono::steady_clock::now();
                    int                          lastScore = 0;
                    chess::bb::Move              lastBest  = chess::bb::MOVE_NONE;
                    int                          lastDepth = 0;
                    chess::bbSearch::SearchState stateOut;

                    for (int d = 1; d < chess::bbSearch::MAX_PLY; ++d) {
                        lastScore = chess::bbSearch::searchEx(board, d, &tt, stateOut);

                        if (stateOut.pvLength[0] > 0)
                            lastBest = stateOut.pvTable[0][0];
                        lastDepth = d;

                        std::cout << "  Finished depth " << d << ". Eval: ";
                        if (chess::bbSearch::isMateScore(lastScore)) {
                            const int plies = chess::bbSearch::MATE_SCORE - std::abs(lastScore);
                            const int moves  = (plies + 1) / 2;
                            std::cout << (lastScore > 0 ? "mate in " : "mated in ") << moves;
                        } else {
                            std::cout << (lastScore > 0 ? "+" : "") << lastScore << " cp";
                        }
                        std::cout << ", BestMove: " << chess::bb::to_uci(lastBest) << '\n';

                        if (std::chrono::steady_clock::now() - start >= timeLimit) break;
                    }

                    std::cout << "  Reached depth " << lastDepth << " in "
                              << seconds << "s."
                              << " bestmove " << chess::bb::to_uci(lastBest) << '\n';
                }
            }
        } else if (cmd == "perft") {
            int depth = 0;
            if (!(std::cin >> depth) || depth < 1) {
                std::cout << "  usage: perft <depth>\n";
            } else {
                const auto t0 = std::chrono::steady_clock::now();
                const uint64_t nodes = chess::bbSearch::perft(board, depth);
                const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - t0).count();
                std::cout << "  perft(" << depth << ") = " << nodes
                          << "  (" << ms << " ms)\n";
            }
        } else if (cmd == "divide") {
            int depth = 0;
            if (!(std::cin >> depth) || depth < 1) {
                std::cout << "  usage: divide <depth>\n";
            } else {
                chess::bbSearch::perftDivide(board, depth);
            }
        } else if (cmd == "undo" || cmd == "u") {
            if (history.empty()) {
                std::cout << "  nothing to undo\n";
            } else {
                auto [m, u] = history.back();
                history.pop_back();
                board.unmakeMove(m, u);
                std::cout << "  undid " << to_uci(m) << '\n';
                printPosition(board);
            }
        } else {
            tryPlay(board, cmd, history);
        }
        std::cout << "> ";
    }

    std::cout << "bye\n";
    return 0;
}
