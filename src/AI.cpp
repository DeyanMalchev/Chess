#include "Headers/AI.h"
#include "Headers/Board.h"
#include "Headers/Piece.h"
#include "Headers/Pieces.h"
#include "Headers/MoveValidator.h"
#include <limits>

AI::AI(PieceColor color, int depth)
    : color(color), depth(depth) {
}

// -------------------------------------------------------
// update — runs the search synchronously on the real board.
// Returns the best move found, or nullopt if no moves exist.
// -------------------------------------------------------
std::optional<Move> AI::update(Board& board, const MoveValidator& validator) {
    return runSearch(board, validator);
}

// -------------------------------------------------------
// runSearch — collects all legal AI moves, evaluates each
// with minimax, and returns the best one.
// -------------------------------------------------------
std::optional<Move> AI::runSearch(Board& board, const MoveValidator& validator) {
    Move bestMove;
    int  bestScore = std::numeric_limits<int>::min();
    bool foundAny = false;

    struct PieceMove { sf::Vector2i from, to; };
    std::vector<PieceMove> allMoves;

    for (auto& p : board.getPieces()) {
        if (p->getColor() != color) continue;

        auto candidates = p->getLegalMoves(board);
        if (p->getType() == PieceType::King) {
            auto castling = validator.getCastlingMoves(board, p.get());
            candidates.insert(candidates.end(), castling.begin(), castling.end());
        }

        // Snapshot the position before filterSafeMoves, which uses
        // makeSimMove/undoSimMove internally.
        sf::Vector2i from = p->getPosition();

        auto safe = validator.filterSafeMoves(board, p.get(), candidates);
        for (auto& to : safe)
            allMoves.push_back({ from, to });
    }

    for (auto& pm : allMoves) {
        auto undo = board.makeSimMove(pm.from, pm.to);

        int score = minimax(board, depth - 1,
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max(),
            false, validator);

        board.undoSimMove(undo);

        if (!foundAny || score > bestScore) {
            bestScore = score;
            bestMove = { pm.from, pm.to };
            foundAny = true;
        }
    }

    if (!foundAny) return std::nullopt;
    return bestMove;
}

// -------------------------------------------------------
// minimax — alpha-beta pruning search on the board.
// Uses raw (unfiltered) moves inside the tree to avoid
// nested makeSimMove/undoSimMove conflicts with filterSafeMoves.
// Illegal moves get a natural penalty: the opponent can
// capture the king next ply, yielding a very bad score.
// -------------------------------------------------------
int AI::minimax(Board& board, int depth, int alpha, int beta,
    bool isMaximizing, const MoveValidator& validator) const
{
    if (depth == 0) return evaluate(board);

    PieceColor sideToMove = isMaximizing ? color
        : (color == PieceColor::White ? PieceColor::Black : PieceColor::White);

    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> moves;
    for (auto& p : board.getPieces()) {
        if (p->getColor() != sideToMove) continue;
        auto raw = p->getLegalMoves(board);
        for (auto& to : raw)
            moves.push_back({ p->getPosition(), to });
    }

    if (moves.empty())
        return isMaximizing ? -50000 : 50000;

    if (isMaximizing) {
        int best = std::numeric_limits<int>::min();
        for (auto& [from, to] : moves) {
            auto undo = board.makeSimMove(from, to);
            best = std::max(best, minimax(board, depth - 1, alpha, beta, false, validator));
            board.undoSimMove(undo);
            alpha = std::max(alpha, best);
            if (beta <= alpha) break;
        }
        return best;
    }
    else {
        int best = std::numeric_limits<int>::max();
        for (auto& [from, to] : moves) {
            auto undo = board.makeSimMove(from, to);
            best = std::min(best, minimax(board, depth - 1, alpha, beta, true, validator));
            board.undoSimMove(undo);
            beta = std::min(beta, best);
            if (beta <= alpha) break;
        }
        return best;
    }
}

int AI::evaluate(const Board& board) const {
    int score = 0;
    for (auto& p : board.getPieces())
        score += (p->getColor() == color ? 1 : -1) * getPieceValue(p->getType());
    return score;
}

int AI::getPieceValue(PieceType type) const {
    switch (type) {
    case PieceType::Pawn:   return 100;
    case PieceType::Knight: return 320;
    case PieceType::Bishop: return 330;
    case PieceType::Rook:   return 500;
    case PieceType::Queen:  return 900;
    case PieceType::King:   return 20000;
    }
    return 0;
}