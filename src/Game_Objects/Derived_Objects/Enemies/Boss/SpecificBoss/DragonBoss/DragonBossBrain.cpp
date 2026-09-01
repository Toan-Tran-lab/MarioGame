#include "DragonBossBrain.h"
#include "DragonBoss.h"
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

    Vector2 bossCenter = {
        boss.GetPosition().x + boss.GetSize().x / 2.0f,
        boss.GetPosition().y + boss.GetSize().y / 2.0f
    };

    // Sort players by distance to boss center
    std::sort(alivePlayers.begin(), alivePlayers.end(),
        [&bossCenter](Player* a, Player* b) {
            float da = std::abs(a->GetPosition().x + a->GetSize().x / 2.0f - bossCenter.x);
            float db = std::abs(b->GetPosition().x + b->GetSize().x / 2.0f - bossCenter.x);
            return da < db;
        });

    primaryTarget_ = alivePlayers[0];
    if (alivePlayers.size() > 1) {
        secondaryTarget_ = alivePlayers[1];

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

    // Update boss facing direction towards primary target if not locked in an attack
    if (primaryTarget_) {
        float pX = primaryTarget_->GetPosition().x + primaryTarget_->GetSize().x / 2.0f;
        float bossCenterX = boss.GetPosition().x + boss.GetSize().x / 2.0f;
        if (pX < bossCenterX) {
            boss.SetFacing(FacingDirection::Left);
        } else {
            boss.SetFacing(FacingDirection::Right);
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

    // 1. Pincer situation with 2 players surrounding the boss
    if (isPincerSituation_) {
        if (attackCounter_ % 2 == 0) {
            return DragonAction::Scream; // Shockwave expands both directions
        } else {
            return DragonAction::Jump;   // Jump over or reposition
        }
    }

    // 2. Player is jumping above boss head
    if (distY < -40.0f && distX < 120.0f) {
        return (attackCounter_ % 2 == 0) ? DragonAction::Jump : DragonAction::Walk;
    }

    // 3. Player is far away
    if (distX > 220.0f) {
        if (attackCounter_ % 3 == 0) {
            return DragonAction::Walk; // Close the distance
        } else {
            return DragonAction::Fire; // Ranged attack
        }
    }

    // 4. Player is in close-medium range
    if (isEnraged) {
        // Enraged rotation: fast mix of attacks
        int choice = attackCounter_ % 4;
        switch (choice) {
            case 0: return DragonAction::Fire;
            case 1: return DragonAction::Scream;
            case 2: return DragonAction::Jump;
            default: return DragonAction::Walk;
        }
    }

    // Standard rotation
    int choice = attackCounter_ % 4;
    switch (choice) {
        case 0: return DragonAction::Walk;
        case 1: return DragonAction::Fire;
        case 2: return DragonAction::Walk;
        default: return DragonAction::Scream;
    }
}
