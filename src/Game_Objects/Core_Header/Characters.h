#pragma once
#include "BaseGameObjects.h"
#include "physics/PhysicsBody.h"

class CharacterVisitor;

class Character : public GameObject {
public:
    ~Character() override = default;

    // Update() and Draw() are still pure virtual here, inherited unimplemented.

    // New state specific to actors (velocity/grounding live in the PhysicsBody)
    const Vector2& GetVelocity() const { return physicsBody_.velocity; }
    void SetVelocity(const Vector2& vel) { physicsBody_.velocity = vel; }

    FacingDirection GetFacing() const { return facing_; }
    void SetFacing(FacingDirection dir) { facing_ = dir; }
    void UpdateFacing() {
        if (physicsBody_.velocity.x > 0.0f) facing_ = FacingDirection::Right;
        else if (physicsBody_.velocity.x < 0.0f) facing_ = FacingDirection::Left;
    }

    bool IsGrounded() const { return physicsBody_.isGrounded; }
    void SetGrounded(bool grounded) { physicsBody_.isGrounded = grounded; }

    // Entry point, called with concrete-typed 'other'
    virtual void InteractWith(Character& other) = 0;
    virtual void AcceptInteract(CharacterVisitor& other) = 0;

    physics::PhysicsBody& GetPhysicsBody() { return physicsBody_; }
    const physics::PhysicsBody& GetPhysicsBody() const { return physicsBody_; }

    // Entity-vs-entity overlap check (axis-aligned). Used to drive interactions.
    bool Overlaps(const Character& other) const {
        return CheckCollisionRecs(GetRect(), other.GetRect());
    }
    
    // Push GameObject state into the PhysicsBody (call before running physics).
    void SyncPhysicsBody() {
        physicsBody_.position = position_;
        physicsBody_.size = size_;
    }

    // Pull PhysicsBody results back into GameObject state (call after physics).
    void SyncPhysics() {
        position_ = physicsBody_.position;
    }

protected:
    // Default movement when spawning into a stage is standing still, facing right.
    FacingDirection facing_ = FacingDirection::Right;
    physics::PhysicsBody physicsBody_;
};
