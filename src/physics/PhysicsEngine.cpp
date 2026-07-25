#include "PhysicsEngine.h"

namespace physics {

    void PhysicsEngine::ApplyPhysics(PhysicsBody& body, const InputState& input, float dt) {
        // Horizontal Movement
        body.velocity.x = 0;
        if (input.moveLeft)  body.velocity.x = -MOVE_SPEED;
        if (input.moveRight) body.velocity.x = MOVE_SPEED;

        // Vertical Movement (Gravity)
        if (!body.isGrounded) {
            body.velocity.y += GRAVITY * dt;
            if (body.velocity.y > TERMINAL_VELOCITY) {
                body.velocity.y = TERMINAL_VELOCITY;
            }
        } else {
            // Grounded
            body.velocity.y = 0;
            if (input.jump) {
                body.velocity.y = JUMP_FORCE;
                body.isGrounded = false; // Player is no longer grounded after jumping
            }
        }

        // Integrate Position
        body.position.x += body.velocity.x * dt;
        body.position.y += body.velocity.y * dt;
    }

} // namespace physics
