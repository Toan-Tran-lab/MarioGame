#include "DragonFlame.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"

namespace {
static Animation s_dragonFlameAnim("boss_flame", 24, 18, 0, 2, {0.12f});
}

DragonFlame::DragonFlame(Vector2 mouthPos, float directionX) {
    if (!TextureManager::Has("boss_flame")) {
        TextureManager::Load("boss_flame", "assets/textures/Boss/flame/boss.png");
    }

    directionX_ = directionX;
    mouthPos_ = mouthPos;
    growth_ = 0.1f;
    facing_ = (directionX < 0.0f) ? FacingDirection::Left : FacingDirection::Right;
    animState_.SetAnimation(&s_dragonFlameAnim);
    SetMouthAnchor(mouthPos, directionX, 0.1f);
}

void DragonFlame::SetMouthAnchor(Vector2 mouthPos, float directionX, float growth) {
    mouthPos_ = mouthPos;
    directionX_ = directionX;
    growth_ = (growth < 0.0f) ? 0.0f : (growth > 1.0f ? 1.0f : growth);
    facing_ = (directionX < 0.0f) ? FacingDirection::Left : FacingDirection::Right;

    // Base size 24x18 scales up horizontally to simulate expanding fire breath
    float baseW = 24.0f * Global::GAME_SCALE;
    float baseH = 18.0f * Global::GAME_SCALE;

    // Width expands from 1.0x to 3.2x as flame breath progresses
    float currentW = baseW * (1.0f + growth_ * 2.2f);
    float currentH = baseH * (1.0f + growth_ * 0.6f);

    size_ = { currentW, currentH };

    if (directionX_ < 0.0f) {
        position_ = { mouthPos_.x - currentW, mouthPos_.y - currentH * 0.5f };
    } else {
        position_ = { mouthPos_.x, mouthPos_.y - currentH * 0.5f };
    }
}

void DragonFlame::StopBreathing() {
    SetActive(false);
}

bool DragonFlame::CanHurtPlayer(const Player& player) const {
    return !player.IsDead() && !player.IsInvincible() && !player.IsHitInvincible();
}

void DragonFlame::Update(float dt) {
    if (!IsActive()) return;
    animState_.Update(dt);
}

void DragonFlame::Draw() {
    if (!IsActive()) return;

    FacingDirection renderFacing = (facing_ == FacingDirection::Left) ? FacingDirection::Right : FacingDirection::Left;
    animState_.Draw(position_, renderFacing, size_);
}
