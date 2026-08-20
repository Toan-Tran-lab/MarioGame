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

public:
    Boss(int maxHp);
    ~Boss() override;

    void SetState(BossState* next);

    bool TakeDamage(int amount);

    bool IsInvulnerable() const { return invulnTimer_ > 0.0f; }
    bool IsDead() const { return isDead_; }
    int GetHp() const { return hp_; }
    int GetMaxHp() const { return maxHp_; }

    void AcceptInteract(CharacterVisitor& other) override;

    void Update(float dt) override;
    void Draw() override;

protected:
    virtual void DrawBoss() = 0;
};