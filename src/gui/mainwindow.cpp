#include "gui/mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      chessBoard(new Board()),
      selectedRow(-1), selectedCol(-1),
      pieceSelected(false), boardFlipped(false), currentGameMode(GameMode::PLAYER_VS_PLAYER)
{
    setupUI();
    currentGameMode = showGameModeDialog();
    updateBoard();

    if (currentGameMode == GameMode::PLAYER_BLACK_VS_COMPUTER) {
        makeComputerMove();
    }
}

MainWindow::~MainWindow() {
    delete chessBoard;
}

void MainWindow::setupUI() {
    setWindowTitle("Blunder Bot");
    setMinimumSize(600, 700);

    // Create central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Status label
    statusLabel = new QLabel("White to move", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px;");
    mainLayout->addWidget(statusLabel);

    // Chess board widget
    boardWidget = new ChessBoardWidget(this);
    mainLayout->addWidget(boardWidget);

    connect(boardWidget, &ChessBoardWidget::squareClicked,
            this, &MainWindow::onSquareClicked);

// Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    //New Game Button
    QPushButton* newGameBtn = new QPushButton("New Game", this);
    connect(newGameBtn, &QPushButton::clicked, this, &MainWindow::onNewGame);
    buttonLayout->addWidget(newGameBtn);

    //Undo Move Button
    QPushButton* undoBtn = new QPushButton("Undo", this);
    connect(undoBtn, &QPushButton::clicked, this, &MainWindow::onUndoMove);
    buttonLayout->addWidget(undoBtn);

    //Flip Board Button
    QPushButton* flipBtn = new QPushButton("Flip Board", this);
    connect(flipBtn, &QPushButton::clicked, this, &MainWindow::onFlipBoard);
    buttonLayout->addWidget(flipBtn);

    mainLayout->addLayout(buttonLayout);

// Menu bar (Found at Top of Dektop)
    QMenu* gameMenu = menuBar()->addMenu("&Game");
    QAction* newGameAction = gameMenu->addAction("&New Game");
    connect(newGameAction, &QAction::triggered, this, &MainWindow::onNewGame);

    QAction* quitAction = gameMenu->addAction("&Quit");
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::updateBoard() {
    boardWidget->updateFromBoard(chessBoard);

    // Update status (PlayerTurn = 1 then it is whites turn)

    if (chessBoard->getTurn() == 1) {
        statusLabel->setText("White to move");
    } else {
        statusLabel->setText("Black to move");
    }

    // Check for game over conditions
    if (chessBoard->inCheckMate(chessBoard->getTurn())) {

        QString winner = (chessBoard->getTurn() == 1) ? "Black" : "White";
        statusLabel->setText("Checkmate! " + winner + " wins!");
        QMessageBox::information(this, "Game Over", "Checkmate! " + winner + " wins!");

    } else if (chessBoard->inStaleMate(chessBoard->getTurn())) {

        statusLabel->setText("Stalemate - Draw!");
        QMessageBox::information(this, "Game Over", "Stalemate - The game is a draw!");

    } else if (chessBoard->inCheck(chessBoard->getTurn())) {

        statusLabel->setText(statusLabel->text() + " - Check!");
        
    }
}

void MainWindow::onSquareClicked(int row, int col) {
    if (!pieceSelected) {
        // First click - select piece
        Piece* piece = chessBoard->board[row][col];
        if (piece != nullptr && piece->isWhite() == (chessBoard->getTurn() == 1)) {
            pieceSelected = true;
            selectedRow = row;
            selectedCol = col;
            boardWidget->highlightSquare(row, col);
            boardWidget->highlightPossibleMoves(row, col);
        }
    } else {
        // Second click - try to move
        try {
            // Convert to chess notation (e.g., "e2e4")
            char fromFile = 'a' + selectedCol;
            char fromRank = '1' + selectedRow;
            char toFile = 'a' + col;
            char toRank = '1' + row;

            std::string moveStr;
            moveStr += fromFile;
            moveStr += fromRank;
            moveStr += toFile;
            moveStr += toRank;

            // Check if this is a pawn promotion move
            Piece* piece = chessBoard->board[selectedRow][selectedCol];
            if (piece && piece->getType() == PieceType::PAWN) {
                // White pawn reaching row 7 or black pawn reaching row 0
                if ((piece->color == 1 && row == 7) || (piece->color == 0 && row == 0)) {
                    // Show promotion dialog
                    PieceType promotionChoice = showPromotionDialog();
                    
                    // Append promotion letter to move string
                    char promotionChar;
                    switch (promotionChoice) {
                        case PieceType::QUEEN:  promotionChar = 'q'; break;
                        case PieceType::ROOK:   promotionChar = 'r'; break;
                        case PieceType::BISHOP: promotionChar = 'b'; break;
                        case PieceType::KNIGHT: promotionChar = 'n'; break;
                        default: promotionChar = 'q'; break;
                    }
                    moveStr += promotionChar;
                }
            }

            chessBoard->makeMove(moveStr);
            updateBoard();

            if (currentGameMode != GameMode::PLAYER_VS_PLAYER) {
                int computerColor = (currentGameMode == GameMode::PLAYER_WHITE_VS_COMPUTER) ? 0 : 1;
                if (chessBoard->getTurn() == computerColor && !chessBoard->gameIsOver()) {
                    makeComputerMove();
                }
            }

        } catch (const std::exception& e) {
            // Invalid move - just deselect
            //QMessageBox::warning(this, "Invalid Move", QString("Invalid move: ") + e.what());
        }

        // Reset selection
        pieceSelected = false;
        selectedRow = -1;
        selectedCol = -1;
        boardWidget->clearSelection();
    }
}

void MainWindow::onUndoMove() {
    try {
        chessBoard->undoMove();
        pieceSelected = false;
        selectedRow = -1;
        selectedCol = -1;
        boardWidget->clearSelection();
        updateBoard();
    } catch (const std::exception& e) {
        // No moves to undo or error occurred
        QMessageBox::warning(this, "Cannot Undo", "No moves to undo!");
    }
}

PieceType MainWindow::showPromotionDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Pawn Promotion");
    dialog.setModal(true);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QLabel* label = new QLabel("Choose promotion piece:", &dialog);
    layout->addWidget(label);
    
    QComboBox* comboBox = new QComboBox(&dialog);
    comboBox->addItem("Queen", static_cast<int>(PieceType::QUEEN));
    comboBox->addItem("Rook", static_cast<int>(PieceType::ROOK));
    comboBox->addItem("Bishop", static_cast<int>(PieceType::BISHOP));
    comboBox->addItem("Knight", static_cast<int>(PieceType::KNIGHT));
    layout->addWidget(comboBox);
    
    QPushButton* okButton = new QPushButton("OK", &dialog);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(okButton);
    
    dialog.exec();
    
    return static_cast<PieceType>(comboBox->currentData().toInt());
}

void MainWindow::onFlipBoard() {
    boardFlipped = !boardFlipped;
    boardWidget->setFlipped(boardFlipped);
}

void MainWindow::onNewGame() {
    currentGameMode = showGameModeDialog();

    delete chessBoard;
    chessBoard = new Board();
    pieceSelected = false;
    selectedRow = -1;
    selectedCol = -1;
    boardWidget->clearSelection();
    updateBoard();

    if (currentGameMode == GameMode::PLAYER_BLACK_VS_COMPUTER) {
        makeComputerMove();
    }
}

GameMode MainWindow::showGameModeDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("New Game - Select Mode");
    dialog.setModal(true);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QLabel* label = new QLabel("Choose game mode:", &dialog);
    label->setStyleSheet("font-size: 14px; font-weight: bold;");
    layout->addWidget(label);
    
    QComboBox* comboBox = new QComboBox(&dialog);
    comboBox->addItem("Player vs Player", static_cast<int>(GameMode::PLAYER_VS_PLAYER));
    comboBox->addItem("Play as White vs Computer", static_cast<int>(GameMode::PLAYER_WHITE_VS_COMPUTER));
    comboBox->addItem("Play as Black vs Computer", static_cast<int>(GameMode::PLAYER_BLACK_VS_COMPUTER));
    layout->addWidget(comboBox);
    
    QPushButton* okButton = new QPushButton("Start Game", &dialog);
    okButton->setStyleSheet("padding: 8px; font-size: 12px;");
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(okButton);
    
    dialog.exec();
    
    return static_cast<GameMode>(comboBox->currentData().toInt());
}

void MainWindow::makeComputerMove(){
    int depth = 6;
    bool maximizingPlayer = (chessBoard->getTurn() == 1); //True for white, false for black

    std::pair<std::string, float> result = chessSearch::searchBestMoveParallel(chessBoard, depth, maximizingPlayer);
    std::string bestMove = result.first;
    float eval = result.second;

    std::cout << "Computer move: " << bestMove << " (eval: " << eval << ")" << std::endl;

    try {
        MoveList legalMoves;
        chessBoard->generateLegalMoves(legalMoves);

        chessBoard->makeMove(bestMove);
        updateBoard();
    } catch (const std::exception& e) {
        std::cerr << "Computer move failed: " << e.what() << std::endl;
    }
}