#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include <vector>
#include "ChessTypes.h"

class Board;

enum class GameState {
    Menu,
    ChoosingSide,       // new: pick White or Black before PvAI starts
    PlayingPvP,
    PlayingPvAI,
    AwaitingPromotion,
    GameOver
};

class UI {
public:
    UI(sf::RenderWindow& window);

    bool loadFont(const std::string& fontPath);

    // --- Menu ---
    void drawMenu() const;
    // Returns PlayingPvP, PlayingPvAI, or Menu (nothing clicked)
    GameState handleMenuClick(sf::Vector2i mousePos) const;

    // --- Side selection (PvAI only) ---
    void drawSideSelection() const;
    // Returns White, Black, or nullopt (nothing clicked)
    std::optional<PieceColor> handleSideSelectionClick(sf::Vector2i mousePos) const;

    // --- In-game sidebar ---
    void drawSidebar(const Board& board,
        float whiteTimeSeconds,
        float blackTimeSeconds,
        PieceColor currentTurn) const;

    // --- Game Over ---
    void drawGameOver(const std::string& resultMessage) const;

    // --- Pawn promotion dialog ---
    void drawPromotionDialog(PieceColor color) const;
    std::optional<PieceType> handlePromotionClick(sf::Vector2i mousePos, PieceColor color) const;

private:
    sf::RenderWindow& window;
    sf::Font font;

    sf::FloatRect pvpButtonRect;
    sf::FloatRect pvaiButtonRect;
    sf::FloatRect whiteButtonRect;
    sf::FloatRect blackButtonRect;

    void drawButton(const std::string& label, sf::FloatRect rect, sf::Color color) const;
    void drawMoveHistory(const std::vector<std::string>& history, sf::Vector2f origin) const;
    void drawCapturedPieces(const std::vector<class Piece*>& pieces,
        PieceColor side, sf::Vector2f origin) const;
    void drawTimer(float seconds, bool isActive, sf::Vector2f origin) const;
};