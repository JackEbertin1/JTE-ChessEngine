#include "chess/rep/Board.h"

Board::Board(){

		initializeZobrist();
		for(int i = 0; i < 8; i++){
			for(int j = 0; j < 8; j++){
				board[i][j] = nullptr;
			}
		} 	

		blackKing = new King(0, 7, 4);		//Black King
		board[7][4] = blackKing;
		blackAlive.insert(board[7][4]);


		whiteKing = new King(1, 0, 4);		//White King
		board[0][4] = whiteKing;
		whiteAlive.insert(board[0][4]);



		for (int i = 0; i < 8; i++){
			board[1][i] = new Pawn(1, 1, i);	//White Pawns
			whiteAlive.insert(board[1][i]);

			board[6][i] = new Pawn(0, 6, i);	//Black Pawns
			blackAlive.insert(board[6][i]);
		}

		board[0][0] = new Rook(1, 0, 0);		//White Rooks
		board[0][7] = new Rook(1, 0, 7);
		board[0][1] = new Knight(1, 0, 1);		//White Knights
		board[0][6] = new Knight(1, 0, 6);
		board[0][2] = new Bishop(1, 0, 2);		//White Bishops
		board[0][5] = new Bishop(1, 0, 5);
		board[0][3] = new Queen(1, 0, 3);		//White Queen

		board[7][0] = new Rook(0, 7, 0);		//Black Rooks
		board[7][7] = new Rook(0, 7, 7);
		board[7][1] = new Knight(0, 7, 1);		//Black Knights
		board[7][6] = new Knight(0, 7, 6);
		board[7][2] = new Bishop(0, 7, 2);		//Black Bishops
		board[7][5] = new Bishop(0, 7, 5);
		board[7][3] = new Queen(0, 7, 3);		//Black Queen

		for(int i = 0; i < 8; i++){		
			if (i == 4){continue;}		
			blackAlive.insert(board[7][i]);
			whiteAlive.insert(board[0][i]);
		}

		enPassantSquare = std::nullopt;
		playerTurn = 1;
		fiftyMoveCounter = 0;
		whiteKingSideCastle = true;
		whiteQueenSideCastle = true;
		blackKingSideCastle = true;
		blackQueenSideCastle = true;

		computeZobristHash();

		this->printBoard();
	}

Board::Board(std::string fen){
		initializeZobrist();
		std::istringstream iss(fen);
	    std::string piecePlacement, activeColor, castling, enPassant;
	    int halfmove, fullmove;

	    iss >> piecePlacement >> activeColor >> castling >> enPassant >> halfmove >> fullmove;

	    int fenRow = 7;  // Start from rank 8
		int col = 0;

		for (char c : piecePlacement) {
		    if (c == '/') {
		        fenRow--;
		        col = 0;
		    } else if (isdigit(c)) {
		        col += c - '0';
		    } else {
		        Piece* piece = makePiece(c, fenRow, col);
		        if (piece->type == PieceType::KING) {
				    if (piece->color == 1) {whiteKing = piece;}
				    else {blackKing = piece;}
				}
		        board[fenRow][col] = piece;
		        if (piece->color == 1) whiteAlive.insert(piece);
		        else blackAlive.insert(piece);
		        col++;
		    }
		}
        // Set turn
    	playerTurn = (activeColor == "w") ? 1 : 0;

    	// 3. Castling rights
	    whiteKingSideCastle = castling.find('K') != std::string::npos;
	    whiteQueenSideCastle = castling.find('Q') != std::string::npos;
	    blackKingSideCastle = castling.find('k') != std::string::npos;
	    blackQueenSideCastle = castling.find('q') != std::string::npos;
	    computeZobristHash();
    }

Board::Board(int dummy){
		// Initialize the 8x8 board to nullptr
	    for (int i = 0; i < 8; ++i) {
	        for (int j = 0; j < 8; ++j) {
	            board[i][j] = nullptr;
	        }
	    }

	    // Clear piece sets
	    whiteAlive.clear();
	    blackAlive.clear();

	    // Clear move stack
	    while (!playedMoves.empty()) {
	        playedMoves.pop();
	    }

	    // Clear king pointers
	    whiteKing = nullptr;
	    blackKing = nullptr;

	    // Clear special state
	    enPassantSquare = std::nullopt;
	    fiftyMoveCounter = 0;
	    playerTurn = 0; // Default; will be overwritten in clone

	    whiteKingSideCastle = true;
	    whiteQueenSideCastle = true;
	    blackKingSideCastle = true;
	    blackQueenSideCastle = true;
	}

Board::Board(const Board& other) {
        for (int i = 0; i < 8; i++){
            for (int j = 0; j < 8; j++){
            	if(other.board[i][j]){
            		board[i][j] = other.board[i][j]->clone();
            	}
            	else{
            		board[i][j] = nullptr;
            	}
            }
        }
    }

Board& Board::operator=(const Board& other) {
    if (this == &other)
        return *this;

    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
        	if (other.board[i][j]){
        		board[i][j] = other.board[i][j]->clone();
        	}
        	else{
        		board[i][j] = nullptr;
        	}
        } 	
    }
    return *this;
}

Board* Board::clone(){
		Board* other = new Board(1);

		for (int i = 0; i < 8; i++){
            for (int j = 0; j < 8; j++){
            	if(this->board[i][j]){
            		other->board[i][j] = this->board[i][j]->clone();
            		if (other->board[i][j]->color == 1){
            			other->whiteAlive.insert(other->board[i][j]);
            		}
            		else{
            			other->blackAlive.insert(other->board[i][j]);
            		}
            		if (other->board[i][j]->type == PieceType::KING) {
					    if (other->board[i][j]->color == 1)
					        other->whiteKing = other->board[i][j];
					    else
					        other->blackKing = other->board[i][j];
					}
            	}
            	else{
            		other->board[i][j] = nullptr;
            	}
            }
        }

        // Copy the move stack
	    std::stack<Move> tempStack = this->playedMoves;
	    std::stack<Move> reverseStack;

	    while (!tempStack.empty()) {
	        reverseStack.push(tempStack.top());
	        tempStack.pop();
	    }
	    while (!reverseStack.empty()) {
	        other->playedMoves.push(reverseStack.top());
	        reverseStack.pop();
	    }

	    other->enPassantSquare = this->enPassantSquare;
		other->playerTurn = this->playerTurn;
		other->fiftyMoveCounter = this->fiftyMoveCounter;
		other->whiteKingSideCastle = this->whiteKingSideCastle;
		other->whiteQueenSideCastle = this->whiteQueenSideCastle;
		other->blackKingSideCastle = this->blackKingSideCastle;
		other->blackQueenSideCastle = this->blackQueenSideCastle;
		other->currentHash = this->currentHash;

		return other;
	}

