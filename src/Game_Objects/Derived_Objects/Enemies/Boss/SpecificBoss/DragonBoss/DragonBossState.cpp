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
    auto& dragon = static_cast<DragonBoss&>(boss);
    attackTimer_ = kAttackInterval * (dragon.IsEnraged() ? DragonBoss::kEnrageMultiplier : 1.0f);
    proximityTimer_ = 0.0f;
}

void IdleState::OnStomped(Boss& boss) {
    if (attackTimer_ < kFlinchCooldown) {
        attackTimer_ = kFlinchCooldown; // extend, never shorten
    }
}

void IdleState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    Player* player = dragon.GetPlayer();

    if (player) {
        if (DistanceToPlayer(boss, *player) <= kProximityRadius) {
            proximityTimer_ += dt;
            if (proximityTimer_ >= kProximityTrigger) {
                boss.SetState(new ProximityAOEState());
                return; // NOTE: SetState deletes 'this' — nothing may follow
            }
        } else {
            proximityTimer_ = 0.0f;
        }
    }

    attackTimer_ -= dt;
    if (attackTimer_ <= 0.0f) {
        DragonAttackType next = dragon.NextAttack();
        boss.SetState(next == DragonAttackType::Flamethrower
                           ? static_cast<BossState*>(new ChargingFlameState())
                           : static_cast<BossState*>(new AimingStompState()));
        return; // NOTE: SetState deletes 'this' — nothing may follow
    }
}


// --- AimingStompState ---

void AimingStompState::Enter(Boss& boss) {
    timer_ = 0.0f;
    auto& dragon = static_cast<DragonBoss&>(boss);
    targetPos_ = dragon.GetPlayer() ? dragon.GetPlayer()->GetPosition() : boss.GetPosition();
}

void AimingStompState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    if (dragon.GetPlayer()) targetPos_ = dragon.GetPlayer()->GetPosition();

    timer_ += dt;
    float duration = kAimDuration * (dragon.IsEnraged() ? DragonBoss::kEnrageMultiplier : 1.0f);
    if (timer_ >= duration) {
        boss.SetState(new ArmSlamState(targetPos_)); // was: new StompJumpState(targetPos_)
        return;
    }
}

// --- ArmSlamState ---
void ArmSlamState::Enter(Boss& boss) {
    phase_ = Phase::Delay;
    timer_ = 0.0f;
    damageApplied_ = false;
}

void ArmSlamState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    switch (phase_) {
        case Phase::Delay:
            if (timer_ >= kDelayBeforeSlam) {
                phase_ = Phase::Bursting;
                timer_ = 0.0f;
            }
            break;

        case Phase::Bursting: {
            if (!damageApplied_) {
                Player* player = dragon.GetPlayer();
                if (player) {
                    Vector2 pCenter = { player->GetPosition().x + player->GetSize().x / 2.0f,
                                         player->GetPosition().y + player->GetSize().y / 2.0f };
                    float dx = pCenter.x - targetPos_.x;
                    float dy = pCenter.y - targetPos_.y;
                    if (std::sqrt(dx * dx + dy * dy) <= kBurstRadius) {
                        player->TakeDamage();
                    }
                }
                damageApplied_ = true; // fires once, hit or miss
            }
            if (timer_ >= kBurstDuration) {
                boss.EnterIdleState(); // no back-dash — boss never left home
                return;
            }
            break;
        }
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