#include "chess/pieces/Piece.h"


    // White will always have color = 1, black will have color = 0    
        Piece::Piece(int c, int startRow, int startCol) : color(c), row(startRow), col(startCol), timesMoved(0) {};           //Constructor for Piece 

        std::string Piece::getColor() const{
            if (color){
                return "White";
            }
            return "Black";
        }

        PieceType Piece::getType() const{
            return type;
        }

        void Piece::updateLocation(int r, int c){
            row = r;
            col = c;
        }

        void Piece::updateTimesMoved(int moveNums){
            timesMoved = moveNums;
        }

        Piece::~Piece() {}


