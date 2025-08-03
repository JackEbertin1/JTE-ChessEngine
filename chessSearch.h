#include "Board.h"
#include "Evaluation.h"
#include <iostream>
#include <exception>
#include <utility>
#include <vector>
#include <future>
#include <numeric>  // for std::accumulate
#include <memory> 
#include <mutex>
#include <shared_mutex>

using namespace Evaluation;


namespace chessSearch {
	int nodesVisited = 0;
	int startingDepth = 0;
	int repeatedPositions = 0;

	struct TTEntry {
		float value;
		int depth;
		//Move bestMove;
		enum Flag { EXACT, LOWERBOUND, UPPERBOUND } type;
	};

	std::unordered_map<uint64_t, TTEntry> transpositionTable;
	std::mutex ttMutex;

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

	void sortMoves(MoveList& moves, Board* board){
		for(int i = 0; i < moves.count; i++){
			board->updateMoveScore(moves[i]);
		}
		// Sort moves
		std::sort(moves.moves, moves.moves + moves.count,[](const Move& a, const Move& b) {return a.moveScore > b.moveScore;});
	}

	void storeTTEntry(uint64_t zobristKey, float value, int depth, float alphaOrig, float beta) {
	    TTEntry entry;
	    entry.value = value;
	    entry.depth = depth;

	    if (value <= alphaOrig) {
	        entry.type = TTEntry::UPPERBOUND;
	    } else if (value >= beta) {
	        entry.type = TTEntry::LOWERBOUND;
	    } else {
	        entry.type = TTEntry::EXACT;
	    }

	    transpositionTable[zobristKey] = entry;
	}

	float minimax(Board* board, int depth, bool maximizingPlayer){
		if (depth == 0){return evaluate(board);} 	//Base Case, if at deepest depth, simply return evaluation of that position
		//if (board->gameIsOver()){return evaluate(board);}

		float bestScore = maximizingPlayer ? -1000000.0f : 1000000.0f;
		MoveList moves;
		board->generatePseudoLegalMoves(moves);

		if(maximizingPlayer){
			for(int i = 0; i < moves.count; i++){
				board->makeMove(moves[i]);
				if(!board->inCheck(1 - board->getTurn())){
					float score = minimax(board, depth - 1, !maximizingPlayer);
					if (score > bestScore){bestScore = score;}
				}
				board->undoMove();
			}
		}
		else{
			for(int i = 0; i < moves.count; i++){
				board->makeMove(moves[i]);
				if(!board->inCheck(1 - board->getTurn())){
					float score = minimax(board, depth - 1, !maximizingPlayer);
					if (score < bestScore){bestScore = score;}
				}
				board->undoMove();
			}
		}
		return bestScore;
	}

