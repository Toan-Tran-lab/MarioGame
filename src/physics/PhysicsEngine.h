#pragma once
#include "PhysicsBody.h"
#include "InputManager.h"

namespace physics {

    class PhysicsEngine {
    public:
        static constexpr float GRAVITY = 980.0f; // Pixels per second squared
        static constexpr float TERMINAL_VELOCITY = 600.0f;
        static constexpr float MAX_WALK_SPEED = 180.0f;
        static constexpr float MAX_SPRINT_SPEED = 300.0f;
        static constexpr float ACCELERATION = 800.0f;
        static constexpr float FRICTION = 1000.0f;
        static constexpr float AIR_ACCEL_MULTIPLIER = 0.15f;
        static constexpr float JUMP_FORCE = -400.0f;
        static constexpr float JUMP_HOVER_GRAVITY_MULTIPLIER = 0.5f;

        // Applies horizontal movement from inputs and vertical movement from gravity
        static void ApplyPhysics(PhysicsBody& body, const InputState& input, float dt);
    };

} // namespace physics
