#ifndef ROOK_H
#define ROOK_H


class Rook : public Piece {
public:
   Rook(int c, int startCol, int startRow) : Piece(c, startCol, startRow) {
      name = "Rook";
      type = PieceType::ROOK;
   }

   int getValue() override { return 500; }

   std::string getRepresentation() override {
      if(color){
         return "R";
      }
      return "r";
   }

   bool operator==(const std::string& name) const override {
        return name == "Rook";
    }

   Piece* clone() const override { return new Rook(*this); }

   int getMobilityScore(bool endGame) const override {
      int lookUpRow = row;
      if(color != 0){lookUpRow = 7 - row;} 

      return endGame ? endgameMobilityTable[lookUpRow][col] : mobilityTable[lookUpRow][col];
   }

   static const std::vector<std::pair<int, int>> directions;
   const std::vector<std::pair<int, int>>& getDirections() const override {return directions;}

   int getZobristPieceIndex() const override{
      return color ? 3 : 9;
   }

private:

   static const int endgameMobilityTable[8][8];
   static const int mobilityTable[8][8];


};

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

#endif