	float minimaxAB(Board* board, int depth, bool maximizingPlayer, float alpha, float beta){
		nodesVisited++;
		if (depth == 0){return evaluate(board);} 	//Base Case, if at deepest depth, simply return evaluation of that position
		float alphaOrig = alpha;

		uint64_t zobristKey = board->getHash();

		{
			std::lock_guard<std::mutex> lock(ttMutex);
			auto it = transpositionTable.find(zobristKey);
			if (it != transpositionTable.end() && it->second.depth >= depth) {
			    const TTEntry& entry = it->second;

			    if (entry.type == TTEntry::EXACT) {return entry.value;}
			    if (entry.type == TTEntry::LOWERBOUND && entry.value >= beta) {return entry.value;}
			    if (entry.type == TTEntry::UPPERBOUND && entry.value <= alpha) {return entry.value;}
			}
		}

		MoveList moves;
		board->generatePseudoLegalMoves(moves);
		// If no moves, it's either checkmate or stalemate
	    if (moves.count == 0) {
		    float eval = evaluate(board, true);
		    {
		    	std::lock_guard<std::mutex> lock(ttMutex);
		    	transpositionTable[zobristKey] = TTEntry{eval, depth, TTEntry::EXACT};
		    }
		    return eval;
		}


		if(depth + 2 >= startingDepth){sortMoves(moves, board);}
		float bestScore = maximizingPlayer ? -1000000.0f : 1000000.0f;
		if(maximizingPlayer){
			for(int i = 0; i < moves.count; i++){
				board->makeMove(moves[i]);
				if(!board->inCheck(1 - board->getTurn())){
					float score = minimaxAB(board, depth - 1, !maximizingPlayer, alpha, beta);
					if (score > bestScore){bestScore = score;}
					alpha = std::max(alpha, score);
				}
				board->undoMove();
				if (alpha >= beta){
					break;
				}
			}
		}
		else{
			for(int i = 0; i < moves.count; i++){
				board->makeMove(moves[i]);
				if(!board->inCheck(1 - board->getTurn())){
					float score = minimaxAB(board, depth - 1, !maximizingPlayer, alpha, beta);
					if (score < bestScore){bestScore = score;}
					beta = std::min(beta, score);
				}
				board->undoMove();
				if (alpha >= beta){
					break;
				}
			}
		}

		{
			std::lock_guard<std::mutex> lock(ttMutex);
			storeTTEntry(zobristKey, bestScore, depth, alphaOrig, beta);
		}
		return bestScore;
	}

	std::pair<std::string, float> searchBestMoveParallel(Board* board, int depth, bool maximizingPlayer) {
	    startingDepth = depth;
	    nodesVisited = 0;
	    repeatedPositions = 0;
	    MoveList moves;
	    board->generateLegalMoves(moves);

	    sortMoves(moves, board);

	    std::vector<std::future<std::pair<Move, float>>> futures;

	    for (int i = 0; i < moves.count; i++) {
	        Move move = moves[i];

	        // Spawn a thread to handle each root move
	        futures.push_back(std::async(std::launch::async, [board, move, depth, maximizingPlayer]() mutable {
	            Board* newBoard = board->clone();
	            newBoard->makeMove(move);

	            float score = -1000000.0f;
	            if (!newBoard->inCheck(1 - newBoard->getTurn())) {
	                score = minimaxAB(newBoard, depth - 1, !maximizingPlayer, -1000000.0f, 1000000.0f);
	            }

	            delete newBoard; // clean up cloned board
	            return std::make_pair(move, score);
	        }));
	    }

	    Move bestMove;
	    float bestScore = maximizingPlayer ? -1000000.0f : 1000000.0f;

	    for (auto& future : futures) {
	        auto [move, score] = future.get();

	        if ((maximizingPlayer && score > bestScore) || (!maximizingPlayer && score < bestScore)) {
	            bestScore = score;
	            bestMove = move;
	        }
	    }

	    return std::make_pair(bestMove.getMoveRepresentation(), bestScore);
	}

	std::pair<std::string, float> searchBestMove(Board* board, int depth, bool maximizingPlayer) {
		startingDepth = depth;
		nodesVisited = 0;
		repeatedPositions = 0;
	    MoveList moves;
	    board->generatePseudoLegalMoves(moves);
	    sortMoves(moves, board);

	    Move bestMove;
	    float bestScore = maximizingPlayer ? -1000000.0f : 1000000.0f;

	    for (int i = 0; i < moves.count; i++) {
	        board->makeMove(moves[i]);
	        if(!board->inCheck(1 - board->getTurn())){
	        	float score = minimaxAB(board, depth - 1, !maximizingPlayer, -1000000.0f, 1000000.0f);
	            
	            if ((maximizingPlayer && score > bestScore) ||
	                (!maximizingPlayer && score < bestScore)) {
	                bestScore = score;
	                bestMove = moves[i];
	            }
	        }

	        board->undoMove();
	    }

	    return std::make_pair(bestMove.getMoveRepresentation(), bestScore);
	}
}


