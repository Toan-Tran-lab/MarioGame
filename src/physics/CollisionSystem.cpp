#include "CollisionSystem.h"
#include <cmath>
#include <algorithm>

namespace physics {

    CollisionInfo CollisionSystem::GetCollisionInfo(const Rectangle& r1, const Rectangle& r2) {
        CollisionInfo info;
        
        if (!CheckCollisionRecs(r1, r2)) {
            return info;
        }

        // Calculate centers
        float r1CenterX = r1.x + r1.width / 2.0f;
        float r1CenterY = r1.y + r1.height / 2.0f;
        float r2CenterX = r2.x + r2.width / 2.0f;
        float r2CenterY = r2.y + r2.height / 2.0f;

        // Calculate differences between centers
        float dx = r1CenterX - r2CenterX;
        float dy = r1CenterY - r2CenterY;

        // Calculate minimum distances needed to not be colliding
        float minDistanceX = r1.width / 2.0f + r2.width / 2.0f;
        float minDistanceY = r1.height / 2.0f + r2.height / 2.0f;

        // Calculate penetration depths
        float overlapX = minDistanceX - std::abs(dx);
        float overlapY = minDistanceY - std::abs(dy);

        // Find the minimum translation vector (the axis of least penetration)
        if (overlapX < overlapY) {
            info.overlap = overlapX;
            info.side = (dx > 0) ? CollisionSide::LEFT : CollisionSide::RIGHT;
        } else {
            info.overlap = overlapY;
            info.side = (dy > 0) ? CollisionSide::TOP : CollisionSide::BOTTOM;
        }

        return info;
    }

    void CollisionSystem::ResolveMapCollisions(PhysicsBody& body, const std::vector<Rectangle>& blocks) {
        body.isGrounded = false; // Assume not grounded until proven otherwise

        for (const auto& block : blocks) {
            CollisionInfo col = GetCollisionInfo(body.GetRect(), block);
            if (col.side != CollisionSide::NONE) {
                // Resolve Collision by pushing the body out of the obstacle
                if (col.side == CollisionSide::LEFT) {
                    body.position.x += col.overlap;
                } 
                else if (col.side == CollisionSide::RIGHT) {
                    body.position.x -= col.overlap;
                } 
                else if (col.side == CollisionSide::TOP) {
                    // Bonked head on ceiling
                    body.position.y += col.overlap;
                    if (body.velocity.y < 0) body.velocity.y = 0; 
                } 
                else if (col.side == CollisionSide::BOTTOM) {
                    // Landed on floor
                    body.position.y -= col.overlap;
                    body.isGrounded = true;
                    if (body.velocity.y > 0) body.velocity.y = 0;
                }
            }
        }
    }

} // namespace physics
