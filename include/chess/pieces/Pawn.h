#ifndef PAWN_H
#define PAWN_H

#include "chess/pieces/Piece.h"

class Pawn : public Piece {

public:
   Pawn(int c, int startCol, int startRow);

   int getValue() const override;

   std::string getRepresentation() const override;

   bool operator==(const PieceType _type) const override;

   int getMobilityScore(bool endGame)const  override;
  
   Piece* clone() override;

   const std::vector<std::pair<int, int>>& getDirections() const override;

   int getZobristPieceIndex() const override;

private:

   static const int endgameMobilityTable[8][8];
   static const int mobilityTable[8][8];

};

#endif