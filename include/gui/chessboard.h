#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <vector>

#include "chess/rep/Board.h"



class ChessBoardWidget : public QGraphicsView {
    Q_OBJECT

public:
    explicit ChessBoardWidget(QWidget *parent = nullptr);
    ~ChessBoardWidget();

    void updateFromBoard(Board* board);
    void highlightSquare(int row, int col);
    void highlightPossibleMoves(int row, int col);
    void clearSelection();
    void setFlipped(bool flipped);

signals:
    void squareClicked(int row, int col);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawBoard();
    void drawPieces();
    QGraphicsTextItem* createPieceItem(Piece* piece);
    QString getPieceSymbol(Piece* piece);

    QGraphicsScene* scene;
    Board* currentBoard;

    static constexpr int BOARD_SIZE = 8;
    static constexpr double SQUARE_SIZE = 60.0;

    std::vector<std::vector<QGraphicsRectItem*>> squares;
    std::vector<QGraphicsTextItem*> pieceItems;
    std::vector<QGraphicsEllipseItem*> moveIndicators;

    int highlightedRow;
    int highlightedCol;
    bool isFlipped;
};

#endif // CHESSBOARD_H
