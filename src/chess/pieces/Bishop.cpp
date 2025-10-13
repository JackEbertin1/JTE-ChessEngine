#include "chess/pieces/Bishop.h"

   Bishop::Bishop(int c, int startCol, int startRow) : Piece(c, startCol, startRow) {
      type = PieceType::BISHOP;
   }

   int Bishop::getValue() const { return 330; }

   std::string Bishop::getRepresentation() const {
      if(color){
         return "B"; //White Bishop
      }
      return "b"; //Black Bishop
   }

   bool Bishop::operator==(const PieceType _type) const {
        return PieceType::BISHOP == _type;
    }

   int Bishop::getMobilityScore(bool endGame) const{
      int lookUpRow = row;
      if(color != 0){lookUpRow = 7 - row;} 

      return endGame ? endgameMobilityTable[lookUpRow][col] : mobilityTable[lookUpRow][col];
   }

   Piece* Bishop::clone() { return new Bishop(*this); }


   const std::vector<std::pair<int, int>>& Bishop::getDirections() const {return directions;}

   int Bishop::getZobristPieceIndex() const {
      return color ? 2 : 8;
   }

   // Sample bishop mobility/positional table (can be tuned later)
   const int Bishop::mobilityTable[8][8] = {
      { -20, -10, -10, -10, -10, -10, -10, -20 },
      { -10,   0,   0,   0,   0,   0,   0, -10 },
      { -10,   0,   5,   5,   5,   5,   0, -10 },
      { -10,   5,   5,  10,  10,   5,   5, -10 },
      { -10,   0,   5,  10,  10,   5,   0, -10 },
      { -10,   5,   5,   5,   5,   5,   5, -10 },
      { -10,   0,   0,   0,   0,   0,   0, -10 },
      { -20, -10, -10, -10, -10, -10, -10, -20 }
   };

   const int Bishop::endgameMobilityTable[8][8] = {
       { -10,  -5,  -5,  -5,  -5,  -5,  -5, -10 },
       {  -5,   0,   0,   0,   0,   0,   0,  -5 },
       {  -5,   0,   5,   5,   5,   5,   0,  -5 },
       {  -5,   0,   5,   8,   8,   5,   0,  -5 },
       {  -5,   0,   5,   8,   8,   5,   0,  -5 },
       {  -5,   0,   5,   5,   5,   5,   0,  -5 },
       {  -5,   0,   0,   0,   0,   0,   0,  -5 },
       { -10,  -5,  -5,  -5,  -5,  -5,  -5, -10 }
   };

   const std::vector<std::pair<int, int>> Bishop::directions = {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}};




