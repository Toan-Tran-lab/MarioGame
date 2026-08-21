#pragma once
#include "raylib.h"

class Boss;

class BossState {
public:
    virtual ~BossState() = default;
    virtual void Enter(Boss& boss) {}
    virtual void Exit(Boss& boss) {}
    virtual void UpdateState(Boss& boss, float dt) = 0;
    virtual bool IsAttackable() const { return true; } // false during e.g. spawn
};

class DeadState : public BossState {
private:
    float timer_ = 0.0f;
    static constexpr float kDeathDuration = 1.5f;
public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
    bool IsAttackable() const override { return false; }
};

class SpawnState : public BossState {
private:
    enum class Phase { Warning, Rising };
    Phase phase_ = Phase::Warning;
    float timer_ = 0.0f;
    Vector2 startPos_{};
    Vector2 finalPos_{};

    static constexpr float kWarningDuration = 3.0f;
    static constexpr float kRiseDuration = 1.0f;
    static constexpr float kRiseOffsetY = 300.0f;
    static constexpr float kPushForce = 60.0f;
    static constexpr float kPushRange = 150.0f;
    static constexpr float kMinSafeDistance = 100.0f;

    void PushPlayerAway(Boss& boss, float dt);
    void EnsurePlayerSafeDistance(Boss& boss);

public:
    void Enter(Boss& boss) override;
    void UpdateState(Boss& boss, float dt) override;
    bool IsAttackable() const override { return false; }
    bool ShowWarningLine() const { return phase_ == Phase::Warning; }
};