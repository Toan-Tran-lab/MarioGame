#pragma once
#include "PhysicsBody.h"

namespace physics {

    class ProximityAI {
    public:
        // Evaluates the distance between an enemy and the player and triggers pursuit behavior
        static void UpdateAI(PhysicsBody& enemy, const PhysicsBody& player, float detectionRadius, float dt);
    };

} // namespace physics
