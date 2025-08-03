#include "Board.h"


namespace Evaluation{

	bool isEndgame = false;

	float sumMaterial(Board* b){
		int whiteTotal = 0;
		int blackTotal = 0;

		for (int i = 0; i < 8; i++){
			for (int j = 0; j < 8; j++){
				if(b->board[i][j]){
					if (b->board[i][j]->color > 0){whiteTotal += b->board[i][j]->getValue() + b->board[i][j]->getMobilityScore(isEndgame);}
					else {blackTotal += b->board[i][j]->getValue() + b->board[i][j]->getMobilityScore(isEndgame);}
				}	
			}
		}
		//std::cout << "whiteTotal: " << whiteTotal << " blackTotal: " << blackTotal << std::endl;
		return static_cast<float>(whiteTotal - blackTotal) / 100;
	}

		float evaluate(Board* board, bool isEndState = false){
			if(isEndState){
				if (board->inCheckMate(board->getTurn())){
					float winValue = board->getTurn() == 1 ? 1000000.0f : -1000000.0f;
					return winValue;
				}
				if (board->inStaleMate(board->getTurn())){return 0;}
			}
			
			return sumMaterial(board);
		}

	void isEndGame(Board* board){
		int blackMaterial = 0;
		int whiteMaterial = 0;
		for(auto piece : board->whiteAlive){
			if (piece->type == PieceType::KING){continue;}
			whiteMaterial += piece->getValue();
		}
		for(auto piece : board->blackAlive){
			if (piece->type == PieceType::KING){continue;}
			blackMaterial += piece->getValue();
		}
		isEndgame = (whiteMaterial < 1400 && blackMaterial < 1400);
	}	

}
