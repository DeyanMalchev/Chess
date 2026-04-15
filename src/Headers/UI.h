#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include <vector>
#include "ChessTypes.h"

class Board;

// Which screen we are currently on
enum class GameState {
    Menu,
    PlayingPvP,
    PlayingPvAI,
    AwaitingPromotion,  // waiting for player to pick a promotion piece
    GameOver
};

class UI {
public:
    UI(sf::RenderWindow& window);

    bool loadFont(const std::string& fontPath);

    // --- Menu ---
    void drawMenu() const;

    // Returns GameState::PlayingPvP, PlayingPvAI, or Menu (nothing clicked)
    GameState handleMenuClick(sf::Vector2i mousePos) const;

    // --- In-game panels ---
    // Draw the sidebar: move history, captured pieces, timer
    void drawSidebar(const Board& board,
        float whiteTimeSeconds,
        float blackTimeSeconds,
        PieceColor currentTurn) const;

    // --- Game Over screen ---
    void drawGameOver(const std::string& resultMessage) const;

    // --- Pawn promotion dialog ---
    // Draws a box asking the player to pick a piece
    void drawPromotionDialog(PieceColor color) const;

    // Returns PieceType if a promotion option was clicked, or nullopt if not
    std::optional<PieceType> handlePromotionClick(sf::Vector2i mousePos, PieceColor color) const;

private:
    sf::RenderWindow& window;
    sf::Font font;

    // Menu button rects — stored so handleMenuClick can hit-test them
    sf::FloatRect pvpButtonRect;
    sf::FloatRect pvaiButtonRect;

    // Helpers
    void drawButton(const std::string& label, sf::FloatRect rect, sf::Color color) const;
    void drawMoveHistory(const std::vector<std::string>& history, sf::Vector2f origin) const;
    void drawCapturedPieces(const std::vector<class Piece*>& pieces,
        PieceColor side, sf::Vector2f origin) const;
    void drawTimer(float seconds, bool isActive, sf::Vector2f origin) const;
};