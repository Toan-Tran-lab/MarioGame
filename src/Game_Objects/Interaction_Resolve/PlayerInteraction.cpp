#include "PlayerInteraction.h"
#include "Game_Objects/Derived_Objects/Enemies/Goomba/Goomba.h"
#include "Game_Objects/Derived_Objects/Enemies/KoopaShell/KoopaShell.h"
#include "Game_Objects/Derived_Objects/Enemies/BuzzyBeetle/BuzzyBeetle.h"
#include "Game_Objects/Derived_Objects/Enemies/Piranha/Piranha.h"
#include "Game_Objects/Derived_Objects/Enemies/Bullet/Bullet.h"
#include "Game_Objects/Derived_Objects/Enemies/Boss/BossEnemy/Boss.h"
#include "Game_Objects/Derived_Objects/Enemies/Boss/BossEnemy/BossState.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "Game_Objects/Derived_Objects/Items/Mushroom/Mushroom.h"
#include "Game_Objects/Derived_Objects/Items/FireFlower/FireFlower.h"
#include "Game_Objects/Derived_Objects/Items/Starman/Starman.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/PlayerState.h"
#include "AudioManager/AudioManager.h"
#include <cmath>

namespace {
// How far below the enemy's top the player's bottom can overlap and still count as a stomp
constexpr float kStompTolerance = 16.0f;
// Upward launch velocity applied to the player when they stomp an enemy
constexpr float kStompBounceVelocity = -350.0f;
// Distinct upward launch velocity applied to the player when they stomp a Buzzy Beetle
constexpr float kBeetleBounceVelocity = -420.0f;
constexpr float kBossKnockbackSpeed = 250.0f;
}

void PlayerInteraction::Visit(Goomba& g) {
    if (self.IsInvincible()) {
        bool hitFromLeft = self.GetPosition().x < g.GetPosition().x;
        g.TriggerUpsideDownDeath(hitFromLeft);
        AudioManager::PlaySFX(AudioKey::HIT_ENEMY);
        return;
    }

    const float playerBottom = self.GetPosition().y + self.GetSize().y;
    const float goombaTop = g.GetPosition().y;
    const bool falling = self.GetVelocity().y > 0.0f;
    const bool aboveGoomba = (playerBottom <= goombaTop + kStompTolerance);

    if (aboveGoomba && falling) {
        // Stomped from above: goomba enters dying state, player bounces off.
        // Score (+100) is added by GameplayState when it detects the state change.
        g.Stomp();
        AudioManager::PlaySFX(AudioKey::HIT_ENEMY);

        Vector2 vel = self.GetVelocity();
        vel.y = kStompBounceVelocity;
        self.SetVelocity(vel);
    } else {
        // Hit from the side/below: player takes damage.
        self.TakeDamage();
    }
}

void PlayerInteraction::Visit(Player& p) {
    // Player vs. player currently does nothing.
    (void)p;
}

void PlayerInteraction::Visit(KoopaShell& k) {
    if (self.IsInvincible()) {
        bool hitFromLeft = self.GetPosition().x < k.GetPosition().x;
        k.TriggerUpsideDownDeath(hitFromLeft);
        AudioManager::PlaySFX(AudioKey::HIT_ENEMY);
        return;
    }

    const float playerBottom = self.GetPosition().y + self.GetSize().y;
    const float koopaTop = k.GetPosition().y;
    const bool falling = self.GetVelocity().y > 0.0f;
    const bool aboveKoopa = (playerBottom <= koopaTop + kStompTolerance);

    if (aboveKoopa && falling) {
        // Stomp from above
        if (k.GetState() == KoopaShellState::Walking) {
            // Walking -> Hiding
            k.Stomp();
            AudioManager::PlaySFX(AudioKey::HIT_ENEMY);
        } else if (k.GetState() == KoopaShellState::Sliding) {
            // Sliding -> Hiding (Stop the sliding shell)
            k.Stomp();
            AudioManager::PlaySFX(AudioKey::HIT_ENEMY);
        } else if (k.GetState() == KoopaShellState::Hiding) {
            // Stomping a hiding shell kicks it
            float playerCenterX = self.GetPosition().x + self.GetSize().x / 2.0f;
            float koopaCenterX = k.GetPosition().x + k.GetSize().x / 2.0f;
            int dir;
            if (std::abs(playerCenterX - koopaCenterX) < 1.0f) {
                dir = (self.GetFacing() == FacingDirection::Right) ? 1 : -1;
            } else {
                dir = (playerCenterX < koopaCenterX) ? 1 : -1;
            }
            k.Kick(dir);
            AudioManager::PlaySFX(AudioKey::HIT_ENEMY);
        }

        // Bounce player
        Vector2 vel = self.GetVelocity();
        vel.y = kStompBounceVelocity;
        self.SetVelocity(vel);
    } else {
        // Hit from the side / below
        if (k.GetState() == KoopaShellState::Hiding) {
            // Kick the hiding shell
            float playerCenterX = self.GetPosition().x + self.GetSize().x / 2.0f;
            float koopaCenterX = k.GetPosition().x + k.GetSize().x / 2.0f;
            int dir;
            if (std::abs(playerCenterX - koopaCenterX) < 1.0f) {
                dir = (self.GetFacing() == FacingDirection::Right) ? 1 : -1;
            } else {
                dir = (playerCenterX < koopaCenterX) ? 1 : -1;
            }
            k.Kick(dir);

            // Nudge shell position slightly in kick direction so it doesn't remain overlapping with player
            Vector2 kPos = k.GetPosition();
            if (dir > 0) {
                kPos.x = self.GetPosition().x + self.GetSize().x + 2.0f;
            } else {
                kPos.x = self.GetPosition().x - k.GetSize().x - 2.0f;
            }
            k.SetPosition(kPos);
            k.SyncPhysicsBody();
        } else if (k.GetState() == KoopaShellState::Sliding) {
            // Sliding shell: hurt player ONLY if moving TOWARDS player
            float playerCenterX = self.GetPosition().x + self.GetSize().x / 2.0f;
            float koopaCenterX = k.GetPosition().x + k.GetSize().x / 2.0f;
            float koopaVelX = k.GetVelocity().x;

            bool movingTowardsPlayer = (playerCenterX < koopaCenterX && koopaVelX < 0.0f) ||
                                       (playerCenterX > koopaCenterX && koopaVelX > 0.0f);

            if (movingTowardsPlayer) {
                self.TakeDamage();
            } else {
                // Moving away from player: kick/nudge it in direction player is moving/facing
                int dir = (self.GetFacing() == FacingDirection::Right) ? 1 : -1;
                k.Kick(dir);
            }
        } else {
            // Walking shell hurts player
            self.TakeDamage();
        }
    }
}