Board::~Board() {
    	for (int i = 0; i < 8; i++) {
       		for (int j = 0; j < 8; j++) {
            	if (board[i][j] != nullptr) {
                	delete board[i][j];  // Free the memory for the piece
            	}
        	}
    	}
	}

// Board Representation and Move Making functions

void Board::printBoard(){	
    for (int i = 7; i > -1; i--) {
        	for (int j = 0; j < 8; j++) {
            	if (board[i][j] == nullptr) {
                	std::cout << "* ";  // Empty square is represented by "*"
            	} else {
                	std::cout << board[i][j]->getRepresentation() << " ";  // Print the value of the piece
            	}
        	}
        	std::cout << std::endl;  // Move to the next line after printing a row
    	}
	}

void Board::makeMove(Move& move){	
		move.previousZobristHash = currentHash;  // Cache current hash
		updateZobristHash(move);


		if (move.isCastle){
			castle(move);
			return;
		}
		if(move.isEnPessant){
			captureEnPessant(move);
			return;
		}
		if(move.isPromotion){
			promote(move);
			return;
		}

		std::unordered_set<Piece*>& otherTeam = (playerTurn == 0) ? whiteAlive : blackAlive;

		int startRow = move.startRow;
		int startCol = move.startCol;
		int endRow = move.endRow;
		int endCol = move.endCol;
		Piece* piece = board[startRow][startCol];
		Piece* pieceCaptured = board[endRow][endCol];

		if(!piece){
			throw std::runtime_error("Move has bad value");
		}

		if (pieceCaptured) {
			move.storeTimesMoved(pieceCaptured->timesMoved);

			capturedPieces.push(pieceCaptured);
		    otherTeam.erase(pieceCaptured);
		}

		board[startRow][startCol] = nullptr;
		board[endRow][endCol] = piece;

		piece->updateLocation(endRow, endCol);
		piece->timesMoved++;

		if (piece->getType() == PieceType::PAWN && std::abs(startRow - endRow) == 2){
			int epRow = (move.startRow + move.endRow) / 2;
    		enPassantSquare = std::make_pair(epRow, move.startCol);
    		move.setEnPessantSquare(enPassantSquare->first, enPassantSquare->second);
		}
		else{
			enPassantSquare = std::nullopt;
			move.setEnPessantSquare(-1, -1);
		}

		playerTurn = 1 - playerTurn;

		// Update 50 move rule counter, resets if move was a pawn move or capture, increments otherwise
		if(piece->getType() == PieceType::PAWN || move.isCapture){fiftyMoveCounter = 0;}
		else{fiftyMoveCounter++;}

		playedMoves.push(move);
	}

void Board::makeMove(std::string move){
		if (!moveMap.contains(move)) {
        	throw std::invalid_argument("Invalid or illegal move: " + move);
    	}
    	makeMove(moveMap[move]);
	}

void Board::castle(Move& move){
		int startRow = move.startRow;
	    int startCol = move.startCol;
	    int endRow = move.endRow;
	    int endCol = move.endCol;
	    Piece* king = board[startRow][startCol];

	    if(!king || king->type != PieceType::KING){
	    	throw std::runtime_error("Move has bad value");
	    }

	    // Move the king
	    board[startRow][startCol] = nullptr;
	    board[endRow][endCol] = king;
	    king->updateLocation(endRow, endCol);
	    king->timesMoved++;

	    // Determine which side of castling this is
	    if (endCol == 6) {  // Kingside
	        if (endRow == 0) {  // White
	            board[0][5] = board[0][7];
	            board[0][7] = nullptr;
	            board[0][5]->updateLocation(0, 5);
	            board[0][5]->timesMoved++;
	        } else {  // Black
	            board[7][5] = board[7][7];
	            board[7][7] = nullptr;
	            board[7][5]->updateLocation(7, 5);
	            board[7][5]->timesMoved++;
	        }
	    } else {  // Queenside
	        if (endRow == 0) {  // White
	            board[0][3] = board[0][0];
	            board[0][0] = nullptr;
	            board[0][3]->updateLocation(0, 3);
	            board[0][3]->timesMoved++;
	        } else {  // Black
	            board[7][3] = board[7][0];
	            board[7][0] = nullptr;
	            board[7][3]->updateLocation(7, 3);
	            board[7][3]->timesMoved++;
	        }
	    }
	    playerTurn = 1 - playerTurn;
	    fiftyMoveCounter++;
	    enPassantSquare = std::nullopt;
		move.setEnPessantSquare(-1, -1);

		playedMoves.push(move);
	}

void Board::captureEnPessant(Move& move){
		int startRow = move.startRow;
	    int startCol = move.startCol;
	    int endRow = move.endRow;
	    int endCol = move.endCol;
	    Piece* pawn = board[startRow][startCol];
	    Piece* pieceCaptured = board[startRow][endCol];

	    std::unordered_set<Piece*>& otherTeam = (playerTurn == 0) ? whiteAlive : blackAlive;

	    board[startRow][startCol] = nullptr;
	    board[endRow][endCol] = pawn;
	    board[pieceCaptured->row][pieceCaptured->col] = nullptr;

	    move.storeTimesMoved(pieceCaptured->timesMoved);
	    capturedPieces.push(pieceCaptured);
	    otherTeam.erase(pieceCaptured);

	    pawn->updateLocation(endRow, endCol);
		pawn->timesMoved++;

		enPassantSquare = std::nullopt;
		move.setEnPessantSquare(-1, -1);
		playerTurn = 1 - playerTurn;
		fiftyMoveCounter = 0;

		playedMoves.push(move);
	}

