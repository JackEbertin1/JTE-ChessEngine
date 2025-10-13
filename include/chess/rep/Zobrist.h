#pragma once
#include <cstdint>

namespace Zobrist{
    // Global variable declarations
    extern uint64_t zobristTable[12][8][8]; // 6 piece types * 2 colors
    extern uint64_t zobristCastling[4];   // KQkq
    extern uint64_t zobristEnPassant[8];  // files a–h
    extern uint64_t zobristSideToMove;    // black to move

    // Function declaration
    void initializeZobrist();
}