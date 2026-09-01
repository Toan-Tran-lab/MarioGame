#include "Bullet.h"
#include "Game_Objects/Interaction_Resolve/Visitor.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"

namespace {
constexpr float kFrameDuration = 0.1f;
}

// 16x16 frame size, 3 frames in the 48x16 sprite sheet
static const Animation bulletAnim("bullet", 16, 16, 0, 3, {kFrameDuration});

Bullet::Bullet(Vector2 startPos, float direction)
    : direction_(direction)
{
    position_ = startPos;
    size_ = { 16.0f * Global::GAME_SCALE, 16.0f * Global::GAME_SCALE };
    physicsBody_.position = position_;
    physicsBody_.size = size_;
    physicsBody_.velocity = { kSpeed * direction_, 0.0f };
    animState.SetAnimation(&bulletAnim);
}

void Bullet::AcceptInteract(CharacterVisitor& other) {
    other.Visit(*this);
}

void Bullet::Update(float dt) {
    if (!IsActive()) return;

    position_.x += kSpeed * direction_ * dt;
    SyncPhysicsBody();
    animState.Update(dt);
}

void Bullet::Draw() {
    if (!IsActive()) return;

    if (!TextureManager::Has("bullet")) {
        TextureManager::Load("bullet", "assets/textures/Bullet/enemies.png");
    }

    // Invert facing so the bullet points in the moving direction
    FacingDirection drawFacing = (direction_ > 0.0f) ? FacingDirection::Right : FacingDirection::Left;
    animState.Draw(position_, drawFacing, size_);
}