void Board::promote(Move& move){
		if (move.endRow == 7){promoteWhite(move);}
		else {promoteBlack(move);}

		enPassantSquare = std::nullopt;
		move.setEnPessantSquare(-1, -1);
		playerTurn = 1 - playerTurn;

		playedMoves.push(move);
	}

void Board::promoteWhite(Move& move){
		int startRow = move.startRow;
		int startCol = move.startCol;
		int endRow = move.endRow;
		int endCol = move.endCol;

		Piece* pawn = board[startRow][startCol];
		if (!(pawn->type == PieceType::PAWN)){throw std::runtime_error("Tried to promote with piece other than pawn");}

		Piece* captured = board[endRow][endCol];

		// Create the promotion piece based on promotionType string
	    Piece* promotePiece = nullptr;
	    if (move.promotionType == PieceType::QUEEN) {promotePiece = new Queen(pawn->color, endRow, endCol);}
	    else if (move.promotionType == PieceType::ROOK) {promotePiece = new Rook(pawn->color, endRow, endCol);}
	    else if (move.promotionType == PieceType::KNIGHT) {promotePiece = new Knight(pawn->color, endRow, endCol);}
	    else if (move.promotionType == PieceType::BISHOP) {promotePiece = new Bishop(pawn->color, endRow, endCol);}

		// Update board
		board[endRow][endCol] = promotePiece;
		board[startRow][startCol] = nullptr;

		// Delete Pawn and Captured piece from respective teams and add new piece to Current Team
		if (captured) {
			move.storeTimesMoved(captured->timesMoved);

			capturedPieces.push(captured);
		    blackAlive.erase(captured);
		}

		// Update White team members by removing pawn and adding promoted piece
		pawnRevival.push(pawn);
		whiteAlive.erase(pawn);
		whiteAlive.insert(promotePiece);
	}

void Board::promoteBlack(Move& move){
		int startRow = move.startRow;
		int startCol = move.startCol;
		int endRow = move.endRow;
		int endCol = move.endCol;

		Piece* pawn = board[startRow][startCol];
		if (!(pawn->type == PieceType::PAWN)){throw std::runtime_error("Tried to promote with piece other than pawn");}

		Piece* captured = board[endRow][endCol];

		// Create the promotion piece based on promotionType string
	    Piece* promotePiece = nullptr;
	    if (move.promotionType == PieceType::QUEEN) {promotePiece = new Queen(pawn->color, endRow, endCol);}
	    else if (move.promotionType == PieceType::ROOK) {promotePiece = new Rook(pawn->color, endRow, endCol);}
	    else if (move.promotionType == PieceType::KNIGHT) {promotePiece = new Knight(pawn->color, endRow, endCol);}
	    else if (move.promotionType == PieceType::BISHOP) {promotePiece = new Bishop(pawn->color, endRow, endCol);}

		// Update board
		board[endRow][endCol] = promotePiece;
		board[startRow][startCol] = nullptr;

		// Delete Pawn and Captured piece from respective teams and add new piece to Current Team
		if (captured) {
			move.storeTimesMoved(captured->timesMoved);

			capturedPieces.push(captured);
		    whiteAlive.erase(captured);
		}

		// Update Black team members by removing pawn and adding promoted piece
		pawnRevival.push(pawn);
		blackAlive.erase(pawn);
		blackAlive.insert(promotePiece);
	}

void Board::undoMove(){
		Move move = playedMoves.top();
		currentHash = move.previousZobristHash; // Restore Zobrist hash from last position

		if(move.isCastle){
			undoCastle(move);
			return;
		}
		if(move.isEnPessant){
			undoEnPessant(move);
			return;
		}
		if(move.isPromotion){
			undoPromotion(move);
			return;
		}

		std::unordered_set<Piece*>& currentTeam = (playerTurn == 0) ? blackAlive : whiteAlive;

		int startRow = move.startRow;
		int startCol = move.startCol;
		int endRow = move.endRow;
		int endCol = move.endCol;
		Piece* piece = board[endRow][endCol];
		Piece* other;

		if (move.isCapture){
			other = capturedPieces.top();
			capturedPieces.pop();
			other->updateTimesMoved(move.capturedPieceTimesMoved);
		}
		else{other = nullptr;}
		

		board[startRow][startCol] = piece;
		board[endRow][endCol] = other;

		if (other){
			currentTeam.insert(other);
			other->updateLocation(endRow, endCol);
		}

		piece->updateLocation(startRow, startCol);
		piece->timesMoved--;

		playedMoves.pop();
		playerTurn = 1 - playerTurn;  
		resetEnPessant();
	}

void Board::undoCastle(Move& move){
		int startRow = move.startRow;
		int startCol = move.startCol;
		int endRow = move.endRow;
		int endCol = move.endCol;

		Piece* king = board[endRow][endCol];

		board[startRow][startCol] = king;
		board[endRow][endCol] = nullptr;
		king->updateLocation(startRow, startCol);
		king->timesMoved--;

		Piece* rook = nullptr;
		if (endCol == 6) { // Kingside
		    rook = board[endRow][5];
		    board[endRow][5] = nullptr;
		    board[endRow][7] = rook;
		    rook->updateLocation(endRow, 7);

		} else if (endCol == 2) { // Queenside
		    rook = board[endRow][3];
		    board[endRow][3] = nullptr;
		    board[endRow][0] = rook;
		    rook->updateLocation(endRow, 0);
		}
		rook->timesMoved--;


		playedMoves.pop();
		playerTurn = 1 - playerTurn;
		resetEnPessant();
	}

