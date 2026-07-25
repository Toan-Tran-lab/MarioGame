#include "TileMap.h"
#include "TextureManager/TextureManager.h"

// ========== Khởi tạo static ==========
TileInfo TileMap::tileInfoTable[(int)TileType::COUNT] = {};
bool TileMap::tableInitialized = false;

void TileMap::InitTileInfoTable() {
    // ===================================================================
    // CẤU HÌNH SOURCE RECT CHO MỖI LOẠI TILE
    // Chỉnh sửa tọa độ (x, y, w, h) để khớp với spritesheet của bạn
    // x, y: pixel top-left trong spritesheet
    // w, h: kích thước cắt (thường 16x16)
    // ===================================================================

    // EMPTY: không vẽ, không va chạm
    tileInfoTable[(int)TileType::EMPTY] = {
        {0, 0, 16, 16},  // không dùng, nhưng khai báo cho đủ
        false
    };

    // GROUND: đất — chỉnh x, y cho đúng vị trí trong spritesheet
    tileInfoTable[(int)TileType::GROUND] = {
        {2*16, 6*16, 16, 16},
        true
    };

    // BRICK: gạch
    tileInfoTable[(int)TileType::BRICK] = {
        {32, 0, 16, 16},
        true
    };

    tableInitialized = true;
}

// ========== Constructor ==========
TileMap::TileMap()
    : mapWidth(0), mapHeight(0), tileSize(32) {}

// ========== Load map từ mảng string ==========
void TileMap::LoadFromStrings(const std::vector<std::string>& mapData) {
    if (mapData.empty()) return;

    mapHeight = (int)mapData.size();
    mapWidth = (int)mapData[0].size();

    grid.resize(mapHeight);
    for (int row = 0; row < mapHeight; row++) {
        grid[row].resize(mapWidth, TileType::EMPTY);
        for (int col = 0; col < (int)mapData[row].size() && col < mapWidth; col++) {
            switch (mapData[row][col]) {
                case 'G': grid[row][col] = TileType::GROUND; break;
                case 'B': grid[row][col] = TileType::BRICK;  break;
                default:  grid[row][col] = TileType::EMPTY;   break;
            }
        }
    }
}

// ========== Setters ==========
void TileMap::SetTextureKey(const std::string& key) {
    textureKey = key;
}

void TileMap::SetTileSize(int size) {
    tileSize = size;
}

// ========== Truy vấn ==========
TileType TileMap::GetTile(int col, int row) const {
    if (!IsInBounds(col, row)) return TileType::EMPTY;
    return grid[row][col];
}

bool TileMap::IsSolidAt(int col, int row) const {
    if (!IsInBounds(col, row)) return false;
    int type = (int)grid[row][col];
    return tileInfoTable[type].solid;
}

bool TileMap::IsInBounds(int col, int row) const {
    return col >= 0 && col < mapWidth && row >= 0 && row < mapHeight;
}

int TileMap::GetMapWidth() const { return mapWidth; }
int TileMap::GetMapHeight() const { return mapHeight; }
int TileMap::GetTileSize() const { return tileSize; }
int TileMap::GetPixelWidth() const { return mapWidth * tileSize; }
int TileMap::GetPixelHeight() const { return mapHeight * tileSize; }

// ========== Chuyển đổi tọa độ ==========
Vector2 TileMap::WorldToTile(float worldX, float worldY) const {
    return { worldX / tileSize, worldY / tileSize };
}

Vector2 TileMap::TileToWorld(int col, int row) const {
    return { (float)(col * tileSize), (float)(row * tileSize) };
}

// ========== Vẽ ==========
void TileMap::Draw(float cameraX, float cameraY) {
    if (!tableInitialized) InitTileInfoTable();

    Texture2D& tex = TextureManager::Get(textureKey);

    // Chỉ vẽ các tile trong viewport (tối ưu hiệu suất)
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int startCol = (int)(cameraX / tileSize);
    int startRow = (int)(cameraY / tileSize);
    int endCol = startCol + (screenW / tileSize) + 2;
    int endRow = startRow + (screenH / tileSize) + 2;

    // Giới hạn trong map
    if (startCol < 0) startCol = 0;
    if (startRow < 0) startRow = 0;
    if (endCol > mapWidth) endCol = mapWidth;
    if (endRow > mapHeight) endRow = mapHeight;

    for (int row = startRow; row < endRow; row++) {
        for (int col = startCol; col < endCol; col++) {
            TileType type = grid[row][col];
            if (type == TileType::EMPTY) continue;

            TileInfo& info = tileInfoTable[(int)type];

            // Vị trí vẽ trên màn hình
            float drawX = col * tileSize - cameraX;
            float drawY = row * tileSize - cameraY;

            // Source rect từ spritesheet, dest rect trên màn hình
            Rectangle dest = { drawX, drawY, (float)tileSize, (float)tileSize };
            DrawTexturePro(tex, info.sourceRect, dest, {0, 0}, 0.0f, WHITE);
        }
    }
}
