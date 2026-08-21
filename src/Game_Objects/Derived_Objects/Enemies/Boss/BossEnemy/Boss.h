#pragma once
#include "Game_Objects/Core_Header/Characters.h"
#include "Animations/Animation.h"

class BossState;

class Boss : public Character {
protected:
    BossState* state_ = nullptr;
    AnimationState animState;

    int hp_;
    int maxHp_;

    float invulnTimer_ = 0.0f;
    static constexpr float kInvulnDuration = 0.5f;

    bool isDead_ = false;

    virtual void OnDamaged() {} // hook: fires once per successful hit, before death check
    virtual void DrawBoss() = 0;

    virtual BossState* CreateIdleState() = 0;

public:
    Boss(int maxHp);
    ~Boss() override;

    void SetState(BossState* next);
    BossState* GetState() const { return state_; }

    bool TakeDamage(int amount);

    bool IsInvulnerable() const { return invulnTimer_ > 0.0f; }
    bool IsDead() const { return isDead_; }
    int GetHp() const { return hp_; }
    int GetMaxHp() const { return maxHp_; }

    void AcceptInteract(CharacterVisitor& other) override;

    void Update(float dt) override;
    void Draw() override;
};