void Board::undoEnPessant(Move& move){
		int startRow = move.startRow;
		int startCol = move.startCol;
		int endRow = move.endRow;
		int endCol = move.endCol;
		Piece* pawn = board[endRow][endCol];

		std::unordered_set<Piece*>& currentTeam = (playerTurn == 0) ? blackAlive : whiteAlive;

		board[endRow][endCol] = nullptr;
		board[startRow][startCol] = pawn;

		Piece* capturedPiece = capturedPieces.top();
		capturedPieces.pop();

		board[startRow][endCol] = capturedPiece;

		pawn->updateLocation(startRow, startCol);
		pawn->timesMoved--;

		currentTeam.insert(capturedPiece);

		playedMoves.pop();
		playerTurn = 1 - playerTurn;
		resetEnPessant();
	}

void Board::undoPromotion(Move& move){
		if (move.endRow == 7){undoPromotionWhite(move);}
		else{undoPromotionBlack(move);}

		playerTurn = 1 - playerTurn;
		resetEnPessant();
	}

void Board::undoPromotionWhite(Move& move){
		int startRow = move.startRow;
		int startCol = move.startCol;
		int endRow = move.endRow;
		int endCol = move.endCol;

		Piece* pawn = pawnRevival.top();
		pawnRevival.pop();

		Piece* promotePiece = board[endRow][endCol];
		Piece* captured = nullptr;

		if (move.isCapture){
			captured = capturedPieces.top();
			capturedPieces.pop();
			captured->updateTimesMoved(move.capturedPieceTimesMoved);
		}

		// Update board
		board[endRow][endCol] = captured;
		board[startRow][startCol] = pawn;

		if(captured){
			blackAlive.insert(captured);
			captured->updateLocation(endRow, endCol);
		}

		whiteAlive.erase(promotePiece);
		whiteAlive.insert(pawn);

		playedMoves.pop();  
	}

void Board::undoPromotionBlack(Move& move){
		int startRow = move.startRow;
		int startCol = move.startCol;
		int endRow = move.endRow;
		int endCol = move.endCol;

		Piece* pawn = pawnRevival.top();
		pawnRevival.pop();

		Piece* promotePiece = board[endRow][endCol];
		Piece* captured = nullptr;

		if (move.isCapture){
			captured = capturedPieces.top();
			capturedPieces.pop();
			captured->updateTimesMoved(move.capturedPieceTimesMoved);
		}

		// Update board
		board[endRow][endCol] = captured;
		board[startRow][startCol] = pawn;

		if(captured){
			whiteAlive.insert(captured);
			captured->updateLocation(endRow, endCol);
		}

		blackAlive.erase(promotePiece);
		blackAlive.insert(pawn);
		delete promotePiece;

		playedMoves.pop();
	}

void Board::updateMoveScore(Move& move) {
	    if (move.isCapture && !move.isEnPessant) {
	        int attacker = board[move.startRow][move.startCol]->getValue();
			int victim   = board[move.endRow][move.endCol]->getValue();
			move.moveScore += 100 * victim - attacker;
	    }
	}

// Move generation functions

int Board::generateLegalMoves(MoveList& legalMoves){
		MoveList pseudoMoves;
    	generatePseudoLegalMoves(pseudoMoves);

    	legalMoves.count = 0;
    	for (int i = 0; i < pseudoMoves.count; i++) {
	        Move& move = pseudoMoves.moves[i];

	        makeMove(move); 
	        if (!inCheck(1 - playerTurn)) {
	           	legalMoves.add(move);
	        }
	        undoMove();
	    }
	    cacheLegalMoves(legalMoves);
		return legalMoves.count;
	}

int Board::generatePseudoLegalMoves(MoveList& PLmoves){
		std::unordered_set<Piece*>& currentTeam = (playerTurn == 0) ? blackAlive : whiteAlive;
		std::unordered_set<Piece*> teamSnapshot = currentTeam;
		PLmoves.count = 0;

		for (Piece* piece : teamSnapshot){
			generateMovesForPiece(piece, PLmoves);
		}
		return PLmoves.count;
	}

void Board::generateMovesForPiece(Piece* piece, MoveList& Pmoves){
		if(piece->type == PieceType::PAWN){pawnMoves(piece, Pmoves);}
		else if(piece->type == PieceType::KNIGHT){knightMoves(piece, Pmoves);}
		else if(piece->type == PieceType::BISHOP){bishopMoves(piece, Pmoves);}
		else if(piece->type == PieceType::ROOK){rookMoves(piece, Pmoves);}
		else if(piece->type == PieceType::QUEEN){queenMoves(piece, Pmoves);}
		else if(piece->type == PieceType::KING){kingMoves(piece, Pmoves);}
		else {throw std::runtime_error("Unknown piece type in generateMovesForPiece");}
	}
 

// Pawn Algortihms
void Board::pawnMoves(Piece* piece, MoveList& Pmoves){
		if (piece->color == 0){blackPawn(piece, Pmoves);}
		else{ whitePawn(piece, Pmoves);}
	}

void Board::whitePawn(Piece* piece, MoveList& Pmoves){
	// Determine if pawn can make normal advancing move
		if(piece->row + 1 <= 6 && board[piece->row + 1][piece->col] == nullptr){
			Pmoves.add(Move(piece->row, piece->col, piece->row + 1, piece->col, PieceType::NONE));
			if (piece->row == 1){
				if (board[piece->row + 2][piece->col] == nullptr){
					Move doubleP(piece->row, piece->col, piece->row + 2, piece->col, PieceType::NONE);
					doubleP.setDoublePawnPush();
					Pmoves.add(doubleP);
				}
			}
		}

	// Check if Pawn can capture on Diagonal Square
		int newCol = piece->col + 1;
		if(newCol < 8 && piece->row + 1 <= 6){
			Piece* landingSquare = board[piece->row + 1][newCol];
			if(landingSquare){
				if (landingSquare->color != piece->color){
					Pmoves.add(Move(piece->row, piece->col, landingSquare->row, landingSquare->col, landingSquare->getType()));
				}
			}
		}
		newCol = piece->col - 1;	
		if(newCol > -1 && piece->row + 1 <= 6){
			Piece* landingSquare = board[piece->row + 1][newCol];
			if(landingSquare){
				if (landingSquare->color != piece->color){
					Pmoves.add(Move(piece->row, piece->col, landingSquare->row, landingSquare->col, landingSquare->getType()));
				}
			}
		}
		checkForPromotion(piece, Pmoves);
		checkForEnPessant(piece, Pmoves);
	}

