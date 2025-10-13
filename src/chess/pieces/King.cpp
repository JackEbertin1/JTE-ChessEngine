#include "chess/pieces/King.h"


   King::King(int c, int startCol, int startRow) : Piece(c, startCol, startRow) {
      type = PieceType::KING;
   }

   int King::getValue() const { return 20000; }

   std::string King::getRepresentation() const {
      if(color){
         return "K";
      }
      return "k";
   }

   bool King::operator==(const PieceType _type) const {
        return _type == PieceType::KING;
    }

   Piece* King::clone(){ return new King(*this); }

   int King::getMobilityScore(bool endGame) const {
      int lookUpRow = row;
      if(color != 0){lookUpRow = 7 - row;} 

      return endGame ? endgameMobilityTable[lookUpRow][col] : mobilityTable[lookUpRow][col];
   }

   const std::vector<std::pair<int, int>>& King::getDirections() const {return directions;}

   int King::getZobristPieceIndex() const{
      return color ? 5 : 11;
   }

    const int King::mobilityTable[8][8] = {
       { -30, -40, -40, -50, -50, -40, -40, -30 },
       { -30, -40, -40, -50, -50, -40, -40, -30 },
       { -30, -40, -40, -50, -50, -40, -40, -30 },
       { -30, -40, -40, -50, -50, -40, -40, -30 },
       { -20, -30, -30, -40, -40, -30, -30, -20 },
       { -10, -20, -20, -20, -20, -20, -20, -10 },
       {  20,  20,   0,   0,   0,   0,  20,  20 },
       {  20,  30,  10,   0,   0,  10,  30,  20 }
   };


 const int King::endgameMobilityTable[8][8] = {
       { -50, -40, -30, -20, -20, -30, -40, -50 },
       { -30, -20, -10,   0,   0, -10, -20, -30 },
       { -30, -10,  20,  30,  30,  20, -10, -30 },
       { -30, -10,  30,  40,  40,  30, -10, -30 },
       { -30, -10,  30,  40,  40,  30, -10, -30 },
       { -30, -10,  20,  30,  30,  20, -10, -30 },
       { -30, -30,   0,   0,   0,   0, -30, -30 },
       { -50, -40, -30, -20, -20, -30, -40, -50 }
   };

   const std::vector<std::pair<int, int>> King::directions = {{1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};

