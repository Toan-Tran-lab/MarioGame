#include "ProximityAI.h"
#include <cmath>

namespace physics {

    void ProximityAI::UpdateAI(PhysicsBody& enemy, const PhysicsBody& player, float detectionRadius, float dt, InputState& outInput, const std::vector<Rectangle>& blocks) {
        float dx = player.position.x - enemy.position.x;
        float dy = player.position.y - enemy.position.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        // Reset inputs and tell physics engine to bypass acceleration/friction
        outInput = InputState();
        outInput.ignorePhysics = true;

        constexpr float ENEMY_SPEED = 100.0f;

        if (distance < detectionRadius) {
            // Track player
            enemy.aiDirection = (dx > 0) ? 1 : -1;
        } else {
            // Idle state: Patrol left/right
            if (std::abs(enemy.velocity.x) < 0.1f) {
                // If velocity was zeroed by the collision system last frame, we hit a wall
                enemy.aiDirection *= -1;
            } else if (enemy.isGrounded) {
                // Check for ledge (no floor directly in front)
                bool groundAhead = false;
                float checkX = (enemy.aiDirection > 0) ? (enemy.position.x + enemy.size.x + 2.0f) : (enemy.position.x - 2.0f);
                float checkY = enemy.position.y + enemy.size.y + 4.0f;
                
                for (const auto& block : blocks) {
                    if (CheckCollisionPointRec(Vector2{checkX, checkY}, block)) {
                        groundAhead = true;
                        break;
                    }
                }
                
                if (!groundAhead) {
                    enemy.aiDirection *= -1; // Switch direction to avoid falling
                }
            }
        }

        // Apply constant velocity directly (gravity is still handled by PhysicsEngine)
        enemy.velocity.x = enemy.aiDirection * ENEMY_SPEED;
    }

} // namespace physics
