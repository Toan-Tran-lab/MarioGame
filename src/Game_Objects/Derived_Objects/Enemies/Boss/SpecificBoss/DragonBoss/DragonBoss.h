#pragma once
#include "Game_Objects/Derived_Objects/Enemies/Boss/BossEnemy/Boss.h"

class Player;

enum class DragonAttackType { Stomp, Flamethrower };

class DragonBoss : public Boss {
private:
    int attackCycleIndex_ = 0;

    int lastHpBucket_ = 4;       // 4 buckets of 25% each (4=full ... 1=1-25%)
    bool enrageTriggered_ = false;

    bool pendingEnrageSignal_ = false;
    bool pendingItemScatter_ = false;
    bool pendingFireballSpawn_ = false;
    Vector2 pendingFireballOrigin_{};

    int HpBucket() const;

    bool pendingCoinBurst_ = false;

public:
    static constexpr int kMaxHp = 20;
    static constexpr int kStompDamage = 2;
    static constexpr int kShellDamage = 5;
    static constexpr float kEnrageMultiplier = 0.65f;

    DragonBoss();
    ~DragonBoss() override;

    bool IsEnraged() const;
    DragonAttackType NextAttack();

    // Call once, after SetPosition()/SetPlayer(), to start the rise-up sequence.
    void BeginSpawn();

    int GetStompDamage() const override { return kStompDamage; }
    int GetShellDamage() const override { return kShellDamage; }

    void RequestFireball(Vector2 origin);
    bool ConsumeFireballRequest(Vector2& outOrigin);

    virtual void OnEnrageTriggered();

    static constexpr int kDeathCoinCount = 20; // tune to taste
    bool ConsumeEnrageTriggerRequest();
    bool ConsumeItemScatterRequest();
    bool ConsumeCoinBurstRequest();
    int GetDeathCoinCount() const { return kDeathCoinCount; }
protected:
    void OnDamaged() override;
    BossState* CreateIdleState() override;

    void DrawBoss() override;
};