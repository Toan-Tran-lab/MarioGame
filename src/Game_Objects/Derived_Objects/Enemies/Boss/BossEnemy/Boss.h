#pragma once
#include "Game_Objects/Core_Header/Characters.h"
#include "Animations/Animation.h"

class BossState;
class Player;

class Boss : public Character {
protected:
    BossState* state_ = nullptr;
    AnimationState animState;
    Player* playerRef_ = nullptr;

    Vector2 stompBouncePos_{};

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
    void EnterIdleState() { SetState(CreateIdleState()); }

    void SetPlayerRef(Player* player) { playerRef_ = player; }
    Player* GetPlayer() const { return playerRef_; }

    void SetStompBouncePosition(const Vector2& pos) { stompBouncePos_ = pos; }
    const Vector2& GetStompBouncePosition() const { return stompBouncePos_; }

    virtual void OnSpawnComplete() {}

    bool TakeDamage(int amount);
    virtual int GetStompDamage() const { return 1; }
    virtual int GetShellDamage() const { return 3; }

    bool IsInvulnerable() const { return invulnTimer_ > 0.0f; }
    bool IsDead() const { return isDead_; }
    int GetHp() const { return hp_; }
    int GetMaxHp() const { return maxHp_; }

    void InteractWith(Character& other) override { (void)other; }
    void AcceptInteract(CharacterVisitor& other) override;

    void Update(float dt) override;
    void Draw() override;
};