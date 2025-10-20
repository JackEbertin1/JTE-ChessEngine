#include <SFML/Graphics.hpp> //Using 3.0.2
#include <optional>

int main() {
    const int SIZE = 8;
    const int TILE_SIZE = 100;

    // SFML 3.x: construct window, then create with correct VideoMode
    sf::RenderWindow window(sf::VideoMode({SIZE * TILE_SIZE, SIZE * TILE_SIZE}), "Chess Board");

    sf::Texture whitePawnTex, whiteRookTex, whiteKnightTex, whiteBishopTex, whiteQueenTex, whiteKingTex;
    sf::Texture blackPawnTex, blackRookTex, blackKnightTex, blackBishopTex, blackQueenTex, blackKingTex;

    whitePawnTex.loadFromFile("assets/whitePawn.png");
    whiteRookTex.loadFromFile("assets/whiteRook.png");
    whiteKnightTex.loadFromFile("assets/whiteKnight.png");
    whiteBishopTex.loadFromFile("assets/whiteBishop.png");
    whiteKingTex.loadFromFile("assets/whiteKing.png");
    whiteRookTex.loadFromFile("assets/whiteQueen.png");

    blackPawnTex.loadFromFile("assets/blackPawn.png");
    blackRookTex.loadFromFile("assets/blackRook.png");
    blackKnightTex.loadFromFile("assets/blackKnight.png");
    blackBishopTex.loadFromFile("assets/blackBishop.png");
    blackKingTex.loadFromFile("assets/blackKing.png");
    blackRookTex.loadFromFile("assets/blackQueen.png");


    while (window.isOpen()) {
        // pollEvent() now returns std::optional<Event>
        std::optional<sf::Event> maybeEvent;
        while ((maybeEvent = window.pollEvent())) {
            auto& event = *maybeEvent;
            // event is a variant; use holds_alternative or visit
             if (event.is<sf::Event::Closed>())
                window.close();
        }

        window.clear();

        // Draw chessboard
        for (int row = 0; row < SIZE; ++row) {
            for (int col = 0; col < SIZE; ++col) {
                sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                tile.setPosition(sf::Vector2f(col * TILE_SIZE, row * TILE_SIZE));
                tile.setFillColor(((row + col) % 2 == 0) ? sf::Color::White : sf::Color::Black);
                window.draw(tile);
            }
        }

        window.display();
    }

    return 0;
}