void Board::blackPawn(Piece* piece, MoveList& Pmoves){
	// Determine if pawn can make normal advancing move
		if(piece->row - 1 >= 1 && board[piece->row - 1][piece->col] == nullptr){
			Pmoves.add(Move(piece->row, piece->col, piece->row - 1, piece->col, PieceType::NONE));
			if (piece->row == 6){
				if (board[piece->row - 2][piece->col] == nullptr){
					Move doubleP(piece->row, piece->col, piece->row - 2, piece->col, PieceType::NONE);
					doubleP.setDoublePawnPush();
					Pmoves.add(doubleP);
				}
			}
		}

	// Check if Pawn can capture on Diagonal Square
		int newCol = piece->col + 1;
		if(newCol < 8 && piece->row - 1 >= 1){
			Piece* landingSquare = board[piece->row - 1][newCol];
			if(landingSquare){
				if (landingSquare->color != piece->color){
					Pmoves.add(Move(piece->row, piece->col, landingSquare->row, landingSquare->col, landingSquare->getType()));
				}
			}
		}
		newCol = piece->col - 1;	
		if(newCol > - 1 && piece->row - 1 >= 1){
			Piece* landingSquare = board[piece->row - 1][newCol];
			if(landingSquare){
				if (landingSquare->color != piece->color){
					Pmoves.add(Move(piece->row, piece->col, landingSquare->row, landingSquare->col, landingSquare->getType()));
				}
			}
		}
		checkForPromotion(piece, Pmoves);
		checkForEnPessant(piece, Pmoves);
	}

void Board::checkForPromotion(Piece* pawn, MoveList& Pmoves){
		if (pawn->color == 1 && pawn->row != 6){return;}
		if (pawn->color == 0 && pawn->row != 1){return;}

		Piece* landingSquare = nullptr;

		// Check if White pawn can promote
		if(pawn->color == 1){ 
			landingSquare = board[pawn->row + 1][pawn->col];
			if(landingSquare == nullptr){
				generatePromotions(pawn, Pmoves, pawn->row + 1, pawn->col);
			}
			if (pawn->col + 1 <= 7){
				landingSquare = board[pawn->row + 1][pawn->col + 1];
				if(landingSquare){
					if (landingSquare->color == 0){
						generatePromotions(pawn, Pmoves, landingSquare->row, landingSquare->col);
					}
				}
			}
			if(pawn->col - 1 >= 0){
				landingSquare = board[pawn->row + 1][pawn->col - 1];
				if(landingSquare){
					if (landingSquare->color == 0){
						generatePromotions(pawn, Pmoves, landingSquare->row, landingSquare->col);
					}
				}
			}
		}
		// Check if Black pawn can promote
		else{
			landingSquare = board[pawn->row - 1][pawn->col];
			if(landingSquare == nullptr){
				generatePromotions(pawn, Pmoves, pawn->row - 1, pawn->col);
			}
			if(pawn->col + 1 <= 7){
				landingSquare = board[pawn->row - 1][pawn->col + 1];
				if(landingSquare){
					if (landingSquare->color == 1){
						generatePromotions(pawn, Pmoves, landingSquare->row, landingSquare->col);
					}
				}
			}
			if(pawn->col - 1 >= 0){
				landingSquare = board[pawn->row - 1][pawn->col - 1];
				if(landingSquare){
					if (landingSquare->color == 1){
						generatePromotions(pawn, Pmoves, landingSquare->row, landingSquare->col);
					}
				}
			}
		}	
	}

void Board::generatePromotions(Piece* pawn, MoveList& Pmoves, int endRow, int endCol){
		Piece* landingSquare = board[endRow][endCol];
		PieceType newLocation = PieceType::NONE;

		if (landingSquare){
			newLocation = landingSquare->getType();
		}

		Move promoteQueen(pawn->row, pawn->col, endRow, endCol, newLocation);
		PieceType q = PieceType::QUEEN;
		promoteQueen.setPromotion(q);
		Pmoves.add(promoteQueen);

		Move promoteRook(pawn->row, pawn->col, endRow, endCol, newLocation);
		PieceType r = PieceType::ROOK;
		promoteRook.setPromotion(r);
		Pmoves.add(promoteRook);

		Move promoteKnight(pawn->row, pawn->col, endRow, endCol, newLocation);
		PieceType k =PieceType:: KNIGHT;
		promoteKnight.setPromotion(k);
		Pmoves.add(promoteKnight);

		Move promoteBishop(pawn->row, pawn->col, endRow, endCol, newLocation);
		PieceType b = PieceType::BISHOP;
		promoteBishop.setPromotion(b);
		Pmoves.add(promoteBishop);
	}

void Board::checkForEnPessant(Piece* pawn, MoveList& Pmoves){
		if (!enPassantSquare) return;
	    int epRow = enPassantSquare->first;
	    int epCol = enPassantSquare->second;

	    if (!(pawn->type == PieceType::PAWN)) {return;}

	    if (pawn->color == 1) {
	        if (epRow == pawn->row + 1) {
	            if (epCol == pawn->col + 1 || epCol == pawn->col - 1) {
	                Move captureEP(pawn->row, pawn->col, epRow, epCol, PieceType::PAWN);
	                captureEP.setEnPessant();
	                Pmoves.add(captureEP);
	            }
	        }
	    } else { // Black
	        if (epRow == pawn->row - 1) {
	            if (epCol == pawn->col + 1 || epCol == pawn->col - 1) {
	                Move captureEP(pawn->row, pawn->col, epRow, epCol, PieceType::PAWN);
	                captureEP.setEnPessant();
	                Pmoves.add(captureEP);
	            }
	        }
	    }}    

