#pragma once
#include "PhysicsBody.h"
#include <vector>
#include "World/BlockGrid.h"
namespace physics {

    enum class CollisionSide { NONE, TOP, BOTTOM, LEFT, RIGHT };

    struct CollisionInfo {
        CollisionSide side = CollisionSide::NONE;
        float overlap = 0.0f;
    };

    class CollisionSystem {
    public:
        // Evaluates two rectangles and returns the side of overlap (minimum translation vector)
        static CollisionInfo GetCollisionInfo(const Rectangle& body, const Rectangle& obstacle);

        // Takes a physics body and an array of solid blocks, resolves any overlaps, and updates grounded state
        static void ResolveMapCollisions(PhysicsBody& body, const std::vector<Rectangle>& blocks);

        // Grid-based collision resolution using BlockGrid
        static void ResolveMapCollisions(PhysicsBody& body, const BlockGrid& blockGrid);

        // Resolve collision between a physics body and a moving platform.
        // Only resolves as a floor if the body is landing from above.
        // Applies platformVelX to the body so the rider moves with the platform.
        static void ResolvePlatformCollision(PhysicsBody& body, const Rectangle& platform, float platformVelX, float dt);
    };

} // namespace physics
