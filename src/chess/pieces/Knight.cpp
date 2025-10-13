#include "chess/pieces/Knight.h"


    Knight::Knight(int c, int startCol, int startRow) : Piece(c, startCol, startRow) {
        type = PieceType::KNIGHT;
    }


    int Knight::getValue() const {
        return 320;
    }

    std::string Knight::getRepresentation() const {
      if(color){
         return "N";
      }
      return "n";
   }


    bool Knight::operator==(const PieceType _type) const  {
        return _type == PieceType::KNIGHT;
    }

    Piece* Knight::clone() { return new Knight(*this); }

    int Knight::getMobilityScore(bool endGame) const {
      int lookUpRow = row;
      if(color != 0){lookUpRow = 7 - row;} 

      return endGame ? endgameMobilityTable[lookUpRow][col] : mobilityTable[lookUpRow][col];
   }

    const std::vector<std::pair<int, int>>& Knight::getDirections() const {return directions;}

    int Knight::getZobristPieceIndex() const {
      return color ? 1 : 7;
   }


 const int Knight::mobilityTable[8][8] = {
        { -50, -40, -30, -30, -30, -30, -40, -50 },
        { -40, -20,   0,   5,   5,   0, -20, -40 },
        { -30,   5,  10,  15,  15,  10,   5, -30 },
        { -30,   0,  15,  20,  20,  15,   0, -30 },
        { -30,   5,  15,  20,  20,  15,   5, -30 },
        { -30,   0,  10,  15,  15,  10,   0, -30 },
        { -40, -20,   0,   0,   0,   0, -20, -40 },
        { -50, -40, -30, -30, -30, -30, -40, -50 }
    };

 const int Knight::endgameMobilityTable[8][8] = {
        { -30, -20, -10, -10, -10, -10, -20, -30 },
        { -20,   0,   0,   0,   0,   0,   0, -20 },
        { -10,   0,  10,  15,  15,  10,   0, -10 },
        { -10,   5,  15,  20,  20,  15,   5, -10 },
        { -10,   5,  15,  20,  20,  15,   5, -10 },
        { -10,   0,  10,  15,  15,  10,   0, -10 },
        { -20,   0,   0,   0,   0,   0,   0, -20 },
        { -30, -20, -10, -10, -10, -10, -20, -30 }
    };

    const std::vector<std::pair<int, int>> Knight::directions = {{1, -2}, {2, -1}, {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}};
