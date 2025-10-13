#ifndef KNIGHT_H
#define KNIGHT_H

#include "chess/pieces/Piece.h"


class Knight : public Piece {
public:
    // Constructor for Knight
    Knight(int c, int startCol, int startRow);

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
   static const std::vector<std::pair<int, int>> directions;



};

#endif // KNIGHT_H
