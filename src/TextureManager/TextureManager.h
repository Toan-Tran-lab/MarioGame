#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>

class TextureManager {
private:
    // Lưu trữ tất cả texture đã load, tránh load trùng lặp
    static std::unordered_map<std::string, Texture2D> textures;

public:
    // Load texture từ file và lưu với tên key
    // Nếu key đã tồn tại thì không load lại
    static void Load(const std::string& key, const std::string& filePath);

    // Lấy texture đã load theo key
    static Texture2D& Get(const std::string& key);

    // Kiểm tra texture đã được load chưa
    static bool Has(const std::string& key);

    // Lấy source rectangle từ spritesheet theo chỉ số (index)
    // tileWidth, tileHeight: kích thước mỗi tile trong spritesheet
    // index: chỉ số tile (đếm từ trái sang phải, trên xuống dưới, bắt đầu từ 0)
    static Rectangle GetSourceRect(const std::string& key, int tileWidth, int tileHeight, int index);

    // Giải phóng một texture theo key
    static void Unload(const std::string& key);

    // Giải phóng tất cả texture
    static void UnloadAll();
};
