#include "Headers/MoveValidator.h"
#include "Headers/Board.h"
#include "Headers/Piece.h"
#include "Headers/Pieces.h"

bool MoveValidator::isInCheck(const Board& board, PieceColor color) const {
    Piece* king = findKing(board, color);
    if (!king) return false;
    PieceColor enemy = (color == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    return isSquareAttackedBy(board, king->getPosition(), enemy);
}

bool MoveValidator::isSquareAttackedBy(const Board& board, sf::Vector2i square, PieceColor attackerColor) const {
    for (auto& p : board.getPieces()) {
        if (p->getColor() != attackerColor) continue;
        for (auto& move : p->getLegalMoves(board))
            if (move == square) return true;
    }
    return false;
}

// Use make-unmake instead of const_cast simulation
bool MoveValidator::wouldBeInCheckAfterMove(Board& board, Piece* piece, sf::Vector2i target, PieceColor color) const {
    auto undo = board.makeSimMove(piece->getPosition(), target);
    bool inCheck = isInCheck(board, color);
    board.undoSimMove(undo);
    return inCheck;
}

std::vector<sf::Vector2i> MoveValidator::filterSafeMoves(const Board& board, Piece* piece,
    const std::vector<sf::Vector2i>& candidates) const
{
    std::vector<sf::Vector2i> safe;
    Board& mutableBoard = const_cast<Board&>(board);  // safe — always undone

    for (auto& target : candidates) {
        if (!wouldBeInCheckAfterMove(mutableBoard, piece, target, piece->getColor()))
            safe.push_back(target);
    }
    return safe;
}

bool MoveValidator::hasLegalMoves(const Board& board, PieceColor color) const {
    for (auto& p : board.getPieces()) {
        if (p->getColor() != color) continue;

        auto candidates = p->getLegalMoves(board);
        if (p->getType() == PieceType::King) {
            auto castling = getCastlingMoves(board, p.get());
            candidates.insert(candidates.end(), castling.begin(), castling.end());
        }

        if (!filterSafeMoves(board, p.get(), candidates).empty())
            return true;
    }
    return false;
}

std::vector<sf::Vector2i> MoveValidator::getCastlingMoves(const Board& board, Piece* king) const {
    std::vector<sf::Vector2i> moves;
    if (king->getHasMoved()) return moves;
    if (isInCheck(board, king->getColor())) return moves;

    int row = king->getPosition().y;
    PieceColor color = king->getColor();
    PieceColor enemy = (color == PieceColor::White) ? PieceColor::Black : PieceColor::White;

    // Kingside
    {
        Piece* rook = board.getPieceAt({ 7, row });
        if (rook && rook->getType() == PieceType::Rook
            && rook->getColor() == color && !rook->getHasMoved()) {
            bool clear = !board.getPieceAt({ 5, row }) && !board.getPieceAt({ 6, row });
            bool safe = !isSquareAttackedBy(board, { 5, row }, enemy)
                && !isSquareAttackedBy(board, { 6, row }, enemy);
            if (clear && safe) moves.push_back({ 6, row });
        }
    }

    // Queenside
    {
        Piece* rook = board.getPieceAt({ 0, row });
        if (rook && rook->getType() == PieceType::Rook
            && rook->getColor() == color && !rook->getHasMoved()) {
            bool clear = !board.getPieceAt({ 1, row })
                && !board.getPieceAt({ 2, row })
                && !board.getPieceAt({ 3, row });
            bool safe = !isSquareAttackedBy(board, { 3, row }, enemy)
                && !isSquareAttackedBy(board, { 2, row }, enemy);
            if (clear && safe) moves.push_back({ 2, row });
        }
    }

    return moves;
}

Piece* MoveValidator::findKing(const Board& board, PieceColor color) const {
    for (auto& p : board.getPieces())
        if (p->getColor() == color && p->getType() == PieceType::King)
            return p.get();
    return nullptr;
}