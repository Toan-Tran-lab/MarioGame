#include "Boss.h"
#include "BossState.h"
#include "Game_Objects/Interaction_Resolve/Visitor.h"

Boss::Boss(int maxHp) : hp_(maxHp), maxHp_(maxHp) {}

Boss::~Boss() {
    delete state_;
}

void Boss::SetState(BossState* next) {
    if (state_) state_->Exit(*this);
    delete state_;
    state_ = next;
    if (state_) state_->Enter(*this);
}

bool Boss::TakeDamage(int amount) {
    if (isDead_ || IsInvulnerable()) return false;
    if (state_ && !state_->IsAttackable()) return false;

    hp_ -= amount;
    invulnTimer_ = kInvulnDuration;
    OnDamaged();

    if (hp_ <= 0) {
        hp_ = 0;
        isDead_ = true;
        physicsBody_.velocity = { 0.0f, 0.0f };
        SetState(new DeadState());
    }
    return true;
}

void Boss::AcceptInteract(CharacterVisitor& other) {
    other.Visit(*this);
}

void Boss::Update(float dt) {
    if (invulnTimer_ > 0.0f) {
        invulnTimer_ -= dt;
        if (invulnTimer_ < 0.0f) invulnTimer_ = 0.0f;
    }
    if (state_) state_->UpdateState(*this, dt);
    animState.Update(dt);
}

void Boss::Draw() {
    DrawBoss();
}