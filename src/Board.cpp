#include "Headers/Board.h"
#include "Headers/Piece.h"
#include "Headers/Pieces.h"
#include "Headers/TextureManager.h"
#include <iostream>

// Board offset in pixels — leave room on the left/top for labels
static const int LABEL_OFFSET = 30;

Board::Board() {
    // Font is loaded separately so we can report errors
}


// Replace the pawn at pos with a new piece of newType (same color).
// Called after a pawn reaches the back rank.
Board::Board(const Board& other) {
    // Clone every piece — the AI's search board is fully independent
    for (auto& p : other.pieces)
        pieces.push_back(p->clone());
    // Font, highlights, and move history are not copied —
    // the AI only needs piece positions
}

void Board::promotePawn(sf::Vector2i pos, PieceType newType, sf::Texture* texture) {
    for (auto it = pieces.begin(); it != pieces.end(); ++it) {
        if ((*it)->getPosition() == pos && (*it)->getType() == PieceType::Pawn) {
            PieceColor color = (*it)->getColor();
            pieces.erase(it);

            std::unique_ptr<Piece> promoted;
            switch (newType) {
            case PieceType::Queen:  promoted = std::make_unique<Queen>(color, pos, texture); break;
            case PieceType::Rook:   promoted = std::make_unique<Rook>(color, pos, texture); break;
            case PieceType::Bishop: promoted = std::make_unique<Bishop>(color, pos, texture); break;
            case PieceType::Knight: promoted = std::make_unique<Knight>(color, pos, texture); break;
            default: break;
            }
            if (promoted) pieces.push_back(std::move(promoted));
            return;
        }
    }
}

void Board::init(TextureManager& textures) {
    pieces.clear();

    // Load font for rank/file labels
    if (!font.openFromFile("assets/font.ttf")) {
        std::cerr << "Failed to load font: assets/font.ttf\n";
    }

    // Helper lambda to add a piece cleanly
    auto add = [&](auto piece) {
        pieces.push_back(std::make_unique<decltype(piece)>(std::move(piece)));
        };

    // -------------------------------------------------------
    // Place pieces in standard chess starting positions
    // Board coords: col 0-7 = a-h, row 0 = top (black side), row 7 = bottom (white side)
    // -------------------------------------------------------

    // --- Black pieces (row 0 = back rank, row 1 = pawns) ---
    pieces.push_back(std::make_unique<Rook>(PieceColor::Black, sf::Vector2i{ 0,0 }, textures.getTexture(PieceType::Rook, PieceColor::Black)));
    pieces.push_back(std::make_unique<Knight>(PieceColor::Black, sf::Vector2i{ 1,0 }, textures.getTexture(PieceType::Knight, PieceColor::Black)));
    pieces.push_back(std::make_unique<Bishop>(PieceColor::Black, sf::Vector2i{ 2,0 }, textures.getTexture(PieceType::Bishop, PieceColor::Black)));
    pieces.push_back(std::make_unique<Queen>(PieceColor::Black, sf::Vector2i{ 3,0 }, textures.getTexture(PieceType::Queen, PieceColor::Black)));
    pieces.push_back(std::make_unique<King>(PieceColor::Black, sf::Vector2i{ 4,0 }, textures.getTexture(PieceType::King, PieceColor::Black)));
    pieces.push_back(std::make_unique<Bishop>(PieceColor::Black, sf::Vector2i{ 5,0 }, textures.getTexture(PieceType::Bishop, PieceColor::Black)));
    pieces.push_back(std::make_unique<Knight>(PieceColor::Black, sf::Vector2i{ 6,0 }, textures.getTexture(PieceType::Knight, PieceColor::Black)));
    pieces.push_back(std::make_unique<Rook>(PieceColor::Black, sf::Vector2i{ 7,0 }, textures.getTexture(PieceType::Rook, PieceColor::Black)));

    for (int col = 0; col < 8; col++)
        pieces.push_back(std::make_unique<Pawn>(PieceColor::Black, sf::Vector2i{ col,1 }, textures.getTexture(PieceType::Pawn, PieceColor::Black)));

    // --- White pieces (row 7 = back rank, row 6 = pawns) ---
    pieces.push_back(std::make_unique<Rook>(PieceColor::White, sf::Vector2i{ 0,7 }, textures.getTexture(PieceType::Rook, PieceColor::White)));
    pieces.push_back(std::make_unique<Knight>(PieceColor::White, sf::Vector2i{ 1,7 }, textures.getTexture(PieceType::Knight, PieceColor::White)));
    pieces.push_back(std::make_unique<Bishop>(PieceColor::White, sf::Vector2i{ 2,7 }, textures.getTexture(PieceType::Bishop, PieceColor::White)));
    pieces.push_back(std::make_unique<Queen>(PieceColor::White, sf::Vector2i{ 3,7 }, textures.getTexture(PieceType::Queen, PieceColor::White)));
    pieces.push_back(std::make_unique<King>(PieceColor::White, sf::Vector2i{ 4,7 }, textures.getTexture(PieceType::King, PieceColor::White)));
    pieces.push_back(std::make_unique<Bishop>(PieceColor::White, sf::Vector2i{ 5,7 }, textures.getTexture(PieceType::Bishop, PieceColor::White)));
    pieces.push_back(std::make_unique<Knight>(PieceColor::White, sf::Vector2i{ 6,7 }, textures.getTexture(PieceType::Knight, PieceColor::White)));
    pieces.push_back(std::make_unique<Rook>(PieceColor::White, sf::Vector2i{ 7,7 }, textures.getTexture(PieceType::Rook, PieceColor::White)));

    for (int col = 0; col < 8; col++)
        pieces.push_back(std::make_unique<Pawn>(PieceColor::White, sf::Vector2i{ col,6 }, textures.getTexture(PieceType::Pawn, PieceColor::White)));
}

