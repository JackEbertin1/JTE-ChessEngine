#include "chess/search/chessSearch.h"

namespace chessSearch {
    // Global variable definitions
    int nodesVisited = 0;
    int startingDepth = 0;
    int repeatedPositions = 0;
    std::unordered_map<uint64_t, TTEntry> transpositionTable;
    std::mutex ttMutex;

    void sortMoves(MoveList& moves, Board* board){
        for(int i = 0; i < moves.count; i++){
            board->updateMoveScore(moves[i]);
        }
        // Sort moves
        std::sort(moves.moves, moves.moves + moves.count,[](const Move& a, const Move& b) {return a.moveScore > b.moveScore;});
    }

    void storeTTEntry(uint64_t zobristKey, float value, int depth, float alphaOrig, float beta,  Move bestMove) {
        TTEntry::Flag flag;

       if (value <= alphaOrig) {
            flag = TTEntry::UPPERBOUND;
        } else if (value >= beta) {
            flag = TTEntry::LOWERBOUND;
        } else {
            flag = TTEntry::EXACT;
        }

        transpositionTable[zobristKey] = TTEntry{value, depth, bestMove, flag};
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
        Move startMove;
        bool startMoveTT = false;

        {
            std::lock_guard<std::mutex> lock(ttMutex);
            auto it = transpositionTable.find(zobristKey);
            if (it != transpositionTable.end()) {
                const TTEntry& entry = it->second;

                // Use bestMove regardless of depth for ordering
                if (entry.bestMove.startRow > -1) {
                    startMove = entry.bestMove;
                    startMoveTT = true;
                }

                if(it->second.depth >= depth){
                    if (entry.type == TTEntry::EXACT) {return entry.value;}
                if (entry.type == TTEntry::LOWERBOUND && entry.value >= beta) {return entry.value;}
                if (entry.type == TTEntry::UPPERBOUND && entry.value <= alpha) {return entry.value;}
                }
            }
        }

        MoveList moves;
        board->generatePseudoLegalMoves(moves);
        // If no moves, it's either checkmate or stalemate
        if (moves.count == 0) {
            float eval = evaluate(board, true);
            {
                std::lock_guard<std::mutex> lock(ttMutex);
                transpositionTable[zobristKey] = TTEntry{eval, depth, Move(-1, -1, -1, -1, PieceType::NONE), TTEntry::EXACT};
            }
            return eval;
        }


        sortMoves(moves, board);
        
        if (startMoveTT) {
            for (int i = 0; i < moves.count; ++i) {
                if (moves[i] == startMove) {
                    std::swap(moves[0], moves[i]);  // Try best move first
                    break;
                }
            }
        } 

        float bestScore = maximizingPlayer ? -1000000.0f : 1000000.0f;
        Move bestMove;


        if(maximizingPlayer){
            for(int i = 0; i < moves.count; i++){
                board->makeMove(moves[i]);
                if(!board->inCheck(1 - board->getTurn())){
                    float score = minimaxAB(board, depth - 1, !maximizingPlayer, alpha, beta);

                    if (score > bestScore){
                         bestMove = moves[i];
                        bestScore = score;
                    }

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

                    if (score < bestScore){
                        bestScore = score;
                        bestMove = moves[i];
                    }

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
            storeTTEntry(zobristKey, bestScore, depth, alphaOrig, beta, bestMove);
        }
        return bestScore;
    }

    std::pair<std::string, float> searchBestMoveParallel(Board* board, int depth, bool maximizingPlayer) {
        std::cout << "Starting SBMP with TT size " << transpositionTable.size() << std::endl;
        startingDepth = depth;
        nodesVisited = 0;
        repeatedPositions = 0;
        MoveList moves;
        uint64_t zobristKey = board->getHash();
        std::cout << "Searching with hash " << zobristKey << std::endl;

        {
            std::lock_guard<std::mutex> lock(ttMutex);
            auto it = transpositionTable.find(zobristKey);
            if (it != transpositionTable.end()) {
                TTEntry& entry = it->second;
                if(entry.depth >= depth){
                    std::cout << "Found Posiition in TT" << std::endl;
                    return std::make_pair(entry.bestMove.getMoveRepresentation(), entry.value);
                }
            }
        }


        board->generatePseudoLegalMoves(moves);
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

        {
            std::lock_guard<std::mutex> lock(ttMutex);
            transpositionTable[zobristKey] = TTEntry{bestScore, depth, bestMove, TTEntry::EXACT};
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

