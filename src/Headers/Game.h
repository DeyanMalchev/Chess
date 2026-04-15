#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include "Board.h"
#include "MoveValidator.h"
#include "AI.h"
#include "UI.h"
#include "TextureManager.h"
#include "ChessTypes.h"

class Game {
public:
    Game();
    void run();

private:
    sf::RenderWindow window;

    Board          board;
    MoveValidator  validator;
    UI             ui;
    TextureManager textures;

    // AI — nullptr in PvP mode, created on startGame for PvAI
    AI* ai = nullptr;

    GameState    gameState = GameState::Menu;
    PieceColor   currentTurn = PieceColor::White;
    PieceColor   aiColor = PieceColor::Black;  // AI always plays black by default

    class Piece* selectedPiece = nullptr;
    std::vector<sf::Vector2i> legalMovesCache;

    // Pawn promotion — stores the square of the pawn waiting for promotion
    std::optional<sf::Vector2i> pendingPromotionSquare;
    GameState stateBeforePromotion = GameState::PlayingPvP;

    float whiteTime = 600.f;
    float blackTime = 600.f;
    sf::Clock clock;

    std::string gameOverMessage;

    // Main loop
    void processEvents();
    void update(float deltaTime);
    void render();

    // Input
    void handleMenuClick(sf::Vector2i& mousePos);
    void handleBoardClick(sf::Vector2i& mousePos);
    void handlePromotionClick(sf::Vector2i& mousePos);

    // Game logic
    void selectPiece(class Piece* piece);
    void makeMove(sf::Vector2i from, sf::Vector2i to);
    void switchTurn();
    void checkGameOver();
    void triggerAIMove();

    void startGame(GameState mode);
};