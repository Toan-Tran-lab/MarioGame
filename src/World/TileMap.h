#pragma once
#include "raylib.h"
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>

// A single tile instance from an LDtk tile layer
struct LdtkTile {
    int px[2];   // pixel position [x, y] in the level
    int src[2];  // source pixel position [x, y] in the tileset
    int f;       // flip flags: 0=none, 1=flipX, 2=flipY, 3=both
};

// A complete tile layer from LDtk
struct TileLayer {
    std::string identifier;       // e.g. "Terrain_1", "Terrain_2"
    std::string tilesetRelPath;   // relative path to tileset image (from LDtk)
    std::string textureKey;       // key used in TextureManager
    int gridSize;                 // tile grid size in pixels (from tileset)
    std::vector<LdtkTile> tiles;  // all placed tiles in this layer
};

class TileMap {
private:
    std::vector<TileLayer> layers;                // tile layers in render order (back to front)
    std::vector<std::vector<int>> collisionGrid;  // from IntGrid "Collisions" layer (0=empty, 1=solid)

    int mapWidth;      // map width in tiles
    int mapHeight;     // map height in tiles
    int gridSize;      // native grid size from LDtk (pixels, e.g. 16)
    int tileSize;      // display tile size on screen (pixels, e.g. 48)

public:
    TileMap();

    // Load a level from an LDtk project file
    // levelId: identifier of the level to load (e.g. "Level_0")
    bool LoadFromLdtk(const std::string& filePath, const std::string& levelId = "Level_0");

    // Setters
    void SetTileSize(int size);

    // Queries
    bool IsSolidAt(int col, int row) const;
    bool IsInBounds(int col, int row) const;
    int GetMapWidth() const;
    int GetMapHeight() const;
    int GetTileSize() const;
    int GetPixelWidth() const;   // mapWidth * tileSize
    int GetPixelHeight() const;  // mapHeight * tileSize

    // Coordinate conversion (world <-> grid)
    Vector2 WorldToTile(float worldX, float worldY) const;
    Vector2 TileToWorld(int col, int row) const;

    // Draw all tile layers (cameraX, cameraY for viewport culling)
    void Draw(float cameraX = 0.0f, float cameraY = 0.0f);
};
