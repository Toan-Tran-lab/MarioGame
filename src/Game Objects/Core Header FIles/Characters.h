#pragma once
#include "BaseGameObjects.h"

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

    // Entry point, called with concrete-typed 'other'
    virtual void InteractWith(Character& other) = 0;
    virtual void AcceptInteract(CharacterVisitor& other) = 0;

protected:
    // Default movement when spawning into a stage is standing still, facing right.
    Vector2 velocity_{ 0.0f, 0.0f };
    FacingDirection facing_ = FacingDirection::Right;

    // A helper ALL characters can reuse: standard physics integration.
    // Shouldn't need to override. Subclasses call this from their own Update().
    void ApplyMotion(float dt);
};