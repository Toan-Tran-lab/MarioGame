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

    if (!isDead_) {
        float barWidth = size_.x * 1.5f;
        float barHeight = 8.0f;
        float barX = position_.x + size_.x / 2.0f - barWidth / 2.0f;
        float barY = position_.y - 20.0f;
        float hpRatio = (maxHp_ > 0) ? (float)hp_ / (float)maxHp_ : 0.0f;

        DrawRectangle((int)barX, (int)barY, (int)barWidth, (int)barHeight, Color{60, 20, 20, 220});
        DrawRectangle((int)barX, (int)barY, (int)(barWidth * hpRatio), (int)barHeight, Color{200, 40, 40, 255});
        DrawRectangleLines((int)barX, (int)barY, (int)barWidth, (int)barHeight, BLACK);

        if (IsInvulnerable()) {
            DrawRectangle((int)position_.x, (int)position_.y, (int)size_.x, (int)size_.y, Fade(WHITE, 0.3f));
        }
    }
}