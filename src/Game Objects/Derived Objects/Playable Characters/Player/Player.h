#pragma once
#include "Game Objects/Core Header Files/Characters.h"
#include "PlayerState.h"

class Player : public Character {
public:
    void Update(float dt) override {
        // game/input-specific logic here (jumping, gravity, input polling)...
        ApplyMotion(dt); // reuse shared physics logic from Character
    }

    void Draw() override {
        // render the player's current animation frame at position_,
        // flipped horizontally if facing_ == FacingDirection::Left
    }
};