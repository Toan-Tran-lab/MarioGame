#pragma once
#include "../../../Core Header Files/Characters.h"

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