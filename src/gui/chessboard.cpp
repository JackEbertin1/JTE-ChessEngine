#include "gui/chessboard.h"
#include <QMouseEvent>
#include <QBrush>
#include <QPen>
#include <QFont>

ChessBoardWidget::ChessBoardWidget(QWidget *parent)
    : QGraphicsView(parent),
      scene(new QGraphicsScene(this)), currentBoard(nullptr),
      highlightedRow(-1), highlightedCol(-1), isFlipped(false)
{
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);
    setMinimumSize(SQUARE_SIZE * BOARD_SIZE, SQUARE_SIZE * BOARD_SIZE);

    // Initialize squares vector
    squares.resize(BOARD_SIZE);
    for (int i = 0; i < BOARD_SIZE; ++i) {
        squares[i].resize(BOARD_SIZE);
    }

    drawBoard();
}

ChessBoardWidget::~ChessBoardWidget() {
}

void ChessBoardWidget::drawBoard() {
    scene->clear();
    pieceItems.clear();

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            int displayRow = isFlipped ? row : (BOARD_SIZE - 1 - row);
            int displayCol = isFlipped ? (BOARD_SIZE - 1 - col) : col;
            
            QGraphicsRectItem* square = scene->addRect(
                displayCol * SQUARE_SIZE,
                displayRow * SQUARE_SIZE,
                SQUARE_SIZE,
                SQUARE_SIZE
            );

            // Checkerboard pattern
            if ((row + col) % 2 == 1) {
                square->setBrush(QBrush(QColor(240, 217, 181))); // Light square
            } else {
                square->setBrush(QBrush(QColor(181, 136, 99))); // Dark square
            }

            square->setPen(QPen(Qt::NoPen));
            squares[row][col] = square;
        }
    }

    setSceneRect(0, 0, SQUARE_SIZE * BOARD_SIZE, SQUARE_SIZE * BOARD_SIZE);
}

void ChessBoardWidget::drawPieces() {
    // Clear old pieces
    for (auto* item : pieceItems) {
        scene->removeItem(item);
        delete item;
    }
    pieceItems.clear();

    if (!currentBoard) return;

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            Piece* piece = currentBoard->board[row][col];
            if (piece != nullptr) {
                // This creates a piece and then places it on the board in the correct square
                QGraphicsTextItem* pieceItem = createPieceItem(piece);
                int displayRow = isFlipped ? row : (BOARD_SIZE - 1 - row);
                int displayCol = isFlipped ? (BOARD_SIZE - 1 - col) : col;
                
                pieceItem->setPos(
                    displayCol * SQUARE_SIZE + (SQUARE_SIZE - pieceItem->boundingRect().width()) / 2,
                    displayRow * SQUARE_SIZE + (SQUARE_SIZE - pieceItem->boundingRect().height()) / 2
                );

                scene->addItem(pieceItem);
                pieceItems.push_back(pieceItem);
            }
        }
    }
}

QGraphicsTextItem* ChessBoardWidget::createPieceItem(Piece* piece) {
    QGraphicsTextItem* item = new QGraphicsTextItem(getPieceSymbol(piece));
    QFont font("Arial", 64);
    item->setFont(font);

    if (piece->isWhite()) {
        item->setDefaultTextColor(Qt::white);
    } else {
        item->setDefaultTextColor(Qt::black);
    }

    return item;
}

QString ChessBoardWidget::getPieceSymbol(Piece* piece) {
    if (!piece) return "";

    // Unicode chess symbols
    bool isWhite = (piece->color == 1);

    switch (piece->getType()) {
        case PieceType::KING:   return isWhite ? QString::fromUtf8("\u2654") : QString::fromUtf8("\u265A");
        case PieceType::QUEEN:  return isWhite ? QString::fromUtf8("\u2655") : QString::fromUtf8("\u265B");
        case PieceType::ROOK:   return isWhite ? QString::fromUtf8("\u2656") : QString::fromUtf8("\u265C");
        case PieceType::BISHOP: return isWhite ? QString::fromUtf8("\u2657") : QString::fromUtf8("\u265D");
        case PieceType::KNIGHT: return isWhite ? QString::fromUtf8("\u2658") : QString::fromUtf8("\u265E");
        case PieceType::PAWN:   return isWhite ? QString::fromUtf8("\u2659") : QString::fromUtf8("\u265F");
        default:     return "";
    }
}

