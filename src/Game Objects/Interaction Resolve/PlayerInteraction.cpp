#include "PlayerInteraction.h"
#include "Game Objects/Derived Objects/Enemies/Goomba/Goomba.h"
#include "Game Objects/Derived Objects/Enemies/KoopaShell/KoopaShell.h"
#include "Game Objects/Derived Objects/Playable Characters/Player/Player.h"

namespace {
// How far above the goomba's center the player's center may sit and still
// count as a stomp. Precise side detection will need hitbox sizes.
constexpr float kStompTolerance = 8.0f;
// Upward launch velocity applied to the player when they stomp a goomba.
constexpr float kStompBounceVelocity = -350.0f;
}

void PlayerInteraction::Visit(Goomba& g) {
    const bool falling = self.GetVelocity().y > 0.0f;
    const bool aboveGoomba = self.GetPosition().y + kStompTolerance < g.GetPosition().y;

    if (aboveGoomba) {
        if (falling) {
            // Stomped from above: goomba enters dying state, player bounces off.
            // Score (+100) is added by GameplayState when it detects the state change.
            g.Stomp();

            Vector2 vel = self.GetVelocity();
            vel.y = kStompBounceVelocity;
            self.SetVelocity(vel);
        }
        // If aboveGoomba but not falling (e.g. bouncing up), do nothing (no damage).
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
    const bool falling = self.GetVelocity().y > 0.0f;
    const bool aboveKoopa = self.GetPosition().y + kStompTolerance < k.GetPosition().y;

    if (aboveKoopa) {
        if (falling) {
            // Stomp from above
            if (k.GetState() == KoopaShellState::Hiding || k.GetState() == KoopaShellState::Sliding) {
                // Kicking a hiding shell, or changing direction of a sliding shell
                float playerCenterX = self.GetPosition().x + self.GetSize().x / 2.0f;
                float koopaCenterX = k.GetPosition().x + k.GetSize().x / 2.0f;
                int dir;
                if (std::abs(playerCenterX - koopaCenterX) < 1.0f) {
                    dir = (self.GetFacing() == FacingDirection::Right) ? 1 : -1;
                } else {
                    dir = (playerCenterX < koopaCenterX) ? 1 : -1;
                }
                k.Kick(dir);
            } else {
                // Walking -> Hiding
                k.Stomp();
            }

            // Bounce player
            Vector2 vel = self.GetVelocity();
            vel.y = kStompBounceVelocity;
            self.SetVelocity(vel);
        }
        // If aboveKoopa but not falling, do nothing.
    } else {
        // Hit from the side
        if (k.GetState() == KoopaShellState::Hiding) {
            // Kick the shell
            float playerCenterX = self.GetPosition().x + self.GetSize().x / 2.0f;
            float koopaCenterX = k.GetPosition().x + k.GetSize().x / 2.0f;
            int dir;
            if (std::abs(playerCenterX - koopaCenterX) < 1.0f) {
                dir = (self.GetFacing() == FacingDirection::Right) ? 1 : -1;
            } else {
                dir = (playerCenterX < koopaCenterX) ? 1 : -1;
            }
            k.Kick(dir);
        } else {
            // Walking or sliding shell hurts player
            self.TakeDamage();
        }
    }
}
