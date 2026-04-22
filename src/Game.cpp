#include "Headers/Game.h"
#include "Headers/Piece.h"
#include "Headers/Pieces.h"
#include "Headers/UI.h"
#include <iostream>

Game::Game()
    : window(sf::VideoMode({ 1100u, 700u }), "Chess"),
    ui(window)
{
    window.setFramerateLimit(60);
}

void Game::run() {
    if (!textures.loadAll("assets/Pieces/"))
        std::cerr << "Some textures failed to load.\n";

    ui.loadFont("assets/font.ttf");
    gameState = GameState::Menu;

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        processEvents();
        update(deltaTime);
        render();
    }
}

void Game::startGame(GameState mode, PieceColor humanColor) {
    gameState = mode;
    stateBeforePromotion = mode;
    currentTurn = PieceColor::White;
    selectedPiece = nullptr;
    legalMovesCache.clear();
    pendingPromotionSquare = std::nullopt;
    whiteTime = 600.f;
    blackTime = 600.f;

    board.init(textures);
    board.clearMoveHistory();

    delete ai;
    ai = nullptr;

    if (mode == GameState::PlayingPvAI) {
        playerColor = humanColor;
        aiColor = (humanColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;
        boardFlipped = (humanColor == PieceColor::Black);
        ai = new AI(aiColor, 4);
    }
    else {
        // PvP — always White perspective
        boardFlipped = false;
    }
}

// -------------------------------------------------------
// Event processing
// -------------------------------------------------------
void Game::processEvents() {
    while (auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();

        if (auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mp->button == sf::Mouse::Button::Left) {
                sf::Vector2i mousePos = { mp->position.x, mp->position.y };

                switch (gameState) {
                case GameState::Menu:
                    handleMenuClick(mousePos);
                    break;
                case GameState::ChoosingSide:
                    handleSideSelectionClick(mousePos);
                    break;
                case GameState::PlayingPvP:
                case GameState::PlayingPvAI:
                    handleBoardClick(mousePos);
                    break;
                case GameState::AwaitingPromotion:
                    handlePromotionClick(mousePos);
                    break;
                case GameState::GameOver:
                    gameState = GameState::Menu;
                    break;
                }
            }
        }
    }
}

// -------------------------------------------------------
// Update — timers + AI turn
// -------------------------------------------------------
void Game::update(float deltaTime) {
    if (gameState == GameState::PlayingPvP || gameState == GameState::PlayingPvAI) {
        if (currentTurn == PieceColor::White)
            whiteTime = std::max(0.f, whiteTime - deltaTime);
        else
            blackTime = std::max(0.f, blackTime - deltaTime);

        if (gameState == GameState::PlayingPvAI && ai && currentTurn == aiColor)
            triggerAIMove();
    }
}

// -------------------------------------------------------
// Render
// -------------------------------------------------------
void Game::render() {
    window.clear(sf::Color(30, 30, 30));

    switch (gameState) {
    case GameState::Menu:
        ui.drawMenu();
        break;
    case GameState::ChoosingSide:
        ui.drawSideSelection();
        break;
    case GameState::PlayingPvP:
    case GameState::PlayingPvAI:
        board.draw(window, boardFlipped);
        ui.drawSidebar(board, whiteTime, blackTime, currentTurn);
        break;
    case GameState::AwaitingPromotion:
        board.draw(window, boardFlipped);
        ui.drawSidebar(board, whiteTime, blackTime, currentTurn);
        ui.drawPromotionDialog(currentTurn);
        break;
    case GameState::GameOver:
        board.draw(window, boardFlipped);
        ui.drawSidebar(board, whiteTime, blackTime, currentTurn);
        ui.drawGameOver(gameOverMessage);
        break;
    }

    window.display();
}

// -------------------------------------------------------
// Menu click
// -------------------------------------------------------
void Game::handleMenuClick(sf::Vector2i& mousePos) {
    GameState chosen = ui.handleMenuClick(mousePos);
    if (chosen == GameState::PlayingPvP)
        startGame(GameState::PlayingPvP);
    else if (chosen == GameState::ChoosingSide)
        gameState = GameState::ChoosingSide;  // show side picker before starting
}

// -------------------------------------------------------
// Side selection click (PvAI only)
// -------------------------------------------------------
void Game::handleSideSelectionClick(sf::Vector2i& mousePos) {
    auto chosen = ui.handleSideSelectionClick(mousePos);
    if (chosen.has_value())
        startGame(GameState::PlayingPvAI, *chosen);
}

// -------------------------------------------------------
// Board click — only when it's the human's turn
// -------------------------------------------------------
void Game::handleBoardClick(sf::Vector2i& mousePos) {
    if (gameState == GameState::PlayingPvAI && currentTurn == aiColor)
        return;

    sf::Vector2i boardPos = board.pixelToBoard(mousePos, boardFlipped);
    if (boardPos.x < 0 || boardPos.x >= 8 || boardPos.y < 0 || boardPos.y >= 8)
        return;

    Piece* clickedPiece = board.getPieceAt(boardPos);

    if (selectedPiece == nullptr) {
        if (clickedPiece && clickedPiece->getColor() == currentTurn)
            selectPiece(clickedPiece);
    }
    else {
        bool isLegal = false;
        for (auto& sq : legalMovesCache)
            if (sq == boardPos) { isLegal = true; break; }

        if (isLegal) {
            makeMove(selectedPiece->getPosition(), boardPos);
        }
        else if (clickedPiece && clickedPiece->getColor() == currentTurn) {
            selectPiece(clickedPiece);
        }
        else {
            selectedPiece = nullptr;
            legalMovesCache.clear();
            board.clearHighlights();
        }
    }
}