void ChessBoardWidget::updateFromBoard(Board* board) {
    currentBoard = board;

    // Clear move indicators when board updates
    for (auto* indicator : moveIndicators) {
        scene->removeItem(indicator);
        delete indicator;
    }
    moveIndicators.clear();

    drawBoard();
    drawPieces();
}

void ChessBoardWidget::highlightSquare(int row, int col) {
    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
        highlightedRow = row;
        highlightedCol = col;
        squares[row][col]->setBrush(QBrush(QColor(255, 255, 0, 120))); // Yellow highlight
    }
}

void ChessBoardWidget::highlightPossibleMoves(int row, int col) {
    // Clear any existing move indicators
    for (auto* indicator : moveIndicators) {
        scene->removeItem(indicator);
        delete indicator;
    }
    moveIndicators.clear();

    if (!currentBoard) return;

    Piece* piece = currentBoard->board[row][col];
    if (!piece) return;

    // Generate legal moves for this piece
    MoveList moves;
    currentBoard->generateMovesForPiece(piece, moves);

    // Draw a dot on each legal move square
    for (int i = 0; i < moves.count; i++) {
        currentBoard->makeMove(moves[i]);
        if(currentBoard->inCheck(1 - currentBoard->getTurn())){
            currentBoard->undoMove();
            continue;
        }
        currentBoard->undoMove();
        int toRow = moves[i].endRow;
        int toCol = moves[i].endCol;

        // Create circular indicator (dot)
        double dotRadius = SQUARE_SIZE / 6;
        int displayRow = isFlipped ? toRow : (BOARD_SIZE - 1 - toRow);
        int displayCol = isFlipped ? (BOARD_SIZE - 1 - toCol) : toCol;
        double centerX = displayCol * SQUARE_SIZE + SQUARE_SIZE / 2 - dotRadius;
        double centerY = displayRow * SQUARE_SIZE + SQUARE_SIZE / 2 - dotRadius;

        QGraphicsEllipseItem* dot = scene->addEllipse(
            centerX, centerY, dotRadius * 2, dotRadius * 2,
            QPen(Qt::NoPen),
            QBrush(QColor(100, 100, 100, 120))  // Semi-transparent gray
        );

        moveIndicators.push_back(dot);
    }
}

void ChessBoardWidget::clearSelection() {
    if (highlightedRow >= 0 && highlightedCol >= 0) {
        // Restore original color
        if ((highlightedRow + highlightedCol) % 2 == 1) {
            squares[highlightedRow][highlightedCol]->setBrush(QBrush(QColor(240, 217, 181)));
        } else {
            squares[highlightedRow][highlightedCol]->setBrush(QBrush(QColor(181, 136, 99)));
        }
    }
    highlightedRow = -1;
    highlightedCol = -1;

    // Clear move indicators
    for (auto* indicator : moveIndicators) {
        scene->removeItem(indicator);
        delete indicator;
    }
    moveIndicators.clear();
}

void ChessBoardWidget::mousePressEvent(QMouseEvent *event) {
    QPointF scenePos = mapToScene(event->pos());

    int displayCol = static_cast<int>(scenePos.x() / SQUARE_SIZE);
    int displayRow = static_cast<int>(scenePos.y() / SQUARE_SIZE);
    
    int col = isFlipped ? (BOARD_SIZE - 1 - displayCol) : displayCol;
    int row = isFlipped ? displayRow : (BOARD_SIZE - 1 - displayRow);

    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
        emit squareClicked(row, col);
    }

    QGraphicsView::mousePressEvent(event);
}


void ChessBoardWidget::resizeEvent(QResizeEvent *event) {
    fitInView(sceneRect(), Qt::KeepAspectRatio);
    QGraphicsView::resizeEvent(event);
}

void ChessBoardWidget::setFlipped(bool flipped) {
    isFlipped = flipped;
    updateFromBoard(currentBoard);
}