#include "KoopaShellInteraction.h"
#include "Game_Objects/Derived_Objects/Enemies/Goomba/Goomba.h"
#include "Game_Objects/Derived_Objects/Enemies/KoopaShell/KoopaShell.h"
#include "Game_Objects/Derived_Objects/Enemies/BuzzyBeetle/BuzzyBeetle.h"
#include "Game_Objects/Derived_Objects/Enemies/Boss/BossEnemy/Boss.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Player/Player.h"
#include "Game_Objects/Derived_Objects/Items/Mushroom/Mushroom.h"

void ShellInteraction::Visit(Goomba& g) {
    // A sliding shell defeats any goomba in its path. Reuses Stomp() —
    // same dying/despawn/score-popup logic as a player stomp.
    if (self.GetState() == KoopaShellState::Sliding && !g.IsDying()) {
        g.Stomp();
    }
}

void ShellInteraction::Visit(KoopaShell& k) {
    if (self.GetState() != KoopaShellState::Sliding || &k == &self) return;

    if (k.GetState() == KoopaShellState::Sliding) {
        // Two sliding shells: bounce apart rather than chain-kill.
        Vector2 selfVel = self.GetVelocity();
        Vector2 otherVel = k.GetVelocity();
        self.SetVelocity({ -selfVel.x, selfVel.y });
        k.SetVelocity({ -otherVel.x, otherVel.y });
    } else {
        // Hitting a walking or hiding shell: kick it onward.
        int dir = (self.GetVelocity().x >= 0.0f) ? 1 : -1;
        k.Kick(dir);
    }
}

void ShellInteraction::Visit(Player& p) {
    // Player-vs-shell is already resolved from the player's side via
    // PlayerInteraction. Left as a no-op to avoid double-processing the
    // same overlap from both directions in one frame.
    (void)p;
}

void ShellInteraction::Visit(Mushroom& m) {
    (void)m; // shells don't interact with items
}

void ShellInteraction::Visit(BuzzyBeetle& b) {
    if (self.GetState() == KoopaShellState::Sliding && !b.IsDefeated()) {
        b.Defeat();
    }
}

void ShellInteraction::Visit(Boss& b) {
    if (self.GetState() != KoopaShellState::Sliding || b.IsDead()) return;
    b.TakeDamage(b.GetShellDamage());\
    self.SetActive(false);
}