void Board::resetEnPessant(){
		if (!playedMoves.empty()) {
		    Move lastMove = playedMoves.top();

		    if (lastMove.epRow == -1 || lastMove.epCol == -1) {
		        enPassantSquare = std::nullopt;
		    } else {
		        enPassantSquare = std::make_pair(lastMove.epRow, lastMove.epCol);
		    }
		} else {
		    enPassantSquare = std::nullopt;
		}
	}        

// Knight Algorithm
void Board::knightMoves(Piece* piece,  MoveList& Pmoves){
		for(const auto& [r, c] : piece->getDirections()){
			int newRow = piece->row + r;
			int newCol = piece->col + c;

			// Ensure Square can be moved to
			if(newRow > 7 || newRow < 0){continue;}
			if(newCol > 7 || newCol < 0){continue;}

			// View what is on landing Square and add move if possible
			Piece* landingSquare = board[newRow][newCol];
			if(landingSquare == nullptr){
				Pmoves.add(Move(piece->row, piece->col, newRow, newCol, PieceType::NONE));
			} 
			else if(landingSquare->color != piece->color){
				Pmoves.add(Move(piece->row, piece->col, newRow, newCol, landingSquare->getType()));
			}
		}	
	}


//Sliding Piece Algorithms    
void Board::directionalMoves(Piece* piece, const std::vector<std::pair<int, int>>& directions, MoveList& Pmoves) {  
	    for (const auto& [r, c] : directions) {
	        int multiplier = 1;
	        while (true) {
	            int newRow = piece->row + (r * multiplier);
	            int newCol = piece->col + (c * multiplier);

	            if (newRow < 0 || newRow > 7 || newCol < 0 || newCol > 7)
	                break;

	            Piece* target = board[newRow][newCol];
	            if (!target) {
	                Pmoves.add(Move(piece->row, piece->col, newRow, newCol, PieceType::NONE));
	            } else {
	                if (target->color != piece->color)
	                    Pmoves.add(Move(piece->row, piece->col, newRow, newCol, target->getType()));
	                break;
	            }
	            multiplier++;
	        }
	    }
	}

void Board::bishopMoves(Piece* piece,  MoveList& Pmoves){
		directionalMoves(piece, piece->getDirections(), Pmoves);}

void Board::rookMoves(Piece* piece,  MoveList& Pmoves){
		directionalMoves(piece, piece->getDirections(), Pmoves);}

void Board::queenMoves(Piece* piece,  MoveList& Pmoves){
		directionalMoves(piece, piece->getDirections(), Pmoves);}


// King Algorithms
void Board::kingMoves(Piece* piece, MoveList& Pmoves){

			for(const auto& [r, c] : piece->getDirections()){
				int newRow = piece->row + r;
				int newCol = piece->col + c;

			// Ensure Square can be moved to
				if(newRow > 7 || newRow < 0){continue;}
				if(newCol > 7 || newCol < 0){continue;}

			// View what is on landing Square and add move if possible
				Piece* landingSquare = board[newRow][newCol];
				if(landingSquare == nullptr){
					Pmoves.add(Move(piece->row, piece->col, newRow, newCol, PieceType::NONE));
				} 
				else if(landingSquare->color != piece->color){
					Pmoves.add(Move(piece->row, piece->col, newRow, newCol, landingSquare->getType()));
				}

			}
		if(piece->color == 1){checkForCastleWhite(piece, Pmoves);}
		else{checkForCastleBlack(piece, Pmoves);}	
	}

void Board::checkForCastleWhite(Piece* king, MoveList& Pmoves){
	    if (king->timesMoved != 0) {return;}

	    // Kingside rook at h1
	    Piece* rookKingside = board[0][7];
	    if (rookKingside && (rookKingside->type == PieceType::ROOK) && rookKingside->timesMoved == 0 && whiteKingSideCastle) {
	        if (!board[0][5] && !board[0][6]) {
	            bool safe = true;
	            for (Piece* enemy : blackAlive) {
	                if (pieceAttacksSquare(enemy, 0, 4) || pieceAttacksSquare(enemy, 0, 5) || pieceAttacksSquare(enemy, 0, 6)) {
	                    safe = false;
	                    break;
	                }
	            }
	            if (safe) {
	                Move kingsideCastle(0, 4, 0, 6, PieceType::NONE);
	                kingsideCastle.setCastle();
	                Pmoves.add(kingsideCastle);
	            }
	        }
	    }

	    // Queenside rook at a1
	    Piece* rookQueenside = board[0][0];
	    if (rookQueenside && (rookQueenside->type == PieceType::ROOK) && rookQueenside->timesMoved <= 0 && whiteQueenSideCastle) {
	        if (!board[0][1] && !board[0][2] && !board[0][3]) {
	            bool safe = true;
	            for (Piece* enemy : blackAlive) {
	                if (pieceAttacksSquare(enemy, 0, 4) || pieceAttacksSquare(enemy, 0, 3) || pieceAttacksSquare(enemy, 0, 2)) {
	                    safe = false;
	                    break;
	                }
	            }
	            if (safe) {
	                Move queensideCastle(0, 4, 0, 2, PieceType::NONE);
	                queensideCastle.setCastle();
	                Pmoves.add(queensideCastle);
	            }
	        }
	    }
	}

void Board::checkForCastleBlack(Piece* king, MoveList& Pmoves){
	    if (king->timesMoved != 0) {return;}

	    // Kingside rook at h1
	    Piece* rookKingside = board[7][7];
	    if (rookKingside && (rookKingside->type == PieceType::ROOK) && rookKingside->timesMoved == 0 && blackKingSideCastle) {
	        if (!board[7][5] && !board[7][6]) {
	            bool safe = true;
	            for (Piece* enemy : whiteAlive) {
	                if (pieceAttacksSquare(enemy, 7, 4) || pieceAttacksSquare(enemy, 7, 5) || pieceAttacksSquare(enemy, 7, 6)) {
	                    safe = false;
	                    break;
	                }
	            }
	            if (safe) {
	                Move kingsideCastle(7, 4, 7, 6, PieceType::NONE);
	                kingsideCastle.setCastle();
	                Pmoves.add(kingsideCastle);
	            }
	        }
	    }

	    // Queenside rook at a1
	    Piece* rookQueenside = board[7][0];
	    if (rookQueenside && (rookQueenside->type == PieceType::ROOK) && rookQueenside->timesMoved == 0 && blackKingSideCastle) {
	        if (!board[7][1] && !board[7][2] && !board[7][3]) {
	            bool safe = true;
	            for (Piece* enemy : whiteAlive) {
	                if (pieceAttacksSquare(enemy, 7, 4) || pieceAttacksSquare(enemy, 7, 3) || pieceAttacksSquare(enemy, 7, 2)) {
	                    safe = false;
	                    break;
	                }
	            }
	            if (safe) {
	                Move queensideCastle(7, 4, 7, 2, PieceType::NONE);
	                queensideCastle.setCastle();
	                Pmoves.add(queensideCastle);
	            }
	        }
	    }
	}

