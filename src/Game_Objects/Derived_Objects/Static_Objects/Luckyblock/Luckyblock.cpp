#include "Luckyblock.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"
#include <cmath>
#include <cstdlib>

namespace {
    const Animation luckyIdleAnim("luckyblock", 16, 16, 0, 4, {0.15f});
    const Animation luckyEmptyAnim("luckyblock", 16, 16, 4, 1, {1.0f});

    constexpr float kBumpGravity = 900.0f;
    constexpr float kBumpInitialVel = -150.0f;
    constexpr float kMushroomChance = 0.35f; // 35% mushroom, 65% coin
}

Luckyblock::Luckyblock()
    : isEmpty_(false)
{
    size_ = { 16.0f * Global::GAME_SCALE, 16.0f * Global::GAME_SCALE };
    animState.SetAnimation(&luckyIdleAnim);
}

Luckyblock::~Luckyblock() {}

bool Luckyblock::Bump() {
    if (!isEmpty_) {
        isEmpty_ = true;
        animState.SetAnimation(&luckyEmptyAnim);

        float roll = (float)rand() / (float)RAND_MAX; // [0, 1)
        lastContents_ = (roll < kMushroomChance) ? LuckyContents::Mushroom : LuckyContents::Coin;

        Block::Bump();
        return true;
    }
    Block::Bump();
    return false;
}

void Luckyblock::Update(float dt) {
    Block::Update(dt);
    if (!isEmpty_) {
        animState.Update(dt);
    }
}

void Luckyblock::Draw() {
    if (!TextureManager::Has("luckyblock")) {
        TextureManager::Load("luckyblock", "assets/textures/Luckyblock/luckyblock.png");
    }
    Vector2 drawPos = { position_.x, position_.y };
    animState.Draw(drawPos, FacingDirection::Right, size_);
}