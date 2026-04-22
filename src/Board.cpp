#include "Headers/Board.h"
#include "Headers/Piece.h"
#include "Headers/Pieces.h"
#include "Headers/TextureManager.h"
#include <iostream>

// Board offset in pixels — leave room on the left/top for labels
static const int LABEL_OFFSET = 30;

Board::Board() {}

Board::Board(const Board& other) {
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

    if (!font.openFromFile("assets/font.ttf"))
        std::cerr << "Failed to load font: assets/font.ttf\n";

    auto add = [&](auto piece) {
        pieces.push_back(std::make_unique<decltype(piece)>(std::move(piece)));
        };

    // Black pieces
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

    // White pieces
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
// Helper: convert a logical board coordinate to a screen
// column/row, flipping the board when playing as Black.
// -------------------------------------------------------
sf::Vector2i Board::toScreen(sf::Vector2i boardPos, bool flipped) const {
    if (flipped)
        return { BOARD_SIZE - 1 - boardPos.x, BOARD_SIZE - 1 - boardPos.y };
    return boardPos;
}

// -------------------------------------------------------
// Drawing
// -------------------------------------------------------

void Board::draw(sf::RenderWindow& window, bool flipped) const {
    drawSquares(window, flipped);
    drawHighlights(window, flipped);
    drawLabels(window, flipped);
    drawPieces(window, flipped);
}

void Board::drawSquares(sf::RenderWindow& window, bool flipped) const {
    sf::RectangleShape square({ static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE) });

    sf::Color lightColor(240, 217, 181);
    sf::Color darkColor(181, 136, 99);

    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            // Logical square colour never changes with flip
            int logicalCol = flipped ? (BOARD_SIZE - 1 - col) : col;
            int logicalRow = flipped ? (BOARD_SIZE - 1 - row) : row;
            bool isLight = (logicalRow + logicalCol) % 2 == 0;
            square.setFillColor(isLight ? lightColor : darkColor);
            square.setPosition({
                static_cast<float>(LABEL_OFFSET + col * TILE_SIZE),
                static_cast<float>(LABEL_OFFSET + row * TILE_SIZE)
                });
            window.draw(square);
        }
    }
}

void Board::drawLabels(sf::RenderWindow& window, bool flipped) const {
    // Files: a-h (or h-a when flipped)
    const std::string files = "abcdefgh";
    for (int col = 0; col < BOARD_SIZE; col++) {
        int fileIdx = flipped ? (BOARD_SIZE - 1 - col) : col;
        sf::Text label(font, std::string(1, files[fileIdx]), 14);
        label.setFillColor(sf::Color(100, 100, 100));
        label.setPosition({
            static_cast<float>(LABEL_OFFSET + col * TILE_SIZE + TILE_SIZE / 2 - 5),
            static_cast<float>(LABEL_OFFSET + BOARD_SIZE * TILE_SIZE + 4)
            });
        window.draw(label);
    }

    // Ranks: 8-1 or 1-8 when flipped
    for (int row = 0; row < BOARD_SIZE; row++) {
        int rank = flipped ? (row + 1) : (8 - row);
        sf::Text label(font, std::to_string(rank), 14);
        label.setFillColor(sf::Color(100, 100, 100));
        label.setPosition({
            4.f,
            static_cast<float>(LABEL_OFFSET + row * TILE_SIZE + TILE_SIZE / 2 - 8)
            });
        window.draw(label);
    }
}

void Board::drawHighlights(sf::RenderWindow& window, bool flipped) const {
    sf::RectangleShape highlight({ static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE) });

    // Selected square
    if (selectedSquare.x >= 0) {
        auto sc = toScreen(selectedSquare, flipped);
        highlight.setFillColor(sf::Color(255, 255, 0, 120));
        highlight.setPosition({
            static_cast<float>(LABEL_OFFSET + sc.x * TILE_SIZE),
            static_cast<float>(LABEL_OFFSET + sc.y * TILE_SIZE)
            });
        window.draw(highlight);
    }

    // Legal move dots
    sf::CircleShape dot(12.f);
    dot.setFillColor(sf::Color(0, 200, 0, 140));

    for (auto& sq : highlightedSquares) {
        auto sc = toScreen(sq, flipped);
        dot.setPosition({
            static_cast<float>(LABEL_OFFSET + sc.x * TILE_SIZE + TILE_SIZE / 2 - 12),
            static_cast<float>(LABEL_OFFSET + sc.y * TILE_SIZE + TILE_SIZE / 2 - 12)
            });
        window.draw(dot);
    }
}

