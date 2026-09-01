#pragma once
#include "Game_Objects/Derived_Objects/Enemies/Boss/BossEnemy/BossState.h"
#include "DragonBossBrain.h"

class DragonBoss;

class DragonIntroState : public BossState {
private:
    float timer_ = 0.0f;
    bool roared_ = false;

public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
    bool IsAttackable() const override { return false; }
};

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
    float duration_ = 2.24f;
    float timer_ = 0.0f;
    float startX_ = 0.0f;
    float targetDistance_ = 0.0f;

public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
};

class DragonJumpState : public BossState {
private:
    enum class Phase { Windup, Ascending, OffScreenWait, Falling, Landing };
    Phase phase_ = Phase::Windup;
    float timer_ = 0.0f;
    static constexpr float kWindupDuration = 0.85f;
    static constexpr float kOffScreenDuration = 0.35f;
    static constexpr float kLandingDuration = 0.25f;

    Vector2 velocity_{ 0.0f, 0.0f };
    float groundY_ = 0.0f;
    float targetX_ = 0.0f;
    static constexpr float kAscendSpeed = -900.0f;
    static constexpr float kFallSpeed = 950.0f;
    static constexpr float kOffScreenY = -150.0f;

public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
};

class DragonFireState : public BossState {
private:
    float timer_ = 0.0f;
    bool flameFired_ = false;
    static constexpr float kWindupDuration = 1.35f;
    static constexpr float kFireDuration = 0.80f;
    static constexpr float kShrinkDuration = 0.90f;
    static constexpr float kStateDuration = kWindupDuration + kFireDuration + kShrinkDuration;
    Vector2 baseSize_{ 0.0f, 0.0f };
    float groundY_ = 0.0f;
    float centerX_ = 0.0f;

public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
};

class DragonScreamState : public BossState {
private:
    float timer_ = 0.0f;
    bool shockwaveTriggered_ = false;
    static constexpr float kStateDuration = 1.60f;

public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
};

// Aliases for legacy compatibility
using IdleState = DragonIdleState;
using ChargingFlameState = DragonFireState;
using CastFlameState = DragonFireState;