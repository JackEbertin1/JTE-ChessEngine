#include "chess/pieces/Rook.h"

   Rook::Rook(int c, int startCol, int startRow) : Piece(c, startCol, startRow) {
      type = PieceType::ROOK;
   }

   int Rook::getValue() const { return 500; }

   std::string Rook::getRepresentation() const {
      if(color){
         return "R";
      }
      return "r";
   }

   bool Rook::operator==(const PieceType _type) const  {
        return _type == PieceType::ROOK;
    }

   Piece* Rook::clone() { return new Rook(*this); }

   int Rook::getMobilityScore(bool endGame) const  {
      int lookUpRow = row;
      if(color != 0){lookUpRow = 7 - row;} 

      return endGame ? endgameMobilityTable[lookUpRow][col] : mobilityTable[lookUpRow][col];
   }

   const std::vector<std::pair<int, int>>& Rook::getDirections() const  {return directions;}

   int Rook::getZobristPieceIndex() const {
      return color ? 3 : 9;
   }


 const int Rook::mobilityTable[8][8] = {
       {  0,   0,   0,   5,   5,   0,   0,   0 },
       { -5,   0,   0,   0,   0,   0,   0,  -5 },
       { -5,   0,   0,   0,   0,   0,   0,  -5 },
       { -5,   0,   0,   0,   0,   0,   0,  -5 },
       { -5,   0,   0,   0,   0,   0,   0,  -5 },
       { -5,   0,   0,   0,   0,   0,   0,  -5 },
       {  5,  10,  10,  10,  10,  10,  10,   5 },
       {  0,   0,   0,   0,   0,   0,   0,   0 }
   };

   const int Rook::endgameMobilityTable[8][8] = {
       {  0,   0,   5,  10,  10,   5,   0,   0 },
       { -5,   0,   0,   5,   5,   0,   0,  -5 },
       { -5,   0,   0,   0,   0,   0,   0,  -5 },
       { -5,   0,   0,   0,   0,   0,   0,  -5 },
       { -5,   0,   0,   0,   0,   0,   0,  -5 },
       { -5,   0,   0,   0,   0,   0,   0,  -5 },
       {  5,  10,  10,  10,  10,  10,  10,   5 },
       {  0,   0,   5,  10,  10,   5,   0,   0 }
   };

   const std::vector<std::pair<int, int>> Rook::directions = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
