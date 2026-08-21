#pragma once
#include <algorithm>
#include "Game_Objects/Derived_Objects/Enemies/Boss/BossEnemy/BossState.h"

class DragonBoss;

class IdleState : public BossState {
private:
    float attackTimer_ = 0.0f;
    float proximityTimer_ = 0.0f;
    static constexpr float kAttackInterval = 4.0f;
    static constexpr float kFlinchCooldown = 1.0f;
    static constexpr float kProximityRadius = 80.0f;
    static constexpr float kProximityTrigger = 3.0f;
public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
    void OnStomped(Boss& boss) override;
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

class ChargingFlameState : public BossState {
private:
    float timer_ = 0.0f;
    static constexpr float kChargeDuration = 2.0f; // within your 1-3s spec
public:
    void UpdateState(Boss& boss, float dt) override;
};

class ArmSlamState : public BossState { // replaces StompJumpState + BackDashState
private:
    enum class Phase { Delay, Bursting };
    Phase phase_ = Phase::Delay;
    float timer_ = 0.0f;
    Vector2 targetPos_;
    bool damageApplied_ = false;
    static constexpr float kDelayBeforeSlam = 1.0f;  // anticipation before the arm hits
    static constexpr float kBurstDuration = 0.5f;    // how long the fire/lava lingers
    static constexpr float kBurstRadius = 40.0f;     // damage radius at the locked-on point
public:
    explicit ArmSlamState(const Vector2& target) : targetPos_(target) {}
    bool IsBursting() const { return phase_ == Phase::Bursting; }
    float GetBurstProgress() const { return std::min(timer_ / kBurstDuration, 1.0f); }
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
    const Vector2& GetTargetPos() const { return targetPos_; } // for drawing the burst VFX
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