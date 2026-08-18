#pragma once
#include "Game_Objects/Core_Header/Characters.h"
#include "Animations/Animation.h"
#include "physics/InputManager.h"

class BlockGrid;

// Shared skeleton for AI-driven ground enemies (chase player, apply physics,
// resolve map collisions). Subclasses override hooks for anything state-specific.
class GroundEnemy : public Character {
protected:
    const physics::PhysicsBody* playerBody_ = nullptr;
    const BlockGrid* collisionGrid_ = nullptr;
    AnimationState animState;

    // How close the player must get before this enemy starts chasing.
    virtual float GetDetectionRadius() const { return 300.0f; }

    // Populate input / adjust velocity before ApplyPhysics runs this frame.
    // Default: chase the player via ProximityAI if both refs are set.
    virtual void UpdateBehavior(float dt, physics::InputState& input);

    // Called after ResolveMapCollisions, before SyncPhysics.
    // prevVelX is velocity.x captured right after ApplyPhysics (pre-collision) —
    // useful for wall-bounce detection. Default: no-op.
    virtual void PostCollision(float prevVelX) {}

    // Called after SyncPhysics. Default: face movement direction, advance animState.
    virtual void UpdateFacingAndAnim(float dt);

public:
    void SetPlayerBody(const physics::PhysicsBody* player) { playerBody_ = player; }
    void SetCollisionGrid(const BlockGrid* grid) { collisionGrid_ = grid; }

    // Enemies don't initiate interaction; the player drives resolution via visitor.
    void InteractWith(Character& other) override { (void)other; }

    // Shared frame: sync -> behavior -> physics -> collision -> post-collision -> sync -> anim.
    void Update(float dt) override;
};