// -------------------------------------------------------
// Promotion click
// -------------------------------------------------------
void Game::handlePromotionClick(sf::Vector2i& mousePos) {
    auto chosen = ui.handlePromotionClick(mousePos, currentTurn);
    if (!chosen) return;

    sf::Texture* tex = textures.getTexture(*chosen, currentTurn);
    board.promotePawn(*pendingPromotionSquare, *chosen, tex);

    pendingPromotionSquare = std::nullopt;
    gameState = stateBeforePromotion;

    switchTurn();
    checkGameOver();
}

// -------------------------------------------------------
// Select piece and cache legal moves
// -------------------------------------------------------
void Game::selectPiece(Piece* piece) {
    selectedPiece = piece;

    auto candidates = piece->getLegalMoves(board);
    if (piece->getType() == PieceType::King) {
        auto castling = validator.getCastlingMoves(board, piece);
        candidates.insert(candidates.end(), castling.begin(), castling.end());
    }

    legalMovesCache = validator.filterSafeMoves(board, piece, candidates);
    board.setSelectedSquare(piece->getPosition());
    board.setHighlightedSquares(legalMovesCache);
}

// -------------------------------------------------------
// makeMove
// -------------------------------------------------------
void Game::makeMove(sf::Vector2i from, sf::Vector2i to) {
    Piece* movingPiece = board.getPieceAt(from);

    if (movingPiece && movingPiece->getType() == PieceType::King) {
        int dx = to.x - from.x;
        if (dx == 2)  board.movePiece({ 7, from.y }, { 5, from.y });
        if (dx == -2) board.movePiece({ 0, from.y }, { 3, from.y });
    }

    if (board.getPieceAt(to)) board.removePiece(to);
    board.movePiece(from, to);

    auto toFile = [](int col) { return std::string(1, 'a' + col); };
    auto toRank = [](int row) { return std::to_string(8 - row); };
    board.addMoveToHistory(toFile(from.x) + toRank(from.y) + "-" + toFile(to.x) + toRank(to.y));

    selectedPiece = nullptr;
    legalMovesCache.clear();
    board.clearHighlights();

    Piece* moved = board.getPieceAt(to);
    int    backRank = (currentTurn == PieceColor::White) ? 0 : 7;

    if (moved && moved->getType() == PieceType::Pawn && to.y == backRank) {
        pendingPromotionSquare = to;
        stateBeforePromotion = gameState;
        gameState = GameState::AwaitingPromotion;
        return;
    }

    switchTurn();
    checkGameOver();
}

// -------------------------------------------------------
// triggerAIMove — synchronous; runs and applies move in one frame.
// -------------------------------------------------------
void Game::triggerAIMove() {
    if (!ai) return;

    auto maybeMove = ai->update(board, validator);
    if (!maybeMove) return;

    Move best = *maybeMove;

    Piece* movingPiece = board.getPieceAt(best.from);
    if (movingPiece && movingPiece->getType() == PieceType::King) {
        int dx = best.to.x - best.from.x;
        if (dx == 2)  board.movePiece({ 7, best.from.y }, { 5, best.from.y });
        if (dx == -2) board.movePiece({ 0, best.from.y }, { 3, best.from.y });
    }

    if (board.getPieceAt(best.to)) board.removePiece(best.to);
    board.movePiece(best.from, best.to);

    Piece* moved = board.getPieceAt(best.to);
    int    backRank = (aiColor == PieceColor::White) ? 0 : 7;
    if (moved && moved->getType() == PieceType::Pawn && best.to.y == backRank) {
        sf::Texture* tex = textures.getTexture(PieceType::Queen, aiColor);
        board.promotePawn(best.to, PieceType::Queen, tex);
    }

    auto toFile = [](int col) { return std::string(1, 'a' + col); };
    auto toRank = [](int row) { return std::to_string(8 - row); };
    board.addMoveToHistory(toFile(best.from.x) + toRank(best.from.y)
        + "-" + toFile(best.to.x) + toRank(best.to.y));

    switchTurn();
    checkGameOver();
}

void Game::switchTurn() {
    currentTurn = (currentTurn == PieceColor::White) ? PieceColor::Black : PieceColor::White;
}

void Game::checkGameOver() {
    bool inCheck = validator.isInCheck(board, currentTurn);
    bool hasMoves = validator.hasLegalMoves(board, currentTurn);

    if (!hasMoves) {
        if (inCheck) {
            PieceColor winner = (currentTurn == PieceColor::White) ? PieceColor::Black : PieceColor::White;
            gameOverMessage = (winner == PieceColor::White) ? "White wins by checkmate!" : "Black wins by checkmate!";
        }
        else {
            gameOverMessage = "Stalemate — draw!";
        }
        gameState = GameState::GameOver;
    }
}