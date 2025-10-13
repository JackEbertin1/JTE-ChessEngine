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
    KING,
    NONE
};



class Piece {

    public:
        int color;
        int row;
        int col;
        int timesMoved;
        PieceType type;

// White will always have color = 1, black will have color = 0    
        Piece(int c, int startRow, int startCol);


// Returns the color of the Piece as a string: (1 is White), (0 is Black)
        std::string getColor() const;


// Updates the location of the Piece on the board
        void updateLocation(int r, int c);


// Updates the number of times the Piece has moved
        void updateTimesMoved(int moveNums);


// Returns the index used for Zobrist Hashing based on the Piece type and color
        virtual int getZobristPieceIndex() const = 0;


// Returns the possible movement directions for the Piece   
        virtual const std::vector<std::pair<int, int>>& getDirections() const = 0;


// Returns the value of the Piece used for Evaluation
        virtual int getValue() const = 0; 


// Returns the string representation of the Piece for printing the board in terminal                                                                   //Virtual Function for == override
        virtual std::string getRepresentation() const = 0;


// Overrides the == operator to compare if two pieces are of the same type
        virtual bool operator==(const PieceType _type) const = 0;  


// Returns the mobility score of the piece based on its position and whether the game has reached the endgame
        virtual int getMobilityScore(bool endGame) const = 0;
        
//Returns the type of the piece
        PieceType getType() const;

// Clones the piece, creating a new instance with the same attributes 
        virtual Piece* clone() = 0;                                                                                     //Virtual Function for cloning pieces which allows deep copy of board

// Destructor for Piece        
        virtual ~Piece() = 0;

};
#endif