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

<<<<<<< HEAD
=======
#include "World/BlockGrid.h"

// Data structure representing a cell imported from Sandbox mode
struct SandboxCellData {
    int type; // 0 = empty, 1 = tile, 2 = player, 3 = goomba, 4 = coin
    std::string texKey;
    Rectangle srcRect;
    bool isSolid;
};

// Represents an entity spawned in the map (e.g. Goomba, Koopa)
struct EntitySpawnInfo {
    std::string id;       // e.g. "Goomba", "Koopa"
    Vector2 position;     // Spawning position
};

>>>>>>> c82ad2234b5ca71bdccf2f742362e3fad6a65b7c
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
    BlockGrid blockGrid;                          // Replaces collisionGrid

    int mapWidth;      // map width in tiles
    int mapHeight;     // map height in tiles
    int gridSize;      // native grid size from LDtk (pixels, e.g. 16)
    int tileSize;      // display tile size on screen (pixels, e.g. 48)

    Color bgColor;     // background color from LDtk level property
    Vector2 playerSpawn; // store player spawn position
    std::vector<EntitySpawnInfo> entities; // parsed enemies/items

    float borderLeft;
    float borderRight;
    float borderTop;
    float borderBottom;

    RenderTexture2D mapTarget;
    bool isTargetBuilt;

    void BuildMapTexture();

public:
    TileMap();
    ~TileMap();

    // Get background color
    Color GetBackgroundColor() const;

    // Get player spawn position (scaled to display size)
    Vector2 GetPlayerSpawn() const;
    
    // Get all parsed entities
    const std::vector<EntitySpawnInfo>& GetEntities() const;
    
    // Get BlockGrid for interactive blocks and collisions
    BlockGrid& GetBlockGrid() { return blockGrid; }
    const BlockGrid& GetBlockGrid() const { return blockGrid; }


    // Load a level from an LDtk project file
    // levelId: identifier of the level to load (e.g. "Level_0")
    bool LoadFromLdtk(const std::string& filePath, const std::string& levelId = "Level_0");

<<<<<<< HEAD
=======
    // Load a custom level from Sandbox editor cells
    bool LoadFromSandbox(const std::vector<std::vector<SandboxCellData>>& sandboxGrid);

    // Remove a tile at grid position (col, row) and return its texture info for debris effect
    bool RemoveTileAt(int col, int row, std::string& outTexKey, Rectangle& outSrcRect);

>>>>>>> c82ad2234b5ca71bdccf2f742362e3fad6a65b7c
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
    
    // Map borders
    float GetBorderLeft() const;
    float GetBorderRight() const;
    float GetBorderTop() const;
    float GetBorderBottom() const;

    // Coordinate conversion (world <-> grid)
    Vector2 WorldToTile(float worldX, float worldY) const;
    Vector2 TileToWorld(int col, int row) const;

    // Draw all tile layers (cameraX, cameraY, and cameraZoom for viewport culling)
    void Draw(float cameraX = 0.0f, float cameraY = 0.0f, float cameraZoom = 1.0f);
};
