#include "chess/pieces/Queen.h"


    Queen::Queen(int c, int startCol, int startRow) : Piece(c, startCol, startRow) {
      type = PieceType::QUEEN;
   }

   int Queen::getValue() const { return 900; }

   std::string Queen::getRepresentation() const {
      if(color){
         return "Q";
      }
      return "q";
   }

   bool Queen::operator==(const PieceType _type) const  {
        return _type == PieceType::QUEEN;
    }

    Piece* Queen::clone() { return new Queen(*this); }

    int Queen::getMobilityScore(bool endGame) const  {
      int lookUpRow = row;
      if(color != 0){lookUpRow = 7 - row;} 

      return endGame ? endgameMobilityTable[lookUpRow][col] : mobilityTable[lookUpRow][col];
   }

   const std::vector<std::pair<int, int>>& Queen::getDirections() const {return directions;}

   int Queen::getZobristPieceIndex() const{
      return color ? 4 : 10;
   }

const int Queen::mobilityTable[8][8] = {
       { -20, -10, -10,  -5,  -5, -10, -10, -20 },
       { -10,   0,   0,   0,   0,   0,   0, -10 },
       { -10,   0,   5,   5,   5,   5,   0, -10 },
       {  -5,   0,   5,   5,   5,   5,   0,  -5 },
       {   0,   0,   5,   5,   5,   5,   0,  -5 },
       { -10,   5,   5,   5,   5,   5,   0, -10 },
       { -10,   0,   0,   0,   0,   0,   0, -10 },
       { -20, -10, -10,  -5,  -5, -10, -10, -20 }
   };

const int Queen::endgameMobilityTable[8][8] = {
       { -10,  -5,  -5,  -2,  -2,  -5,  -5, -10 },
       {  -5,   0,   0,   0,   0,   0,   0,  -5 },
       {  -5,   0,   3,   3,   3,   3,   0,  -5 },
       {  -2,   0,   3,   5,   5,   3,   0,  -2 },
       {  -2,   0,   3,   5,   5,   3,   0,  -2 },
       {  -5,   0,   3,   3,   3,   3,   0,  -5 },
       {  -5,   0,   0,   0,   0,   0,   0,  -5 },
       { -10,  -5,  -5,  -2,  -2,  -5,  -5, -10 }
   };

   const std::vector<std::pair<int, int>> Queen::directions = {{1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};
