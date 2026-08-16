#pragma once
#include "Game Objects/Core Header Files/Characters.h"
#include "Animations/Animation.h"
#include <vector>

class Mushroom : public Character {
private:
    AnimationState animState;
    const std::vector<Rectangle>* collisionBlocks_ = nullptr;
    bool active = true;

public:
    Mushroom();

    void SetCollisionBlocks(const std::vector<Rectangle>* blocks) { collisionBlocks_ = blocks; }
    void Update(float dt) override;
    void Draw() override;
    
    void InteractWith(Character& other) override;
    void AcceptInteract(CharacterVisitor& visitor) override;

    bool IsActive() const { return active; }
    void SetActive(bool a) { active = a; }
};
