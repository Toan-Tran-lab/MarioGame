#include "DragonFlame.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"

namespace {
static Animation s_dragonFlameAnim("boss_flame", 24, 18, 0, 2, {0.12f});
}

DragonFlame::DragonFlame(Vector2 startPos, float directionX) {
    if (!TextureManager::Has("boss_flame")) {
        TextureManager::Load("boss_flame", "assets/textures/Boss/flame/boss.png");
    }

    position_ = startPos;
    size_ = { 24.0f * Global::GAME_SCALE, 18.0f * Global::GAME_SCALE };
    velocity_ = { kSpeed * directionX, 0.0f };
    facing_ = (directionX < 0.0f) ? FacingDirection::Left : FacingDirection::Right;

    animState_.SetAnimation(&s_dragonFlameAnim);
}

bool DragonFlame::CanHurtPlayer(const Player& player) const {
    return !player.IsDead() && !player.IsInvincible() && !player.IsHitInvincible();
}

void DragonFlame::Update(float dt) {
    if (exploded_) {
        explodeTimer_ += dt;
        if (explodeTimer_ >= kExplodeDuration) {
            SetActive(false);
        }
        return;
    }

    animState_.Update(dt);
    position_.x += velocity_.x * dt;
    position_.y += velocity_.y * dt;

    lifeTimer_ += dt;
    if (lifeTimer_ >= kMaxLifetime) {
        SetActive(false);
    }
}

void DragonFlame::Draw() {
    if (!IsActive()) return;

    if (exploded_) {
        float t = explodeTimer_ / kExplodeDuration;
        DrawCircle((int)(position_.x + size_.x / 2.0f), (int)(position_.y + size_.y / 2.0f),
                   (size_.x / 2.0f) * (1.0f - t * 0.5f), Fade(RED, 1.0f - t));
        return;
    }

    animState_.Draw(position_, facing_, size_);
}
