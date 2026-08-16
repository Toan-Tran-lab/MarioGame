#include "TileMap.h"
#include "TextureManager/TextureManager.h"
#include <nlohmann/json.hpp>
#include <algorithm>

// ========== Constructor ==========
TileMap::TileMap()
    : mapWidth(0), mapHeight(0), gridSize(16), tileSize(48), bgColor(SKYBLUE), playerSpawn({0,0}),
      borderLeft(0.0f), borderRight(0.0f), borderTop(0.0f), borderBottom(0.0f) {}

TileMap::~TileMap() {}

// ========== Load from LDtk ==========
bool TileMap::LoadFromLdtk(const std::string& filePath, const std::string& levelId) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        TraceLog(LOG_ERROR, "TILEMAP: Cannot open LDtk file: %s", filePath.c_str());
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;

        // --- Find the requested level ---
        const nlohmann::json* levelPtr = nullptr;
        if (j.contains("levels") && j["levels"].is_array()) {
            for (const auto& lvl : j["levels"]) {
                if (lvl.value("identifier", "") == levelId) {
                    levelPtr = &lvl;
                    break;
                }
            }
        }
        if (!levelPtr) {
            TraceLog(LOG_ERROR, "TILEMAP: Level '%s' not found in %s", levelId.c_str(), filePath.c_str());
            return false;
        }
        const auto& level = *levelPtr;

        // --- Read level dimensions ---
        int pxWid = level.value("pxWid", 0);
        int pxHei = level.value("pxHei", 0);

        // --- Read background color ---
        std::string hexStr = level.value("__bgColor", "#87CEEB"); // default skyblue
        if (hexStr.length() > 0 && hexStr[0] == '#') hexStr = hexStr.substr(1);
        if (hexStr.length() == 6) hexStr += "FF";
        try {
            unsigned int val = std::stoul(hexStr, nullptr, 16);
            bgColor = GetColor(val);
        } catch (...) {
            bgColor = SKYBLUE;
        }

        // --- Build a tileset uid -> info lookup from the defs ---
        struct TilesetInfo {
            std::string relPath;
            int tileGridSize;
            int spacing;
            std::vector<int> luckyTiles;
        };
        std::unordered_map<int, TilesetInfo> tilesetLookup;

        if (j.contains("defs") && j["defs"].contains("tilesets") && j["defs"]["tilesets"].is_array()) {
            for (const auto& ts : j["defs"]["tilesets"]) {
                int uid = ts.value("uid", -1);
                TilesetInfo info;
                info.relPath = ts.value("relPath", "");
                info.tileGridSize = ts.value("tileGridSize", 16);
                info.spacing = ts.value("spacing", 0);
                
                if (ts.contains("customData") && ts["customData"].is_array()) {
                    for (const auto& cd : ts["customData"]) {
                        if (cd.value("data", "") == "Luckyblock") {
                            info.luckyTiles.push_back(cd.value("tileId", -1));
                        }
                    }
                }
                
                tilesetLookup[uid] = info;
            }
        }

        // --- Determine native grid size from the first tileset or default ---
        gridSize = 16;
        if (!tilesetLookup.empty()) {
            gridSize = tilesetLookup.begin()->second.tileGridSize;
        }

        // Calculate map dimensions in tiles
        mapWidth  = pxWid / gridSize;
        mapHeight = pxHei / gridSize;

        if (mapWidth <= 0 || mapHeight <= 0) {
            TraceLog(LOG_ERROR, "TILEMAP: Invalid level dimensions %dx%d", pxWid, pxHei);
            return false;
        }

        // --- Parse layer instances ---
        layers.clear();
        entities.clear();

        if (!level.contains("layerInstances") || !level["layerInstances"].is_array()) {
            TraceLog(LOG_ERROR, "TILEMAP: No layerInstances in level '%s'", levelId.c_str());
            return false;
        }

        // LDtk stores layers top-to-bottom (front-to-back in the editor),
        // so we reverse to get back-to-front render order
        std::vector<nlohmann::json> layerList;
        for (const auto& li : level["layerInstances"]) {
            layerList.push_back(li);
        }
        std::reverse(layerList.begin(), layerList.end());

        for (const auto& li : layerList) {
            std::string type = li.value("__type", "");
            std::string identifier = li.value("__identifier", "");

            if (type == "Tiles") {
                // --- Tile layer ---
                TileLayer tl;
                tl.identifier = identifier;
                tl.gridSize = li.value("__gridSize", gridSize);

                // Get tileset info
                int tilesetUid = li.value("__tilesetDefUid", -1);
                std::string tilesetRelPath = li.value("__tilesetRelPath", "");
                tl.tilesetRelPath = tilesetRelPath;

                // Derive texture key from identifier
                tl.textureKey = "ldtk_" + identifier;

                // Resolve the tileset path relative to the LDtk file
                // LDtk paths are relative to the .ldtk file location
                // e.g. "../textures/Tileset/tileset.png" -> "assets/textures/Tileset/tileset.png"
                std::string resolvedPath;
                {
                    // Get the directory of the LDtk file
                    std::string ldtkDir = filePath;
                    size_t lastSlash = ldtkDir.find_last_of("/\\");
                    if (lastSlash != std::string::npos) {
                        ldtkDir = ldtkDir.substr(0, lastSlash + 1);
                    } else {
                        ldtkDir = "";
                    }
                    resolvedPath = ldtkDir + tilesetRelPath;

                    // Normalize: replace backslashes with forward slashes
                    for (char& c : resolvedPath) {
                        if (c == '\\') c = '/';
                    }

                    // Resolve ".." components simply
                    // e.g. "assets/maps/../textures/..." -> "assets/textures/..."
                    std::string result;
                    std::vector<std::string> parts;
                    size_t start = 0;
                    for (size_t i = 0; i <= resolvedPath.size(); i++) {
                        if (i == resolvedPath.size() || resolvedPath[i] == '/') {
                            std::string part = resolvedPath.substr(start, i - start);
                            if (part == ".." && !parts.empty()) {
                                parts.pop_back();
                            } else if (part != "." && !part.empty()) {
                                parts.push_back(part);
                            }
                            start = i + 1;
                        }
                    }
                    for (size_t i = 0; i < parts.size(); i++) {
                        if (i > 0) result += "/";
                        result += parts[i];
                    }
                    resolvedPath = result;
                }

                // Load the texture
                if (!TextureManager::Has(tl.textureKey)) {
                    TextureManager::Load(tl.textureKey, resolvedPath);
                }

                // Parse grid tiles
                if (li.contains("gridTiles") && li["gridTiles"].is_array()) {
                    const auto& luckyList = tilesetLookup[tilesetUid].luckyTiles;
                    
                    for (const auto& gt : li["gridTiles"]) {
                        int tId = gt.value("t", -1);
                        bool isLucky = false;
                        for (int id : luckyList) {
                            if (id == tId) { isLucky = true; break; }
                        }
                        
                        int pxX = gt["px"][0].get<int>();
                        int pxY = gt["px"][1].get<int>();

                        if (isLucky) {
                            float scale = (float)tileSize / (float)gridSize;
                            Vector2 pos = { pxX * scale, pxY * scale };
                            entities.push_back({ "Luckyblock", pos });
                            TraceLog(LOG_INFO, "TILEMAP: Found Luckyblock at (%f, %f)", pos.x, pos.y);
                        } else {
                            LdtkTile tile;
                            tile.px[0]  = pxX;
                            tile.px[1]  = pxY;
                            tile.src[0] = gt["src"][0].get<int>();
                            tile.src[1] = gt["src"][1].get<int>();
                            tile.f      = gt.value("f", 0);
                            tl.tiles.push_back(tile);
                        }
                    }
                }

                // Only add layer if it has tiles and is visible
                bool visible = li.value("visible", true);
                if (!tl.tiles.empty() && visible) {
                    int tileCount = (int)tl.tiles.size();
                    layers.push_back(std::move(tl));
                    TraceLog(LOG_INFO, "TILEMAP: Loaded tile layer '%s' (%d tiles, tileset: %s)",
                             identifier.c_str(), tileCount, resolvedPath.c_str());
                } else {
                    TraceLog(LOG_INFO, "TILEMAP: Skipped tile layer '%s' (%d tiles, visible=%s)",
                             identifier.c_str(), (int)tl.tiles.size(), visible ? "true" : "false");
                }
            }
            else if (type == "IntGrid" && identifier == "Collisions") {
                // --- Collision IntGrid layer ---
                int cWid = li.value("__cWid", mapWidth);
                int cHei = li.value("__cHei", mapHeight);

                blockGrid.Init(cWid, cHei, tileSize);

                if (li.contains("intGridCsv") && li["intGridCsv"].is_array()) {
                    const auto& csv = li["intGridCsv"];
                    for (int i = 0; i < (int)csv.size() && i < cWid * cHei; i++) {
                        int row = i / cWid;
                        int col = i % cWid;
                        int val = csv[i].get<int>();
                        if (val != 0) {
                            auto tb = std::make_unique<TerrainBlock>();
                            tb->SetPosition({ (float)(col * tileSize), (float)(row * tileSize) });
                            tb->SetSize({ (float)tileSize, (float)tileSize });
                            blockGrid.SetBlock(col, row, std::move(tb));
                        }
                    }
                }

                TraceLog(LOG_INFO, "TILEMAP: Loaded BlockGrid (%dx%d)", cWid, cHei);
            }
            else if (type == "Entities") {
                // --- Entity layer ---
                if (li.contains("entityInstances") && li["entityInstances"].is_array()) {
                    for (const auto& ent : li["entityInstances"]) {
                        std::string entId = ent.value("__identifier", "");
                        if (ent.contains("px") && ent["px"].is_array() && ent["px"].size() >= 2) {
                            float pxX = ent["px"][0].get<float>();
                            float pxY = ent["px"][1].get<float>();
                            float scale = (float)tileSize / (float)gridSize;
                            Vector2 pos = { pxX * scale, pxY * scale };
                            
                            if (entId == "Player") {
                                playerSpawn = pos;
                                TraceLog(LOG_INFO, "TILEMAP: Found player spawn at (%f, %f)", playerSpawn.x, playerSpawn.y);
                            } else {
                                entities.push_back({ entId, pos });
                                TraceLog(LOG_INFO, "TILEMAP: Found entity %s at (%f, %f)", entId.c_str(), pos.x, pos.y);
                            }
                        }
                    }
                }
            }
        }

        TraceLog(LOG_INFO, "TILEMAP: Loaded LDtk level '%s' from %s (%dx%d tiles, %d layers, gridSize=%d, displaySize=%d)",
                 levelId.c_str(), filePath.c_str(), mapWidth, mapHeight, (int)layers.size(), gridSize, tileSize);
                 
        // Set default borders based on map size
        borderLeft = 0.0f;
        borderTop = 0.0f;
        borderRight = (float)GetPixelWidth();
        borderBottom = (float)GetPixelHeight();
        // We no longer clear the collision grid here to prevent internal edge collisions (snagging).
        // The block interactions will be handled via grid coordinates.
                 
        return true;

    } catch (const nlohmann::json::exception& e) {
        TraceLog(LOG_ERROR, "TILEMAP: JSON parse error in %s: %s", filePath.c_str(), e.what());
        return false;
    }
}

