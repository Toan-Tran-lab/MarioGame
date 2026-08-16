#include "TextureManager.h"
#include <stdexcept>

// Khởi tạo static map
std::unordered_map<std::string, Texture2D> TextureManager::textures;

void TextureManager::Load(const std::string& key, const std::string& filePath) {
    // Nếu đã load rồi thì bỏ qua
    if (textures.find(key) != textures.end()) return;

    Texture2D tex = LoadTexture(filePath.c_str());

    // Kiểm tra load thành công (texture hợp lệ có id > 0)
    if (tex.id == 0) {
        TraceLog(LOG_ERROR, "TextureManager: Không thể load texture '%s' từ '%s'",
                 key.c_str(), filePath.c_str());
        return;
    }

    // LỆNH QUAN TRỌNG: Tắt nội suy mượt (setSmooth(false)) để pixel không bị mờ khi phóng to
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    textures[key] = tex;
    TraceLog(LOG_INFO, "TextureManager: Đã load texture '%s' (%dx%d)",
             key.c_str(), tex.width, tex.height);
}

Texture2D& TextureManager::Get(const std::string& key) {
    auto it = textures.find(key);
    if (it == textures.end()) {
        TraceLog(LOG_ERROR, "TextureManager: Không tìm thấy texture '%s'", key.c_str());
        throw std::runtime_error("TextureManager: texture '" + key + "' not found");
    }
    return it->second;
}

bool TextureManager::Has(const std::string& key) {
    return textures.find(key) != textures.end();
}

Rectangle TextureManager::GetSourceRect(const std::string& key, int tileWidth, int tileHeight, int index) {
    Texture2D& tex = Get(key);

    // Số cột trong spritesheet
    int cols = tex.width / tileWidth;

    // Tính vị trí hàng và cột từ index
    int col = index % cols;
    int row = index / cols;

    float x = (float)(col * tileWidth);
    float y = (float)(row * tileHeight);
    float w = (float)tileWidth;
    float h = (float)tileHeight;

    // Inset nửa texel để tránh bleeding pixel từ frame/tile kề nhau trong spritesheet
    // (khi vẽ ở tọa độ không nguyên, GPU có thể sample đúng đường nối giữa 2 ô)
    const float e = 0.5f;
    x += e;
    y += e;
    w -= 2.0f * e;
    h -= 2.0f * e;

    return Rectangle{ x, y, w, h };
}

void TextureManager::Unload(const std::string& key) {
    auto it = textures.find(key);
    if (it != textures.end()) {
        UnloadTexture(it->second);
        textures.erase(it);
    }
}

void TextureManager::UnloadAll() {
    for (auto& [key, tex] : textures) {
        UnloadTexture(tex);
    }
    textures.clear();
    TraceLog(LOG_INFO, "TextureManager: Đã giải phóng tất cả texture");
}
