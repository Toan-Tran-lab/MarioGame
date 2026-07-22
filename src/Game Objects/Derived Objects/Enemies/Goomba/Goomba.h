#pragma once
#include "Game Objects/Core Header Files/Characters.h"

class Goomba : public Character {
public:
    void Update(float dt) override {
        // Simple AI: patrol back and forth
        ApplyMotion(dt);
    }

    void Draw() override;
};