// ========== Setters ==========
void TileMap::SetTileSize(int size) {
    tileSize = size;
}

// ========== Queries ==========
bool TileMap::IsInBounds(int col, int row) const {
    return col >= 0 && col < mapWidth && row >= 0 && row < mapHeight;
}

int TileMap::GetMapWidth() const { return mapWidth; }
int TileMap::GetMapHeight() const { return mapHeight; }
int TileMap::GetTileSize() const { return tileSize; }
int TileMap::GetPixelWidth() const { return mapWidth * tileSize; }
int TileMap::GetPixelHeight() const { return mapHeight * tileSize; }
Color TileMap::GetBackgroundColor() const { return bgColor; }
Vector2 TileMap::GetPlayerSpawn() const {
    return playerSpawn;
}

const std::vector<EntitySpawnInfo>& TileMap::GetEntities() const {
    return entities;
}



float TileMap::GetBorderLeft() const { return borderLeft; }
float TileMap::GetBorderRight() const { return borderRight; }
float TileMap::GetBorderTop() const { return borderTop; }
float TileMap::GetBorderBottom() const { return borderBottom; }

bool TileMap::IsSolidAt(int col, int row) const {
    return blockGrid.IsSolidAt(col, row);
}

// GetCollisionRects removed in favor of BlockGrid

