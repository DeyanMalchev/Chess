#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <optional>
#include "ChessTypes.h"

class Board;

class Piece {
public:
    Piece(PieceType type, PieceColor color, sf::Vector2i position, sf::Texture* texture);
    virtual ~Piece() = default;

    virtual std::vector<sf::Vector2i> getLegalMoves(const Board& board) const = 0;

    // Clone is needed by Board's copy constructor for AI simulation
    virtual std::unique_ptr<Piece> clone() const = 0;

    void draw(sf::RenderWindow& window, int tileSize) const;
    void drawWithOffset(sf::RenderWindow& window, int tileSize, int offset) const;

    PieceType    getType()     const;
    PieceColor   getColor()    const;
    sf::Vector2i getPosition() const;
    bool         getHasMoved() const;

    void setPosition(sf::Vector2i newPos);
    void setHasMoved(bool moved);

protected:
    PieceType    type;
    PieceColor   color;
    sf::Vector2i position;
    sf::Texture* texture;
    std::optional<sf::Sprite> sprite;
    bool hasMoved = false;
};