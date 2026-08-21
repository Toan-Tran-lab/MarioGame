#pragma once
#include "Game_Objects/Derived_Objects/Enemies/Boss/BossEnemy/BossState.h"

class DragonBoss;

class IdleState : public BossState {
private:
    float attackTimer_ = 0.0f;
    float proximityTimer_ = 0.0f;
    static constexpr float kAttackInterval = 4.0f;
    static constexpr float kProximityRadius = 80.0f;
    static constexpr float kProximityTrigger = 3.0f;
public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
};

class AimingStompState : public BossState {
private:
    float timer_ = 0.0f;
    Vector2 targetPos_{};
    static constexpr float kAimDuration = 1.5f;
public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
    const Vector2& GetTargetPos() const { return targetPos_; }
};

class StompJumpState : public BossState {
private:
    enum class Phase { Delay, Jumping, Landed };
    Phase phase_ = Phase::Delay;
    float timer_ = 0.0f;
    Vector2 startPos_{};
    Vector2 targetPos_;
    static constexpr float kDelayBeforeStomp = 1.5f; // within your 1-3s spec
    static constexpr float kJumpDuration = 0.35f;
public:
    explicit StompJumpState(const Vector2& target) : targetPos_(target) {}
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
};

class BackDashState : public BossState {
private:
    float timer_ = 0.0f;
    Vector2 startPos_{};
    static constexpr float kDashDuration = 0.6f;
public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
};

class ChargingFlameState : public BossState {
private:
    float timer_ = 0.0f;
    static constexpr float kChargeDuration = 2.0f; // within your 1-3s spec
public:
    void UpdateState(Boss& boss, float dt) override;
};

class CastFlameState : public BossState {
private:
    float recoveryTimer_ = 0.0f;
    static constexpr float kRecoveryDuration = 0.4f;
public:
    void Enter(Boss& boss) override; // reserved: will spawn a Fireball once that class exists
    void UpdateState(Boss& boss, float dt) override;
};

class ProximityAOEState : public BossState {
private:
    float timer_ = 0.0f;
    bool damageApplied_ = false;
    static constexpr float kAOEDuration = 0.3f;
    static constexpr float kAOERadius = 100.0f;
public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
};