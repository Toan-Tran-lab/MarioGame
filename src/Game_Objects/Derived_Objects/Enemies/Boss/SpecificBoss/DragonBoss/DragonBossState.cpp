#include "DragonBossState.h"
#include "DragonBoss.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "AudioManager/AudioManager.h"
#include "Global/Global.h"
#include "physics/CollisionSystem.h"
#include "World/BlockGrid.h"
#include <cmath>
#include <vector>

namespace {
static Animation s_bossJumpAirAnim("boss_jump", 40, 40, 1, 1, {0.2f});
}

// --- DragonIntroState ---

void DragonIntroState::Enter(Boss& boss) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    dragon.PlayIntroRoarAnim();
    timer_ = 0.0f;
    roared_ = false;
}

void DragonIntroState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    if (timer_ < 1.8f) {
        if (!roared_) {
            roared_ = true;
            AudioManager::PlaySFX(AudioKey::DRAGON_SCREAM);
        }
    } else if (timer_ < 3.6f) {
        dragon.PlayIdleAnim();
    } else {
        boss.SetState(new DragonIdleState());
        return;
    }
}

// --- DragonIdleState ---

void DragonIdleState::Enter(Boss& boss) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    dragon.PlayIdleAnim();
    // Rest interval between actions: 1.35s in normal mode, 0.75s when enraged
    duration_ = dragon.IsEnraged() ? 0.75f : 1.35f;
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
    duration_ = dragon.IsEnraged() ? 1.8f : 2.4f;
    startX_ = boss.GetPosition().x;
    targetDistance_ = 15.0f * 16.0f * Global::GAME_SCALE; // 15 tiles
}

void DragonWalkState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    float progress = timer_ / duration_;
    if (progress > 1.0f) progress = 1.0f;

    float dir = (dragon.GetFacing() == FacingDirection::Left) ? -1.0f : 1.0f;
    boss.SetPosition({ startX_ + dir * targetDistance_ * progress, boss.GetPosition().y });

    if (timer_ >= duration_) {
        boss.SetPosition({ startX_ + dir * targetDistance_, boss.GetPosition().y });
        boss.SetState(new DragonIdleState());
        return;
    }
}

// --- DragonJumpState ---

void DragonJumpState::Enter(Boss& boss) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    dragon.PlayJumpAnim(); // Animation 1: Crouch / Windup
    phase_ = Phase::Windup;
    timer_ = 0.0f;
    groundY_ = (dragon.GetGroundY() > 0.0f) ? dragon.GetGroundY() : boss.GetPosition().y;
    targetX_ = boss.GetPosition().x;
    velocity_ = { 0.0f, 0.0f };
}

void DragonJumpState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);

    // 1. Ở 1 tí ở animation 1 (Crouch / Nạp đà)
    if (phase_ == Phase::Windup) {
        timer_ += dt;
        if (timer_ >= kWindupDuration) {
            phase_ = Phase::Ascending;
            dragon.GetAnimState().SetAnimation(&s_bossJumpAirAnim); // Animation 2: Airborne
            velocity_ = { 0.0f, kAscendSpeed };
        }
    }
    // 2. Nhảy vút lên cao ra khỏi màn hình
    else if (phase_ == Phase::Ascending) {
        boss.SetPosition({ boss.GetPosition().x, boss.GetPosition().y + velocity_.y * dt });

        if (boss.GetPosition().y <= kOffScreenY) {
            phase_ = Phase::OffScreenWait;
            timer_ = 0.0f;
        }
    }
    // 3. Định vị phía trên đầu người chơi khi ở ngoài màn hình
    else if (phase_ == Phase::OffScreenWait) {
        timer_ += dt;
        if (dragon.GetPlayer()) {
            targetX_ = dragon.GetPlayer()->GetPosition().x + dragon.GetPlayer()->GetSize().x * 0.5f - boss.GetSize().x * 0.5f;
            boss.SetPosition({ targetX_, kOffScreenY });

            if (dragon.GetPlayer()->GetPosition().x < boss.GetPosition().x + boss.GetSize().x * 0.5f) {
                dragon.SetFacing(FacingDirection::Left);
            } else {
                dragon.SetFacing(FacingDirection::Right);
            }
        }

        if (timer_ >= kOffScreenDuration) {
            phase_ = Phase::Falling;
            velocity_ = { 0.0f, kFallSpeed };
        }
    }
    // 4. Rơi thẳng xuống đè bẹp vị trí người chơi cho đến khi chạm đất
    else if (phase_ == Phase::Falling) {
        float nextY = boss.GetPosition().y + velocity_.y * dt;
        boss.SetPosition({ targetX_, nextY });

        float floor = dragon.GetFloorYUnderFeet();
        float targetGroundY = (floor > 0.0f) ? (floor - boss.GetSize().y) : groundY_;

        if (boss.GetPosition().y >= targetGroundY) {
            boss.SetPosition({ targetX_, targetGroundY });
            phase_ = Phase::Landing;
            timer_ = 0.0f;
            dragon.PlayJumpAnim(); // Animation 1: Landing impact / crouch on ground
        }
    }
    // 5. Chạm đất nén người lại 0.25s tạo độ nặng rồi mới đứng dậy
    else if (phase_ == Phase::Landing) {
        timer_ += dt;
        if (timer_ >= kLandingDuration) {
            boss.SetState(new DragonIdleState());
            return;
        }
    }
}

