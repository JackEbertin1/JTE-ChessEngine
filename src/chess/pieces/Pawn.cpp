#include "chess/pieces/Pawn.h"




   Pawn::Pawn(int c, int startCol, int startRow) : Piece(c, startCol, startRow) {
    type = PieceType::PAWN;
   }

   int Pawn::getValue() const { return 100; }

   std::string Pawn::getRepresentation() const {
      if(color){
         return "P";
      }
      return "p";
   }

   bool Pawn::operator==(const PieceType _type) const  {
        return _type == PieceType::PAWN;
    }

   Piece* Pawn::clone() { return new Pawn(*this); }

   int Pawn::getMobilityScore(bool endGame) const {
      int lookUpRow = row;
      if(color != 0){lookUpRow = 7 - row;} 

      return endGame ? endgameMobilityTable[lookUpRow][col] : mobilityTable[lookUpRow][col];
   }

   const std::vector<std::pair<int, int>>& Pawn::getDirections() const {
      static const std::vector<std::pair<int, int>> emptyDirections;
      return emptyDirections;
   }

   int Pawn::getZobristPieceIndex() const {
      return color ? 0 : 6;
   }

   const int Pawn::mobilityTable[8][8] = {
       {  0,   0,   0,   0,   0,   0,   0,   0 },
       { 50,  50,  50,  50,  50,  50,  50,  50 },
       { 10,  10,  20,  30,  30,  20,  10,  10 },
       {  5,   5,  10,  25,  25,  10,   5,   5 },
       {  0,   0,   0,  20,  20,   0,   0,   0 },
       {  5,  -5, -10,   0,   0, -10,  -5,   5 },
       {  5,  10,  10, -20, -20,  10,  10,   5 },
       {  0,   0,   0,   0,   0,   0,   0,   0 }
   };

 const int Pawn::endgameMobilityTable[8][8] = {
       {  0,   0,   0,   0,   0,   0,   0,   0 },
       { 50,  50,  50,  50,  50,  50,  50,  50 },
       { 10,  10,  15,  20,  20,  15,  10,  10 },
       {  5,   5,  10,  25,  25,  10,   5,   5 },
       {  0,   0,  10,  30,  30,  10,   0,   0 },
       {  5,  -5, -10,   0,   0, -10,  -5,   5 },
       {  5,  10,  10, -20, -20,  10,  10,   5 },
       {  0,   0,   0,   0,   0,   0,   0,   0 }
   };


