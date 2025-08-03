#ifndef PAWN_H
#define PAWN_H



class Pawn : public Piece {

public:
   Pawn(int c, int startCol, int startRow) : Piece(c, startCol, startRow) {
    name = "Pawn";
    type = PieceType::PAWN;
   }

   int getValue() override { return 100; }

   std::string getRepresentation() override {
      if(color){
         return "P";
      }
      return "p";
   }

   bool operator==(const std::string& name) const override {
        return name == "Pawn";
    }

   Piece* clone() const override { return new Pawn(*this); }

   int getMobilityScore(bool endGame) const override {
      int lookUpRow = row;
      if(color != 0){lookUpRow = 7 - row;} 

      return endGame ? endgameMobilityTable[lookUpRow][col] : mobilityTable[lookUpRow][col];
   }

   const std::vector<std::pair<int, int>>& getDirections() const override {
      static const std::vector<std::pair<int, int>> emptyDirections;
      return emptyDirections;
   }

   int getZobristPieceIndex() const override{
      return color ? 0 : 6;
   }


private:

   static const int endgameMobilityTable[8][8];
   static const int mobilityTable[8][8];


};

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

#endif