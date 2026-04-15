#include "Headers/UI.h"
#include "Headers/Board.h"
#include "Headers/Piece.h"
#include <iostream>
#include <iomanip>
#include <sstream>

// The board takes up 30 (label offset) + 8*80 (tiles) + 30 (padding) = 700px wide
// Sidebar starts at x = 700
static const float SIDEBAR_X = 700.f;
static const float WINDOW_H = 700.f;

UI::UI(sf::RenderWindow& window) : window(window) {
    // SFML 3: FloatRect takes two Vector2f arguments — position and size
    pvpButtonRect = sf::FloatRect({ 300.f, 260.f }, { 200.f, 55.f });
    pvaiButtonRect = sf::FloatRect({ 300.f, 340.f }, { 200.f, 55.f });
}

bool UI::loadFont(const std::string& fontPath) {
    if (!font.openFromFile(fontPath)) {
        std::cerr << "UI: Failed to load font: " << fontPath << "\n";
        return false;
    }
    return true;
}

// -------------------------------------------------------
// Menu
// -------------------------------------------------------

void UI::drawMenu() const {
    // Title
    sf::Text title(font, "Chess", 64);
    title.setFillColor(sf::Color::White);
    title.setPosition({ 330.f, 140.f });
    window.draw(title);

    drawButton("Player vs Player", pvpButtonRect, sf::Color(70, 130, 180));
    drawButton("Player vs AI", pvaiButtonRect, sf::Color(70, 130, 180));
}

GameState UI::handleMenuClick(sf::Vector2i mousePos) const {
    sf::Vector2f pos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    if (pvpButtonRect.contains(pos))  return GameState::PlayingPvP;
    if (pvaiButtonRect.contains(pos)) return GameState::PlayingPvAI;

    return GameState::Menu;  // nothing was clicked
}

// -------------------------------------------------------
// In-game sidebar
// -------------------------------------------------------

void UI::drawSidebar(const Board& board,
    float whiteTimeSeconds,
    float blackTimeSeconds,
    PieceColor currentTurn) const
{
    // Background panel
    sf::RectangleShape panel({ 400.f, WINDOW_H });
    panel.setPosition({ SIDEBAR_X, 0.f });
    panel.setFillColor(sf::Color(45, 45, 45));
    window.draw(panel);

    // Timers
    sf::Text timerLabel(font, "Black", 16);
    timerLabel.setFillColor(currentTurn == PieceColor::Black ? sf::Color::Yellow : sf::Color(180, 180, 180));
    timerLabel.setPosition({ SIDEBAR_X + 20.f, 20.f });
    window.draw(timerLabel);
    drawTimer(blackTimeSeconds, currentTurn == PieceColor::Black, { SIDEBAR_X + 20.f, 44.f });

    timerLabel.setString("White");
    timerLabel.setFillColor(currentTurn == PieceColor::White ? sf::Color::Yellow : sf::Color(180, 180, 180));
    timerLabel.setPosition({ SIDEBAR_X + 20.f, WINDOW_H - 90.f });
    window.draw(timerLabel);
    drawTimer(whiteTimeSeconds, currentTurn == PieceColor::White, { SIDEBAR_X + 20.f, WINDOW_H - 66.f });

    // Captured pieces
    drawCapturedPieces(board.getCapturedPieces(PieceColor::White), PieceColor::White, { SIDEBAR_X + 20.f, 110.f });
    drawCapturedPieces(board.getCapturedPieces(PieceColor::Black), PieceColor::Black, { SIDEBAR_X + 20.f, 160.f });

    // Move history
    sf::Text histLabel(font, "Moves", 16);
    histLabel.setFillColor(sf::Color(180, 180, 180));
    histLabel.setPosition({ SIDEBAR_X + 20.f, 210.f });
    window.draw(histLabel);

    drawMoveHistory(board.getMoveHistory(), { SIDEBAR_X + 20.f, 234.f });
}

// -------------------------------------------------------
// Game Over
// -------------------------------------------------------

void UI::drawGameOver(const std::string& resultMessage) const {
    // Semi-transparent overlay over the board
    sf::RectangleShape overlay({ 700.f, 700.f });
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(overlay);

    // Result message — centered
    sf::Text msg(font, resultMessage, 36);
    msg.setFillColor(sf::Color::White);
    auto bounds = msg.getLocalBounds();
    msg.setPosition({
        (700.f - bounds.size.x) / 2.f,
        280.f
        });
    window.draw(msg);

    // Subtitle
    sf::Text sub(font, "Click anywhere to return to menu", 18);
    sub.setFillColor(sf::Color(180, 180, 180));
    auto subBounds = sub.getLocalBounds();
    sub.setPosition({
        (700.f - subBounds.size.x) / 2.f,
        340.f
        });
    window.draw(sub);
}