// -------------------------------------------------------
// Drawing
// -------------------------------------------------------

void Board::draw(sf::RenderWindow& window) const {
    drawSquares(window);
    drawHighlights(window);
    drawLabels(window);
    drawPieces(window);
}

void Board::drawSquares(sf::RenderWindow& window) const {
    sf::RectangleShape square({ static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE) });

    // Classic chess board colors
    sf::Color lightColor(240, 217, 181);  // cream
    sf::Color darkColor(181, 136, 99);  // brown

    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            bool isLight = (row + col) % 2 == 0;
            square.setFillColor(isLight ? lightColor : darkColor);
            square.setPosition({
                static_cast<float>(LABEL_OFFSET + col * TILE_SIZE),
                static_cast<float>(LABEL_OFFSET + row * TILE_SIZE)
                });
            window.draw(square);
        }
    }
}

void Board::drawLabels(sf::RenderWindow& window) const {
    // Files: a-h along the bottom
    const std::string files = "abcdefgh";
    for (int col = 0; col < BOARD_SIZE; col++) {
        sf::Text label(font, std::string(1, files[col]), 14);
        label.setFillColor(sf::Color(100, 100, 100));
        label.setPosition({
            static_cast<float>(LABEL_OFFSET + col * TILE_SIZE + TILE_SIZE / 2 - 5),
            static_cast<float>(LABEL_OFFSET + BOARD_SIZE * TILE_SIZE + 4)
            });
        window.draw(label);
    }

    // Ranks: 8-1 along the left side (row 0 = rank 8, row 7 = rank 1)
    for (int row = 0; row < BOARD_SIZE; row++) {
        sf::Text label(font, std::to_string(8 - row), 14);
        label.setFillColor(sf::Color(100, 100, 100));
        label.setPosition({
            4.f,
            static_cast<float>(LABEL_OFFSET + row * TILE_SIZE + TILE_SIZE / 2 - 8)
            });
        window.draw(label);
    }
}

void Board::drawHighlights(sf::RenderWindow& window) const {
    sf::RectangleShape highlight({ static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE) });

    // Selected square — yellow tint
    if (selectedSquare.x >= 0) {
        highlight.setFillColor(sf::Color(255, 255, 0, 120));
        highlight.setPosition({
            static_cast<float>(LABEL_OFFSET + selectedSquare.x * TILE_SIZE),
            static_cast<float>(LABEL_OFFSET + selectedSquare.y * TILE_SIZE)
            });
        window.draw(highlight);
    }

    // Legal move squares — green dots
    sf::CircleShape dot(12.f);
    dot.setFillColor(sf::Color(0, 200, 0, 140));

    for (auto& sq : highlightedSquares) {
        dot.setPosition({
            static_cast<float>(LABEL_OFFSET + sq.x * TILE_SIZE + TILE_SIZE / 2 - 12),
            static_cast<float>(LABEL_OFFSET + sq.y * TILE_SIZE + TILE_SIZE / 2 - 12)
            });
        window.draw(dot);
    }
}

void Board::drawPieces(sf::RenderWindow& window) const {
    for (auto& piece : pieces) {
        // Offset piece drawing by the label margin
        // We do this by temporarily adjusting the draw call
        sf::Vector2i boardPos = piece->getPosition();

        // Create a temporary sprite positioned with the label offset
        // Piece::draw handles the board-to-pixel conversion,
        // so we pass adjusted pixel coords via a helper offset
        // The simplest approach: just offset the piece's draw position here

        // We draw manually here to apply the LABEL_OFFSET
        piece->drawWithOffset(window, TILE_SIZE, LABEL_OFFSET);
    }
}

// -------------------------------------------------------
// Board logic
// -------------------------------------------------------

Piece* Board::getPieceAt(sf::Vector2i pos) const {
    for (auto& p : pieces) {
        if (p->getPosition() == pos)
            return p.get();
    }
    return nullptr;
}

void Board::movePiece(sf::Vector2i from, sf::Vector2i to) {
    for (auto& p : pieces) {
        if (p->getPosition() == from) {
            p->setPosition(to);
            p->setHasMoved(true);
            return;
        }
    }
}

