#include "PlayerState.h"
#include "Player.h"

// Base default: no per-frame state logic by default.
void PlayerState::UpdateState(Player& player, float dt) {}

// --- SmallState ---

void SmallState::OnHit(Player& player) {
    // Already the smallest form; there is no smaller state to shrink to.
    // The player dies, which the gameplay layer reacts to (respawn/game over).
    player.SetDead(true);
}

void SmallState::OnPowerup(Player& player, PowerupType type) {
    switch (type) {
        case PowerupType::Mushroom:   player.SetState(new SuperState()); break;
        case PowerupType::FireFlower: player.SetState(new FireState()); break;
        case PowerupType::Star:       player.SetState(new StarState()); break;
    }
}

// --- SuperState ---

void SuperState::Enter(Player& player) {
    // Hook: swap sprite/hitbox to the "big" size.
    (void)player;
}

void SuperState::OnHit(Player& player) {
    player.SetState(new SmallState());
}

void SuperState::OnPowerup(Player& player, PowerupType type) {
    switch (type) {
        case PowerupType::FireFlower: player.SetState(new FireState()); break;
        case PowerupType::Star:       player.SetState(new StarState()); break;
    }
}

// --- FireState ---

void FireState::OnHit(Player& player) {
    // Classic Mario rule: Fire -> Small directly, skips Super.
    player.SetState(new SmallState());
}

void FireState::OnPowerup(Player& player, PowerupType type) {
    switch (type) {
        case PowerupType::Star:       player.SetState(new StarState()); break;
    }
}

// --- StarState ---

void StarState::Enter(Player& player) {
    // Hook: start the invincibility timer here.
    (void)player;
}

void StarState::OnHit(Player& player) {
    // Invincible while star-powered: damage has no effect.
    (void)player;
}
