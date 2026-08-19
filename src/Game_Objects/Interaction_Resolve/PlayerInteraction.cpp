#include "PlayerInteraction.h"
#include "Game_Objects/Derived_Objects/Enemies/Goomba/Goomba.h"
#include "Game_Objects/Derived_Objects/Enemies/KoopaShell/KoopaShell.h"
#include "Game_Objects/Derived_Objects/Enemies/BuzzyBeetle/BuzzyBeetle.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "Game_Objects/Derived_Objects/Items/Mushroom/Mushroom.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/PlayerState.h"
#include <cmath>

namespace {
// How far below the enemy's top the player's bottom can overlap and still count as a stomp
constexpr float kStompTolerance = 16.0f;
// Upward launch velocity applied to the player when they stomp an enemy
constexpr float kStompBounceVelocity = -350.0f;
// Distinct upward launch velocity applied to the player when they stomp a Buzzy Beetle
constexpr float kBeetleBounceVelocity = -420.0f;
}

void PlayerInteraction::Visit(Goomba& g) {
    const float playerBottom = self.GetPosition().y + self.GetSize().y;
    const float goombaTop = g.GetPosition().y;
    const bool falling = self.GetVelocity().y > 0.0f;
    const bool aboveGoomba = (playerBottom <= goombaTop + kStompTolerance);

    if (aboveGoomba && falling) {
        // Stomped from above: goomba enters dying state, player bounces off.
        // Score (+100) is added by GameplayState when it detects the state change.
        g.Stomp();

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
    const float playerBottom = self.GetPosition().y + self.GetSize().y;
    const float koopaTop = k.GetPosition().y;
    const bool falling = self.GetVelocity().y > 0.0f;
    const bool aboveKoopa = (playerBottom <= koopaTop + kStompTolerance);

    if (aboveKoopa && falling) {
        // Stomp from above
        if (k.GetState() == KoopaShellState::Walking) {
            // Walking -> Hiding
            k.Stomp();
        } else if (k.GetState() == KoopaShellState::Sliding) {
            // Sliding -> Hiding (Stop the sliding shell)
            k.Stomp();
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
        self.TakePowerup(PowerupType::Mushroom);
    }
}

void PlayerInteraction::Visit(BuzzyBeetle& b) {
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