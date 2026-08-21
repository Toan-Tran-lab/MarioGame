#include "Fireball.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "TextureManager/TextureManager.h"

Fireball::Fireball(Vector2 startPos, float direction) {
    position_ = startPos;
    size_ = { 16.0f, 16.0f };
    velocity_ = { kSpeed * direction, 0.0f };
}

bool Fireball::CanHurtPlayer(const Player& player) const {
    return !player.IsProjectileImmune();
}

void Fireball::Update(float dt) {
    if (exploded_) {
        explodeTimer_ += dt;
        if (explodeTimer_ >= kExplodeVisualDuration) {
            SetActive(false);
        }
        return;
    }

    position_ = { position_.x + velocity_.x * dt, position_.y + velocity_.y * dt };

    lifeTimer_ += dt;
    if (lifeTimer_ >= kMaxLifetime) {
        SetActive(false); // despawn quietly if it flew off without hitting anything
    }
}

void Fireball::Draw() {
    if (exploded_) {
        float t = explodeTimer_ / kExplodeVisualDuration;
        DrawCircle((int)(position_.x + size_.x / 2.0f), (int)(position_.y + size_.y / 2.0f),
                   kAOERadius * (1.0f - t) + 8.0f, Fade(ORANGE, 1.0f - t));
        return;
    }
    // Placeholder until real sprite exists.
    DrawCircle((int)(position_.x + size_.x / 2.0f), (int)(position_.y + size_.y / 2.0f), 8.0f, ORANGE);
}