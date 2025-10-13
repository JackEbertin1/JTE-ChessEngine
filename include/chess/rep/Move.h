#ifndef MOVE_H
#define MOVE_H

#include "chess/pieces/Piece.h"


class Move{

public:
// Constructors for Move 
	Move(int startR, int startC, int endR, int endC, PieceType cap);

	Move();

	Move(const Move& other);

	Move& operator=(const Move& other);
		
// Sets move as a castle move and updates move score
	void setCastle();


// Sets move as an en pessant move and updates move score	
	void setEnPessant();


// Sets move as a promotion move, newPiece is promotion type. Calls setPromotionZobrist to update Zobrist index for promotion piece	
	void setPromotion(PieceType newPiece);
	
// Sets the promotionZobrist index based on the promotion piece type and color
	void setPromotionZobrist();


// Sets move as a double pawn push move	
	void setDoublePawnPush();


//Sets the square where en pessant can be performed after this move
	void setEnPessantSquare(int row, int col);


// If this move is a capture, stores the number of times the captured piece has moved	
	void storeTimesMoved(int moveNums);


// Returns a string representation of the move in notation (Square to Square, e.g., e2e4), adds other details like promotion or capture for readability	
	std::string getMoveRepresentation();


// Overrides the == operator to compare if two moves are the same based on start and end locations, promotion type, and special move types
	bool operator==(const Move& other) const;


//String representation of the move	
	std::string moveRep;


// Board location of this move
	int startRow;
	int startCol;
	int endRow;
	int endCol;

	int epRow;
	int epCol;

// Piece Types involved in this move
	PieceType promotionType;
	int capturedPieceTimesMoved;

// Details about what type of move this move is
	bool isCheck;
	bool isCastle;
	bool isPromotion;
	bool isEnPessant;
	bool isCapture;
	bool isDoublePawnPush;

// Score for the move, used later for move ordering in Minimax search
	int moveScore;

// Zobrist Hash from the previous position, used in undoing move to efficiently grab hash for position
	uint64_t previousZobristHash;	
	int promotionZobrist;


	~Move();

private:
// Converts board coordinates to standard chess notation (e.g., (0,0) -> a8, (7,7) -> h1)
	std::string squareToCoord(int row, int col);
};

struct MoveList {
    Move moves[256];
    int count = 0;

    void add(const Move& m) {
        moves[count++] = m;
    }

    Move& operator[](int index) {
        return moves[index];
    }
};

#endif