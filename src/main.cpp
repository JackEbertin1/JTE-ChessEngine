
#include "chess/search/chessSearch.h" 

#include <cstdlib>  // for rand
#include <ctime>    // for seeding rand

using namespace std;
using namespace chessSearch;
using namespace Evaluation;


int main() {
    
    std::string fen;
    cout << "Please enter a FEN (Enter 0 for new game): " << endl;
    std::getline(std::cin, fen);

    Board* test;
    if(fen == "0"){
         test = new Board();
    }
    else
    {
        test = new Board(fen);
    }

    std::string move;
    MoveList moves;
    int depth = 2;

    while (true){
        int playerTurn = test->getTurn();

        if(playerTurn != 1){
            pair<string, float> eval = searchBestMoveParallel(test, depth, test->getTurn());
            cout << "Eval: " << eval.second << endl;
            cout << "Best move is " << eval.first << endl;
        }
        
        test->generateLegalMoves(moves);

        while(true){
            cout << "Please enter a move: " << endl;
            std::getline(std::cin, move);

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

            if(move == "print"){
                test->printBoard();
                continue;
            }

            if(move == "Quit"){
                return 0;
            }

            if(move == "undo"){
                test->undoMove();
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

                break;
            } catch (const std::exception& e){
                cout << "Caught exception: " << e.what() << endl;
            }
        }
    }



    delete test;
    return 0;
}


