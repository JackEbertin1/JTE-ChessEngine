#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialog>
#include <QComboBox>
#include <memory>

#include "chess/search/chessSearch.h" 
#include "chessboard.h"

enum class GameMode {
    PLAYER_VS_PLAYER,
    PLAYER_WHITE_VS_COMPUTER,
    PLAYER_BLACK_VS_COMPUTER
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewGame();
    void onSquareClicked(int row, int col);
    void onUndoMove();
    void onFlipBoard();

private:

/*
SetupUI is called at start of application, creates main game window with title "Blunder Bot", establishes chessBoard widget
and adds buttons for newGame and quitting application
*/
    void setupUI();

/*
Calls updateFromBoard() which natively updates chess board dispaly, then handles surrounding game status messages
*/
    void updateBoard();

/*
Allows the user to choose which piece type to promote a pawn to
*/    
    PieceType showPromotionDialog();
    GameMode showGameModeDialog();

/*
Makes a move by calling SearchBestMoveParallel() with a given depth and updating board based on result
*/
     void makeComputerMove();
    

    Board* chessBoard;                  //Actual Backend game tracker
    ChessBoardWidget* boardWidget;      //Board rep for gui
    QLabel* statusLabel;

    int selectedRow;
    int selectedCol;
    bool pieceSelected;
    bool boardFlipped;

    GameMode currentGameMode;
};

#endif // MAINWINDOW_H
