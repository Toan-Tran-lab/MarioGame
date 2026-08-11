#include "PhysicsEngine.h"
#include <algorithm>

namespace physics {

    void PhysicsEngine::ApplyPhysics(PhysicsBody& body, const InputState& input, float dt) {
        float currentMaxSpeed = input.sprint ? MAX_SPRINT_SPEED : MAX_WALK_SPEED;
        float currentAccel = body.isGrounded ? ACCELERATION : (ACCELERATION * AIR_ACCEL_MULTIPLIER);
        float currentFriction = FRICTION;

        // Horizontal Movement (Acceleration & Friction)
        if (!input.ignorePhysics) {
            if (input.moveLeft) {
                float prevVx = body.velocity.x;
                body.velocity.x -= currentAccel * dt;
                float cap = body.isGrounded ? -currentMaxSpeed : std::min(-currentMaxSpeed, prevVx);
                if (body.velocity.x < cap) body.velocity.x = cap;
            } else if (input.moveRight) {
                float prevVx = body.velocity.x;
                body.velocity.x += currentAccel * dt;
                float cap = body.isGrounded ? currentMaxSpeed : std::max(currentMaxSpeed, prevVx);
                if (body.velocity.x > cap) body.velocity.x = cap;
            } else {
                // Apply friction when no directional input
                if (body.velocity.x > 0) {
                    body.velocity.x -= currentFriction * dt;
                    if (body.velocity.x < 0) body.velocity.x = 0;
                } else if (body.velocity.x < 0) {
                    body.velocity.x += currentFriction * dt;
                    if (body.velocity.x > 0) body.velocity.x = 0;
                }
            }
        }

        // Jump Buffering: Remember the jump input for 0.15 seconds (150ms)
        if (input.jumpPressed) {
            body.jumpBufferTimer = 0.15f; 
        }
        if (body.jumpBufferTimer > 0) {
            body.jumpBufferTimer -= dt;
        }

        // Vertical Movement (Gravity & Jump)
        float appliedGravity = GRAVITY;
        // Jump hover mechanic: less gravity while ascending and holding jump
        if (body.velocity.y < 0 && input.jumpHeld && !body.isGrounded) {
            appliedGravity *= JUMP_HOVER_GRAVITY_MULTIPLIER;
        }
        
        body.velocity.y += appliedGravity * dt;
        if (body.velocity.y > TERMINAL_VELOCITY) {
            body.velocity.y = TERMINAL_VELOCITY;
        }

        if (body.isGrounded) {
            // If we buffered a jump, execute it immediately upon touching the ground!
            if (body.jumpBufferTimer > 0) {
                body.velocity.y = JUMP_FORCE;
                body.isGrounded = false;
                body.jumpBufferTimer = 0.0f; // Consume the buffer
            }
        }

        // Integrate Position
        body.position.x += body.velocity.x * dt;
        body.position.y += body.velocity.y * dt;
    }

} // namespace physics
