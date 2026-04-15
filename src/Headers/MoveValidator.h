#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "ChessTypes.h"

class Board;
class Piece;

class MoveValidator {
public:
    bool isInCheck(const Board& board, PieceColor color) const;
    bool hasLegalMoves(const Board& board, PieceColor color) const;

    std::vector<sf::Vector2i> filterSafeMoves(
        const Board& board,
        Piece* piece,
        const std::vector<sf::Vector2i>& candidates) const;

    std::vector<sf::Vector2i> getCastlingMoves(const Board& board, Piece* king) const;

private:
    // Takes mutable board ref — applies and undoes a move to test for check
    bool wouldBeInCheckAfterMove(Board& board, Piece* piece,
        sf::Vector2i target, PieceColor color) const;

    Piece* findKing(const Board& board, PieceColor color) const;
    bool isSquareAttackedBy(const Board& board, sf::Vector2i square,
        PieceColor attackerColor) const;
};