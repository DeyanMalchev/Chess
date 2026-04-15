#pragma once

#include <SFML/Graphics.hpp>
#include <thread>
#include <atomic>
#include <mutex>
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
    ~AI();

    // Called once per frame. Starts search if idle, returns move when done.
    std::optional<Move> update(Board& board, const MoveValidator& validator);

    bool isThinking() const { return thinking.load(); }

private:
    PieceColor        color;
    int               depth; 

    std::thread       searchThread;
    std::atomic<bool> thinking{ false };

    std::mutex        resultMutex;
    std::optional<Move> result;

    void runSearch(Board* boardCopy, const MoveValidator& validator);

    int minimax(Board& board, int depth, int alpha, int beta,
        bool isMaximizing, const MoveValidator& validator) const;

    int evaluate(const Board& board) const;
    int getPieceValue(PieceType type) const;
};