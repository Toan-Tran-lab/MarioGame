#pragma once
#include "Game_Objects/Core_Header/Characters.h"
#include "Animations/Animation.h"
#include <vector>

class BlockGrid;

class Mushroom : public Character {
private:
    AnimationState animState;
    const BlockGrid* collisionGrid_ = nullptr;

    bool physicsSynced_ = false;

    bool isEmerging_ = false;
    float blockOriginY_ = 0.0f;
    float emergeTargetY_ = 0.0f;
    static constexpr float kEmergeSpeed = 45.0f;

public:
    Mushroom();

    void Spawn(Vector2 pos) { SetPosition(pos); SetActive(true); }
    void StartEmerging(float blockTopY);
    bool IsEmerging() const { return isEmerging_; }

    void SetCollisionGrid(const BlockGrid* grid) { collisionGrid_ = grid; }
    void Update(float dt) override;
    void Draw() override;
    
    void InteractWith(Character& other) override;
    void AcceptInteract(CharacterVisitor& visitor) override;
};
