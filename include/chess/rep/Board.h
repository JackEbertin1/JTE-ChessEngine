#ifndef BOARD_H
#define BOARD_H

#include "chess/rep/Move.h"
#include "chess/pieces/King.h"
#include "chess/pieces/Queen.h"
#include "chess/pieces/Pawn.h"
#include "chess/pieces/Rook.h"
#include "chess/pieces/Bishop.h"
#include "chess/pieces/Knight.h"
#include "chess/rep/Zobrist.h"
#include <exception>
#include <iostream>
#include <sstream>

#include <utility>
#include <unordered_set>
#include <vector>
#include <unordered_map>
#include <stack>
#include <optional>
#include <cstdlib>
#include <cstdint>


using namespace Zobrist;
using namespace std;

class Board {

public:

// Representation of both the board and each team.
//
	Piece* board[8][8];
	std::unordered_set<Piece*> whiteAlive;
	std::unordered_set<Piece*> blackAlive;


					// Constructors	

// Initializes chessboard set up for new game. Black pieces are given color 0, white = 1
	Board();


// Initializes chessboard to position given by FEN string. Throws exception if FEN string is invalid
	Board(std::string fen);

// Copy constructor (deep copy)	
// Assignment operator (deep copy)
//
    Board(const Board& other);
    																										
    Board& operator=(const Board& other);

	Board(int dummy);
	Board* clone();


// Prints board with Letter Representations for each piece  
//
	void printBoard();

// Applies a given move to the board and adds move made to history of moves 
// Moves will be made from console by typing in string representation of move. Automatic check will be performed to ensure submitted string corresponds to a legal move in the given position
//	
	void makeMove(Move& move);

	void makeMove(std::string move);

	void castle(Move& move);

	void captureEnPessant(Move& move);

	void promote(Move& move);

	void promoteWhite(Move& move);

	void promoteBlack(Move& move);

	void updateMoveScore(Move& move);

// Undoes a move and returns the board to the state of the previous turn
// 	
	void undoMove();

	void undoCastle(Move& move);

	void undoEnPessant(Move& move);

	void undoPromotion(Move& move);

	void undoPromotionWhite(Move& move);

	void undoPromotionBlack(Move& move);


// Generates all moves for the given player and enters them into move lists for each player.
//	
	int generateLegalMoves(MoveList& legalMoves);

// Checks whether a move is legal by determining if playing the move will leave the player in check
//	
	bool isMoveLegal(Move& move);

// Returns true if player is currently in named condition (Check, Checkmate, Stalemate)
	bool inCheck(int player);

	bool inCheckMate(int player);

	bool inStaleMate(int player);

	bool gameIsOver();


// Generates the locational moves that a piece can make in a given position. Does not account for check as this is done later. Returns a vector of Moves. All of these in theory run in O(1) because while there 
// is looping, the loops are of fixed size with max loop guaranteed to be under 56 iterations because a piece can only move at most 7 squares and can move in at most 8 directions
//	

	int generatePseudoLegalMoves(MoveList& PLmoves);

	void generateMovesForPiece(Piece* piece, MoveList& Pmoves);


// Pawn Algortihms
	void pawnMoves(Piece* piece, MoveList& Pmoves);

	void whitePawn(Piece* piece, MoveList& Pmoves);

	void blackPawn(Piece* piece, MoveList& Pmoves);

	void checkForPromotion(Piece* pawn, MoveList& Pmoves);

	void generatePromotions(Piece* pawn, MoveList& Pmoves, int endRow, int endCol);

	void checkForEnPessant(Piece* pawn, MoveList& Pmoves);

	void resetEnPessant();

	// Knight Algorithm
	void knightMoves(Piece* piece,  MoveList& Pmoves);


	// Bishop Algorithm
	void bishopMoves(Piece* piece,  MoveList& Pmoves);


	// Rook Algorithm
	void rookMoves(Piece* piece,  MoveList& Pmoves);


	// Queen Algorithm
	void queenMoves(Piece* piece,  MoveList& Pmoves);


	// King Algorithms
	void kingMoves(Piece* piece, MoveList& Pmoves);

	void checkForCastleWhite(Piece* king, MoveList& Pmoves);

	void checkForCastleBlack(Piece* king, MoveList& Pmoves);


	// Returns the possible moves for a sliding piece. Input is the directions that a piece can slide in
	void directionalMoves(Piece* piece, const std::vector<std::pair<int, int>>& directions, MoveList& Pmoves);


// Used to help determine if a team is in check by finding all of the squares that a piece can attack. Does not regard check or special moves as these are not necessary for the use case of this function
// 	
	bool pieceAttacksSquare(Piece* piece, int kingRow, int kingCol);

	bool pawnAttacksKing(Piece* piece, int kingRow, int kingCol);

	bool knightAttacksKing(Piece* piece, int kingRow, int kingCol);

	bool kingAttacksKing(Piece* piece, int kingRow, int kingCol);

	bool queenAttacksKing(Piece* piece, int kingRow, int kingCol);

	bool rookAttacksKing(Piece* piece, int kingRow, int kingCol);

	bool bishopAttacksKing(Piece* piece, int kingRow, int kingCol);

	bool slideAttacksKing(Piece* piece, const std::vector<std::pair<int, int>>& directions, int kingRow, int kingCol);


// Helper Functions, mostly used for debugging purposes
	int getTurn();

	void setTurn(int turn);

	std::string getMoveHistory();

	uint64_t getHash();

	std::string generateFen();



// Destructor to clean up dynamically allocated pieces
//																											
	~Board();


private:
	int playerTurn;
	
	int fiftyMoveCounter;
	std::optional<std::pair<int, int>> enPassantSquare;
	bool whiteKingSideCastle;
	bool whiteQueenSideCastle;
	bool blackKingSideCastle;
	bool blackQueenSideCastle;

	Piece* whiteKing;
	Piece* blackKing;

	std::unordered_map<std::string, Move> moveMap;

	std::stack<Move> playedMoves;

	std::stack<Piece*> capturedPieces;
	std::stack<Piece*> pawnRevival;

	std::stack<Piece*> promotionPiecesWhite;
	std::stack<Piece*> promotionPiecesBlack;

	uint64_t currentHash;

	void computeZobristHash();

	void updateZobristHash(Move& move);

// Stores the legal moves for a player in their respective move pool and populates moveMap which holds all legal moves as a value where the string representation for the move is the key
// This map is used to check legality when attemmpting to make a move by inputting string notation
//	
	void cacheLegalMoves(MoveList& legalMoves);

	Piece* makePiece(char c, int row, int col);

};



#endif
