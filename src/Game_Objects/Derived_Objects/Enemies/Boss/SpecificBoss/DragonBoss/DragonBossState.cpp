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
    turnsCount_ = 0;
    currentFacing_ = dragon.GetFacing();

    if (dragon.IsEnraged()) {
        maxTurns_ = 4; // 2 full round trips (4 wall rebounds)
        baseSpeed_ = 160.0f;
        maxSpeed_ = 420.0f;
        accelRate_ = 100.0f; // Rapid acceleration during rage
    } else {
        maxTurns_ = 1; // Walks until hitting an obstacle/wall
        baseSpeed_ = 120.0f;
        maxSpeed_ = 280.0f;
        accelRate_ = 70.0f;  // Smoothly increases speed over time
    }
    currentSpeed_ = baseSpeed_; // Reset to base speed at start of walk
}

void DragonWalkState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    // Gradually accelerate walking speed over time
    currentSpeed_ += accelRate_ * dt;
    if (currentSpeed_ > maxSpeed_) {
        currentSpeed_ = maxSpeed_;
    }

    float dir = (currentFacing_ == FacingDirection::Left) ? -1.0f : 1.0f;
    float nextX = boss.GetPosition().x + dir * currentSpeed_ * dt;

    // Check if the boss can move to nextX or if it hits an obstacle/wall
    if (dragon.CanMoveHorizontal(nextX)) {
        boss.SetPosition({ nextX, boss.GetPosition().y });
    } else {
        // Hit obstacle/wall!
        turnsCount_++;
        if (turnsCount_ >= maxTurns_) {
            boss.SetState(new DragonIdleState());
            return;
        }

        // Rebound off obstacle: turn around and walk in opposite direction
        currentFacing_ = (currentFacing_ == FacingDirection::Left) ? FacingDirection::Right : FacingDirection::Left;
        dragon.SetFacing(currentFacing_);
    }

    // Safety timeout in case no wall is reached
    float maxDuration = dragon.IsEnraged() ? 7.0f : 3.5f;
    if (timer_ >= maxDuration) {
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
    timer_ = 0.0f;
    flameFired_ = false;
}

void DragonFireState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    // 1. Frames 1 to 6 (Windup/Charge, t = 0 to 1.35s):
    if (timer_ < kWindupDuration) {
        dragon.EndFlameStream();
    }
    // 2. Frame 7 (Fire stream active, t = 1.35s to 2.15s):
    else if (timer_ < kStateDuration) {
        if (!flameFired_) {
            flameFired_ = true;
            AudioManager::PlaySFX(AudioKey::FIREBALL);
        }
        float growth = (timer_ - kWindupDuration) / kFireDuration;
        growth = (growth < 0.0f) ? 0.0f : (growth > 1.0f ? 1.0f : growth);
        float dir = (dragon.GetFacing() == FacingDirection::Left) ? -1.0f : 1.0f;
        Vector2 mouthPos = {
            boss.GetPosition().x + (dir < 0.0f ? 2.0f : (boss.GetSize().x - 2.0f)),
            boss.GetPosition().y + boss.GetSize().y * 0.65f
        };
        dragon.UpdateFlameStream(mouthPos, dir, growth);
    }

    if (timer_ >= kStateDuration) {
        dragon.EndFlameStream();
        boss.SetState(new DragonIdleState());
        return;
    }
}

// --- DragonScreamState ---

void DragonScreamState::Enter(Boss& boss) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    dragon.PlayScreamAnim();
    timer_ = 0.0f;
    shockwavesTriggered_ = 0;
    isEnraged_ = dragon.IsEnraged();
    AudioManager::PlaySFX(AudioKey::DRAGON_SCREAM);
}

void DragonScreamState::UpdateState(Boss& boss, float dt) {
    auto& dragon = static_cast<DragonBoss&>(boss);
    timer_ += dt;

    if (!isEnraged_) {
        // Normal Mode:
        // Animation frames 1, 2, 3, 4 with durations {0.35s, 0.35s, 0.40s, 0.50s}
        // Shockwave triggers only at animation 4 (frame offset 3) starting at t = 1.10s
        if (timer_ >= 1.10f && shockwavesTriggered_ == 0) {
            shockwavesTriggered_ = 1;
            Vector2 footPos = {
                boss.GetPosition().x + boss.GetSize().x / 2.0f,
                boss.GetPosition().y + boss.GetSize().y
            };
            dragon.RequestShockwave(footPos, footPos.y);
        }

        if (timer_ >= 1.60f) {
            boss.SetState(new DragonIdleState());
            return;
        }
    } else {
        // Enraged Mode (< 50% HP):
        // Sequence: Frame 1 (0.25s) -> Frame 2 (0.25s) -> Frame 3 (0.30s) -> Frame 4 (0.40s, Shockwave #1)
        // -> Frame 3 (0.30s) -> Frame 4 (0.45s, Shockwave #2)
        if (timer_ < 0.25f) {
            dragon.GetAnimState().SetCurrentFrameOffset(0);
        } else if (timer_ < 0.50f) {
            dragon.GetAnimState().SetCurrentFrameOffset(1);
        } else if (timer_ < 0.80f) {
            dragon.GetAnimState().SetCurrentFrameOffset(2);
        } else if (timer_ < 1.20f) {
            dragon.GetAnimState().SetCurrentFrameOffset(3); // Animation 4
            if (shockwavesTriggered_ == 0) {
                shockwavesTriggered_ = 1;
                Vector2 footPos = {
                    boss.GetPosition().x + boss.GetSize().x / 2.0f,
                    boss.GetPosition().y + boss.GetSize().y
                };
                dragon.RequestShockwave(footPos, footPos.y);
            }
        } else if (timer_ < 1.55f) {
            dragon.GetAnimState().SetCurrentFrameOffset(2); // Repeat Animation 3
        } else if (timer_ < 2.05f) {
            dragon.GetAnimState().SetCurrentFrameOffset(3); // Repeat Animation 4
            if (shockwavesTriggered_ == 1) {
                shockwavesTriggered_ = 2;
                Vector2 footPos = {
                    boss.GetPosition().x + boss.GetSize().x / 2.0f,
                    boss.GetPosition().y + boss.GetSize().y
                };
                dragon.RequestShockwave(footPos, footPos.y);
                AudioManager::PlaySFX(AudioKey::DRAGON_SCREAM);
            }
        } else {
            boss.SetState(new DragonIdleState());
            return;
        }
    }
}