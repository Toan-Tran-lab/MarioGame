#include "PhysicsEngine.h"

namespace physics {

    void PhysicsEngine::ApplyPhysics(PhysicsBody& body, const InputState& input, float dt) {
        // Horizontal Movement (Acceleration & Friction)
        if (input.moveLeft) {
            body.velocity.x -= ACCELERATION * dt;
            if (body.velocity.x < -MAX_MOVE_SPEED) body.velocity.x = -MAX_MOVE_SPEED;
        } else if (input.moveRight) {
            body.velocity.x += ACCELERATION * dt;
            if (body.velocity.x > MAX_MOVE_SPEED) body.velocity.x = MAX_MOVE_SPEED;
        } else {
            // Apply friction when no directional input
            if (body.velocity.x > 0) {
                body.velocity.x -= FRICTION * dt;
                if (body.velocity.x < 0) body.velocity.x = 0;
            } else if (body.velocity.x < 0) {
                body.velocity.x += FRICTION * dt;
                if (body.velocity.x > 0) body.velocity.x = 0;
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
        if (!body.isGrounded) {
            float appliedGravity = GRAVITY;
            // Jump hover mechanic: less gravity while ascending and holding jump
            // We use input.jumpHeld here so holding the key keeps the hover active
            if (body.velocity.y < 0 && input.jumpHeld) {
                appliedGravity *= JUMP_HOVER_GRAVITY_MULTIPLIER;
            }
            body.velocity.y += appliedGravity * dt;
            if (body.velocity.y > TERMINAL_VELOCITY) {
                body.velocity.y = TERMINAL_VELOCITY;
            }
        } else {
            // Grounded
            body.velocity.y = 0;
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
