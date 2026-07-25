#pragma once
#include "PhysicsBody.h"
#include "InputManager.h"

namespace physics {

    class PhysicsEngine {
    public:
        static constexpr float GRAVITY = 980.0f; // Pixels per second squared
        static constexpr float TERMINAL_VELOCITY = 600.0f;
        static constexpr float MOVE_SPEED = 200.0f;
        static constexpr float JUMP_FORCE = -450.0f;

        // Applies horizontal movement from inputs and vertical movement from gravity
        static void ApplyPhysics(PhysicsBody& body, const InputState& input, float dt);
    };

} // namespace physics
