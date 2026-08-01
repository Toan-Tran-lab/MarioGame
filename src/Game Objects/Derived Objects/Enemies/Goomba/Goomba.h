#pragma once
#include "Game Objects/Core Header Files/Characters.h"

class Goomba : public Character {
public:
    ~Goomba();

    void InteractWith(Character& other) override;
    void AcceptInteract(CharacterVisitor& other) override;

    void Update(float dt) override {
        // Simple AI: patrol back and forth
        ApplyMotion(dt);
    }

    void Draw() override;
};