void PlayerInteraction::Visit(Mushroom& m) {
    if (m.IsActive()) {
        m.SetActive(false);
        AudioManager::PlaySFX(AudioKey::POWER_UP);
        self.TakePowerup(PowerupType::Mushroom);
    }
}

void PlayerInteraction::Visit(BuzzyBeetle& b) {
    if (self.IsInvincible()) {
        bool hitFromLeft = self.GetPosition().x < b.GetPosition().x;
        b.TriggerUpsideDownDeath(hitFromLeft);
        AudioManager::PlaySFX(AudioKey::HIT_ENEMY);
        return;
    }

    const float playerBottom = self.GetPosition().y + self.GetSize().y;
    const float beetleTop = b.GetPosition().y;
    const bool falling = self.GetVelocity().y > 0.0f;
    const bool aboveBeetle = (playerBottom <= beetleTop + kStompTolerance);

    if (aboveBeetle && falling) {
        // Immune: no damage to the beetle, just a slightly higher bounce.
        Vector2 vel = self.GetVelocity();
        vel.y = kBeetleBounceVelocity;
        self.SetVelocity(vel);
    } else {
        // Side/bottom contact: regular enemy logic, player takes damage.
        self.TakeDamage();
    }
}

void PlayerInteraction::Visit(Piranha& p) {
    if (!p.IsExposedOrMoving()) return;
    // Touching a Piranha plant always damages the player (cannot be stomped)
    self.TakeDamage();
}

void PlayerInteraction::Visit(Bullet& b) {
    if (!b.IsActive()) return;

    const float playerBottom = self.GetPosition().y + self.GetSize().y;
    const float bulletTop = b.GetPosition().y;
    const bool aboveBullet = (playerBottom <= bulletTop + 10.0f);

    if (aboveBullet) {
        // Riding / standing on top of the bullet like a FlyingBridge — no damage taken!
        return;
    } else {
        // Side/bottom contact: causes damage like a sliding Koopa shell
        b.SetActive(false);
        self.TakeDamage();
    }
}

void PlayerInteraction::Visit(Boss& b) {
    if (b.IsDead()) return;

    if (self.IsInvincible()) {
        b.TakeDamage(1000); // Massive damage to boss
        AudioManager::PlaySFX(AudioKey::HIT_ENEMY);
        return;
    }

    const float playerBottom = self.GetPosition().y + self.GetSize().y;
    const float bossTop = b.GetPosition().y;
    const bool falling = self.GetVelocity().y > 0.0f;
    const bool aboveBoss = (playerBottom <= bossTop + kStompTolerance);

    if (aboveBoss && falling) {
        if (b.TakeDamage(b.GetStompDamage())) {
            if (BossState* state = b.GetState()) {
                state->OnStomped(b); // only IdleState reacts (flinch); attack states ignore it
            }
            float bossCenterX = b.GetPosition().x + b.GetSize().x / 2.0f;
            float playerCenterX = self.GetPosition().x + self.GetSize().x / 2.0f;
            float dir = (playerCenterX < bossCenterX) ? -1.0f : 1.0f;
            Vector2 vel = self.GetVelocity();
            vel.y = kStompBounceVelocity;
            vel.x = dir * kBossKnockbackSpeed;
            self.SetVelocity(vel);
        }
    } else {
        self.TakeDamage();
    }
}

void PlayerInteraction::Visit(FireFlower& f) {
    self.TakePowerup(PowerupType::FireFlower);
    AudioManager::PlaySFX(AudioKey::POWER_UP);
    f.SetActive(false);
}

void PlayerInteraction::Visit(Starman& s) {
    self.GrantStarman();
    AudioManager::StartStarmanBGM();
    s.SetActive(false);
}