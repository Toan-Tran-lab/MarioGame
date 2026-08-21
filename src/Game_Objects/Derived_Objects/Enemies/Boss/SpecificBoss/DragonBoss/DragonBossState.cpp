#include "DragonBossState.h"
#include "DragonBoss.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include <cmath>
#include <algorithm>

namespace {
// Center-to-center distance between the boss and the player, used by both
// the proximity anti-cheese check and the AOE hit test.
float DistanceToPlayer(const Boss& boss, const Player& player) {
    Vector2 bossCenter = { boss.GetPosition().x + boss.GetSize().x / 2.0f,
                            boss.GetPosition().y + boss.GetSize().y / 2.0f };
    Vector2 playerCenter = { player.GetPosition().x + player.GetSize().x / 2.0f,
                              player.GetPosition().y + player.GetSize().y / 2.0f };
    float dx = playerCenter.x - bossCenter.x;
    float dy = playerCenter.y - bossCenter.y;
    return std::sqrt(dx * dx + dy * dy);
}
}

// --- IdleState ---

void IdleState::Enter(Boss& boss) {
    attackTimer_ = 0.0f;
    proximityTimer_ = 0.0f;
}

void IdleState::UpdateState(Boss& boss, float dt) {}

// --- AimingStompState ---

void AimingStompState::Enter(Boss& boss) {
    timer_ = 0.0f;
    auto& dragon = static_cast<DragonBoss&>(boss);
    targetPos_ = dragon.GetPlayer() ? dragon.GetPlayer()->GetPosition() : boss.GetPosition();
}

void AimingStompState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);

    // The aim marker follows the player live until the timer expires.
    if (dragon.GetPlayer()) targetPos_ = dragon.GetPlayer()->GetPosition();

    timer_ += dt;
    float duration = kAimDuration * (dragon.IsEnraged() ? DragonBoss::kEnrageMultiplier : 1.0f);
    if (timer_ >= duration) {
        boss.SetState(new StompJumpState(targetPos_));
        return; // NOTE: SetState deletes 'this' — nothing may follow
    }
}

// --- StompJumpState ---

void StompJumpState::Enter(Boss& boss) {
    startPos_ = boss.GetPosition();
    phase_ = Phase::Delay;
    timer_ = 0.0f;
}

void StompJumpState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    switch (phase_) {
        case Phase::Delay:
            if (timer_ >= kDelayBeforeStomp) {
                phase_ = Phase::Jumping;
                timer_ = 0.0f;
            }
            break;

        case Phase::Jumping: {
            float t = std::min(timer_ / kJumpDuration, 1.0f);
            Vector2 newPos = {
                startPos_.x + (targetPos_.x - startPos_.x) * t,
                startPos_.y + (targetPos_.y - startPos_.y) * t
            };
            boss.SetPosition(newPos);
            boss.SyncPhysicsBody();

            // Damage-on-path: direct overlap check, not a visitor (per design decision).
            Player* player = dragon.GetPlayer();
            if (player && boss.Overlaps(*player)) {
                player->TakeDamage();
            }

            if (t >= 1.0f) {
                phase_ = Phase::Landed;
                timer_ = 0.0f;
            }
            break;
        }

        case Phase::Landed:
            boss.SetState(new BackDashState());
            return; // NOTE: SetState deletes 'this' — nothing may follow
    }
}

// --- BackDashState ---

void BackDashState::Enter(Boss& boss) {
    startPos_ = boss.GetPosition();
    timer_ = 0.0f;
}

void BackDashState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;
    float t = std::min(timer_ / kDashDuration, 1.0f);

    Vector2 home = dragon.GetHomePosition();
    Vector2 newPos = {
        startPos_.x + (home.x - startPos_.x) * t,
        startPos_.y + (home.y - startPos_.y) * t
    };
    boss.SetPosition(newPos);
    boss.SyncPhysicsBody();

    if (t >= 1.0f) {
        boss.SetState(new IdleState());
        return; // NOTE: SetState deletes 'this' — nothing may follow
    }
}

// --- ChargingFlameState ---

void ChargingFlameState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;
    float duration = kChargeDuration * (dragon.IsEnraged() ? DragonBoss::kEnrageMultiplier : 1.0f);
    if (timer_ >= duration) {
        boss.SetState(new CastFlameState());
        return; // NOTE: SetState deletes 'this' — nothing may follow
    }
}

// --- CastFlameState ---

void CastFlameState::Enter(Boss& boss) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    Vector2 origin = { boss.GetPosition().x, boss.GetPosition().y + boss.GetSize().y / 2.0f };
    dragon.RequestFireball(origin);
    recoveryTimer_ = 0.0f;
}

void CastFlameState::UpdateState(Boss& boss, float dt) {
    recoveryTimer_ += dt;
    if (recoveryTimer_ >= kRecoveryDuration) {
        boss.SetState(new IdleState());
        return; // NOTE: SetState deletes 'this' — nothing may follow
    }
}

// --- ProximityAOEState ---

void ProximityAOEState::Enter(Boss& boss) {
    timer_ = 0.0f;
    damageApplied_ = false;
}

void ProximityAOEState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    if (!damageApplied_) {
        Player* player = dragon.GetPlayer();
        if (player && DistanceToPlayer(boss, *player) <= kAOERadius) {
            player->TakeDamage();
        }
        damageApplied_ = true; // fires once per activation, regardless of hit or miss
    }

    if (timer_ >= kAOEDuration) {
        boss.SetState(new IdleState());
        return; // NOTE: SetState deletes 'this' — nothing may follow
    }
}