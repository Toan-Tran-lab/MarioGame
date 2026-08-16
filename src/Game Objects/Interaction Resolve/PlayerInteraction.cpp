#include "PlayerInteraction.h"
#include "Game Objects/Derived Objects/Enemies/Goomba/Goomba.h"
#include "Game Objects/Derived Objects/Playable Characters/Player/Player.h"
#include "Game Objects/Derived Objects/Items/Mushroom.h"
#include "Game Objects/Derived Objects/Playable Characters/Player/PlayerState.h"

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

    if (falling && aboveGoomba) {
        // Stomped from above: goomba dies, player bounces off.
        g.SetActive(false);

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
    // KoopaShell is not implemented yet; stub.
    (void)k;
}

void PlayerInteraction::Visit(Mushroom& m) {
    if (m.IsActive()) {
        m.SetActive(false);
        self.TakePowerup(PowerupType::Mushroom);
    }
}
