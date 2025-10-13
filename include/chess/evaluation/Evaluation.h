#include "chess/rep/Board.h"


namespace Evaluation{
    // Global variable declaration
    extern bool isEndgame;

    // Function declarations
    float sumMaterial(Board* b);
    float evaluate(Board* board, bool isEndState = false);
    void isEndGame(Board* board);
}
