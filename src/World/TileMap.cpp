#include "TileMap.h"
#include "TextureManager/TextureManager.h"
#include <nlohmann/json.hpp>
#include <algorithm>

// ========== Constructor ==========
TileMap::TileMap()
    : mapWidth(0), mapHeight(0), gridSize(16), tileSize(48), bgColor(SKYBLUE), playerSpawn({0,0}),
      borderLeft(0.0f), borderRight(0.0f), borderTop(0.0f), borderBottom(0.0f), isTargetBuilt(false) {
    mapTarget.id = 0;
}

TileMap::~TileMap() {
    if (mapTarget.id != 0) {
        UnloadRenderTexture(mapTarget);
    }
}

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
        };
        std::unordered_map<int, TilesetInfo> tilesetLookup;

        if (j.contains("defs") && j["defs"].contains("tilesets") && j["defs"]["tilesets"].is_array()) {
            for (const auto& ts : j["defs"]["tilesets"]) {
                int uid = ts.value("uid", -1);
                TilesetInfo info;
                info.relPath = ts.value("relPath", "");
                info.tileGridSize = ts.value("tileGridSize", 16);
                info.spacing = ts.value("spacing", 0);
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
        collisionGrid.clear();

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
                    for (const auto& gt : li["gridTiles"]) {
                        LdtkTile tile;
                        tile.px[0]  = gt["px"][0].get<int>();
                        tile.px[1]  = gt["px"][1].get<int>();
                        tile.src[0] = gt["src"][0].get<int>();
                        tile.src[1] = gt["src"][1].get<int>();
                        tile.f      = gt.value("f", 0);
                        tl.tiles.push_back(tile);
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

                collisionGrid.resize(cHei);
                for (int row = 0; row < cHei; row++) {
                    collisionGrid[row].resize(cWid, 0);
                }

                if (li.contains("intGridCsv") && li["intGridCsv"].is_array()) {
                    const auto& csv = li["intGridCsv"];
                    for (int i = 0; i < (int)csv.size() && i < cWid * cHei; i++) {
                        int row = i / cWid;
                        int col = i % cWid;
                        collisionGrid[row][col] = csv[i].get<int>();
                    }
                }

                TraceLog(LOG_INFO, "TILEMAP: Loaded collision grid (%dx%d)", cWid, cHei);
            }
            else if (type == "Entities") {
                // --- Entity layer ---
                if (li.contains("entityInstances") && li["entityInstances"].is_array()) {
                    for (const auto& ent : li["entityInstances"]) {
                        std::string entId = ent.value("__identifier", "");
                        if (entId == "Player") {
                            if (ent.contains("px") && ent["px"].is_array() && ent["px"].size() >= 2) {
                                float pxX = ent["px"][0].get<float>();
                                float pxY = ent["px"][1].get<float>();
                                float scale = (float)tileSize / (float)gridSize;
                                playerSpawn.x = pxX * scale;
                                playerSpawn.y = pxY * scale;
                                TraceLog(LOG_INFO, "TILEMAP: Found player spawn at (%f, %f)", playerSpawn.x, playerSpawn.y);
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
                 
        BuildMapTexture();
                 
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
bool TileMap::IsSolidAt(int col, int row) const {
    if (!IsInBounds(col, row)) return false;
    if (collisionGrid.empty()) return false;
    if (row < 0 || row >= (int)collisionGrid.size()) return false;
    if (col < 0 || col >= (int)collisionGrid[row].size()) return false;
    return collisionGrid[row][col] != 0;
}

bool TileMap::IsInBounds(int col, int row) const {
    return col >= 0 && col < mapWidth && row >= 0 && row < mapHeight;
}

int TileMap::GetMapWidth() const { return mapWidth; }
int TileMap::GetMapHeight() const { return mapHeight; }
int TileMap::GetTileSize() const { return tileSize; }
int TileMap::GetPixelWidth() const { return mapWidth * tileSize; }
int TileMap::GetPixelHeight() const { return mapHeight * tileSize; }
Color TileMap::GetBackgroundColor() const { return bgColor; }
Vector2 TileMap::GetPlayerSpawn() const { return playerSpawn; }

float TileMap::GetBorderLeft() const { return borderLeft; }
float TileMap::GetBorderRight() const { return borderRight; }
float TileMap::GetBorderTop() const { return borderTop; }
float TileMap::GetBorderBottom() const { return borderBottom; }

std::vector<Rectangle> TileMap::GetCollisionRects() const {
    std::vector<Rectangle> rects;
    if (collisionGrid.empty()) return rects;

    for (int row = 0; row < (int)collisionGrid.size(); row++) {
        int startCol = -1;
        for (int col = 0; col < (int)collisionGrid[row].size(); col++) {
            if (collisionGrid[row][col] != 0) {
                if (startCol == -1) startCol = col; // Start a new block
            } else {
                if (startCol != -1) { // End the block
                    rects.push_back({
                        (float)(startCol * tileSize),
                        (float)(row * tileSize),
                        (float)((col - startCol) * tileSize),
                        (float)tileSize
                    });
                    startCol = -1;
                }
            }
        }
        // If the row ended with a block, push it
        if (startCol != -1) {
            rects.push_back({
                (float)(startCol * tileSize),
                (float)(row * tileSize),
                (float)((collisionGrid[row].size() - startCol) * tileSize),
                (float)tileSize
            });
        }
    }
    return rects;
}

// ========== Coordinate conversion ==========
Vector2 TileMap::WorldToTile(float worldX, float worldY) const {
    return { worldX / tileSize, worldY / tileSize };
}

Vector2 TileMap::TileToWorld(int col, int row) const {
    return { (float)(col * tileSize), (float)(row * tileSize) };
}

void TileMap::BuildMapTexture() {
    if (mapWidth <= 0 || mapHeight <= 0) return;

    if (mapTarget.id != 0) {
        UnloadRenderTexture(mapTarget);
    }
    
    mapTarget = LoadRenderTexture(GetPixelWidth(), GetPixelHeight());
    SetTextureFilter(mapTarget.texture, TEXTURE_FILTER_POINT);
    
    BeginTextureMode(mapTarget);
    ClearBackground(bgColor);

    float scale = (float)tileSize / (float)gridSize;

    for (const auto& layer : layers) {
        Texture2D& tex = TextureManager::Get(layer.textureKey);
        int layerGridSize = layer.gridSize;

        for (const auto& tile : layer.tiles) {
            float drawX = tile.px[0] * scale;
            float drawY = tile.px[1] * scale;
            float drawSize = layerGridSize * scale;

            float srcW = (float)layerGridSize;
            float srcH = (float)layerGridSize;

            if (tile.f & 1) srcW = -srcW;
            if (tile.f & 2) srcH = -srcH;

            // Shrink source rect slightly to prevent texture atlas bleeding (sampling neighboring pixels)
            float margin = 0.02f;
            Rectangle srcRect = { 
                (float)tile.src[0] + margin, 
                (float)tile.src[1] + margin, 
                srcW > 0 ? srcW - 2.0f*margin : srcW + 2.0f*margin, 
                srcH > 0 ? srcH - 2.0f*margin : srcH + 2.0f*margin 
            };
            
            // Expand destination rect slightly to prevent 1-pixel transparent seams when baking
            Rectangle destRect = { drawX, drawY, drawSize + 0.5f, drawSize + 0.5f };

            DrawTexturePro(tex, srcRect, destRect, {0, 0}, 0.0f, WHITE);
        }
    }
    
    EndTextureMode();
    isTargetBuilt = true;
    
    TraceLog(LOG_INFO, "TILEMAP: Built map render target (%dx%d)", mapTarget.texture.width, mapTarget.texture.height);
}

// ========== Draw ==========
void TileMap::Draw(float cameraX, float cameraY, float cameraZoom) {
    if (!isTargetBuilt) return;
    
    // The negative height flips it right-side up (OpenGL requirement for RenderTextures)
    Rectangle source = { 0.0f, 0.0f, (float)mapTarget.texture.width, -(float)mapTarget.texture.height };
    DrawTextureRec(mapTarget.texture, source, { 0.0f, 0.0f }, WHITE);
}