// -------------------------------------------------------
// Private helpers
// -------------------------------------------------------

void UI::drawButton(const std::string& label, sf::FloatRect rect, sf::Color color) const {
    sf::RectangleShape btn({ rect.size.x, rect.size.y });
    btn.setPosition(rect.position);
    btn.setFillColor(color);
    btn.setOutlineColor(sf::Color::White);
    btn.setOutlineThickness(1.f);
    window.draw(btn);

    sf::Text text(font, label, 20);
    text.setFillColor(sf::Color::White);
    // Center text in button
    auto bounds = text.getLocalBounds();
    text.setPosition({
        rect.position.x + (rect.size.x - bounds.size.x) / 2.f,
        rect.position.y + (rect.size.y - bounds.size.y) / 2.f - 4.f
        });
    window.draw(text);
}

void UI::drawMoveHistory(const std::vector<std::string>& history, sf::Vector2f origin) const {
    // Show the last 16 moves (8 pairs), two columns: white move | black move
    int startIdx = std::max(0, (int)history.size() - 16);

    for (int i = startIdx; i < (int)history.size(); i++) {
        int displayIdx = i - startIdx;
        float x = origin.x + (displayIdx % 2 == 0 ? 0.f : 160.f);
        float y = origin.y + (displayIdx / 2) * 22.f;

        // Move number on the left of white's move
        std::string text = history[i];
        if (i % 2 == 0)
            text = std::to_string(i / 2 + 1) + ". " + text;

        sf::Text entry(font, text, 14);
        entry.setFillColor(sf::Color(210, 210, 210));
        entry.setPosition({ x, y });
        window.draw(entry);
    }
}

void UI::drawCapturedPieces(const std::vector<Piece*>& pieces, PieceColor side, sf::Vector2f origin) const {
    // For now just show a count — sprites can be added later
    sf::Text label(font,
        (side == PieceColor::White ? "White captured: " : "Black captured: ") + std::to_string(pieces.size()),
        14);
    label.setFillColor(sf::Color(180, 180, 180));
    label.setPosition(origin);
    window.draw(label);
}

void UI::drawTimer(float seconds, bool isActive, sf::Vector2f origin) const {
    int mins = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;

    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << mins << ":"
        << std::setw(2) << std::setfill('0') << secs;

    sf::Text timer(font, ss.str(), 28);
    timer.setFillColor(isActive ? sf::Color::White : sf::Color(130, 130, 130));
    timer.setPosition(origin);
    window.draw(timer);
}

// -------------------------------------------------------
// Pawn Promotion Dialog
// Shows 4 buttons in the center: Queen, Rook, Bishop, Knight
// -------------------------------------------------------

// Layout: 4 boxes of 80x80 centered on the board (board = 700px wide)
static sf::FloatRect promotionRect(int index) {
    float totalW = 4 * 90.f;
    float startX = (700.f - totalW) / 2.f;
    return sf::FloatRect({ startX + index * 90.f, 290.f }, { 80.f, 80.f });
}

static const PieceType PROMO_TYPES[4] = {
    PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight
};
static const char* PROMO_LABELS[4] = { "Q", "R", "B", "N" };

void UI::drawPromotionDialog(PieceColor color) const {
    // Dim background
    sf::RectangleShape overlay({ 700.f, 700.f });
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    sf::Text title(font, "Promote pawn to:", 22);
    title.setFillColor(sf::Color::White);
    title.setPosition({ 220.f, 250.f });
    window.draw(title);

    for (int i = 0; i < 4; i++) {
        auto rect = promotionRect(i);
        sf::RectangleShape box({ rect.size.x, rect.size.y });
        box.setPosition(rect.position);
        box.setFillColor(sf::Color(70, 130, 180));
        box.setOutlineColor(sf::Color::White);
        box.setOutlineThickness(2.f);
        window.draw(box);

        sf::Text label(font, PROMO_LABELS[i], 36);
        label.setFillColor(sf::Color::White);
        auto bounds = label.getLocalBounds();
        label.setPosition({
            rect.position.x + (rect.size.x - bounds.size.x) / 2.f - 2.f,
            rect.position.y + (rect.size.y - bounds.size.y) / 2.f - 6.f
            });
        window.draw(label);
    }
}

std::optional<PieceType> UI::handlePromotionClick(sf::Vector2i mousePos, PieceColor color) const {
    sf::Vector2f pos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    for (int i = 0; i < 4; i++) {
        if (promotionRect(i).contains(pos))
            return PROMO_TYPES[i];
    }
    return std::nullopt;
}