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

    AI* ai = nullptr;

    GameState    gameState = GameState::Menu;
    PieceColor   currentTurn = PieceColor::White;

    // In PvAI: the color the human chose; AI takes the opposite.
    PieceColor   playerColor = PieceColor::White;
    PieceColor   aiColor = PieceColor::Black;

    // true when the board should be drawn from Black's perspective
    bool boardFlipped = false;

    // Pending PvAI mode — stored between Menu and ChoosingSide
    GameState pendingMode = GameState::PlayingPvAI;

    class Piece* selectedPiece = nullptr;
    std::vector<sf::Vector2i> legalMovesCache;

    std::optional<sf::Vector2i> pendingPromotionSquare;
    GameState stateBeforePromotion = GameState::PlayingPvP;

    float whiteTime = 600.f;
    float blackTime = 600.f;
    sf::Clock clock;

    std::string gameOverMessage;

    void processEvents();
    void update(float deltaTime);
    void render();

    void handleMenuClick(sf::Vector2i& mousePos);
    void handleSideSelectionClick(sf::Vector2i& mousePos);
    void handleBoardClick(sf::Vector2i& mousePos);
    void handlePromotionClick(sf::Vector2i& mousePos);

    void selectPiece(class Piece* piece);
    void makeMove(sf::Vector2i from, sf::Vector2i to);
    void switchTurn();
    void checkGameOver();
    void triggerAIMove();

    void startGame(GameState mode, PieceColor humanColor = PieceColor::White);
};