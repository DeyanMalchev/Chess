#include "Headers/Pieces.h"
#include "Headers/Board.h"

// -------------------------------------------------------
// Constructors
// -------------------------------------------------------

Pawn::Pawn(PieceColor color, sf::Vector2i position, sf::Texture* texture)
    : Piece(PieceType::Pawn, color, position, texture) {
}

Rook::Rook(PieceColor color, sf::Vector2i position, sf::Texture* texture)
    : Piece(PieceType::Rook, color, position, texture) {
}

Knight::Knight(PieceColor color, sf::Vector2i position, sf::Texture* texture)
    : Piece(PieceType::Knight, color, position, texture) {
}

Bishop::Bishop(PieceColor color, sf::Vector2i position, sf::Texture* texture)
    : Piece(PieceType::Bishop, color, position, texture) {
}

Queen::Queen(PieceColor color, sf::Vector2i position, sf::Texture* texture)
    : Piece(PieceType::Queen, color, position, texture) {
}

King::King(PieceColor color, sf::Vector2i position, sf::Texture* texture)
    : Piece(PieceType::King, color, position, texture) {
}

// -------------------------------------------------------
// Helper — is this square inside the 8x8 board?
// -------------------------------------------------------
static bool inBounds(sf::Vector2i pos) {
    return pos.x >= 0 && pos.x < 8 && pos.y >= 0 && pos.y < 8;
}

// -------------------------------------------------------
// PAWN
// - Moves forward 1 square (white = y-1, black = y+1)
// - Moves forward 2 squares on first move if both squares empty
// - Captures diagonally forward only
// - Cannot move forward into a blocked square
// -------------------------------------------------------
std::vector<sf::Vector2i> Pawn::getLegalMoves(const Board& board) const {
    std::vector<sf::Vector2i> moves;

    int dir = (color == PieceColor::White) ? -1 : 1;

    sf::Vector2i oneStep = { position.x, position.y + dir };
    sf::Vector2i twoStep = { position.x, position.y + dir * 2 };

    // One step forward — only if square is empty
    if (inBounds(oneStep) && board.getPieceAt(oneStep) == nullptr) {
        moves.push_back(oneStep);

        // Two steps — only on first move and intermediate square also empty
        if (!hasMoved && inBounds(twoStep) && board.getPieceAt(twoStep) == nullptr) {
            moves.push_back(twoStep);
        }
    }

    // Diagonal captures — only if enemy piece is there
    sf::Vector2i captureLeft = { position.x - 1, position.y + dir };
    sf::Vector2i captureRight = { position.x + 1, position.y + dir };

    for (auto& sq : { captureLeft, captureRight }) {
        if (inBounds(sq)) {
            Piece* target = board.getPieceAt(sq);
            if (target != nullptr && target->getColor() != color)
                moves.push_back(sq);
        }
    }

    return moves;
}

// -------------------------------------------------------
// ROOK
// - Slides horizontally and vertically any number of squares
// - Stops before friendly pieces, stops ON enemy pieces (capture)
// -------------------------------------------------------
std::vector<sf::Vector2i> Rook::getLegalMoves(const Board& board) const {
    std::vector<sf::Vector2i> moves;

    const sf::Vector2i directions[] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

    for (auto& dir : directions) {
        sf::Vector2i current = position;
        while (true) {
            current += dir;
            if (!inBounds(current)) break;

            Piece* target = board.getPieceAt(current);
            if (target == nullptr) {
                moves.push_back(current);          // empty — keep sliding
            }
            else if (target->getColor() != color) {
                moves.push_back(current); break;   // enemy — capture and stop
            }
            else {
                break;                             // friendly — blocked
            }
        }
    }

    return moves;
}

// -------------------------------------------------------
// KNIGHT
// - Moves in L-shapes (2+1 squares)
// - Only piece that jumps over others
// -------------------------------------------------------
std::vector<sf::Vector2i> Knight::getLegalMoves(const Board& board) const {
    std::vector<sf::Vector2i> moves;

    const sf::Vector2i offsets[] = {
        {-2,-1},{-2,1},{2,-1},{2,1},
        {-1,-2},{1,-2},{-1,2},{1,2}
    };

    for (auto& offset : offsets) {
        sf::Vector2i sq = position + offset;
        if (!inBounds(sq)) continue;

        Piece* occupant = board.getPieceAt(sq);
        if (occupant == nullptr || occupant->getColor() != color)
            moves.push_back(sq);
    }

    return moves;
}

