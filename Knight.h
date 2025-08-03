#ifndef KNIGHT_H
#define KNIGHT_H

#include <iostream>


class Knight : public Piece {
public:
    // Constructor for Knight
    Knight(int c, int startCol, int startRow) : Piece(c, startCol, startRow) {
        name = "Knight";
        type = PieceType::KNIGHT;
    }

    // Override getValue to return the value of the Knight (3)
    int getValue() override {
        return 320;
    }

    std::string getRepresentation() override {
      if(color){
         return "N";
      }
      return "n";
   }

    // Override == to compare with "bishop"
    bool operator==(const std::string& name) const override {
        return name == "Knight";
    }

    Piece* clone() const override { return new Knight(*this); }

    int getMobilityScore(bool endGame) const override {
      int lookUpRow = row;
      if(color != 0){lookUpRow = 7 - row;} 

      return endGame ? endgameMobilityTable[lookUpRow][col] : mobilityTable[lookUpRow][col];
   }

    static const std::vector<std::pair<int, int>> directions;
    const std::vector<std::pair<int, int>>& getDirections() const override {return directions;}

    int getZobristPieceIndex() const override{
      return color ? 1 : 7;
   }

private:

    static const int endgameMobilityTable[8][8];
    static const int mobilityTable[8][8];

};

 const int Knight::mobilityTable[8][8] = {
        { -50, -40, -30, -30, -30, -30, -40, -50 },
        { -40, -20,   0,   5,   5,   0, -20, -40 },
        { -30,   5,  10,  15,  15,  10,   5, -30 },
        { -30,   0,  15,  20,  20,  15,   0, -30 },
        { -30,   5,  15,  20,  20,  15,   5, -30 },
        { -30,   0,  10,  15,  15,  10,   0, -30 },
        { -40, -20,   0,   0,   0,   0, -20, -40 },
        { -50, -40, -30, -30, -30, -30, -40, -50 }
    };

 const int Knight::endgameMobilityTable[8][8] = {
        { -30, -20, -10, -10, -10, -10, -20, -30 },
        { -20,   0,   0,   0,   0,   0,   0, -20 },
        { -10,   0,  10,  15,  15,  10,   0, -10 },
        { -10,   5,  15,  20,  20,  15,   5, -10 },
        { -10,   5,  15,  20,  20,  15,   5, -10 },
        { -10,   0,  10,  15,  15,  10,   0, -10 },
        { -20,   0,   0,   0,   0,   0,   0, -20 },
        { -30, -20, -10, -10, -10, -10, -20, -30 }
    };

    const std::vector<std::pair<int, int>> Knight::directions = {{1, -2}, {2, -1}, {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}};

#endif // KNIGHT_H
