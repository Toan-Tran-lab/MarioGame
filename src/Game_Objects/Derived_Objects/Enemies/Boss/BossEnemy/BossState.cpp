#include "BossState.h"
#include "Boss.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include <algorithm>
#include <cmath>

// --- DeadState ---

void DeadState::Enter(Boss& boss) {
    timer_ = 0.0f;
}

void DeadState::UpdateState(Boss& boss, float dt) {
    timer_ += dt;
    if (timer_ >= kDeathDuration) {
        boss.SetActive(false);
    }
}

// --- SpawnState ---

void SpawnState::Enter(Boss& boss) {
    finalPos_ = boss.GetPosition();
    startPos_ = { finalPos_.x + kSlideOffsetX, finalPos_.y }; // enter from the right, same Y
    boss.SetPosition(startPos_);
    boss.SyncPhysicsBody();
    phase_ = Phase::Warning;
    timer_ = 0.0f;
}

void SpawnState::PushPlayerAway(Boss& boss, float dt) {
    Player* player = boss.GetPlayer();
    if (!player) return;

    float dx = player->GetPosition().x - boss.GetPosition().x;
    if (std::abs(dx) < kPushRange) {
        float dir = (dx >= 0.0f) ? 1.0f : -1.0f;
        Vector2 pv = player->GetVelocity();
        pv.x += dir * kPushForce * dt;
        player->SetVelocity(pv);
    }
}

void SpawnState::EnsurePlayerSafeDistance(Boss& boss) {
    Player* player = boss.GetPlayer();
    if (!player) return;

    float dx = player->GetPosition().x - boss.GetPosition().x;
    if (std::abs(dx) < kMinSafeDistance) {
        float dir = (dx >= 0.0f) ? 1.0f : -1.0f;
        Vector2 pos = player->GetPosition();
        pos.x = boss.GetPosition().x + dir * kMinSafeDistance;
        player->SetPosition(pos);
        player->SyncPhysicsBody();
    }
}

void SpawnState::UpdateState(Boss& boss, float dt) {
    PushPlayerAway(boss, dt);
    timer_ += dt;

    if (phase_ == Phase::Warning) {
        if (timer_ >= kWarningDuration) {
            phase_ = Phase::Sliding;
            timer_ = 0.0f;
        }
        return;
    }

    float t = std::min(timer_ / kSlideDuration, 1.0f);
    Vector2 newPos = {
        startPos_.x + (finalPos_.x - startPos_.x) * t,
        startPos_.y + (finalPos_.y - startPos_.y) * t
    };
    boss.SetPosition(newPos);
    boss.SyncPhysicsBody();

    if (t >= 1.0f) {
        boss.OnSpawnComplete();
        EnsurePlayerSafeDistance(boss);
        boss.EnterIdleState();
        return;
    }
}