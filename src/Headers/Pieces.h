#pragma once

#include "Piece.h"

// Each subclass implements getLegalMoves() and clone()
// clone() is used by Board's copy constructor for AI simulation

class Pawn : public Piece {
public:
    Pawn(PieceColor color, sf::Vector2i position, sf::Texture* texture);
    std::vector<sf::Vector2i> getLegalMoves(const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
};

class Rook : public Piece {
public:
    Rook(PieceColor color, sf::Vector2i position, sf::Texture* texture);
    std::vector<sf::Vector2i> getLegalMoves(const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
};

class Knight : public Piece {
public:
    Knight(PieceColor color, sf::Vector2i position, sf::Texture* texture);
    std::vector<sf::Vector2i> getLegalMoves(const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
};

class Bishop : public Piece {
public:
    Bishop(PieceColor color, sf::Vector2i position, sf::Texture* texture);
    std::vector<sf::Vector2i> getLegalMoves(const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
};

class Queen : public Piece {
public:
    Queen(PieceColor color, sf::Vector2i position, sf::Texture* texture);
    std::vector<sf::Vector2i> getLegalMoves(const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
};

class King : public Piece {
public:
    King(PieceColor color, sf::Vector2i position, sf::Texture* texture);
    std::vector<sf::Vector2i> getLegalMoves(const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
};