#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include "ChessTypes.h"

class Board;
class MoveValidator;

struct Move {
    sf::Vector2i from = { 0, 0 };
    sf::Vector2i to = { 0, 0 };
};

class AI {
public:
    AI(PieceColor color, int depth);

    // Runs the search synchronously and returns the best move.
    std::optional<Move> update(Board& board, const MoveValidator& validator);

    bool isThinking() const { return false; }  // kept for API compatibility

private:
    PieceColor color;
    int        depth;

    std::optional<Move> runSearch(Board& board, const MoveValidator& validator);

    int minimax(Board& board, int depth, int alpha, int beta,
        bool isMaximizing, const MoveValidator& validator) const;

    int evaluate(const Board& board) const;
    int getPieceValue(PieceType type) const;
};