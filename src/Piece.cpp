#include "Headers/Piece.h"

Piece::Piece(PieceType type, PieceColor color, sf::Vector2i position, sf::Texture* texture)
    : type(type), color(color), position(position), texture(texture)
{
    if (texture) {
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
    drawAtScreen(window, tileSize, offset, position);
}

void Piece::drawAtScreen(sf::RenderWindow& window, int tileSize, int offset,
    sf::Vector2i screenPos) const {
    if (!sprite) return;
    sf::Sprite s = *sprite;
    s.setPosition({
        static_cast<float>(offset + screenPos.x * tileSize),
        static_cast<float>(offset + screenPos.y * tileSize)
        });
    window.draw(s);
}

PieceType    Piece::getType()     const { return type; }
PieceColor   Piece::getColor()    const { return color; }
sf::Vector2i Piece::getPosition() const { return position; }
bool         Piece::getHasMoved() const { return hasMoved; }

void Piece::setPosition(sf::Vector2i newPos) { position = newPos; }
void Piece::setHasMoved(bool moved) { hasMoved = moved; }