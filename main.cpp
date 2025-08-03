#include "chessSearch.h"

#include <cstdlib>  // for rand
#include <ctime>    // for seeding rand

using namespace std;
using namespace chessSearch;
using namespace Evaluation;

/*
g2g4
b7b5
g4g5
b5b4
a2a4
g5h6xEP

r1bqkbnr/pppp1ppp/2n5/4p3/4P3/2N5/PPPP1PPP/R1BQKBNR w KQkq - 0 1

*/


int main() {
    std::string fen;
    cout << "Please enter a FEN: " << endl;
            std::getline(std::cin, fen);

    Board* test = new Board();
    //cout << test->getHash() << endl;

    std::string move;
    MoveList moves;
    int depth = 4;

    while (true){
        int playerTurn = test->getTurn();

        if(playerTurn == 1){
            pair<string, float> eval = searchBestMoveParallel(test, depth, test->getTurn());
            cout << "Eval: " << eval.second << endl;
            cout << "Best move is " << eval.first << endl;
        }
        
        test->generateLegalMoves(moves);

        while(true){
            cout << "Please enter a move: " << endl;
            std::getline(std::cin, move);

             if (std::cin.fail()) {
                std::cout << "[ERROR] cin failed. Clearing state." << std::endl;
                std::cin.clear(); // clear fail state
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard leftover input
                continue;
            }

            if(move == "wm"){
                if(test->getTurn() == 0){cout << "Not White's Turn"; continue;}
                for(int i = 0; i < moves.count; i++){
                    cout << moves[i].getMoveRepresentation() << endl;
                }
                continue;
            }

            if(move == "bm"){
                if(test->getTurn() == 1){cout << "Not Black's Turn"; continue;}
                for(int i = 0; i < moves.count; i++){
                    cout << moves[i].getMoveRepresentation() << endl;
                }
                continue;
            }

            if(move == "FEN"){
                cout << test->generateFen() << endl;
                continue;
            }

            if (move == "pieces"){
                test->printAlivePieces();
                continue;
            }

            if(move == "print"){
                test->printBoard();
                continue;
            }

            if (move == "perft"){
                int dep;
                cout << "Please enter a depth: " << endl;
                        cin >> dep;
                cout << "Nodes Found: " << endl << perftParallel(test, dep) << endl;
                continue;
            }

            if(move == "Quit"){
                break;
            }

            if(move == "undo"){
                test->undoMove();
                cout << "New ZH: " << test->getHash() << endl;
                break;
            }

            try{
                test->makeMove(move);
                test->printBoard();
                if (test->inCheck(test->getTurn())){
                    cout << "Move put player in check" << endl;
                }
                if(test->inCheckMate(test->getTurn())){ 
                     cout << "Checkmate: " << ((test->getTurn() == 0) ? "White wins!" : "Black wins!") << endl;
                     return 0;
                }
                if(test->inStaleMate(test->getTurn())){
                    cout << "Stalemate: Match ends in a draw" << endl;
                    return 0;
                }

                cout << "New ZH: " << test->getHash() << endl;

                break;
            } catch (const std::exception& e){
                cout << "Caught exception: " << e.what() << endl;
            }
        }
    }

    /*cout << "Beginning Perft Search with depth 4 from position : " << endl;
    test->printBoard();

    cout << "Nodes Found: " << endl << perft(test, 1) << endl;*/




    delete test;
    return 0;
}


