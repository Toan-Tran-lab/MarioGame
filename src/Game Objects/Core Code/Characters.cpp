#include "Game Objects/Core Header Files/Characters.h"

namespace {
constexpr float kGravity = 500.0f;      // pixels/second^2
constexpr float kMaxFallSpeed = 800.0f; // pixels/second
}

void Character::ApplyMotion(float dt) {
    // Gravity only pulls while airborne; landing is handled by the collision
    // system calling SetGrounded(true) and zeroing vertical velocity.
    if (!grounded_) {
        velocity_.y += kGravity * dt;
        if (velocity_.y > kMaxFallSpeed) velocity_.y = kMaxFallSpeed;
    }

    position_ = position_ + velocity_ * dt;

    // Auto-update facing direction based on horizontal movement
    if (velocity_.x > 0.0f) facing_ = FacingDirection::Right;
    else if (velocity_.x < 0.0f) facing_ = FacingDirection::Left;
}