// ========== Coordinate conversion ==========
Vector2 TileMap::WorldToTile(float worldX, float worldY) const {
    return { worldX / tileSize, worldY / tileSize };
}

Vector2 TileMap::TileToWorld(int col, int row) const {
    return { (float)(col * tileSize), (float)(row * tileSize) };
}

bool TileMap::RemoveTileAt(int col, int row, std::string& outTexKey, Rectangle& outSrcRect) {
    int nativePxX = col * gridSize;
    int nativePxY = row * gridSize;

    for (auto& layer : layers) {
        for (auto it = layer.tiles.begin(); it != layer.tiles.end(); ++it) {
            if (it->px[0] == nativePxX && it->px[1] == nativePxY) {
                outTexKey = layer.textureKey;
                outSrcRect = { (float)it->src[0], (float)it->src[1], (float)layer.gridSize, (float)layer.gridSize };
                layer.tiles.erase(it);
                return true;
            }
        }
    }
    return false;
}

// ========== Draw ==========
void TileMap::Draw(float cameraX, float cameraY, float cameraZoom) {
    // 100% Dynamic drawing with viewport culling (prevents VRAM overhead and works on all map sizes!)
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    // Calculate screen boundaries in world space to draw only visible tiles
    float left = cameraX;
    float right = cameraX + sw / cameraZoom;
    float top = cameraY;
    float bottom = cameraY + sh / cameraZoom;

    float scale = (float)tileSize / (float)gridSize;

    for (const auto& layer : layers) {
        if (!TextureManager::Has(layer.textureKey)) continue;
        Texture2D& tex = TextureManager::Get(layer.textureKey);
        int layerGridSize = layer.gridSize;
        float drawSize = layerGridSize * scale;

        for (const auto& tile : layer.tiles) {
            float drawX = tile.px[0] * scale;
            float drawY = tile.px[1] * scale;

            // Offset Y if the block is undergoing a bump bounce animation
            int tileCol = tile.px[0] / gridSize;
            int tileRow = tile.px[1] / gridSize;
            Block* b = blockGrid.GetBlock(tileCol, tileRow);
            if (b != nullptr) {
                drawY += b->GetBumpOffsetY();
            }

            // Viewport culling check
            if (drawX + drawSize < left || drawX > right ||
                drawY + drawSize < top || drawY > bottom) {
                continue; // Skip out-of-bounds tiles
            }

            // --- Snap to screen pixel grid to eliminate seams ---
            float sx0 = floorf((drawX - cameraX) * cameraZoom);
            float sy0 = floorf((drawY - cameraY) * cameraZoom);
            float sx1 = floorf((drawX + drawSize - cameraX) * cameraZoom);
            float sy1 = floorf((drawY + drawSize - cameraY) * cameraZoom);

            // Convert back to world space for DrawTexturePro (we're inside BeginMode2D)
            float snappedX = sx0 / cameraZoom + cameraX;
            float snappedY = sy0 / cameraZoom + cameraY;
            float snappedW = (sx1 - sx0) / cameraZoom;
            float snappedH = (sy1 - sy0) / cameraZoom;

            // Inset nửa texel để tránh bleeding pixel giữa các tile kề nhau trong tileset
            const float e = 0.5f;
            float srcW = (float)layerGridSize - 2.0f * e;
            float srcH = (float)layerGridSize - 2.0f * e;

            if (tile.f & 1) srcW = -srcW;
            if (tile.f & 2) srcH = -srcH;

            Rectangle srcRect = { (float)tile.src[0] + e, (float)tile.src[1] + e, srcW, srcH };
            Rectangle destRect = { snappedX, snappedY, snappedW, snappedH };

            DrawTexturePro(tex, srcRect, destRect, {0, 0}, 0.0f, WHITE);
        }
    }
}

