#pragma once
#include "raylib.h"
#include "TextureManager/TextureManager.h"
#include <string>
#include <vector>

struct DebrisPiece {
    Vector2 position = {0, 0};
    Vector2 velocity = {0, 0};
    float rotation = 0.0f;
    float rotationSpeed = 0.0f;
    Rectangle srcRect = {0, 0, 0, 0};
    std::string textureKey;
    Vector2 size = {0, 0};
    float lifeTimer = 0.0f;
    bool active = true;

    void Update(float dt) {
        if (!active) return;
        position.x += velocity.x * dt;
        position.y += velocity.y * dt;
        velocity.y += 1000.0f * dt; // gravity
        rotation += rotationSpeed * dt;
        lifeTimer += dt;
        if (lifeTimer > 2.0f) {
            active = false;
        }
    }

    void Draw() const {
        if (!active) return;
        if (!textureKey.empty() && TextureManager::Has(textureKey)) {
            Texture2D& tex = TextureManager::Get(textureKey);
            const float e = 0.5f;
            Rectangle insetSrc = { srcRect.x + e, srcRect.y + e,
                                   (srcRect.width  > 0 ? srcRect.width  - 2.0f * e : srcRect.width  + 2.0f * e),
                                   (srcRect.height > 0 ? srcRect.height - 2.0f * e : srcRect.height + 2.0f * e) };
            if (srcRect.width == 0 || srcRect.height == 0) insetSrc = srcRect;
            Rectangle destRect = { position.x + size.x / 2.0f, position.y + size.y / 2.0f, size.x, size.y };
            Vector2 origin = { size.x / 2.0f, size.y / 2.0f };
            DrawTexturePro(tex, insetSrc, destRect, origin, rotation, WHITE);
        } else {
            // Fallback rectangle if no texture
            Rectangle destRect = { position.x + size.x / 2.0f, position.y + size.y / 2.0f, size.x, size.y };
            Vector2 origin = { size.x / 2.0f, size.y / 2.0f };
            DrawRectanglePro(destRect, origin, rotation, BROWN);
        }
    }
};

inline void SpawnBlockDebris(std::vector<DebrisPiece>& debrisList, Rectangle blockRect, const std::string& texKey, Rectangle srcRect) {
    float halfW = blockRect.width / 2.0f;
    float halfH = blockRect.height / 2.0f;
    float srcHalfW = (srcRect.width != 0) ? (srcRect.width / 2.0f) : 8.0f;
    float srcHalfH = (srcRect.height != 0) ? (srcRect.height / 2.0f) : 8.0f;

    struct QuadInfo {
        Vector2 posOffset;
        Vector2 vel;
        Rectangle srcOffset;
        float rotSpeed;
    } quads[4] = {
        { { 0, 0 },         { -120.0f, -380.0f }, { srcRect.x,            srcRect.y,            srcHalfW, srcHalfH }, -500.0f },
        { { halfW, 0 },     {  120.0f, -380.0f }, { srcRect.x + srcHalfW, srcRect.y,            srcHalfW, srcHalfH },  500.0f },
        { { 0, halfH },     { -90.0f,  -220.0f }, { srcRect.x,            srcRect.y + srcHalfH, srcHalfW, srcHalfH }, -400.0f },
        { { halfW, halfH }, {  90.0f,  -220.0f }, { srcRect.x + srcHalfW, srcRect.y + srcHalfH, srcHalfW, srcHalfH },  400.0f }
    };

    for (int i = 0; i < 4; ++i) {
        DebrisPiece p;
        p.position = { blockRect.x + quads[i].posOffset.x, blockRect.y + quads[i].posOffset.y };
        p.velocity = quads[i].vel;
        p.rotation = 0.0f;
        p.rotationSpeed = quads[i].rotSpeed;
        p.srcRect = quads[i].srcOffset;
        p.textureKey = texKey;
        p.size = { halfW, halfH };
        p.lifeTimer = 0.0f;
        p.active = true;
        debrisList.push_back(p);
    }
}
