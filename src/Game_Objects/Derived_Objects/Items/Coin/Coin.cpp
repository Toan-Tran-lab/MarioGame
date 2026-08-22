#include "Coin.h"
#include "TextureManager/TextureManager.h"

void Coin::Update(float dt) {
    animTimer_ += dt;
    if (animTimer_ >= 0.15f) {
        animTimer_ = 0.0f;
        animFrame_ = (animFrame_ + 1) % 4;
    }

    if (isPopping_) {
        const float gravity = 900.0f;
        position_.y += velocityY_ * dt;
        velocityY_ += gravity * dt;

        if (velocityY_ > 0.0f && position_.y >= basePositionY_) {
            position_.y = basePositionY_;
            velocityY_ = 0.0f;
            isPopping_ = false;
            hasLanded_ = true;
            settledTimer_ = 0.0f;
        }
    } else if (hasLanded_ && awardsScoreOnCollect_) {
        settledTimer_ += dt;
        if (settledTimer_ >= 3.0f) SetActive(false);
    }
}

void Coin::Draw() {
    if (!TextureManager::Has("coin")) {
        TextureManager::Load("coin", "assets/textures/coin/coin.png");
        return;
    }

    const float e = 0.5f;
    Rectangle src = { (float)(animFrame_/2 * 16) + e, (float)(animFrame_%2 * 16) + e, 16.0f - 2.0f * e, 16.0f - 2.0f * e };
    Rectangle dest = { position_.x, position_.y, size_.x, size_.y };
    DrawTexturePro(TextureManager::Get("coin"), src, dest, { 0.0f, 0.0f }, 0.0f, WHITE);
}