// Move Legality Functions
bool Board::isMoveLegal(Move& move){
	    makeMove(move);
	    bool inCheckAfterMove = inCheck(1 - playerTurn);
	    undoMove();
	    return !inCheckAfterMove;
	}

bool Board::pieceAttacksSquare(Piece* piece, int kingRow, int kingCol){
		if (piece->type == PieceType::PAWN){       return pawnAttacksKing(piece, kingRow, kingCol);}
	    if (piece->type == PieceType::KNIGHT){     return knightAttacksKing(piece, kingRow, kingCol);}
	    if (piece->type == PieceType::BISHOP){ return bishopAttacksKing(piece, kingRow, kingCol);}
	    if (piece->type == PieceType::ROOK){      return rookAttacksKing(piece, kingRow, kingCol);}
	    if (piece->type == PieceType::QUEEN){      return queenAttacksKing(piece, kingRow, kingCol);}
	    if (piece->type == PieceType::KING){       return kingAttacksKing(piece, kingRow, kingCol);}
	    return false;
	}

bool Board::pawnAttacksKing(Piece* piece, int kingRow, int kingCol){
		if (piece->color == 1){
			return (piece->row + 1 == kingRow && (piece->col + 1 == kingCol || piece->col - 1 == kingCol));
		} 
		else{
			return (piece->row - 1 == kingRow && (piece->col + 1 == kingCol || piece->col - 1 == kingCol));	
		}
	}

bool Board::knightAttacksKing(Piece* piece, int kingRow, int kingCol){
		std::vector<std::pair<int, int>> directions = {{1, -2}, {2, -1}, {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}};
		for(const auto& [r, c] : directions){
			if (piece->row + r == kingRow && piece->col + c == kingCol){return true;}
		}
		return false;
	}

