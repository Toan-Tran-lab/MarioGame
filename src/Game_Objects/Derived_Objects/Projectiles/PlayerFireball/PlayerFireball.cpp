#include "PlayerFireball.h"
#include "Global/Global.h"
#include "TextureManager/TextureManager.h"
#include "physics/PhysicsEngine.h" // For gravity
#include "Game_Objects/Derived_Objects/Enemies/KoopaShell/KoopaShell.h"
#include "Game_Objects/Derived_Objects/Enemies/BuzzyBeetle/BuzzyBeetle.h"

static const Animation playerFireballAnim("player_fireball", 29, 48, 0, 4, {0.1f});
static const Animation explosionAnim("fireball_explosion", 166, 141, 0, 4, {0.05f});

PlayerFireball::PlayerFireball(Vector2 startPos, float direction) {
    position_ = startPos;
    // Set logical size to 16x16 for collisions
    size_ = { 16.0f * Global::GAME_SCALE, 16.0f * Global::GAME_SCALE };
    velocity_ = { kSpeed * direction, 0.0f };
    animState.SetAnimation(&playerFireballAnim);
}

void PlayerFireball::Update(float dt) {
    animState.Update(dt);

    if (exploded_) {
        // Wait until explosion animation finishes (4 frames * 0.05f = 0.2f)
        explosionTimer_ += dt;
        if (explosionTimer_ >= 0.2f) {
            SetActive(false);
        }
        return;
    }

    lifeTimer_ += dt;
    if (lifeTimer_ >= kMaxLifetime) {
        Explode();
        return;
    }

    // Apply basic gravity. GameplayState handles collisions and bouncing.
    velocity_.y += 1800.0f * dt; // Gravity
}

void PlayerFireball::Draw() {
    if (!IsActive()) return;

    if (!TextureManager::Has("player_fireball")) {
        TextureManager::Load("player_fireball", "assets/textures/Fireball/Fireball.png");
    }
    if (!TextureManager::Has("fireball_explosion")) {
        TextureManager::Load("fireball_explosion", "assets/textures/Explosion/Explosion.png");
    }

    Vector2 drawPos = position_;
    
    // Scale down if it's the big explosion, otherwise use GAME_SCALE
    float scaleX = exploded_ ? 48.0f : 29.0f;
    float scaleY = exploded_ ? 48.0f : 48.0f;
    
    // Center explosion, otherwise align fireball bottom
    if (exploded_) {
        drawPos.x -= (scaleX - size_.x) / 2.0f;
        drawPos.y -= (scaleY - size_.y) / 2.0f;
    } else {
        drawPos.x -= (scaleX - size_.x) / 2.0f;
        drawPos.y -= (scaleY - size_.y);
    }

    animState.Draw(drawPos, velocity_.x > 0 ? FacingDirection::Right : FacingDirection::Left, {scaleX, scaleY});
}

void PlayerFireball::Explode() {
    if (!exploded_) {
        Projectile::Explode(); // Sets exploded_ = true
        velocity_ = {0.0f, 0.0f}; // Stop moving
        animState.SetAnimation(&explosionAnim);
    }
}

void PlayerFireball::OnHitShell(KoopaShell& shell) {
    bool hitFromLeft = position_.x < shell.GetPosition().x;
    shell.TriggerUpsideDownDeath(hitFromLeft);
    Explode();
}

void PlayerFireball::OnHitEnemy(GroundEnemy& enemy) {
    // Fireballs do nothing to BuzzyBeetle, but still explode
    if (dynamic_cast<class BuzzyBeetle*>(&enemy)) {
        Explode();
        return;
    }
    bool hitFromLeft = position_.x < enemy.GetPosition().x;
    enemy.TriggerUpsideDownDeath(hitFromLeft);
    Explode();
}
