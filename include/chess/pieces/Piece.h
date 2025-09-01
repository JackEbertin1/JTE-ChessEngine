#ifndef PIECE_H
#define PIECE_H

#include <string>
#include <vector>
#include <utility>
//#include "Deque.h"

enum class PieceType{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};



class Piece {

    public:
        int color;
        int row;
        int col;
        int timesMoved;
        PieceType type;

    // White will always have color = 1, black will have color = 0    
        Piece(int c, int startRow, int startCol) : color(c), row(startRow), col(startCol), timesMoved(0) {};           //Constructor for Piece 

        std::string getColor() const{
            if (color){
                return "White";
            }
            return "Black";
        }

        std::string getName() const{
            return name;
        }

        void updateLocation(int r, int c){
            row = r;
            col = c;
        }

        void updateTimesMoved(int moveNums){
            timesMoved = moveNums;
        }


        virtual int getZobristPieceIndex() const = 0;
        virtual const std::vector<std::pair<int, int>>& getDirections() const = 0;
        virtual int getValue() = 0;                                                                                      //Virtual Function for getValue()
        virtual bool operator==(const std::string& name) const = 0;                                                      //Virtual Function for == override
        virtual std::string getRepresentation() = 0;

        virtual int getMobilityScore(bool endGame) const = 0;

        virtual Piece* clone() const = 0;                                                                               //Virtual Function for cloning pieces which allows deep copy of board

        virtual ~Piece(){
        }


    protected:
        std::string name;
};
#endif