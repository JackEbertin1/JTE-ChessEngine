#include "chess/rep/Move.h"


Move::Move(int startR, int startC, int endR, int endC, PieceType cap) : startRow(startR), startCol(startC), endRow(endR), endCol(endC) {
		isCheck = false;;
	 	isCastle =  false;
	 	isPromotion = false;
		isEnPessant = false;
		isDoublePawnPush = false;
		epRow = -1;
		epCol = -1;
        moveScore = 0;
		if (cap != PieceType::NONE){
			isCapture = true;
			capturedPieceTimesMoved = 0;
		}
		else{
			isCapture = false;
			capturedPieceTimesMoved = -1;
		}
	}

Move::Move() 
    : moveRep(""), startRow(0), startCol(0), endRow(0), endCol(0), epRow(-1), epCol(-1),
      isCheck(false), isCastle(false),
      isPromotion(false), isEnPessant(false) {}

Move::Move(const Move& other): moveRep(other.moveRep), 
	  startRow(other.startRow), startCol(other.startCol), endRow(other.endRow), endCol(other.endCol), epRow(other.epRow), epCol(other.epCol),
      promotionType(other.promotionType), capturedPieceTimesMoved(other.capturedPieceTimesMoved),
      isCheck(other.isCheck), isCastle(other.isCastle), isPromotion(other.isPromotion),isEnPessant(other.isEnPessant), isCapture(other.isCapture), isDoublePawnPush(other.isDoublePawnPush),
      moveScore(other.moveScore), previousZobristHash(other.previousZobristHash), promotionZobrist(other.promotionZobrist) {}

Move& Move::operator=(const Move& other) {
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
	        moveScore = other.moveScore;
	        capturedPieceTimesMoved = other.capturedPieceTimesMoved;
	        isDoublePawnPush = other.isDoublePawnPush;
	        promotionZobrist = other.promotionZobrist;
	        previousZobristHash = other.previousZobristHash;
	    }
	    return *this;
	} 

void Move::setCastle(){
		isCastle = true;
		moveScore += 15;
	}

void Move::setEnPessant(){
		isEnPessant = true;
		moveScore += 99;
	}

void Move::setPromotion(PieceType newPiece){
		isPromotion = true;
		promotionType = newPiece;
		moveScore += 850;
		setPromotionZobrist();
	}

void Move::setPromotionZobrist(){
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

void Move::setDoublePawnPush(){
		isDoublePawnPush = true;
	}

void Move::setEnPessantSquare(int row, int col){
		epRow = row;
		epCol = col;
	}

void Move::storeTimesMoved(int moveNums){
		capturedPieceTimesMoved = moveNums;
	}

std::string Move::getMoveRepresentation(){
		std::string rep = squareToCoord(startRow, startCol) + squareToCoord(endRow, endCol);
		if(isCapture){rep += "x";}

	    if (isPromotion) {
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

	    if(isCastle){rep += "castle";}

	    if(isEnPessant){rep += "EP";}

	    moveRep = rep;
	    return rep;
	}

bool Move::operator==(const Move& other) const {
	    return startRow == other.startRow && startCol == other.startCol && endRow == other.endRow && endCol == other.endCol && promotionType == other.promotionType;
	}

std::string Move::squareToCoord(int row, int col) {
	    char file = 'a' + col;
	    char rank = '1' + row;
	    return std::string{file, rank};
	}

Move::~Move() {}

