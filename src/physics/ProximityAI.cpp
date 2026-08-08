#include "ProximityAI.h"
#include <cmath>

namespace physics {

    void ProximityAI::UpdateAI(PhysicsBody& enemy, const PhysicsBody& player, float detectionRadius, float dt) {
        float dx = player.position.x - enemy.position.x;
        float dy = player.position.y - enemy.position.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        float accel = 400.0f;
        float maxSprint = 120.0f;
        float patrolSpeed = 40.0f;

        if (distance < detectionRadius) {
            // Smoothly accelerate toward the player
            float dir = (dx > 0) ? 1.0f : -1.0f;
            enemy.velocity.x += dir * accel * dt;
            
            // Clamp to max sprint speed
            if (enemy.velocity.x > maxSprint) enemy.velocity.x = maxSprint;
            if (enemy.velocity.x < -maxSprint) enemy.velocity.x = -maxSprint;
        } else {
            // Patrol behavior: move left by default. 
            if (enemy.velocity.x > -patrolSpeed) {
                enemy.velocity.x -= accel * dt;
            } else {
                enemy.velocity.x = -patrolSpeed;
            }
        }
    }

} // namespace physics
