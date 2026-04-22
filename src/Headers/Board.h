#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>
#include "ChessTypes.h"
#include "Piece.h"

class Piece;

// Everything needed to undo a simulated AI move.
struct UndoInfo {
    sf::Vector2i movedFrom;
    sf::Vector2i movedTo;
    bool         movedHadMoved;

    std::unique_ptr<Piece> capturedPiece;  // nullptr if no capture

    bool         wasCastle = false;
    sf::Vector2i rookFrom;
    sf::Vector2i rookTo;
    bool         rookHadMoved = false;

    bool         wasPromotion = false;
    PieceColor   promotionColor = PieceColor::White;
};

class Board {
public:
    static const int TILE_SIZE = 80;
    static const int BOARD_SIZE = 8;

    Board();

    // Deep copy — clones all pieces. Used by AI to get a private search board.
    Board(const Board& other);

    void init(class TextureManager& textures);
    void promotePawn(sf::Vector2i pos, PieceType newType, sf::Texture* texture);

    // flipped=true draws board from Black's perspective
    void draw(sf::RenderWindow& window, bool flipped = false) const;

    Piece* getPieceAt(sf::Vector2i pos) const;

    // Real game moves
    void movePiece(sf::Vector2i from, sf::Vector2i to);
    void removePiece(sf::Vector2i pos);

    // AI simulation: apply and undo moves on the same board
    UndoInfo makeSimMove(sf::Vector2i from, sf::Vector2i to);
    void     undoSimMove(UndoInfo& undo);

    // pixel → board coord, respecting flip
    sf::Vector2i pixelToBoard(sf::Vector2i pixel, bool flipped = false) const;

    void setSelectedSquare(sf::Vector2i pos);
    void setHighlightedSquares(const std::vector<sf::Vector2i>& squares);
    void clearHighlights();

    const std::vector<std::unique_ptr<Piece>>& getPieces() const;

    const std::vector<std::string>& getMoveHistory() const;
    void addMoveToHistory(const std::string& move);
    void clearMoveHistory();

    const std::vector<Piece*>& getCapturedPieces(PieceColor color) const;

private:
    std::vector<std::unique_ptr<Piece>> pieces;
    std::vector<Piece*> capturedByWhite;
    std::vector<Piece*> capturedByBlack;
    std::vector<std::string> moveHistory;

    sf::Vector2i selectedSquare = { -1, -1 };
    std::vector<sf::Vector2i> highlightedSquares;
    sf::Font font;

    void drawSquares(sf::RenderWindow& window, bool flipped) const;
    void drawLabels(sf::RenderWindow& window, bool flipped) const;
    void drawHighlights(sf::RenderWindow& window, bool flipped) const;
    void drawPieces(sf::RenderWindow& window, bool flipped) const;

    // Convert logical board coord to screen coord given flip setting
    sf::Vector2i toScreen(sf::Vector2i boardPos, bool flipped) const;
};