#pragma once
#include "Game_Objects/Derived_Objects/Enemies/Ground_Enemy/GroundEnemy.h"
#include "Animations/Animation.h"

enum class BuzzyBeetleState {
    CeilingPatrol,  // Walking upside-down on the ceiling
    Dropping,       // Free-falling after detecting player below
    Hiding,         // Briefly tucked into shell after landing
    GroundPatrol    // Walking on the ground like a normal enemy
};

class BuzzyBeetle : public GroundEnemy {
private:
    BuzzyBeetleState state_ = BuzzyBeetleState::CeilingPatrol;

    // Remember where we spawned so we can patrol the ceiling at that Y level.
    Vector2 spawnPosition_ = {0.0f, 0.0f};
    bool spawnRecorded_ = false;

    // Hiding state
    float hideTimer_ = 0.0f;
    static constexpr float kHideTransitionDuration = 0.2f; // brief tuck-in visual

    // Detection: drop when player is within this many pixels on X axis
    static constexpr float kDropDetectionX = 16.0f * 3.0f; // 1 tile * GAME_SCALE

    // Ceiling patrol speed (px/sec)
    static constexpr float kCeilingSpeed = 60.0f;

    // How many ceiling tiles ahead to check before reversing
    void CeilingPatrolLogic(float dt);
    void DroppingLogic(float dt);
    void HidingLogic(float dt);

protected:
    void UpdateBehavior(float dt, physics::InputState& input) override;
    void UpdateFacingAndAnim(float dt) override;

public:
    BuzzyBeetle();
    ~BuzzyBeetle();

    void TriggerUpsideDownDeath(bool hitFromLeft) override;

    BuzzyBeetleState GetBuzzyState() const { return state_; }

    // BuzzyBeetle is invincible — these are no-ops or bounce-only.
    // Kept for interface compatibility.
    bool IsInvincible() const { return true; }

    void AcceptInteract(CharacterVisitor& other) override;
    void Update(float dt) override;
    void Draw() override;
};