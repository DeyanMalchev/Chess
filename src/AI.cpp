#include "Headers/AI.h"
#include "Headers/Board.h"
#include "Headers/Piece.h"
#include "Headers/Pieces.h"
#include "Headers/MoveValidator.h"
#include <limits>

AI::AI(PieceColor color, int depth)
    : color(color), depth(depth) {
}

AI::~AI() {
    if (searchThread.joinable())
        searchThread.join();
}

// -------------------------------------------------------
// update � called every frame from Game::update()
// -------------------------------------------------------
std::optional<Move> AI::update(Board& board, const MoveValidator& validator) {
    if (!thinking.load()) {
        // Check if there's a result ready
        std::lock_guard<std::mutex> lock(resultMutex);
        if (result.has_value()) {
            Move best = *result;
            result = std::nullopt;
            return best;
        }

        // Start a new search on a COPY of the board
        // The copy is made here on the main thread (board is safe to read right now)
        // and ownership is transferred to the search thread.
        if (searchThread.joinable())
            searchThread.join();

        thinking.store(true);

        // Clone the board � the thread owns this copy and never touches the real board
        Board* boardCopy = new Board(board);
        searchThread = std::thread(&AI::runSearch, this, boardCopy, std::ref(validator));
    }
    return std::nullopt;
}

// -------------------------------------------------------
// runSearch � runs on the background thread
//
// Works entirely on boardCopy � the real board is never touched.
// Once done, writes the best move to result and signals thinking=false.
// -------------------------------------------------------
void AI::runSearch(Board* boardCopy, const MoveValidator& validator) {
    Move bestMove;
    int  bestScore = std::numeric_limits<int>::min();
    bool foundAny = false;

    // Collect all legal moves for AI pieces on the copy
    struct PieceMove { sf::Vector2i from, to; };
    std::vector<PieceMove> allMoves;

    for (auto& p : boardCopy->getPieces()) {
        if (p->getColor() != color) continue;

        auto candidates = p->getLegalMoves(*boardCopy);
        if (p->getType() == PieceType::King) {
            auto castling = validator.getCastlingMoves(*boardCopy, p.get());
            candidates.insert(candidates.end(), castling.begin(), castling.end());
        }

        // Snapshot the position BEFORE filterSafeMoves, because it calls
        // makeSimMove/undoSimMove internally and can transiently alter the
        // piece's position field even after the undo restores it.
        sf::Vector2i from = p->getPosition();

        // Filter safe moves once here, not inside minimax
        auto safe = validator.filterSafeMoves(*boardCopy, p.get(), candidates);
        for (auto& to : safe)
            allMoves.push_back({ from, to });
    }

    for (auto& pm : allMoves) {
        // Apply move on the copy
        auto undo = boardCopy->makeSimMove(pm.from, pm.to);

        int score = minimax(*boardCopy, depth - 1,
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max(),
            false, validator);

        boardCopy->undoSimMove(undo);

        if (!foundAny || score > bestScore) {
            bestScore = score;
            bestMove = { pm.from, pm.to };
            foundAny = true;
        }
    }

    delete boardCopy;

    {
        std::lock_guard<std::mutex> lock(resultMutex);
        if (foundAny) result = bestMove;
    }
    thinking.store(false);
}

// -------------------------------------------------------
// minimax � operates entirely on the copied board
//
// Key improvement: moves are collected and filtered ONCE
// per node. filterSafeMoves uses make-unmake internally
// but only on the copy, never the real board.
// -------------------------------------------------------
int AI::minimax(Board& board, int depth, int alpha, int beta,
    bool isMaximizing, const MoveValidator& validator) const
{
    if (depth == 0) return evaluate(board);

    PieceColor sideToMove = isMaximizing ? color
        : (color == PieceColor::White ? PieceColor::Black : PieceColor::White);

    // Use RAW moves inside minimax � no filterSafeMoves.
    // filterSafeMoves calls makeSimMove internally which corrupts the
    // board state when nested inside our own makeSimMove calls.
    // Instead, illegal moves (leaving king in check) get a huge penalty
    // score naturally because the opponent can capture the king next move.
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> moves;
    for (auto& p : board.getPieces()) {
        if (p->getColor() != sideToMove) continue;
        auto raw = p->getLegalMoves(board);
        for (auto& to : raw)
            moves.push_back({ p->getPosition(), to });
    }

    if (moves.empty())
        return isMaximizing ? -50000 : 50000;  // no moves � bad position

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