#pragma once
#include "Game_Objects/Derived_Objects/Enemies/Boss/BossEnemy/Boss.h"
#include "DragonBossBrain.h"

class Player;

class DragonBoss : public Boss {
private:
    DragonBossBrain brain_;

    int lastHpBucket_ = 4;
    bool enrageTriggered_ = false;

    bool pendingEnrageSignal_ = false;
    bool pendingItemScatter_ = false;
    bool pendingCoinBurst_ = false;
    bool pendingFireFlowerDrop_ = false;

    bool pendingFlameSpawn_ = false;
    Vector2 pendingFlameOrigin_{ 0.0f, 0.0f };
    float pendingFlameDir_ = -1.0f;

    bool pendingShockwave_ = false;
    Vector2 pendingShockwaveOrigin_{ 0.0f, 0.0f };
    float pendingShockwaveFloorY_ = 0.0f;

    bool hasFlameStreamUpdate_ = false;
    Vector2 flameStreamMouth_{ 0.0f, 0.0f };
    float flameStreamDir_ = -1.0f;
    float flameStreamGrowth_ = 0.0f;
    bool flameStreamEnded_ = false;

    float groundY_ = 0.0f;
    class BlockGrid* collisionGrid_ = nullptr;

    int HpBucket() const;

public:
    static constexpr int kMaxHp = 60;
    static constexpr int kStompDamage = 2;
    static constexpr int kShellDamage = 5;
    static constexpr float kEnrageMultiplier = 0.65f;
    static constexpr int kDeathCoinCount = 30;

    DragonBoss();
    ~DragonBoss() override;

    bool IsEnraged() const;

    // Call once, after SetPosition()/SetPlayer(), to start the rise-up sequence.
    void BeginSpawn();

    int GetStompDamage() const override { return kStompDamage; }
    int GetShellDamage() const override { return kShellDamage; }

    void RequestFlame(Vector2 origin, float direction);
    bool ConsumeFlameRequest(Vector2& outOrigin, float& outDir);

    void UpdateFlameStream(Vector2 mouthPos, float dir, float growth);
    void EndFlameStream();
    bool GetFlameStreamUpdate(Vector2& outMouthPos, float& outDir, float& outGrowth, bool& outEnded);

    // Keep legacy name for backward compatibility
    void RequestFireball(Vector2 origin) { RequestFlame(origin, (facing_ == FacingDirection::Left) ? -1.0f : 1.0f); }
    bool ConsumeFireballRequest(Vector2& outOrigin) { float d = -1.0f; return ConsumeFlameRequest(outOrigin, d); }

    void RequestShockwave(Vector2 origin, float floorY);
    bool ConsumeShockwaveRequest(Vector2& outOrigin, float& outFloorY);

    void UpdateAI(const std::vector<Player*>& players, float dt);
    DragonBossBrain& GetBrain() { return brain_; }

    void Update(float dt) override;
    float GetFloorYUnderFeet() const;
    bool CanMoveHorizontal(float nextX) const;

    void SetCollisionGrid(class BlockGrid* grid) { collisionGrid_ = grid; }
    class BlockGrid* GetCollisionGrid() const { return collisionGrid_; }

    void SetGroundY(float y) { groundY_ = y; }
    float GetGroundY() const { return groundY_; }

    void SetFacing(FacingDirection facing) { facing_ = facing; }
    FacingDirection GetFacing() const { return facing_; }

    AnimationState& GetAnimState() { return animState; }

    Rectangle GetRect() const override;

    void PlayIdleAnim();
    void PlayWalkAnim();
    void PlayJumpAnim();
    void PlayFireAnim();
    void PlayScreamAnim();
    void PlayIntroRoarAnim();

    virtual void OnEnrageTriggered();

    bool ConsumeEnrageTriggerRequest();
    bool ConsumeItemScatterRequest();
    bool ConsumeCoinBurstRequest();
    int GetDeathCoinCount() const { return kDeathCoinCount; }

    static constexpr int kFireDropChance = 50; // 50% chance to drop a FireFlower when stomped
    void RequestFireFlowerDrop() { pendingFireFlowerDrop_ = true; }
    bool ConsumeFireFlowerDropRequest() {
        bool v = pendingFireFlowerDrop_;
        pendingFireFlowerDrop_ = false;
        return v;
    }

protected:
    void OnDamaged() override;
    BossState* CreateIdleState() override;

    void DrawBoss() override;
};