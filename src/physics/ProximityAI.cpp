#include "ProximityAI.h"
#include <cmath>

namespace physics {

    void ProximityAI::UpdateAI(PhysicsBody& enemy, const PhysicsBody* player, float detectionRadius, float dt, InputState& outInput, const BlockGrid& blockGrid) {
        // Reset inputs and tell physics engine to bypass acceleration/friction
        outInput = InputState();
        outInput.ignorePhysics = true;

        constexpr float ENEMY_SPEED = 100.0f;
        
        bool chase = false;
        float dx = 0.0f;
        
        if (player) {
            dx = player->position.x - enemy.position.x;
            float dy = player->position.y - enemy.position.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance < detectionRadius) {
                chase = true;
            }
        }
        
        enemy.isTracking = chase;

        if (chase) {
            // Player is close. Check if directly above or below to avoid jitter.
            if (std::abs(dx) < 16.0f) {
                enemy.aiDirection = 0; // Stop moving if directly aligned vertically
            } else {
                enemy.aiDirection = (dx > 0) ? 1 : -1;
            }
        } else {
            // Idle state: Patrol left/right
            if (enemy.aiDirection == 0) {
                enemy.aiDirection = 1; // Kickstart movement if we were stopped
            }
            
            if (std::abs(enemy.velocity.x) < 0.1f) {
                // If velocity was zeroed by the collision system last frame, we hit a wall
                enemy.aiDirection *= -1;
            } else if (enemy.isGrounded) {
                // Check for ledge (no floor directly in front)
                bool groundAhead = false;
                float checkX = (enemy.aiDirection > 0) ? (enemy.position.x + enemy.size.x + 2.0f) : (enemy.position.x - 2.0f);
                float checkY = enemy.position.y + enemy.size.y + 4.0f;
                
                int col = (int)(checkX / blockGrid.GetTileSize());
                int row = (int)(checkY / blockGrid.GetTileSize());
                
                if (blockGrid.IsSolidAt(col, row)) {
                    groundAhead = true;
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
