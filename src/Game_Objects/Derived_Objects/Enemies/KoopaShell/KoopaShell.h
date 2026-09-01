#pragma once
#include "Game_Objects/Derived_Objects/Enemies/Ground_Enemy/GroundEnemy.h"
#include "Animations/Animation.h"
#include <vector>

enum class KoopaShellState { Walking, Hiding, Sliding };

class KoopaShell : public GroundEnemy {
private:
    KoopaShellState state_ = KoopaShellState::Walking;
    float hidingTimer_ = 0.0f;
    static constexpr float kHideTransitionDuration = 0.2f; // Time to show the 'getting in' frame
    
    // For sliding animation (spinning)
    float spinTimer_ = 0.0f;
    int spinFrame_ = 0;

protected:
    void UpdateBehavior(float dt, physics::InputState& input) override;
    void PostCollision(float prevVelX) override;
    void UpdateFacingAndAnim(float dt) override;

public:
    KoopaShell();
    ~KoopaShell();
    
    void TriggerUpsideDownDeath(bool hitFromLeft) override;

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
