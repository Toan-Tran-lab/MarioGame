#pragma once
#include "Game_Objects/Core_Header/Characters.h"
#include "Animations/Animation.h"
#include <vector>

class BlockGrid;

enum class GoombaState { Alive, Dying };

class Goomba : public Character {
private:
    const physics::PhysicsBody* playerBody_ = nullptr;
    const BlockGrid* collisionGrid_ = nullptr;
    AnimationState animState;

    GoombaState state_ = GoombaState::Alive;
    float dyingTimer_ = 0.0f;
    static constexpr float kDyingDuration = 0.5f; // seconds before disappearing

public:
    Goomba();
    ~Goomba();

    void SetPlayerBody(const physics::PhysicsBody* player);
    void SetCollisionGrid(const BlockGrid* grid);

    // Called when stomped by the player — switches to Dying state
    void Stomp();
    bool IsDying() const { return state_ == GoombaState::Dying; }

    void InteractWith(Character& other) override;
    void AcceptInteract(CharacterVisitor& other) override;

    void Update(float dt) override;
    void Draw() override;
};
