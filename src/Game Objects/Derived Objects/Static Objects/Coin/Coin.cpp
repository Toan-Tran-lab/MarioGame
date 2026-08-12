#include "Coin.h"
#include "TextureManager/TextureManager.h"

void Coin::Draw() {
    if (!TextureManager::Has("coin")) {
        TextureManager::Load("coin", "assets/textures/coin.png");
        return;
    }

    Rectangle src = { 0.0f, 0.0f, (float)TextureManager::Get("coin").width,
                      (float)TextureManager::Get("coin").height };
    Rectangle dest = { position_.x, position_.y, size_.x, size_.y };
    DrawTexturePro(TextureManager::Get("coin"), src, dest, { 0.0f, 0.0f }, 0.0f, WHITE);
}