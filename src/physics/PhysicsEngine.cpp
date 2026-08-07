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

        // Vertical Movement (Gravity & Jump)
        if (!body.isGrounded) {
            float appliedGravity = GRAVITY;
            // Jump hover mechanic: less gravity while ascending and holding jump
            if (body.velocity.y < 0 && input.jump) {
                appliedGravity *= JUMP_HOVER_GRAVITY_MULTIPLIER;
            }
            body.velocity.y += appliedGravity * dt;
            if (body.velocity.y > TERMINAL_VELOCITY) {
                body.velocity.y = TERMINAL_VELOCITY;
            }
        } else {
            // Grounded
            body.velocity.y = 0;
            if (input.jump) {
                body.velocity.y = JUMP_FORCE;
                body.isGrounded = false;
            }
        }

        // Integrate Position
        body.position.x += body.velocity.x * dt;
        body.position.y += body.velocity.y * dt;
    }

} // namespace physics
