#include "PlayerFireball.h"
#include "Global/Global.h"
#include "TextureManager/TextureManager.h"
#include "physics/PhysicsEngine.h" // For gravity

static const Animation playerFireballAnim("player_fireball", 29, 48, 0, 4, {0.1f});

PlayerFireball::PlayerFireball(Vector2 startPos, float direction) {
    position_ = startPos;
    // Set logical size to 16x16 for collisions, even though sprite is 16x21
    size_ = { 16.0f * Global::GAME_SCALE, 16.0f * Global::GAME_SCALE };
    velocity_ = { kSpeed * direction, 0.0f };
    animState.SetAnimation(&playerFireballAnim);
}

void PlayerFireball::Update(float dt) {
    if (exploded_) {
        SetActive(false); // Destroy instantly upon explosion
        return;
    }

    lifeTimer_ += dt;
    if (lifeTimer_ >= kMaxLifetime) {
        SetActive(false);
        return;
    }

    // Apply basic gravity. GameplayState handles collisions and bouncing.
    velocity_.y += 1800.0f * dt; // Gravity

    // GameplayState applies velocity to position_
    animState.Update(dt);
}

void PlayerFireball::Draw() {
    if (exploded_ || !IsActive()) return;

    if (!TextureManager::Has("player_fireball")) {
        TextureManager::Load("player_fireball", "assets/textures/Fireball/Fireball.png");
    }

    // The sprite is 16x21, but logical size is 16x16. 
    // We can draw it aligned to the bottom by subtracting the extra height.
    Vector2 drawPos = position_;
    float drawHeight = 21.0f * Global::GAME_SCALE;
    drawPos.y -= (drawHeight - size_.y);

    animState.Draw(drawPos, velocity_.x > 0 ? FacingDirection::Right : FacingDirection::Left, {size_.x, drawHeight});
}

void PlayerFireball::Explode() {
    if (exploded_) return;
    Projectile::Explode(); // Sets exploded_ = true
    SetActive(false); // Can despawn immediately, no AOE
}
