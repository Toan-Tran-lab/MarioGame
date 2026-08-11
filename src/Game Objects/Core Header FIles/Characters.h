#pragma once
#include "BaseGameObjects.h"
#include "physics/PhysicsBody.h"

class CharacterVisitor;

class Character : public GameObject {
public:
    ~Character() override = default;

    // Update() and Draw() are still pure virtual here, inherited unimplemented.

    // New state specific to actors
    const Vector2& GetVelocity() const { return velocity_; }
    void SetVelocity(const Vector2& vel) { velocity_ = vel; }

    FacingDirection GetFacing() const { return facing_; }
    void SetFacing(FacingDirection dir) { facing_ = dir; }

    bool IsGrounded() const { return grounded_; }
    void SetGrounded(bool grounded) { grounded_ = grounded; }

    // Entry point, called with concrete-typed 'other'
    virtual void InteractWith(Character& other) = 0;
    virtual void AcceptInteract(CharacterVisitor& other) = 0;

    physics::PhysicsBody& GetPhysicsBody() { return physicsBody_; }
    const physics::PhysicsBody& GetPhysicsBody() const { return physicsBody_; }
    Rectangle GetRect() const { return { position_.x, position_.y, size_.x, size_.y }; }
    
    // Push Character state into the PhysicsBody (call before running physics).
    void SyncPhysicsBody() {
        physicsBody_.position = position_;
        physicsBody_.velocity = velocity_;
        physicsBody_.isGrounded = grounded_;
        physicsBody_.size = size_;
    }

    // Pull PhysicsBody results back into Character state (call after physics).
    void SyncPhysics() {
        position_ = physicsBody_.position;
        velocity_ = physicsBody_.velocity;
        grounded_ = physicsBody_.isGrounded;
    }

protected:
    // Default movement when spawning into a stage is standing still, facing right.
    Vector2 velocity_{ 0.0f, 0.0f };
    FacingDirection facing_ = FacingDirection::Right;
    physics::PhysicsBody physicsBody_;

    bool grounded_ = true;

    // A helper ALL characters can reuse: standard physics integration.
    // Shouldn't need to override. Subclasses call this from their own Update().
    void ApplyMotion(float dt);
};
