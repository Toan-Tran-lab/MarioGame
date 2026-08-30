#pragma once
#include "PhysicsBody.h"
#include "InputManager.h"

namespace physics {

    class PhysicsEngine {
    public:
        static constexpr float GRAVITY = 2200.0f; // Snappier gravity
        static constexpr float TERMINAL_VELOCITY = 900.0f;
        static constexpr float MAX_WALK_SPEED = 200.0f;
        static constexpr float MAX_SPRINT_SPEED = 320.0f;
        static constexpr float ACCELERATION = 1600.0f; // Snappier start
        static constexpr float FRICTION = 2000.0f;     // Snappier stop
        static constexpr float AIR_ACCEL_MULTIPLIER = 0.35f; // Better air control
        static constexpr float JUMP_FORCE = -650.0f;
        static constexpr float JUMP_HOVER_GRAVITY_MULTIPLIER = 0.5f;

        // Applies horizontal movement from inputs and vertical movement from gravity
        static void ApplyPhysics(PhysicsBody& body, const InputState& input, float dt);
    };

} // namespace physics
