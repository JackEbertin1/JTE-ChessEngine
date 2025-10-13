#include "chess/rep/Board.h"



namespace perft{
    // Function declarations
    long perft(Board* myBoard, int depth);
    int perftCount(Board* myBoard, int depth);
    uint64_t perftParallel(Board* board, int depth);
}