void Board::drawPieces(sf::RenderWindow& window, bool flipped) const {
    for (auto& piece : pieces) {
        auto sc = toScreen(piece->getPosition(), flipped);
        piece->drawAtScreen(window, TILE_SIZE, LABEL_OFFSET, sc);
    }
}

// -------------------------------------------------------
// Board logic
// -------------------------------------------------------

Piece* Board::getPieceAt(sf::Vector2i pos) const {
    for (auto& p : pieces)
        if (p->getPosition() == pos) return p.get();
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

UndoInfo Board::makeSimMove(sf::Vector2i from, sf::Vector2i to) {
    UndoInfo undo;
    undo.movedFrom = from;
    undo.movedTo = to;

    Piece* moving = getPieceAt(from);
    undo.movedHadMoved = moving->getHasMoved();

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

    moving->setPosition(to);
    moving->setHasMoved(true);

    if (moving->getType() == PieceType::Pawn) {
        int backRank = (moving->getColor() == PieceColor::White) ? 0 : 7;
        if (to.y == backRank) {
            undo.wasPromotion = true;
            undo.promotionColor = moving->getColor();
            PieceColor col = moving->getColor();
            for (auto it = pieces.begin(); it != pieces.end(); ++it) {
                if (it->get() == moving) { pieces.erase(it); break; }
            }
            pieces.push_back(std::make_unique<Queen>(col, to, nullptr));
        }
    }

    return undo;
}

void Board::undoSimMove(UndoInfo& undo) {
    if (undo.wasPromotion) {
        for (auto it = pieces.begin(); it != pieces.end(); ++it) {
            if ((*it)->getPosition() == undo.movedTo
                && (*it)->getType() == PieceType::Queen
                && (*it)->getColor() == undo.promotionColor) {
                pieces.erase(it);
                break;
            }
        }
        pieces.push_back(std::make_unique<Pawn>(undo.promotionColor, undo.movedFrom, nullptr));
        Piece* pawn = getPieceAt(undo.movedFrom);
        if (pawn) pawn->setHasMoved(undo.movedHadMoved);
    }
    else {
        Piece* moving = getPieceAt(undo.movedTo);
        if (moving) {
            moving->setPosition(undo.movedFrom);
            moving->setHasMoved(undo.movedHadMoved);
        }
    }

    if (undo.capturedPiece)
        pieces.push_back(std::move(undo.capturedPiece));

    if (undo.wasCastle) {
        Piece* rook = getPieceAt(undo.rookTo);
        if (rook) {
            rook->setPosition(undo.rookFrom);
            rook->setHasMoved(undo.rookHadMoved);
        }
    }
}

sf::Vector2i Board::pixelToBoard(sf::Vector2i pixel, bool flipped) const {
    int col = (pixel.x - LABEL_OFFSET) / TILE_SIZE;
    int row = (pixel.y - LABEL_OFFSET) / TILE_SIZE;
    if (flipped) {
        col = BOARD_SIZE - 1 - col;
        row = BOARD_SIZE - 1 - row;
    }
    return { col, row };
}

void Board::setSelectedSquare(sf::Vector2i pos) { selectedSquare = pos; }
void Board::setHighlightedSquares(const std::vector<sf::Vector2i>& squares) { highlightedSquares = squares; }
void Board::clearHighlights() { selectedSquare = { -1, -1 }; highlightedSquares.clear(); }

const std::vector<std::unique_ptr<Piece>>& Board::getPieces() const { return pieces; }
const std::vector<std::string>& Board::getMoveHistory() const { return moveHistory; }
void Board::addMoveToHistory(const std::string& move) { moveHistory.push_back(move); }
void Board::clearMoveHistory() { moveHistory.clear(); }

const std::vector<Piece*>& Board::getCapturedPieces(PieceColor color) const {
    return (color == PieceColor::White) ? capturedByWhite : capturedByBlack;
}