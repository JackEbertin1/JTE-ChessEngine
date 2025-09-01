#include "chessSearch.h"



namespace perft{

	long perft(Board* myBoard, int depth){
			long nodes = 0;

			if (depth == 0){
				return 1;
			}

			MoveList moves;
			myBoard->generatePseudoLegalMoves(moves);
			for(int i = 0; i < moves.count; i++){

				myBoard->makeMove(moves[i]);

				if (!myBoard->inCheck(1 - myBoard->getTurn())){
					nodes += perft(myBoard, depth - 1);
				}

				myBoard->undoMove();
			}
			return nodes;
	}

	int perftCount(Board* myBoard, int depth){
		long nodes = 0;

		MoveList moves;
		myBoard->generateLegalMoves(moves);
		for(int i = 0; i < moves.count; i++){
			myBoard->makeMove(moves[i]);

			int moveNodes = perft(myBoard, depth - 1);
			std::cout << "Move: " << moves[i].getMoveRepresentation() << ": " << moveNodes << std::endl;
			nodes += moveNodes;

			myBoard->undoMove();
		}
		return nodes;
	}

	uint64_t perftParallel(Board* board, int depth) {
	    if (depth == 0) {return 1;}

	    MoveList moves;
	    board->generatePseudoLegalMoves(moves);
	    std::vector<std::future<uint64_t>> futures;

	    for (int i = 0; i < moves.count; i++) {
	        // Clone the board for each thread
	        Board* clonedBoard = board->clone();  // Assuming this returns a deep-copied pointer
	        clonedBoard->makeMove(moves[i]);

	        if (clonedBoard->inCheck(1 - clonedBoard->getTurn())) {
	            delete clonedBoard;
	            continue; // Skip illegal move
	        }

	        // Launch async task
	        futures.push_back(std::async(std::launch::async, [clonedBoard, depth]() -> uint64_t {
	            uint64_t result = perft(clonedBoard, depth - 1);
	            delete clonedBoard;  // Free memory
	            return result;
	        }));
	    }

	    uint64_t total = 0;
	    for (auto& future : futures) {
	        total += future.get();
	    }
	    return total;
	}



}