// --- DragonFireState ---

void DragonFireState::Enter(Boss& boss) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    dragon.PlayFireAnim();
    baseSize_ = boss.GetSize();
    groundY_ = (dragon.GetGroundY() > 0.0f) ? dragon.GetGroundY() : boss.GetPosition().y;
    centerX_ = boss.GetPosition().x + baseSize_.x * 0.5f; // Anchor scaling to Center X
    timer_ = 0.0f;
    flameFired_ = false;
}

void DragonFireState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    float scale = 1.0f;

    // 1. Frames 1 to 6 (Windup/Charge, t = 0 to 1.35s):
    // Smoothly grow from 1.0x to 2.0x symmetrically from Center X
    if (timer_ < kWindupDuration) {
        float progress = timer_ / kWindupDuration;
        scale = 1.0f + progress * 1.0f; // 1.0x -> 2.0x
        dragon.EndFlameStream();
    }
    // 2. Frame 7 (Fire stream active, t = 1.35s to 2.15s):
    // Stay at 2.0x size throughout the fire stream
    else if (timer_ < (kWindupDuration + kFireDuration)) {
        scale = 2.0f;

        if (!flameFired_) {
            flameFired_ = true;
            AudioManager::PlaySFX(AudioKey::FIREBALL);
        }
        float growth = (timer_ - kWindupDuration) / kFireDuration;
        growth = (growth < 0.0f) ? 0.0f : (growth > 1.0f ? 1.0f : growth);
        float dir = (dragon.GetFacing() == FacingDirection::Left) ? -1.0f : 1.0f;
        Vector2 mouthPos = {
            (centerX_ - baseSize_.x * scale * 0.5f) + (dir < 0.0f ? 4.0f : (baseSize_.x * scale - 4.0f)),
            (groundY_ - (baseSize_.y * scale - baseSize_.y)) + (baseSize_.y * scale) * 0.65f
        };
        dragon.UpdateFlameStream(mouthPos, dir, growth);
    }
    // 3. Smooth Shrink Transition (t = 2.15s to 2.55s):
    // Smoothly contract from 2.0x back down to 1.0x symmetrically to Center X
    else if (timer_ < kStateDuration) {
        dragon.EndFlameStream();
        float progress = (timer_ - (kWindupDuration + kFireDuration)) / kShrinkDuration;
        progress = (progress < 0.0f) ? 0.0f : (progress > 1.0f ? 1.0f : progress);
        scale = 2.0f - progress * 1.0f; // 2.0x -> 1.0x
    }
    else {
        scale = 1.0f;
        dragon.EndFlameStream();
    }

    // Apply scaling centered at centerX_ and anchored to groundY_
    Vector2 curSize = { baseSize_.x * scale, baseSize_.y * scale };
    boss.SetSize(curSize);
    boss.SetPosition({ centerX_ - curSize.x * 0.5f, groundY_ - (curSize.y - baseSize_.y) });

    if (timer_ >= kStateDuration) {
        dragon.EndFlameStream();
        boss.SetSize(baseSize_);
        boss.SetPosition({ centerX_ - baseSize_.x * 0.5f, groundY_ });
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

    // Trigger foot stomp shockwave at frame 3/4
    if (timer_ >= 0.76f && !shockwaveTriggered_) {
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