bool Board::kingAttacksKing(Piece* piece, int kingRow, int kingCol){
		std::vector<std::pair<int, int>> directions = {{1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};
		for(const auto& [r, c] : directions){
			if (piece->row + r == kingRow && piece->col + c == kingCol){
				return true;
			}
		}
		return false;
	}

bool Board::queenAttacksKing(Piece* piece, int kingRow, int kingCol){
		std::vector<std::pair<int, int>> directions = {{1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};
		return slideAttacksKing(piece, directions, kingRow, kingCol);
	}

bool Board::rookAttacksKing(Piece* piece, int kingRow, int kingCol){
		std::vector<std::pair<int, int>> directions = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
		return slideAttacksKing(piece, directions, kingRow, kingCol);
	}

bool Board::bishopAttacksKing(Piece* piece, int kingRow, int kingCol){
		return slideAttacksKing(piece, piece->getDirections(), kingRow, kingCol);
	}

bool Board::slideAttacksKing(Piece* piece, const std::vector<std::pair<int, int>>& directions, int kingRow, int kingCol){
		for (const auto& [r, c] : directions) {
	        int multiplier = 1;
	        while (true) {
	            int newRow = piece->row + (r * multiplier);
	            int newCol = piece->col + (c * multiplier);

	            if (newRow < 0 || newRow > 7 || newCol < 0 || newCol > 7){break;}

	            Piece* target = board[newRow][newCol];
	            if (!target) {
	            	if (newRow == kingRow && newCol == kingCol){return true;}
	            	multiplier++;
	            } 
	            else {
	                if (newRow == kingRow && newCol == kingCol){return true;}
	                else{break;}	
	            }
	        }
	    }
	    return false;
	}


// Board State Evaluation Functions    
bool Board::inCheck(int player){
		std::unordered_set<Piece*>& otherTeam = (player == 0) ? whiteAlive : blackAlive;

		Piece* king = (player == 0) ? blackKing : whiteKing;
		int kingRow = king->row;
		int kingCol = king->col;
			
		
		for(Piece* piece : otherTeam){
			if (pieceAttacksSquare(piece, kingRow, kingCol)){
				return true;
			}
		}
		return false;
	}

bool Board::inCheckMate(int player){
		if (inCheck(player)){
			MoveList moves;
			generateLegalMoves(moves);
			return moves.count == 0;
		}
		return false;
	}

bool Board::inStaleMate(int player){
		MoveList moves;
		generateLegalMoves(moves);
		return (!inCheck(player) && moves.count == 0);
	}

bool Board::gameIsOver(){
		return inCheckMate(getTurn()) || inStaleMate(getTurn()) || fiftyMoveCounter >= 50;
	}


// Helper Functions
int Board::getTurn(){
		return playerTurn;
	}

void Board::setTurn(int turn){
		playerTurn = turn;
	}

Piece* Board::makePiece(char c, int row, int col) {
	    char lower = std::tolower(c);
	    int color = isupper(c) ? 1 : 0;

	    switch (lower) {
	        case 'p': return new Pawn(color, row, col);
	        case 'n': return new Knight(color, row, col);
	        case 'b': return new Bishop(color, row, col);
	        case 'r': return new Rook(color, row, col);
	        case 'q': return new Queen(color, row, col);
	        case 'k': return new King(color, row, col);
	        default: return nullptr;
	    }
	}

uint64_t Board::getHash(){
		return currentHash;
	}

std::string Board::getMoveHistory(){
		std::stack<Move> tempStack = playedMoves;  // Copy to preserve original stack
	    std::vector<Move> reversedOrder;

	    while (!tempStack.empty()) {
	        reversedOrder.push_back(tempStack.top());
	        tempStack.pop();
	    }

	    std::reverse(reversedOrder.begin(), reversedOrder.end());

	    std::string result;
	    int moveNumber = 1;

	    for (size_t i = 0; i < reversedOrder.size(); ++i) {
	        if (i % 2 == 0) {
	            result += std::to_string(moveNumber++) + ". ";
	        }
	        result += reversedOrder[i].getMoveRepresentation() + " ";
	    }

	    return result;
	}																					

std::string Board::generateFen(){
		std::ostringstream fen;

	    // 1. Piece placement
	    for (int row = 7; row > -1; --row) {
	        int emptyCount = 0;
	        for (int col = 0; col < 8; ++col) {
	            Piece* piece = board[row][col];
	            if (piece == nullptr) {
	                emptyCount++;
	            } else {
	                if (emptyCount > 0) {
	                    fen << emptyCount;
	                    emptyCount = 0;
	                }
	                fen << piece->getRepresentation();
	            }
	        }
	        if (emptyCount > 0)
	            fen << emptyCount;
	        if (row != 0)
	            fen << '/';
	    }

	    // 2. Active color
	    fen << ' ' << (playerTurn == 1 ? 'w' : 'b');

	    // 3. Castling availability — optional for now, unless you're tracking it
	    std::string castling = "";
	    // Example logic if you track castling rights
	    if (whiteKingSideCastle)  castling += 'K';
	    if (whiteQueenSideCastle) castling += 'Q';
	    if (blackKingSideCastle)  castling += 'k';
	    if (blackQueenSideCastle) castling += 'q';
	    if (castling.empty()) castling = "-";
	    fen << ' ' << castling;

	    // 4. En passant target square
	    if (enPassantSquare.has_value()) {
	        auto [row, col] = enPassantSquare.value();
	        fen << ' ' << (char)('a' + col) << (8 - row);
	    } else {
	        fen << " -";
	    }

	    // 5. Halfmove clock — if you're not tracking it, default to 0
	    fen << " 0";

	    // 6. Fullmove number — if you're not tracking it, default to 1
	    fen << " 1";

	    return fen.str();
	}

void Board::cacheLegalMoves(MoveList& legalMoves){
		moveMap.clear();
		for (int i = 0; i < legalMoves.count; i++){
			moveMap[legalMoves.moves[i].getMoveRepresentation()] = legalMoves.moves[i];
		}
	}


// Private Helper Functions for Zobrist Hashing
void Board::computeZobristHash() {
    	uint64_t hash = 0;
    	for(int i = 0; i < 8; ++i){
    		for(int j = 0; j < 8; ++j){
    			Piece* piece = board[i][j];
    			if (piece){
    				 int pieceIndex = piece->getZobristPieceIndex();
    				 hash ^= zobristTable[pieceIndex][i][j];
    			}
    		}
    	}

    	// Castling rights
	    if (whiteKingSideCastle) hash ^= zobristCastling[0];
	    if (whiteQueenSideCastle) hash ^= zobristCastling[1];
	    if (blackKingSideCastle) hash ^= zobristCastling[2];
	    if (blackQueenSideCastle) hash ^= zobristCastling[3];

	     // En passant file (only if there's a valid en passant square)
	    if (enPassantSquare.has_value()) {
	        int file = enPassantSquare->second;
	        hash ^= zobristEnPassant[file];
	    }

	    // Side to move
	    if (getTurn() == 0) {hash ^= zobristSideToMove;}

	    currentHash = hash;
	}

void Board::updateZobristHash(Move& move) {
	    Piece* movedPiece = board[move.startRow][move.startCol];
	    int pieceIndex = movedPiece->getZobristPieceIndex();

	    // 1. Remove piece from start square
	    currentHash ^= zobristTable[pieceIndex][move.startRow][move.startCol];

	    // 2. Handle captures
	    if (move.isCapture) {
	    	int captureRow;
	    	if (move.isEnPessant) {
	            captureRow = (move.endRow == 5) ? 4 : 3;
	        }
	   		else{
	   			captureRow = move.endRow;
	   		}
	        int captureCol = move.endCol;
			int captureIndex = board[captureRow][move.endCol]->getZobristPieceIndex();
	       
	        currentHash ^= zobristTable[captureIndex][captureRow][captureCol];
	    }

	    // 3. Promotions
	    if (move.isPromotion) {
	        int promotedType = move.promotionZobrist;
	        currentHash ^= zobristTable[promotedType][move.endRow][move.endCol];
	    } else {
	        // 4. Normal piece placed on end square
	        currentHash ^= zobristTable[pieceIndex][move.endRow][move.endCol];
	    }

	    // 5. Castling
	    if (move.isCastle) {
	        if (move.endCol == 6) { // Kingside
	            // XOR rook from f1/f8 and to h1/h8
	            Piece* rook = board[move.startRow][7];
	            int rookIndex = rook->getZobristPieceIndex();

	            currentHash ^= zobristTable[rookIndex][move.endRow][5];
	            currentHash ^= zobristTable[rookIndex][move.endRow][7];
	        } else if (move.endCol == 2) { 
	        	Piece* rook = board[move.startRow][0];
	            int rookIndex = rook->getZobristPieceIndex();

	            currentHash ^= zobristTable[rookIndex][move.endRow][3];
	            currentHash ^= zobristTable[rookIndex][move.endRow][0];
	        }
	    }

	    // 6. En passant square (remove old, add new)
	    if (enPassantSquare->first != -1) {
	        int file = enPassantSquare->second;
	        currentHash ^= zobristEnPassant[file];
	    }

	    if (move.isDoublePawnPush) {
	        int file = move.startCol;
	        currentHash ^= zobristEnPassant[file];
	    }

	    

	    // 8. Side to move
	    currentHash ^= zobristSideToMove; // Flip side
	}



