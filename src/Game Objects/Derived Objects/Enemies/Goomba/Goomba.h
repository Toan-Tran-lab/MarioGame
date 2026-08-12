#pragma once
#include "Game Objects/Core Header Files/Characters.h"
#include <vector>

class Goomba : public Character {
private:
    const physics::PhysicsBody* playerBody_ = nullptr;
    const std::vector<Rectangle>* collisionBlocks_ = nullptr;
    int goombaFrame_ = 0;
    float goombaAnimTimer_ = 0.0f;
public:
    ~Goomba();

    void SetPlayerBody(const physics::PhysicsBody* player);
    void SetCollisionBlocks(const std::vector<Rectangle>* blocks);

    void InteractWith(Character& other) override;
    void AcceptInteract(CharacterVisitor& other) override;

    void Update(float dt) override;
    void Draw() override;
};
