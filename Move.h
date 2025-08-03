#ifndef MOVE_H
#define MOVE_H

#include "Piece.h"
#include "Queen.h"
#include "Rook.h"
#include "Bishop.h"
#include "Knight.h"

class Move{

public:
	Move(int startR, int startC, int endR, int endC, std::string cap) : startRow(startR), startCol(startC), endRow(endR), endCol(endC) {
		capturedType = cap;
		isCheck = false;;
	 	isCastle =  false;
	 	isPromotion = false;
		isEnPessant = false;
		isDoublePawnPush = false;
		epRow = -1;
		epCol = -1;
		if (
			cap.length()){
			isCapture = true;
			capturedPieceTimesMoved = 0;
		}
		else{
			isCapture = false;
			capturedPieceTimesMoved = -1;
		}
	}

	void setCastle(){
		isCastle = true;
		moveScore += 15;
	}

	void setEnPessant(){
		isEnPessant = true;
		moveScore += 99;
	}

	void setEnPessantSquare(int row, int col){
		epRow = row;
		epCol = col;
	}

	void setPromotion(PieceType newPiece){
		isPromotion = true;
		promotionType = newPiece;
		moveScore += 850;
		setPromotionZobrist();
	}

	void setPromotionZobrist(){
		if (promotionType == PieceType::QUEEN){
			promotionZobrist = (endRow == 7 ? 4 : 10);
		}
		else if(promotionType == PieceType::ROOK){
			promotionZobrist = (endRow == 7 ? 3 : 9);
		}
		else if(promotionType == PieceType::KNIGHT){
			promotionZobrist = (endRow == 7 ? 1 : 7);
		}
		else if (promotionType == PieceType::BISHOP){
			promotionZobrist = (endRow == 7 ? 2 : 8);
		}
	}

	void storeTimesMoved(int moveNums){
		capturedPieceTimesMoved = moveNums;
	}

	void setDoublePawnPush(){
		isDoublePawnPush = true;
	}



	std::string getMoveRepresentation(){
		std::string rep = squareToCoord(startRow, startCol) + squareToCoord(endRow, endCol);

			if(isCapture){
				rep += "x";
			}

	    if (isPromotion) {
	        // Assume promotion is always to Queen for now. You can generalize it later.
	        if (promotionType == PieceType::QUEEN){
	        	rep += "Queen";
	        }
	        else if (promotionType == PieceType::ROOK){
	        	rep += "Rook";
	        }
	        else if (promotionType == PieceType::KNIGHT){
	        	rep += "Knight";
	        }
	        else if (promotionType == PieceType::BISHOP){
	        	rep += "Bishop";
	        }
	    }

	    if(isCastle){
	    	rep += "castle";
	    }

	    if(isEnPessant){
	    	rep += "EP";
	    }

	    moveRep = rep;

	    return rep;
	}


	std::string moveRep;


// Board location of this move
	int startRow;
	int startCol;
	int endRow;
	int endCol;

	int epRow;
	int epCol;

// Piece Types involved in this move
	std::string capturedType;
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



	Move() 
    : startRow(0), startCol(0), endRow(0), endCol(0),
      isCheck(false), isCastle(false),
      isPromotion(false), isEnPessant(false), moveRep(""), epRow(-1), epCol(-1) {}

	Move(const Move& other): startRow(other.startRow), startCol(other.startCol), endRow(other.endRow), endCol(other.endCol),
      isCheck(other.isCheck), isCastle(other.isCastle), isPromotion(other.isPromotion),isEnPessant(other.isEnPessant), 
      moveRep(other.moveRep), epRow(other.epRow), epCol(other.epCol), isCapture(other.isCapture), promotionType(other.promotionType), capturedType(other.capturedType),
      moveScore(other.moveScore), capturedPieceTimesMoved(other.capturedPieceTimesMoved), isDoublePawnPush(other.isDoublePawnPush), promotionZobrist(other.promotionZobrist), 
      previousZobristHash(other.previousZobristHash) {
      }

    Move& operator=(const Move& other) {
	    if (this != &other) {
	        startRow = other.startRow;
	        startCol = other.startCol;
	        endRow = other.endRow;
	        endCol = other.endCol;
	        isCheck = other.isCheck;
	        isCastle = other.isCastle;
	        isPromotion = other.isPromotion;
	        isEnPessant = other.isEnPessant;
	        isCapture = other.isCapture;
	        moveRep = other.moveRep;
	        epRow = other.epRow;
	        epCol = other.epCol;
	        promotionType = other.promotionType;
	        capturedType = other.capturedType; 
	        moveScore = other.moveScore;
	        capturedPieceTimesMoved = other.capturedPieceTimesMoved;
	        isDoublePawnPush = other.isDoublePawnPush;
	        promotionZobrist = other.promotionZobrist;
	        previousZobristHash = other.previousZobristHash;
	    }
	    return *this;
	}  

	~Move() {
	}

private:

	std::string squareToCoord(int row, int col) {
	    char file = 'a' + col;
	    char rank = '1' + row;
	    return std::string{file, rank};
	}


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