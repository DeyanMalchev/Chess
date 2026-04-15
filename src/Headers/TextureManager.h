#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include "ChessTypes.h"

// Loads all piece textures once and hands out pointers to them.
// This avoids storing copies of textures inside every Piece.
class TextureManager {
public:
    // Load all piece textures from a folder (e.g. "assets/pieces/")
    // Expected filenames: white_pawn.png, black_king.png, etc.
    bool loadAll(const std::string& folderPath);

    // Get a pointer to a specific piece's texture
    sf::Texture* getTexture(PieceType type, PieceColor color);

private:
    // Key is e.g. "white_pawn", "black_king"
    std::map<std::string, sf::Texture> textures;

    // Helper to build the key string from type and color
    std::string buildKey(PieceType type, PieceColor color);
};
