#include "Headers/Piece.h"

Piece::Piece(PieceType type, PieceColor color, sf::Vector2i position, sf::Texture* texture)
    : type(type), color(color), position(position), texture(texture)
{
    if (texture) {
        // SFML 3: sf::Sprite must be constructed with a texture — no default constructor
        sprite.emplace(*texture);

        sf::Vector2u texSize = texture->getSize();
        float scaleX = 80.f / texSize.x;
        float scaleY = 80.f / texSize.y;
        sprite->setScale({ scaleX, scaleY });
    }
}

void Piece::draw(sf::RenderWindow& window, int tileSize) const {
    drawWithOffset(window, tileSize, 0);
}

void Piece::drawWithOffset(sf::RenderWindow& window, int tileSize, int offset) const {
    if (!sprite) return;

    sf::Sprite s = *sprite;  // copy the optional's value
    s.setPosition({
        static_cast<float>(offset + position.x * tileSize),
        static_cast<float>(offset + position.y * tileSize)
        });
    window.draw(s);
}

// --- Getters ---

PieceType Piece::getType() const {
    return type;
}

PieceColor Piece::getColor() const {
    return color;
}

sf::Vector2i Piece::getPosition() const {
    return position;
}

bool Piece::getHasMoved() const {
    return hasMoved;
}

// --- Setters ---

void Piece::setPosition(sf::Vector2i newPos) {
    position = newPos;
}

void Piece::setHasMoved(bool moved) {
    hasMoved = moved;
} 