// -------------------------------------------------------
// BISHOP
// - Slides diagonally any number of squares
// - Same sliding logic as Rook but in 4 diagonal directions
// -------------------------------------------------------
std::vector<sf::Vector2i> Bishop::getLegalMoves(const Board& board) const {
    std::vector<sf::Vector2i> moves;

    const sf::Vector2i directions[] = { {1,1},{1,-1},{-1,1},{-1,-1} };

    for (auto& dir : directions) {
        sf::Vector2i current = position;
        while (true) {
            current += dir;
            if (!inBounds(current)) break;

            Piece* target = board.getPieceAt(current);
            if (target == nullptr) {
                moves.push_back(current);
            }
            else if (target->getColor() != color) {
                moves.push_back(current); break;
            }
            else {
                break;
            }
        }
    }

    return moves;
}

// -------------------------------------------------------
// QUEEN
// - Combines Rook + Bishop: slides in all 8 directions
// -------------------------------------------------------
std::vector<sf::Vector2i> Queen::getLegalMoves(const Board& board) const {
    std::vector<sf::Vector2i> moves;

    const sf::Vector2i directions[] = {
        {1,0},{-1,0},{0,1},{0,-1},   // straight
        {1,1},{1,-1},{-1,1},{-1,-1}  // diagonal
    };

    for (auto& dir : directions) {
        sf::Vector2i current = position;
        while (true) {
            current += dir;
            if (!inBounds(current)) break;

            Piece* target = board.getPieceAt(current);
            if (target == nullptr) {
                moves.push_back(current);
            }
            else if (target->getColor() != color) {
                moves.push_back(current); break;
            }
            else {
                break;
            }
        }
    }

    return moves;
}

// -------------------------------------------------------
// KING
// - Moves exactly 1 square in any direction
// - Moving into check is filtered by MoveValidator in Week 3
// -------------------------------------------------------
std::vector<sf::Vector2i> King::getLegalMoves(const Board& board) const {
    std::vector<sf::Vector2i> moves;

    const sf::Vector2i offsets[] = {
        {1,0},{-1,0},{0,1},{0,-1},
        {1,1},{1,-1},{-1,1},{-1,-1}
    };

    for (auto& offset : offsets) {
        sf::Vector2i sq = position + offset;
        if (!inBounds(sq)) continue;

        Piece* occupant = board.getPieceAt(sq);
        if (occupant == nullptr || occupant->getColor() != color)
            moves.push_back(sq);
    }

    return moves;
}

// -------------------------------------------------------
// clone() — creates an identical copy of each piece
// Used by Board's copy constructor so AI can simulate
// moves without touching the real board.
// The texture pointer is shared (TextureManager owns it).
// -------------------------------------------------------
std::unique_ptr<Piece> Pawn::clone()   const { auto p = std::make_unique<Pawn>(color, position, texture); p->setHasMoved(hasMoved); return p; }
std::unique_ptr<Piece> Rook::clone()   const { auto p = std::make_unique<Rook>(color, position, texture); p->setHasMoved(hasMoved); return p; }
std::unique_ptr<Piece> Knight::clone() const { auto p = std::make_unique<Knight>(color, position, texture); p->setHasMoved(hasMoved); return p; }
std::unique_ptr<Piece> Bishop::clone() const { auto p = std::make_unique<Bishop>(color, position, texture); p->setHasMoved(hasMoved); return p; }
std::unique_ptr<Piece> Queen::clone()  const { auto p = std::make_unique<Queen>(color, position, texture); p->setHasMoved(hasMoved); return p; }
std::unique_ptr<Piece> King::clone()   const { auto p = std::make_unique<King>(color, position, texture); p->setHasMoved(hasMoved); return p; }