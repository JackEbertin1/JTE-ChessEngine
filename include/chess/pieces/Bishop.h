#ifndef BISHOP_H
#define BISHOP_H



class Bishop : public Piece {

public:
   Bishop(int c, int startCol, int startRow) : Piece(c, startCol, startRow) {
      name = "Bishop";
      type = PieceType::BISHOP;
   }

   int getValue() override { return 330; }

   std::string getRepresentation() override {
      if(color){
         return "B";
      }
      return "b";
   }

   bool operator==(const std::string& name) const override {
        return name == "Bishop";
    }

   int getMobilityScore(bool endGame) const override {
      int lookUpRow = row;
      if(color != 0){lookUpRow = 7 - row;} 

      return endGame ? endgameMobilityTable[lookUpRow][col] : mobilityTable[lookUpRow][col];
   }

   Piece* clone() const override { return new Bishop(*this); }

   static const std::vector<std::pair<int, int>> directions;
   const std::vector<std::pair<int, int>>& getDirections() const override {return directions;}

   int getZobristPieceIndex() const override{
      return color ? 2 : 8;
   }

private:

   static const int endgameMobilityTable[8][8];
   static const int mobilityTable[8][8];


};

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

#endif