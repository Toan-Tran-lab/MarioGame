#include "Game Objects/Core Header Files/Characters.h"

void Character::ApplyMotion(float dt) {
    position_ = position_ + velocity_ * dt;

    // Auto-update facing direction based on horizontal movement
    if (velocity_.x > 0.0f) facing_ = FacingDirection::Right;
    else if (velocity_.x < 0.0f) facing_ = FacingDirection::Left;
}