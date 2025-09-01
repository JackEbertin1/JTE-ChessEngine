#ifndef QUEEN_H
#define QUEEN_H



class Queen : public Piece {

public:
   Queen(int c, int startCol, int startRow) : Piece(c, startCol, startRow) {
      name = "Queen";
      type = PieceType::QUEEN;
   }

   int getValue() override { return 900; }

   std::string getRepresentation() override {
      if(color){
         return "Q";
      }
      return "q";
   }

   bool operator==(const std::string& name) const override {
        return name == "Queen";
    }

    Piece* clone() const override { return new Queen(*this); }

    int getMobilityScore(bool endGame) const override {
      int lookUpRow = row;
      if(color != 0){lookUpRow = 7 - row;} 

      return endGame ? endgameMobilityTable[lookUpRow][col] : mobilityTable[lookUpRow][col];
   }

   static const std::vector<std::pair<int, int>> directions;
   const std::vector<std::pair<int, int>>& getDirections() const override {return directions;}

   int getZobristPieceIndex() const override{
      return color ? 4 : 10;
   }

private:

   static const int endgameMobilityTable[8][8];
   static const int mobilityTable[8][8];


};

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

#endif