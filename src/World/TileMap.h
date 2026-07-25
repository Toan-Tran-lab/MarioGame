#pragma once
#include "raylib.h"
#include <vector>
#include <string>

// Các loại tile
enum class TileType : int {
    EMPTY = 0,     // Trống (bầu trời)
    GROUND,        // Đất
    BRICK,         // Gạch
    COUNT
};

// Thông tin của mỗi loại tile
struct TileInfo {
    Rectangle sourceRect;  // Vị trí cắt trong spritesheet (pixel)
    bool solid;            // Có va chạm không
};

class TileMap {
private:
    std::vector<std::vector<TileType>> grid;
    int mapWidth;      // Số cột
    int mapHeight;     // Số hàng
    int tileSize;      // Kích thước vẽ ra màn hình (pixel)

    std::string textureKey;  // Key texture trong TextureManager

    // Bảng thông tin cho mỗi loại tile
    static TileInfo tileInfoTable[(int)TileType::COUNT];
    static bool tableInitialized;

public:
    TileMap();

    // Khởi tạo bảng source rect (gọi 1 lần sau khi load texture)
    // spriteW, spriteH: kích thước mỗi tile trong spritesheet
    static void InitTileInfoTable();

    // Tạo map từ mảng string
    // Ký tự: '.' = EMPTY, 'G' = GROUND, 'B' = BRICK
    void LoadFromStrings(const std::vector<std::string>& mapData);

    // Thiết lập
    void SetTextureKey(const std::string& key);
    void SetTileSize(int size);

    // Truy vấn
    TileType GetTile(int col, int row) const;
    bool IsSolidAt(int col, int row) const;
    bool IsInBounds(int col, int row) const;
    int GetMapWidth() const;
    int GetMapHeight() const;
    int GetTileSize() const;
    int GetPixelWidth() const;   // mapWidth * tileSize
    int GetPixelHeight() const;  // mapHeight * tileSize

    // Chuyển đổi tọa độ thế giới <-> tọa độ grid
    Vector2 WorldToTile(float worldX, float worldY) const;
    Vector2 TileToWorld(int col, int row) const;

    // Vẽ map (cameraX, cameraY: vị trí camera trong thế giới)
    void Draw(float cameraX = 0.0f, float cameraY = 0.0f);
};
