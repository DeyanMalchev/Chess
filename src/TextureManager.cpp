#include "Headers/TextureManager.h"
#include <iostream>

bool TextureManager::loadAll(const std::string& folderPath) {
    // All piece combinations to load
    // Expected files e.g: assets/pieces/white_pawn.png, black_king.png etc.
    std::vector<std::pair<PieceType, std::string>> types = {
        { PieceType::Pawn,   "pawn"   },
        { PieceType::Rook,   "rook"   },
        { PieceType::Knight, "knight" },
        { PieceType::Bishop, "bishop" },
        { PieceType::Queen,  "queen"  },
        { PieceType::King,   "king"   }
    };

    std::vector<std::pair<PieceColor, std::string>> colors = {
        { PieceColor::White, "white" },
        { PieceColor::Black, "black" }
    };

    bool allLoaded = true;
    std::cout << "Loading textures from folder: " << folderPath << "\n";

    for (auto& [color, colorStr] : colors) {
        for (auto& [type, typeStr] : types) {
            std::string key = colorStr + "_" + typeStr;
            std::string path = folderPath + key + ".png";

            sf::Texture texture;
            if (!texture.loadFromFile(path)) {
                std::cerr << "Failed to load texture: " << path << "\n";
                allLoaded = false;
            }
            else {
                textures[key] = std::move(texture);
            }
        }
    }

    return allLoaded;
}

sf::Texture* TextureManager::getTexture(PieceType type, PieceColor color) {
    std::string key = buildKey(type, color);
    auto it = textures.find(key);
    if (it != textures.end())
        return &it->second;

    std::cerr << "Texture not found for key: " << key << "\n";
    return nullptr;
}

std::string TextureManager::buildKey(PieceType type, PieceColor color) {
    std::string colorStr = (color == PieceColor::White) ? "white" : "black";
    std::string typeStr;

    switch (type) {
    case PieceType::Pawn:   typeStr = "pawn";   break;
    case PieceType::Rook:   typeStr = "rook";   break;
    case PieceType::Knight: typeStr = "knight"; break;
    case PieceType::Bishop: typeStr = "bishop"; break;
    case PieceType::Queen:  typeStr = "queen";  break;
    case PieceType::King:   typeStr = "king";   break;
    }

    return colorStr + "_" + typeStr;
}