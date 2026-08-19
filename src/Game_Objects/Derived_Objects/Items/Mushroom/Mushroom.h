#pragma once
#include "Game_Objects/Core_Header/Characters.h"
#include "Animations/Animation.h"
#include <vector>

class BlockGrid;

class Mushroom : public Character {
private:
    AnimationState animState;
    const BlockGrid* collisionGrid_ = nullptr;
    bool active = true;

public:
    Mushroom();

    void Spawn(Vector2 pos);

    void SetCollisionGrid(const BlockGrid* grid) { collisionGrid_ = grid; }
    void Update(float dt) override;
    void Draw() override;
    
    void InteractWith(Character& other) override;
    void AcceptInteract(CharacterVisitor& visitor) override;

    bool IsActive() const { return active; }
    void SetActive(bool a) { active = a; }
};