void Board::removePiece(sf::Vector2i pos) {
    for (auto it = pieces.begin(); it != pieces.end(); ++it) {
        if ((*it)->getPosition() == pos) {
            pieces.erase(it);
            return;
        }
    }
}


// -------------------------------------------------------
// makeSimMove
//
// Applies a move directly on this board and returns an
// UndoInfo with everything needed to reverse it.
// Handles captures, castling, and pawn promotion to Queen.
// Does NOT touch move history or highlights.
// -------------------------------------------------------
UndoInfo Board::makeSimMove(sf::Vector2i from, sf::Vector2i to) {
    UndoInfo undo;
    undo.movedFrom = from;
    undo.movedTo = to;

    Piece* moving = getPieceAt(from);
    undo.movedHadMoved = moving->getHasMoved();

    // Save and remove any captured piece
    Piece* target = getPieceAt(to);
    if (target) {
        for (auto it = pieces.begin(); it != pieces.end(); ++it) {
            if (it->get() == target) {
                undo.capturedPiece = std::move(*it);
                pieces.erase(it);
                break;
            }
        }
    }

    // Castling: move the rook too
    if (moving->getType() == PieceType::King) {
        int dx = to.x - from.x;
        if (dx == 2 || dx == -2) {
            undo.wasCastle = true;
            undo.rookFrom = { (dx == 2) ? 7 : 0, from.y };
            undo.rookTo = { (dx == 2) ? 5 : 3, from.y };
            Piece* rook = getPieceAt(undo.rookFrom);
            if (rook) {
                undo.rookHadMoved = rook->getHasMoved();
                rook->setPosition(undo.rookTo);
                rook->setHasMoved(true);
            }
        }
    }

    // Move the piece
    moving->setPosition(to);
    moving->setHasMoved(true);

    // Pawn promotion: replace with Queen for simulation purposes
    if (moving->getType() == PieceType::Pawn) {
        int backRank = (moving->getColor() == PieceColor::White) ? 0 : 7;
        if (to.y == backRank) {
            undo.wasPromotion = true;
            PieceColor col = moving->getColor();
            undo.promotionColor = col;
            // Remove the pawn
            for (auto it = pieces.begin(); it != pieces.end(); ++it) {
                if (it->get() == moving) {
                    pieces.erase(it);
                    break;
                }
            }
            // Insert a Queen (texture nullptr is fine for AI — it never draws)
            pieces.push_back(std::make_unique<Queen>(col, to, nullptr));
        }
    }

    return undo;
}

// -------------------------------------------------------
// undoSimMove
//
// Reverses exactly what makeSimMove did, restoring the
// board to its prior state.
// -------------------------------------------------------
void Board::undoSimMove(UndoInfo& undo) {
    if (undo.wasPromotion) {
        // Remove the Queen we inserted
        for (auto it = pieces.begin(); it != pieces.end(); ++it) {
            if ((*it)->getPosition() == undo.movedTo
                && (*it)->getType() == PieceType::Queen
                && (*it)->getColor() == undo.promotionColor) {
                pieces.erase(it);
                break;
            }
        }
        // Restore the original pawn at its pre-move square
        pieces.push_back(std::make_unique<Pawn>(
            undo.promotionColor, undo.movedFrom, nullptr));
        Piece* pawn = getPieceAt(undo.movedFrom);
        if (pawn) pawn->setHasMoved(undo.movedHadMoved);
    }
    else {
        // Find the moved piece at movedTo and move it back
        Piece* moving = getPieceAt(undo.movedTo);
        if (moving) {
            moving->setPosition(undo.movedFrom);
            moving->setHasMoved(undo.movedHadMoved);
        }
    }

    // Restore captured piece
    if (undo.capturedPiece)
        pieces.push_back(std::move(undo.capturedPiece));

    // Undo castling rook
    if (undo.wasCastle) {
        Piece* rook = getPieceAt(undo.rookTo);
        if (rook) {
            rook->setPosition(undo.rookFrom);
            rook->setHasMoved(undo.rookHadMoved);
        }
    }
}


sf::Vector2i Board::pixelToBoard(sf::Vector2i pixel) const {
    return {
        (pixel.x - LABEL_OFFSET) / TILE_SIZE,
        (pixel.y - LABEL_OFFSET) / TILE_SIZE
    };
}

void Board::setSelectedSquare(sf::Vector2i pos) {
    selectedSquare = pos;
}

void Board::setHighlightedSquares(const std::vector<sf::Vector2i>& squares) {
    highlightedSquares = squares;
}

void Board::clearHighlights() {
    selectedSquare = { -1, -1 };
    highlightedSquares.clear();
}

const std::vector<std::unique_ptr<Piece>>& Board::getPieces() const {
    return pieces;
}

const std::vector<std::string>& Board::getMoveHistory() const {
    return moveHistory;
}

void Board::addMoveToHistory(const std::string& move) {
    moveHistory.push_back(move);
}

const std::vector<Piece*>& Board::getCapturedPieces(PieceColor color) const {
    return (color == PieceColor::White) ? capturedByWhite : capturedByBlack;
}