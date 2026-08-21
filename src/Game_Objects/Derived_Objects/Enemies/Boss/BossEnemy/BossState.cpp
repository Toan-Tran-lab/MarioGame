#include "BossState.h"
#include "Boss.h"
#include "DragonBoss.h" // needed for the static_cast<DragonBoss&> calls below
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
    finalPos_ = boss.GetPosition(); // caller must SetPosition to the resting spot BEFORE calling BeginSpawn()
    startPos_ = { finalPos_.x, finalPos_.y + kRiseOffsetY };
    boss.SetPosition(startPos_);
    boss.SyncPhysicsBody();
    phase_ = Phase::Warning;
    timer_ = 0.0f;
}

void SpawnState::PushPlayerAway(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    Player* player = dragon.GetPlayer();
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
    auto& dragon = static_cast<DragonBoss&>(boss);
    Player* player = dragon.GetPlayer();
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
    auto& dragon = static_cast<DragonBoss&>(boss);
    PushPlayerAway(boss, dt);
    timer_ += dt;

    if (phase_ == Phase::Warning) {
        if (timer_ >= kWarningDuration) {
            phase_ = Phase::Rising;
            timer_ = 0.0f;
        }
        return;
    }

    float t = std::min(timer_ / kRiseDuration, 1.0f);
    Vector2 newPos = {
        startPos_.x + (finalPos_.x - startPos_.x) * t,
        startPos_.y + (finalPos_.y - startPos_.y) * t
    };
    boss.SetPosition(newPos);
    boss.SyncPhysicsBody();

    if (t >= 1.0f) {
        dragon.SetHomePosition(finalPos_);
        EnsurePlayerSafeDistance(boss);
        boss.CreateIdleState();
        return; // NOTE: SetState deletes 'this' — nothing may follow
    }
}