bool TileMap::LoadFromSandbox(const std::vector<std::vector<SandboxCellData>>& sandboxGrid) {
    if (sandboxGrid.empty() || sandboxGrid[0].empty()) return false;

    mapHeight = sandboxGrid.size();
    mapWidth = sandboxGrid[0].size();
    gridSize = 16;
    tileSize = 48;
    bgColor = Color{ 40, 20, 60, 255 }; // dark background for Sandbox custom level

    borderLeft = 0.0f;
    borderRight = (float)(mapWidth * tileSize);
    borderTop = 0.0f;
    borderBottom = (float)(mapHeight * tileSize);

    layers.clear();
    blockGrid.Init(mapWidth, mapHeight, tileSize);

    playerSpawn = { 100.0f, (float)((mapHeight - 3) * tileSize) }; // Default spawn if Player Start is not set

    // Group sandbox cells by their texture keys to build virtual TileLayers
    std::unordered_map<std::string, TileLayer> layerMap;

    for (int r = 0; r < mapHeight; ++r) {
        for (int c = 0; c < mapWidth; ++c) {
            const auto& cell = sandboxGrid[r][c];
            if (cell.type == 1) { // Tile brush
                if (layerMap.find(cell.texKey) == layerMap.end()) {
                    TileLayer tl;
                    tl.identifier = cell.texKey;
                    tl.textureKey = cell.texKey;
                    tl.gridSize = 16;
                    layerMap[cell.texKey] = tl;
                }
                
                LdtkTile tile;
                tile.px[0] = c * 16;
                tile.px[1] = r * 16;
                tile.src[0] = (int)cell.srcRect.x;
                tile.src[1] = (int)cell.srcRect.y;
                tile.f = 0;
                
                layerMap[cell.texKey].tiles.push_back(tile);

                if (cell.isSolid) {
                    auto tb = std::make_unique<TerrainBlock>();
                    tb->SetPosition({ (float)(c * tileSize), (float)(r * tileSize) });
                    tb->SetSize({ (float)tileSize, (float)tileSize });
                    blockGrid.SetBlock(c, r, std::move(tb));
                }
            } else if (cell.type == 2) {
                // Player Spawn (type 2)
                playerSpawn = { (float)(c * tileSize), (float)(r * tileSize) };
            }
        }
    }

    // Move layer instances to our layers list
    for (auto& pair : layerMap) {
        layers.push_back(pair.second);
    }

    return true;
}
