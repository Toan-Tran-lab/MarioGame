#pragma once
#include "PhysicsBody.h"
#include "InputManager.h"
#include <vector>

namespace physics {

    class ProximityAI {
    public:
        // Evaluates the distance between an enemy and the player and triggers pursuit behavior
        static void UpdateAI(PhysicsBody& enemy, const PhysicsBody& player, float detectionRadius, float dt, InputState& outInput, const std::vector<Rectangle>& blocks);
    };

} // namespace physics
