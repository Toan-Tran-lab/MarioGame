#include "ProximityAI.h"
#include <cmath>

namespace physics {

    void ProximityAI::UpdateAI(PhysicsBody& enemy, const PhysicsBody& player, float detectionRadius, float dt) {
        float dx = player.position.x - enemy.position.x;
        float dy = player.position.y - enemy.position.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance < detectionRadius) {
            // Track and sprint toward the player
            float dir = (dx > 0) ? 1.0f : -1.0f;
            enemy.velocity.x = dir * 120.0f; // Sprint speed
        } else {
            // Default patrol or idle behavior
            enemy.velocity.x = 0; 
        }
    }

} // namespace physics
