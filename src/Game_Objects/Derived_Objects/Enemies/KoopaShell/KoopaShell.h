#pragma once
#include "Game_Objects/Core_Header/Characters.h"
#include "Animations/Animation.h"
#include <vector>

class BlockGrid;

enum class KoopaShellState { Walking, Hiding, Sliding };

class KoopaShell : public Character {
private:
    const physics::PhysicsBody* playerBody_ = nullptr;
    const BlockGrid* collisionGrid_ = nullptr;
    AnimationState animState;

    KoopaShellState state_ = KoopaShellState::Walking;
    float hidingTimer_ = 0.0f;
    static constexpr float kHideTransitionDuration = 0.2f; // Time to show the 'getting in' frame
    
    // For sliding animation (spinning)
    float spinTimer_ = 0.0f;
    int spinFrame_ = 0;

public:
    KoopaShell();
    ~KoopaShell();

    void SetPlayerBody(const physics::PhysicsBody* player);
    void SetCollisionGrid(const BlockGrid* grid);

    // Called when stomped by the player.
    // If Walking -> Hiding
    // If Sliding -> Hiding
    void Stomp();

    // Called when hit/kicked from the side, or hit again while hiding.
    // dir is the direction to slide (-1 for left, 1 for right).
    void Kick(int dir);

    KoopaShellState GetState() const { return state_; }

    void InteractWith(Character& other) override;
    void AcceptInteract(CharacterVisitor& other) override;

    void Update(float dt) override;
    void Draw() override;
};
