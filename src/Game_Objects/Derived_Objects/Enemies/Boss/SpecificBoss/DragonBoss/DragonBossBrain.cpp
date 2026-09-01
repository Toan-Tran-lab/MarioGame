#include "DragonBossBrain.h"
#include "DragonBoss.h"
#include "DragonBossState.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include <cmath>
#include <algorithm>

void DragonBossBrain::EvaluatePlayers(DragonBoss& boss, const std::vector<Player*>& players) {
    primaryTarget_ = nullptr;
    secondaryTarget_ = nullptr;
    isPincerSituation_ = false;

    std::vector<Player*> alivePlayers;
    for (Player* p : players) {
        if (p && !p->IsDead()) {
            alivePlayers.push_back(p);
        }
    }

    if (alivePlayers.empty()) {
        return;
    }

    primaryTarget_ = alivePlayers[0];
    if (alivePlayers.size() > 1) {
        secondaryTarget_ = alivePlayers[1];

        // Determine if primary target is the closer one
        Vector2 bossCenter = {
            boss.GetPosition().x + boss.GetSize().x / 2.0f,
            boss.GetPosition().y + boss.GetSize().y / 2.0f
        };
        float d1 = std::abs(primaryTarget_->GetPosition().x - bossCenter.x);
        float d2 = std::abs(secondaryTarget_->GetPosition().x - bossCenter.x);
        if (d2 < d1) {
            std::swap(primaryTarget_, secondaryTarget_);
        }

        float p1X = primaryTarget_->GetPosition().x + primaryTarget_->GetSize().x / 2.0f;
        float p2X = secondaryTarget_->GetPosition().x + secondaryTarget_->GetSize().x / 2.0f;

        // One player on left, one on right
        if ((p1X < bossCenter.x && p2X > bossCenter.x) || (p2X < bossCenter.x && p1X > bossCenter.x)) {
            isPincerSituation_ = true;
        }
    }
}

void DragonBossBrain::Update(DragonBoss& boss, const std::vector<Player*>& players, float dt) {
    decisionTimer_ += dt;
    EvaluatePlayers(boss, players);

    // Update boss facing direction towards primary target ONLY when in Idle or Intro state
    if (primaryTarget_) {
        if (dynamic_cast<DragonIdleState*>(boss.GetState()) || dynamic_cast<DragonIntroState*>(boss.GetState())) {
            float pX = primaryTarget_->GetPosition().x + primaryTarget_->GetSize().x / 2.0f;
            float bossCenterX = boss.GetPosition().x + boss.GetSize().x / 2.0f;
            float diffX = pX - bossCenterX;
            // 20px deadzone to eliminate rapid flipping/jittering
            if (std::abs(diffX) > 20.0f) {
                if (diffX < 0.0f) {
                    boss.SetFacing(FacingDirection::Left);
                } else {
                    boss.SetFacing(FacingDirection::Right);
                }
            }
        }
    }
}

DragonAction DragonBossBrain::DecideNextAction(DragonBoss& boss, const std::vector<Player*>& players) {
    EvaluatePlayers(boss, players);
    attackCounter_++;

    if (!primaryTarget_) {
        return DragonAction::Idle;
    }

    Vector2 bossCenter = {
        boss.GetPosition().x + boss.GetSize().x / 2.0f,
        boss.GetPosition().y + boss.GetSize().y / 2.0f
    };
    Vector2 pCenter = {
        primaryTarget_->GetPosition().x + primaryTarget_->GetSize().x / 2.0f,
        primaryTarget_->GetPosition().y + primaryTarget_->GetSize().y / 2.0f
    };

    float distX = std::abs(pCenter.x - bossCenter.x);
    float distY = pCenter.y - bossCenter.y;
    bool isEnraged = boss.IsEnraged();
    bool inFireRange = (distX <= 420.0f && std::abs(distY) <= 90.0f);

    // 1. Pincer situation with 2 players surrounding the boss
    if (isPincerSituation_) {
        if (attackCounter_ % 2 == 0) {
            return DragonAction::Scream; // Shockwave expands both directions
        } else {
            return isEnraged ? DragonAction::Walk : DragonAction::Jump;
        }
    }

    // 2. Player is jumping above boss head: Contest with jump (if normal) or shockwave roar
    if (distY < -40.0f && distX < 120.0f) {
        if (isEnraged) {
            return (attackCounter_ % 3 == 0) ? DragonAction::Jump : DragonAction::Scream;
        } else {
            return DragonAction::Jump;
        }
    }

    // 3. Enraged rotation (< 50% HP):
    // Reduced jumps (1/6), increased 2-lap walks, frequent double screams, and fire ONLY when in range!
    if (isEnraged) {
        int choice = attackCounter_ % 6;
        switch (choice) {
            case 0: return DragonAction::Walk;   // 2-lap aggressive patrol
            case 1: return DragonAction::Scream; // 2x shockwave
            case 2: return inFireRange ? DragonAction::Fire : DragonAction::Walk;
            case 3: return DragonAction::Walk;   // 2-lap aggressive patrol
            case 4: return DragonAction::Scream; // 2x shockwave
            default: return DragonAction::Jump;  // Reduced jump (only 1 out of 6)
        }
    }

    // 4. Standard rotation (>= 50% HP):
    // Balanced cycle of Walk, Fire (if in range), Scream, Jump
    int choice = attackCounter_ % 6;
    switch (choice) {
        case 0: return DragonAction::Walk;   // March forward
        case 1: return inFireRange ? DragonAction::Fire : DragonAction::Jump;
        case 2: return DragonAction::Scream; // Shockwave roar
        case 3: return DragonAction::Jump;   // Jump slam
        case 4: return DragonAction::Scream; // Shockwave roar
        default: return inFireRange ? DragonAction::Fire : DragonAction::Walk;
    }
}
