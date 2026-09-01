#pragma once
#include "Game_Objects/Derived_Objects/Enemies/Boss/BossEnemy/BossState.h"
#include "DragonBossBrain.h"

class DragonBoss;

class DragonIdleState : public BossState {
private:
    float timer_ = 0.0f;
    float duration_ = 1.0f;

public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
    void OnStomped(Boss& boss) override;
};

class DragonWalkState : public BossState {
private:
    float duration_ = 1.44f;
    float timer_ = 0.0f;
    int lastFrameOffset_ = -1;
    bool steppedFrame2_ = false;
    bool steppedFrame4_ = false;

public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
};

class DragonJumpState : public BossState {
private:
    enum class Phase { Windup, Airborne };
    Phase phase_ = Phase::Windup;
    float windupTimer_ = 0.0f;
    static constexpr float kWindupDuration = 0.35f;

    Vector2 velocity_{ 0.0f, 0.0f };
    float groundY_ = 0.0f;

public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
};

class DragonFireState : public BossState {
private:
    float timer_ = 0.0f;
    bool flameFired_ = false;
    static constexpr float kStateDuration = 0.96f;

public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
};

class DragonScreamState : public BossState {
private:
    float timer_ = 0.0f;
    bool shockwaveTriggered_ = false;
    static constexpr float kStateDuration = 0.88f;

public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
};

// Aliases for legacy compatibility
using IdleState = DragonIdleState;
using ChargingFlameState = DragonFireState;
using CastFlameState = DragonFireState;