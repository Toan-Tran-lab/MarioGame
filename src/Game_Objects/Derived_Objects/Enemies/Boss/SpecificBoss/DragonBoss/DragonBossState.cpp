#include "DragonBossState.h"
#include "DragonBoss.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "AudioManager/AudioManager.h"
#include "Global/Global.h"
#include <cmath>
#include <vector>

namespace {
static Animation s_bossJumpAirAnim("boss_jump", 40, 40, 1, 1, {0.2f});
}

// --- DragonIdleState ---

void DragonIdleState::Enter(Boss& boss) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    dragon.PlayIdleAnim();
    duration_ = dragon.IsEnraged() ? 0.6f : 1.2f;
    timer_ = 0.0f;
}

void DragonIdleState::OnStomped(Boss& boss) {
    if (timer_ < 0.8f) {
        timer_ = 0.8f;
    }
}

void DragonIdleState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    if (timer_ >= duration_) {
        std::vector<Player*> players;
        if (dragon.GetPlayer()) {
            players.push_back(dragon.GetPlayer());
        }

        DragonAction next = dragon.GetBrain().DecideNextAction(dragon, players);
        switch (next) {
            case DragonAction::Walk:
                boss.SetState(new DragonWalkState());
                return;
            case DragonAction::Jump:
                boss.SetState(new DragonJumpState());
                return;
            case DragonAction::Fire:
                boss.SetState(new DragonFireState());
                return;
            case DragonAction::Scream:
                boss.SetState(new DragonScreamState());
                return;
            case DragonAction::Idle:
            default:
                timer_ = 0.0f;
                break;
        }
    }
}

// --- DragonWalkState ---

void DragonWalkState::Enter(Boss& boss) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    dragon.PlayWalkAnim();
    timer_ = 0.0f;
    duration_ = dragon.IsEnraged() ? 1.08f : 1.44f;
    lastFrameOffset_ = -1;
    steppedFrame2_ = false;
    steppedFrame4_ = false;
}

void DragonWalkState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    int currentOffset = 0;
    const Animation* anim = dragon.GetAnimState().GetAnimation();
    if (anim && anim->frameCount > 0) {
        currentOffset = dragon.GetAnimState().GetCurrentFrameIndex() - anim->startFrame;
    }

    if (currentOffset != lastFrameOffset_) {
        // Frame 2 (index 1 in 0-based indexing)
        if (currentOffset == 1 && !steppedFrame2_) {
            float step = 6.0f * Global::GAME_SCALE;
            float dir = (dragon.GetFacing() == FacingDirection::Left) ? -1.0f : 1.0f;
            boss.SetPosition({ boss.GetPosition().x + dir * step, boss.GetPosition().y });
            steppedFrame2_ = true;
        }
        // Frame 4 (index 3 in 0-based indexing)
        else if (currentOffset == 3 && !steppedFrame4_) {
            float step = 6.0f * Global::GAME_SCALE;
            float dir = (dragon.GetFacing() == FacingDirection::Left) ? -1.0f : 1.0f;
            boss.SetPosition({ boss.GetPosition().x + dir * step, boss.GetPosition().y });
            steppedFrame4_ = true;
        }
        else if (currentOffset == 0) {
            steppedFrame2_ = false;
            steppedFrame4_ = false;
        }
        lastFrameOffset_ = currentOffset;
    }

    if (timer_ >= duration_) {
        boss.SetState(new DragonIdleState());
        return;
    }
}

// --- DragonJumpState ---

void DragonJumpState::Enter(Boss& boss) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    dragon.PlayJumpAnim(); // Frame 1: Crouch / Windup
    phase_ = Phase::Windup;
    windupTimer_ = 0.0f;
    groundY_ = boss.GetPosition().y;
    velocity_ = { 0.0f, 0.0f };
}

void DragonJumpState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);

    if (phase_ == Phase::Windup) {
        windupTimer_ += dt;
        if (windupTimer_ >= kWindupDuration) {
            phase_ = Phase::Airborne;
            dragon.GetAnimState().SetAnimation(&s_bossJumpAirAnim); // Frame 2: Airborne

            float dir = (dragon.GetFacing() == FacingDirection::Left) ? -1.0f : 1.0f;
            velocity_ = { dir * 180.0f, -460.0f };
        }
    } else if (phase_ == Phase::Airborne) {
        velocity_.y += 980.0f * dt;
        boss.SetPosition({ boss.GetPosition().x + velocity_.x * dt, boss.GetPosition().y + velocity_.y * dt });

        if (velocity_.y > 0.0f && boss.GetPosition().y >= groundY_) {
            boss.SetPosition({ boss.GetPosition().x, groundY_ });
            boss.SetState(new DragonIdleState());
            return;
        }
    }
}

// --- DragonFireState ---

void DragonFireState::Enter(Boss& boss) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    dragon.PlayFireAnim();
    timer_ = 0.0f;
    flameFired_ = false;
}

void DragonFireState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    if (timer_ >= 0.36f && !flameFired_) {
        flameFired_ = true;
        float dir = (dragon.GetFacing() == FacingDirection::Left) ? -1.0f : 1.0f;
        Vector2 flamePos = {
            boss.GetPosition().x + (dir < 0.0f ? -10.0f : boss.GetSize().x),
            boss.GetPosition().y + boss.GetSize().y * 0.35f
        };
        dragon.RequestFlame(flamePos, dir);
        AudioManager::PlaySFX(AudioKey::FIREBALL);
    }

    if (timer_ >= kStateDuration) {
        boss.SetState(new DragonIdleState());
        return;
    }
}

// --- DragonScreamState ---

void DragonScreamState::Enter(Boss& boss) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    dragon.PlayScreamAnim();
    timer_ = 0.0f;
    shockwaveTriggered_ = false;
    AudioManager::PlaySFX(AudioKey::DRAGON_SCREAM);
}

void DragonScreamState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    // Trigger foot stomp shockwave near frame 4
    if (timer_ >= 0.66f && !shockwaveTriggered_) {
        shockwaveTriggered_ = true;
        Vector2 footPos = {
            boss.GetPosition().x + boss.GetSize().x / 2.0f,
            boss.GetPosition().y + boss.GetSize().y
        };
        dragon.RequestShockwave(footPos, footPos.y);
    }

    if (timer_ >= kStateDuration) {
        boss.SetState(new DragonIdleState());
        return;
    }
}